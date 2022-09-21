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
#include "satellite-geo-feeder-mac.h"
#include "satellite-mac.h"
#include "satellite-time-tag.h"
#include "satellite-signal-parameters.h"


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

SatGeoFeederMac::SatGeoFeederMac (uint32_t beamId,
                                  SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                  SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatMac (beamId),
  m_fwdScheduler (),
  m_guardTime (MicroSeconds (1))
{
  NS_LOG_FUNCTION (this);

  m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
  m_returnLinkRegenerationMode = returnLinkRegenerationMode;
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
  Simulator::Schedule (txDuration, &SatGeoFeederMac::StartTransmission, this, 0);
}

void
SatGeoFeederMac::SendPackets (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams)
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

      m_llc->Enque (*it, addressE2ETag.GetE2EDestAddress (), 0);
    }
}

void
SatGeoFeederMac::SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this);

  // Add a SatMacTimeTag tag for packet delay computation at the receiver end.
  if (m_isStatisticsTagsEnabled)
    {
      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
           it != packets.end (); ++it)
        {
          (*it)->AddPacketTag (SatMacTimeTag (Simulator::Now ()));
        }
    }

  Ptr<SatSignalParameters> txParams = Create<SatSignalParameters> ();
  txParams->m_duration = duration;
  txParams->m_packetsInBurst = packets;
  txParams->m_beamId = m_beamId;
  txParams->m_carrierId = carrierId;
  txParams->m_txInfo = txInfo;

  // Use call back to send packet to lower layer
  m_txFeederCallback (txParams);
}

void
SatGeoFeederMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  m_rxFeederCallback (packets, rxParams);
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
      // TODO

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
