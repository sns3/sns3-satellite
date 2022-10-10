/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>

#include "satellite-utils.h"
#include "satellite-mac.h"
#include "satellite-time-tag.h"
#include "satellite-address-tag.h"
#include "satellite-signal-parameters.h"

#include "satellite-geo-feeder-mac.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoFeederMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoFeederMac);

TypeId
SatGeoFeederMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoFeederMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatGeoFeederMac> ()
    .AddAttribute ("Scheduler",
                   "Return link scheduler used by this Sat Geo Feeder MAC (SCPC).",
                   PointerValue (),
                   MakePointerAccessor (&SatGeoFeederMac::m_fwdScheduler),
                   MakePointerChecker<SatFwdLinkScheduler> ())
    .AddAttribute ("GuardTime",
                   "Guard time in SCPC link",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&SatGeoFeederMac::m_guardTime),
                   MakeTimeChecker ())
    .AddTraceSource ("BBFrameTxTrace",
                     "Trace for transmitted BB Frames.",
                     MakeTraceSourceAccessor (&SatGeoFeederMac::m_bbFrameTxTrace),
                     "ns3::SatBbFrame::BbFrameCallback")
  ;
  return tid;
}

TypeId
SatGeoFeederMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGeoFeederMac::SatGeoFeederMac (void)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoFeederMac default constructor is not allowed to use");
}

SatGeoFeederMac::SatGeoFeederMac (SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                  SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatMac (forwardLinkRegenerationMode, returnLinkRegenerationMode),
  m_fwdScheduler (),
  m_guardTime (MicroSeconds (1))
{
  NS_LOG_FUNCTION (this);
}

SatGeoFeederMac::~SatGeoFeederMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoFeederMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatGeoFeederMac::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatGeoFeederMac::StartPeriodicTransmissions ()
{
  NS_LOG_FUNCTION (this);

  if ( m_fwdScheduler == NULL )
    {
      NS_FATAL_ERROR ("Scheduler not set for GEO FEEDER MAC!!!");
    }

  Simulator::Schedule (Seconds (0), &SatGeoFeederMac::StartTransmission, this, 0);
}

void
SatGeoFeederMac::StartTransmission (uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);

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

      NS_LOG_INFO ("TX is disabled, thus nothing is transmitted!");
      txDuration = m_fwdScheduler->GetDefaultFrameDuration ();
    }

  Simulator::Schedule (txDuration, &SatGeoFeederMac::StartTransmission, this, 0);
}

void
SatGeoFeederMac::EnquePackets (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this);

  // Update MAC address source and destination
  for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
       it != packets.end (); ++it)
    {
      SatMacTag mTag;
      bool success = (*it)->RemovePacketTag (mTag);

      SatAddressE2ETag addressE2ETag;
      success &= (*it)->PeekPacketTag (addressE2ETag);

      // MAC tag and E2E address tag found
      if (success)
        {
          mTag.SetDestAddress (addressE2ETag.GetE2EDestAddress ());
          mTag.SetSourceAddress (m_nodeInfo->GetMacAddress ());
          (*it)->AddPacketTag (mTag);
        }

      uint8_t flowId = 1;
      SatControlMsgTag ctrlTag;
      if ((*it)->PeekPacketTag (ctrlTag))
        {
          flowId = 0;
        }

      m_llc->Enque (*it, addressE2ETag.GetE2EDestAddress (), flowId);
    }
}

void
SatGeoFeederMac::SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this);

  // Add a SatMacTimeTag tag for packet delay computation at the receiver end.
  SetTimeTag (packets);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_MAC,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (packets));

  Ptr<SatSignalParameters> txParams = Create<SatSignalParameters> ();
  txParams->m_duration = duration;
  txParams->m_packetsInBurst = packets;
  txParams->m_carrierId = carrierId;
  txParams->m_txInfo = txInfo;

  // Use call back to send packet to lower layer
  m_txFeederCallback (txParams);
}

