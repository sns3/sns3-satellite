/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>
#include <ns3/singleton.h>
#include <ns3/pointer.h>
#include <ns3/boolean.h>
#include <ns3/packet.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>

#include "satellite-mac-tag.h"
#include "satellite-utils.h"
#include "satellite-log.h"
#include "satellite-rtn-link-time.h"
#include "satellite-bbframe.h"
#include "satellite-signal-parameters.h"
#include "satellite-control-message.h"
#include "satellite-fwd-link-scheduler.h"
#include "satellite-time-tag.h"
#include "satellite-gw-mac.h"


NS_LOG_COMPONENT_DEFINE ("SatGwMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwMac);

TypeId
SatGwMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGwMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatGwMac> ()
    .AddAttribute ("Scheduler",
                   "Forward link scheduler used by this Sat GW MAC.",
                   PointerValue (),
                   MakePointerAccessor (&SatGwMac::m_fwdScheduler),
                   MakePointerChecker<SatFwdLinkScheduler> ())
    .AddAttribute ("GuardTime",
                   "Guard time in forward link",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&SatGwMac::m_guardTime),
                   MakeTimeChecker ())
    .AddAttribute ("NcrBroadcastPeriod",
                   "Interval between two broadcast of NCR dates",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&SatGwMac::m_ncrInterval),
                   MakeTimeChecker ())
    .AddAttribute ("UseCmt",
                   "Use CMT control messages to correct time on the UTs",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatGwMac::m_useCmt),
                   MakeBooleanChecker ())
    .AddAttribute ("CmtPeriodMin",
                   "Minimum interval between two CMT control messages for a same UT",
                   TimeValue (MilliSeconds (550)),
                   MakeTimeAccessor (&SatGwMac::m_cmtPeriodMin),
                   MakeTimeChecker ())
    .AddTraceSource ("BBFrameTxTrace",
                     "Trace for transmitted BB Frames.",
                     MakeTraceSourceAccessor (&SatGwMac::m_bbFrameTxTrace),
                     "ns3::SatBbFrame::BbFrameCallback")
  ;
  return tid;
}

TypeId
SatGwMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGwMac::SatGwMac ()
  : SatMac (),
  m_fwdScheduler (),
  m_guardTime (MicroSeconds (1)),
  m_ncrInterval (MilliSeconds (100)),
  m_useCmt (false),
  m_lastCmtSent (),
  m_cmtPeriodMin (MilliSeconds (550))
{
  NS_LOG_FUNCTION (this);
}

SatGwMac::SatGwMac (uint32_t beamId)
  : SatMac (beamId),
  m_fwdScheduler (),
  m_guardTime (MicroSeconds (1)),
  m_ncrInterval (MilliSeconds (100)),
  m_useCmt (false),
  m_lastCmtSent (),
  m_cmtPeriodMin (MilliSeconds (550))
{
  NS_LOG_FUNCTION (this);
}

SatGwMac::~SatGwMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGwMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_txOpportunityCallback.Nullify ();
  m_crReceiveCallback.Nullify ();
  m_handoverCallback.Nullify ();
  m_logonCallback.Nullify ();
  m_controlMessageReceivedCallback.Nullify ();

  SatMac::DoDispose ();
}

void
SatGwMac::StartPeriodicTransmissions ()
{
  NS_LOG_FUNCTION (this);

  if ( m_fwdScheduler == NULL )
    {
      NS_FATAL_ERROR ("Scheduler not set for GW MAC!!!");
    }

  /**
   * It is currently assumed that there is only one carrier in FWD link. This
   * carrier has a default index of 0.
   * TODO: When enabling multi-carrier support for FWD link, we need to
   * modify the FWD link scheduler to schedule separately each FWD link
   * carrier.
   */
  Simulator::Schedule (Seconds (0), &SatGwMac::StartTransmission, this, 0);

  Simulator::Schedule (MilliSeconds (50), &SatGwMac::StartNcrTransmission, this);
}

