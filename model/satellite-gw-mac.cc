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

#include <ns3/satellite-mac-tag.h>
#include <ns3/satellite-utils.h>
#include <ns3/satellite-log.h>
#include "satellite-gw-mac.h"

#include <ns3/packet.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-bbframe.h>
#include <ns3/satellite-signal-parameters.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-fwd-link-scheduler.h>

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
    .AddTraceSource ("BBFrameTxTrace",
                     "Trace for transmitted BB Frames.",
                     MakeTraceSourceAccessor (&SatGwMac::m_bbFrameTxTrace),
                     "ns3::SatBbFrame::BbFrameCallback")
  ;
  return tid;
}

SatGwMac::SatGwMac ()
  : SatMac (),
    m_fwdScheduler (),
    m_guardTime (MicroSeconds (1))
{
  NS_LOG_FUNCTION (this);
}

SatGwMac::SatGwMac (uint32_t beamId)
  : SatMac (beamId),
    m_fwdScheduler (),
    m_guardTime (MicroSeconds (1))
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
}

void
SatGwMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
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
}

void
SatGwMac::StartTransmission (uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);

  Time txDuration;

  if (m_txEnabled)
    {
  	  std::pair<Ptr<SatBbFrame>, const Time> bbFrameInfo = m_fwdScheduler->GetNextFrame ();
      Ptr<SatBbFrame> bbFrame = bbFrameInfo.first;
      txDuration = bbFrameInfo.second;

      // Handle both dummy frames and normal frames
			if ( bbFrame != NULL )
				{
          // trace out BB frames sent.
          m_bbFrameTxTrace (bbFrame->GetFrameType ());

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
          txInfo.frameType = bbFrame->GetFrameType ();
          txInfo.waveformId = 0;

          /**
           * Decrease a guard time from BB frame duration.
           */
          SendPacket (bbFrame->GetPayload (), carrierId, txDuration - m_guardTime, txInfo);
        }
      // Pass information of dummy frame even if dummy frames are not generated
      else m_bbFrameTxTrace (SatEnums::DUMMY_FRAME);
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
    case SatControlMsgTag::SAT_TBTP_CTRL_MSG:
    case SatControlMsgTag::SAT_RA_CTRL_MSG:
      {
        NS_FATAL_ERROR ("SatGwMac received a non-supported control packet!");
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
SatGwMac::SetCrReceiveCallback (SatGwMac::CrReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_crReceiveCallback = cb;
}

} // namespace ns3