void
SatGeoFeederMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_LINK || m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
      // Add packet trace entry:
      m_packetTrace (Simulator::Now (),
                     SatEnums::PACKET_RECV,
                     m_nodeInfo->GetNodeType (),
                     m_nodeInfo->GetNodeId (),
                     m_nodeInfo->GetMacAddress (),
                     SatEnums::LL_MAC,
                     SatEnums::LD_FORWARD,
                     SatUtils::GetPacketInfo (packets));

      RxTraces (packets);
    }

  //rxParams->m_packetsInBurst.clear ();

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

      Mac48Address destAddress = macTag.GetDestAddress ();
      if (destAddress == m_nodeInfo->GetMacAddress ())
        {
          // Remove control msg tag
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
              //rxParams->m_packetsInBurst.push_back (*i);
            }
        }
      else
        {
          //rxParams->m_packetsInBurst.push_back (*i);
        }
    }

  m_rxFeederCallback (packets, rxParams);
}

void
SatGeoFeederMac::ReceiveSignalingPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

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
              NS_LOG_WARN ("Control message " << ctrlTag.GetMsgType () << " is not found from the RTN link control msg container!" << " at: " << Now ().GetSeconds () << "s");
            }

          //packet->RemovePacketTag (macTag);
          //packet->RemovePacketTag (ctrlTag);

          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Control message unkonwn on feeder MAC");
        }
    }
}

void
SatGeoFeederMac::SetTransmitFeederCallback (TransmitFeederCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_txFeederCallback = cb;
}

void
SatGeoFeederMac::SetReceiveFeederCallback (ReceiveFeederCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxFeederCallback = cb;
}

void
SatGeoFeederMac::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  if (m_isStatisticsTagsEnabled)
    {
      for (SatPhy::PacketContainer_t::const_iterator it1 = packets.begin ();
           it1 != packets.end (); ++it1)
        {
          // Remove packet tag
          SatMacTag macTag;
          bool mSuccess = (*it1)->PeekPacketTag (macTag);
          if (!mSuccess)
            {
              NS_FATAL_ERROR ("MAC tag was not found from the packet!");
            }

          // If the packet is intended for this receiver
          Mac48Address destAddress = macTag.GetDestAddress ();

          if (destAddress == m_nodeInfo->GetMacAddress ())
            {
              Address addr; // invalid address.

              bool isTaggedWithAddress = false;
              ByteTagIterator it2 = (*it1)->GetByteTagIterator ();

              while (!isTaggedWithAddress && it2.HasNext ())
                {
                  ByteTagIterator::Item item = it2.Next ();

                  if (item.GetTypeId () == SatAddressTag::GetTypeId ())
                    {
                      NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                         << " start=" << item.GetStart ()
                                         << " end=" << item.GetEnd ());
                      SatAddressTag addrTag;
                      item.GetTag (addrTag);
                      addr = addrTag.GetSourceAddress ();
                      isTaggedWithAddress = true; // this will exit the while loop.
                    }
                }

              m_rxTrace (*it1, addr);

              SatMacLinkTimeTag linkTimeTag;
              if ((*it1)->RemovePacketTag (linkTimeTag))
                {
                  NS_LOG_DEBUG (this << " contains a SatMacLinkTimeTag tag");
                  Time delay = Simulator::Now () - linkTimeTag.GetSenderLinkTimestamp ();
                  m_rxLinkDelayTrace (delay, addr);
                  if (m_lastLinkDelay.IsZero() == false)
                    {
                      Time jitter = Abs (delay - m_lastLinkDelay);
                      m_rxLinkJitterTrace (jitter, addr);
                    }
                  m_lastLinkDelay = delay;
                }
            } // end of `if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast ())`
        } // end of `for it1 = packets.begin () -> packets.end ()`
    } // end of `if (m_isStatisticsTagsEnabled)`
}

SatEnums::SatLinkDir_t
SatGeoFeederMac::GetSatLinkTxDir ()
{
  return SatEnums::LD_FORWARD;
}

SatEnums::SatLinkDir_t
SatGeoFeederMac::GetSatLinkRxDir ()
{
  return SatEnums::LD_RETURN;
}

void
SatGeoFeederMac::SetFwdScheduler (Ptr<SatFwdLinkScheduler> fwdScheduler)
{
  m_fwdScheduler = fwdScheduler;
}

void
SatGeoFeederMac::SetLlc (Ptr<SatGeoFeederLlc> llc)
{
  m_llc = llc;
}

} // namespace ns3