void
SatGwMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_MAC,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (packets));

  // Invoke the `Rx` and `RxDelay` trace sources.
  RxTraces (packets);

  Address utId;

  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      // Remove packet tag
      SatMacTag macTag;
      bool mSuccess = (*i)->PeekPacketTag (macTag);
      if (!mSuccess)
        {
          NS_FATAL_ERROR ("MAC tag was not found from the packet!");
        }

      NS_LOG_INFO ("Packet from " << macTag.GetSourceAddress () << " to " << macTag.GetDestAddress ());
      NS_LOG_INFO ("Receiver " << m_nodeInfo->GetMacAddress ());

      // If the packet is intended for this receiver
      Mac48Address destAddress = macTag.GetDestAddress ();
      utId = macTag.GetSourceAddress ();

      if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast ())
        {
          // Peek control msg tag
          SatControlMsgTag ctrlTag;
          bool cSuccess = (*i)->PeekPacketTag (ctrlTag);

          if (cSuccess)
            {
              SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType ();

              if ( cType != SatControlMsgTag::SAT_NON_CTRL_MSG )
                {
                  ReceiveSignalingPacket (*i);
                }
              else
                {
                  NS_FATAL_ERROR ("A control message received with not valid msg type!");
                }
            }
          else
            {
              // Pass the source address to LLC
              m_rxCallback (*i, macTag.GetSourceAddress (), macTag.GetDestAddress ());
            }
        }
      else
        {
          NS_LOG_INFO ("Packet intended for others received by MAC: " << m_nodeInfo->GetMacAddress ());
        }
    }

  if (rxParams->m_txInfo.waveformId == 2)
    {
      if (m_useCmt)
        {
          SendCmtMessage (utId, rxParams->m_duration);
        }
      m_controlMessageReceivedCallback (utId, m_beamId);
    }
}

void
SatGwMac::StartTransmission (uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);

  if (m_nodeInfo->GetNodeType () == SatEnums::NT_GW)
    {
      m_lastSOF.push (Simulator::Now ());
      uint8_t lastSOFSize = m_ncrV2 ? 3 : 1;
      if (m_lastSOF.size () > lastSOFSize)
        {
          m_lastSOF.pop();
        }
    }

  Time txDuration;

  if (m_txEnabled)
    {
      std::pair<Ptr<SatBbFrame>, const Time> bbFrameInfo = m_fwdScheduler->GetNextFrame ();
      Ptr<SatBbFrame> bbFrame = bbFrameInfo.first;
      txDuration = bbFrameInfo.second;

      // trace out BB frames sent.
      m_bbFrameTxTrace (bbFrame);

      // Handle both dummy frames and normal frames
      if ( bbFrame != NULL )
        {
          // Add packet trace entry:
          m_packetTrace (Simulator::Now (),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_MAC,
                         SatEnums::LD_FORWARD,
                         SatUtils::GetPacketInfo (bbFrame->GetPayload ()));

          SatSignalParameters::txInfo_s txInfo;
          txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
          txInfo.modCod = bbFrame->GetModcod ();
          txInfo.sliceId = bbFrame->GetSliceId ();
          txInfo.frameType = bbFrame->GetFrameType ();
          txInfo.waveformId = 0;

          /**
           * Decrease a guard time from BB frame duration.
           */
          SendPacket (bbFrame->GetPayload (), carrierId, txDuration - m_guardTime, txInfo);
        }
    }
  else
    {
      /**
       * GW MAC is disabled, thus get the duration of the default BB frame
       * and try again then.
       */

      NS_LOG_INFO ("Beam id: " << m_beamId << " is disabled, thus nothing is transmitted!");
      txDuration = m_fwdScheduler->GetDefaultFrameDuration ();
    }

  /**
   * It is currently assumed that there is only one carrier in FWD link. This
   * carrier has a default index of 0.
   * TODO: When enabling multi-carrier support for FWD link, we need to
   * modify the FWD link scheduler to schedule separately each FWD link
   * carrier.
   */
  Simulator::Schedule (txDuration, &SatGwMac::StartTransmission, this, 0);
}

void
SatGwMac::TbtpSent (Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);

  uint32_t superframeCounter = tbtp->GetSuperframeCounter ();

  if(m_tbtps.find(superframeCounter) == m_tbtps.end())
    {
      m_tbtps[superframeCounter] = std::vector<Ptr<SatTbtpMessage>> ();
    }
  m_tbtps[superframeCounter].push_back (tbtp);

  Simulator::Schedule (Seconds (1), &SatGwMac::RemoveTbtp, this, superframeCounter);
}

void
SatGwMac::RemoveTbtp (uint32_t superframeCounter)
{
  m_tbtps.erase (superframeCounter);
}

void
SatGwMac::StartNcrTransmission ()
{
  NS_LOG_FUNCTION (this);

  SendNcrMessage ();

  Simulator::Schedule (m_ncrInterval, &SatGwMac::StartNcrTransmission, this);
}

void
SatGwMac::ReceiveSignalingPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  // Remove the mac tag
  SatMacTag macTag;
  packet->PeekPacketTag (macTag);

  // Peek control msg tag
  SatControlMsgTag ctrlTag;
  bool cSuccess = packet->PeekPacketTag (ctrlTag);

  if (!cSuccess)
    {
      NS_FATAL_ERROR ("SatControlMsgTag not found in the packet!");
    }

  switch (ctrlTag.GetMsgType ())
    {
    case SatControlMsgTag::SAT_CR_CTRL_MSG:
      {
        uint32_t msgId = ctrlTag.GetMsgId ();
        Ptr<SatCrMessage> crMsg = DynamicCast<SatCrMessage> ( m_readCtrlCallback (msgId) );

        if ( crMsg != NULL )
          {
            m_fwdScheduler->CnoInfoUpdated (macTag.GetSourceAddress (), crMsg->GetCnoEstimate ());

            if ( m_crReceiveCallback.IsNull () == false )
              {
                m_crReceiveCallback (m_beamId, macTag.GetSourceAddress (), crMsg);
              }
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the RTN link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }

        packet->RemovePacketTag (macTag);
        packet->RemovePacketTag (ctrlTag);

        break;
      }
    case SatControlMsgTag::SAT_CN0_REPORT:
      {
        uint32_t msgId = ctrlTag.GetMsgId ();
        Ptr<SatCnoReportMessage> cnoReport = DynamicCast<SatCnoReportMessage> ( m_readCtrlCallback (msgId) );

        if ( cnoReport != NULL )
          {
            m_fwdScheduler->CnoInfoUpdated (macTag.GetSourceAddress (), cnoReport->GetCnoEstimate ());
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the RTN link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }

        packet->RemovePacketTag (macTag);
        packet->RemovePacketTag (ctrlTag);

        break;
      }
    case SatControlMsgTag::SAT_ARQ_ACK:
      {
        // ARQ ACK messages are forwarded to LLC, since they may be fragmented
        m_rxCallback (packet, macTag.GetSourceAddress (), macTag.GetDestAddress ());
        break;
      }
    case SatControlMsgTag::SAT_HR_CTRL_MSG:
      {
        uint32_t msgId = ctrlTag.GetMsgId ();
        Ptr<SatHandoverRecommendationMessage> handoverRecommendation = DynamicCast<SatHandoverRecommendationMessage> ( m_readCtrlCallback (msgId) );

        if ( handoverRecommendation != NULL )
          {
            uint32_t beamId = handoverRecommendation->GetRecommendedBeamId ();
            m_handoverCallback (macTag.GetSourceAddress (), m_beamId, beamId);
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the RTN link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }

        break;
      }
    case SatControlMsgTag::SAT_LOGON_CTRL_MSG:
      {
        uint32_t msgId = ctrlTag.GetMsgId ();
        Ptr<SatLogonMessage> logonMessage = DynamicCast<SatLogonMessage> ( m_readCtrlCallback (msgId) );

        if ( logonMessage != NULL )
          {
            Address utId = macTag.GetSourceAddress ();
            Callback<void, uint32_t> raChannelCallback = MakeBoundCallback (&SatGwMac::SendLogonResponseHelper, this, utId);
            m_logonCallback (utId, m_beamId, raChannelCallback);
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the RTN link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatGwMac received a non-supported control packet!");
        break;
      }
    }
}

void
SatGwMac::SendNcrMessage ()
{
  NS_LOG_FUNCTION (this);
  Ptr<SatNcrMessage> ncrMessage = CreateObject<SatNcrMessage> ();
  m_fwdScheduler->SendControlMsg (ncrMessage, Mac48Address::GetBroadcast ());
  m_ncrMessagesToSend.push (ncrMessage);
}

void
SatGwMac::SendCmtMessage (Address utId, Time burstDuration)
{
  NS_LOG_FUNCTION (this << utId);

  Time lastCmtSent = Seconds (0);
  if (m_lastCmtSent.find(utId) != m_lastCmtSent.end())
  {
    lastCmtSent = m_lastCmtSent[utId];
  }

  if (Simulator::Now () < lastCmtSent + m_cmtPeriodMin)
    {
      return;
    }

  uint32_t indexClosest = 0;
  uint32_t tbtpIndexClosest = 0;
  uint32_t timeSlotIndexClosest = 0;
  Time differenceClosest = Seconds (1000000);
  std::vector<Ptr<SatTbtpMessage>> tbtpsForCurrentSF;
  Ptr<SatTbtpMessage> tbtp;
  for (uint32_t i = 0; i < m_tbtps.size (); i++)
    {
      tbtpsForCurrentSF = m_tbtps[i];
      for (uint32_t tbtpIndex = 0; tbtpIndex < tbtpsForCurrentSF.size (); tbtpIndex++)
        {
          tbtp = tbtpsForCurrentSF[tbtpIndex];
          std::pair<uint8_t, std::vector< Ptr<SatTimeSlotConf> >> timeslots = tbtp->GetDaTimeslots (utId);
          for (uint32_t j = 0; j < timeslots.second.size (); j++)
            {
              Ptr<SatTimeSlotConf> tsConf = timeslots.second[j];
              if (tsConf->GetSlotType () == SatTimeSlotConf::SLOT_TYPE_C)
                {
                  Time frameStartTime = Singleton<SatRtnLinkTime>::Get ()->GetSuperFrameTxTime (SatConstVariables::SUPERFRAME_SEQUENCE, i, Seconds (0));
                  Time slotStartTime = tsConf->GetStartTime ();
                  Time difference = Simulator::Now () - frameStartTime - slotStartTime - burstDuration;
                  if (Abs(difference) < differenceClosest)
                    {
                      differenceClosest = Abs (difference);
                      indexClosest = i;
                      timeSlotIndexClosest = j;
                      tbtpIndexClosest = tbtpIndex;
                    }
                }
            }
        }
    }

  if (indexClosest == 0)
    {
      return;
    }

  tbtp = m_tbtps[indexClosest][tbtpIndexClosest];
  std::pair<uint8_t, std::vector< Ptr<SatTimeSlotConf> >> timeslots = tbtp->GetDaTimeslots (utId);

  if (timeslots.second[timeSlotIndexClosest]->GetSlotType () == 0)
    {
      Time frameStartTime = Singleton<SatRtnLinkTime>::Get ()->GetSuperFrameTxTime (SatConstVariables::SUPERFRAME_SEQUENCE, indexClosest, Seconds (0));
      Time slotStartTime = timeslots.second[timeSlotIndexClosest]->GetStartTime ();

      Time difference = frameStartTime + slotStartTime + burstDuration - Simulator::Now ();
      int32_t differenceNcr = difference.GetMicroSeconds ()*27;

      if (differenceNcr > 16256 || differenceNcr < -16256)
        {
          NS_LOG_INFO ("Burst Time Correction outside bounds, should be at least -16256 and at most 16256, but got " << differenceNcr << ". Forcing logoff of UT");
          Ptr<SatLogoffMessage> logoffMsg = CreateObject<SatLogoffMessage> ();
          m_fwdScheduler->SendControlMsg (logoffMsg, utId);
          m_removeUtCallback (utId, m_beamId);
        }
      else
        {
          Ptr<SatCmtMessage> cmt = CreateObject<SatCmtMessage> ();
          cmt->SetBurstTimeCorrection (differenceNcr);
          m_fwdScheduler->SendControlMsg (cmt, utId);
          m_lastCmtSent[utId] = Simulator::Now ();
        }

      return;
    }
}

void
SatGwMac::SendLogonResponse (Address utId, uint32_t raChannel)
{
  NS_LOG_FUNCTION (this << utId << raChannel);
  Ptr<SatLogonResponseMessage> logonResponse = CreateObject<SatLogonResponseMessage> ();
  logonResponse->SetRaChannel (raChannel);
  m_fwdScheduler->SendControlMsg (logonResponse, utId);
}

void
SatGwMac::SendLogonResponseHelper (SatGwMac* self, Address utId, uint32_t raChannel)
{
  self->SendLogonResponse (utId, raChannel);
}

void
SatGwMac::SetCrReceiveCallback (SatGwMac::CrReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_crReceiveCallback = cb;
}

void
SatGwMac::SetHandoverCallback (SatGwMac::HandoverCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_handoverCallback = cb;
}

void
SatGwMac::SetLogonCallback (SatGwMac::LogonCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_logonCallback = cb;
}

void
SatGwMac::SetControlMessageReceivedCallback (SatGwMac::ControlMessageReceivedCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_controlMessageReceivedCallback = cb;
}

void
SatGwMac::SetRemoveUtCallback (SatGwMac::RemoveUtCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_removeUtCallback = cb;
}

void
SatGwMac::SetFwdScheduler (Ptr<SatFwdLinkScheduler> fwdScheduler)
{
  m_fwdScheduler = fwdScheduler;

  if (m_ncrV2)
    {
      m_fwdScheduler->SetDummyFrameSendingEnabled (true);
    }
}

} // namespace ns3
