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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *         Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <limits>

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>

#include "satellite-utils.h"
#include "satellite-geo-user-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel-estimation-error-container.h"
#include "satellite-address-tag.h"
#include "satellite-time-tag.h"
#include "satellite-uplink-info-tag.h"
#include "satellite-mac-tag.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoUserPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoUserPhy);

TypeId
SatGeoUserPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoUserPhy")
    .SetParent<SatPhy> ()
    .AddConstructor<SatGeoUserPhy> ()
    .AddAttribute ("PhyRx", "The PhyRx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatPhy::GetPhyRx, &SatPhy::SetPhyRx),
                   MakePointerChecker<SatPhyRx> ())
    .AddAttribute ("PhyTx", "The PhyTx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatPhy::GetPhyTx, &SatPhy::SetPhyTx),
                   MakePointerChecker<SatPhyTx> ())
    .AddAttribute ( "RxTemperatureDbk",
                    "RX noise temperature in Geo User in dBK.",
                    DoubleValue (28.4),
                    MakeDoubleAccessor (&SatPhy::GetRxNoiseTemperatureDbk, &SatPhy::SetRxNoiseTemperatureDbk),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("RxMaxAntennaGainDb", "Maximum RX antenna gain in Db",
                   DoubleValue (54.00),
                   MakeDoubleAccessor (&SatPhy::GetRxAntennaGainDb, &SatPhy::SetRxAntennaGainDb),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxAntennaGainDb", "Maximum TX gain in dB",
                   DoubleValue (54.00),
                   MakeDoubleAccessor (&SatPhy::GetTxAntennaGainDb, &SatPhy::SetTxAntennaGainDb),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxPowerDbw", "Maximum TX power in dB",
                   DoubleValue (15.00),
                   MakeDoubleAccessor (&SatPhy::GetTxMaxPowerDbw, &SatPhy::SetTxMaxPowerDbw),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxOutputLossDb", "TX Output loss in dB",
                   DoubleValue (2.85),
                   MakeDoubleAccessor (&SatPhy::GetTxOutputLossDb, &SatPhy::SetTxOutputLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPointingLossDb", "TX Pointing loss in dB",
                   DoubleValue (0.00),
                   MakeDoubleAccessor (&SatPhy::GetTxPointingLossDb, &SatPhy::SetTxPointingLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxOboLossDb", "TX OBO loss in dB",
                   DoubleValue (0.00),
                   MakeDoubleAccessor (&SatPhy::GetTxOboLossDb, &SatPhy::SetTxOboLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxAntennaLossDb", "TX Antenna loss in dB",
                   DoubleValue (1.00),
                   MakeDoubleAccessor (&SatPhy::GetTxAntennaLossDb, &SatPhy::SetTxAntennaLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxAntennaLossDb", "RX Antenna loss in dB",
                   DoubleValue (1.00),
                   MakeDoubleAccessor (&SatPhy::GetRxAntennaLossDb, &SatPhy::SetRxAntennaLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DefaultFadingValue", "Default value for fading",
                   DoubleValue (1.00),
                   MakeDoubleAccessor (&SatPhy::GetDefaultFading, &SatPhy::SetDefaultFading),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ( "OtherSysIfCOverIDb",
                    "Other system interference, C over I in dB.",
                    DoubleValue (27.5),
                    MakeDoubleAccessor (&SatGeoUserPhy::m_otherSysInterferenceCOverIDb),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "AciIfCOverIDb",
                    "Adjacent channel interference, C over I in dB.",
                    DoubleValue (17.0),
                    MakeDoubleAccessor (&SatGeoUserPhy::m_aciInterferenceCOverIDb),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "QueueSize",
                    "Maximum size of FIFO m_queue in bytes.",
                    UintegerValue (100000),
                    MakeUintegerAccessor (&SatGeoUserPhy::m_queueSizeMax),
                    MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("QueueSizeBytes",
                     "Send number of bytes in FIFO return feeder queue",
                     MakeTraceSourceAccessor (&SatGeoUserPhy::m_queueSizeBytesTrace),
                     "ns3::SatStatsRtnFeederQueueHelper::QueueSizeCallback")
    .AddTraceSource ("QueueSizePackets",
                     "Send number of packets in FIFO return feeder queue",
                     MakeTraceSourceAccessor (&SatGeoUserPhy::m_queueSizePacketsTrace),
                     "ns3::SatStatsRtnFeederQueueHelper::QueueSizeCallback")
  ;
  return tid;
}

TypeId
SatGeoUserPhy::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGeoUserPhy::SatGeoUserPhy (void)
  : m_aciInterferenceCOverIDb (17.0),
  m_otherSysInterferenceCOverIDb (27.5),
  m_aciInterferenceCOverI (SatUtils::DbToLinear (m_aciInterferenceCOverIDb)),
  m_otherSysInterferenceCOverI (SatUtils::DbToLinear (m_otherSysInterferenceCOverIDb)),
  m_queueSizeBytes (0),
  m_queueSizePackets (0)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoUserPhy default constructor is not allowed to use");
}

SatGeoUserPhy::SatGeoUserPhy (SatPhy::CreateParam_t& params,
                              Ptr<SatLinkResults> linkResults,
                              SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                              Ptr<SatSuperframeConf> superFrameConf,
                              SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                              SatEnums::RegenerationMode_t returnLinkRegenerationMode)
  : SatPhy (params),
  m_queueSizeBytes (0),
  m_queueSizePackets (0)
{
  NS_LOG_FUNCTION (this);

  m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
  m_returnLinkRegenerationMode = returnLinkRegenerationMode;
  m_isSending = false;
  m_queueSizeBytes = 0;
  m_queueSizePackets = 0;

  if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_PHY)
    {
      m_queue = std::queue<std::tuple<Ptr<SatSignalParameters>, uint32_t, uint32_t>> ();
    }

  if (forwardLinkRegenerationMode == SatEnums::TRANSPARENT)
    {
      SatPhy::GetPhyTx ()->SetAttribute ("TxMode", EnumValue (SatPhyTx::TRANSPARENT));
    }
  else
    {
      SatPhy::GetPhyTx ()->SetAttribute ("TxMode", EnumValue (SatPhyTx::NORMAL));
    }

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  m_aciInterferenceCOverI = SatUtils::DbToLinear (m_aciInterferenceCOverIDb);
  m_otherSysInterferenceCOverI = SatUtils::DbToLinear (m_otherSysInterferenceCOverIDb);

  parameters.m_rxTemperatureK = SatUtils::DbToLinear (SatPhy::GetRxNoiseTemperatureDbk ());
  parameters.m_aciIfWrtNoiseFactor = 0.0;
  parameters.m_extNoiseDensityWhz = 0.0;
  if (returnLinkRegenerationMode == SatEnums::TRANSPARENT)
    {
      parameters.m_rxMode = SatPhyRxCarrierConf::TRANSPARENT;
    }
  else
    {
      parameters.m_rxMode = SatPhyRxCarrierConf::NORMAL;
    }
  parameters.m_linkRegenerationMode = returnLinkRegenerationMode;
  parameters.m_chType = SatEnums::RETURN_USER_CH;

  Ptr<SatPhyRxCarrierConf> carrierConf = CreateObject<SatPhyRxCarrierConf> (parameters);

  if (linkResults)
    {
      carrierConf->SetLinkResults (linkResults);
    }

  carrierConf->SetAdditionalInterferenceCb (MakeCallback (&SatGeoUserPhy::GetAdditionalInterference, this));

  SatPhy::ConfigureRxCarriers (carrierConf, superFrameConf);
}

SatGeoUserPhy::~SatGeoUserPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoUserPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatGeoUserPhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatGeoUserPhy::SendPduWithParams (Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << txParams);
  NS_LOG_INFO (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

  if (m_forwardLinkRegenerationMode != SatEnums::TRANSPARENT)
    {
      SetTimeTag (txParams->m_packetsInBurst);
    }

  // copy as sender own PhyTx object (at satellite) to ensure right distance calculation
  // and antenna gain getting at receiver (UT or GW)
  // copy on tx power too.

  txParams->m_phyTx = m_phyTx;
  txParams->m_txPower_W = m_eirpWoGainW;

  SatEnums::SatPacketEvent_t event;

  if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_PHY)
    {
      uint32_t nbBytes = 0;
      for (Ptr<Packet> pkt : txParams->m_packetsInBurst)
        {
          nbBytes += pkt->GetSize ();
        }
      uint32_t nbPackets = txParams->m_packetsInBurst.size ();
      if (m_queueSizeBytes + nbBytes < m_queueSizeMax)
        {
          event = SatEnums::PACKET_ENQUE;
          m_queue.push (std::make_tuple(txParams, nbBytes, nbPackets));
          m_queueSizeBytes += nbBytes;
          m_queueSizePackets += nbPackets;

          m_queueSizeBytesTrace (m_queueSizeBytes, GetE2EDestinationAddress (txParams->m_packetsInBurst));
          m_queueSizePacketsTrace (m_queueSizePackets, GetE2EDestinationAddress (txParams->m_packetsInBurst));

          if (m_isSending == false)
            {
              SendFromQueue ();
            }
        }
      else
        {
          event = SatEnums::PACKET_DROP;
          NS_LOG_INFO ("Packet dropped because REGENERATION_PHY queue is full");
        }
    }
  else
    {
      event = SatEnums::PACKET_SENT;
      m_phyTx->StartTx (txParams);
    }

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 event,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_FORWARD,
                 SatUtils::GetPacketInfo (txParams->m_packetsInBurst));
}

void
SatGeoUserPhy::SendFromQueue ()
{
  if (m_queue.empty ())
    {
      NS_FATAL_ERROR ("Trying to deque an empty queue");
    }

  m_isSending = true;
  std::tuple<Ptr<SatSignalParameters>, uint32_t, uint32_t> element = m_queue.front();
  m_queue.pop ();

  Ptr<SatSignalParameters> txParams = std::get<0> (element);
  m_queueSizeBytes -= std::get<1> (element);
  m_queueSizePackets -= std::get<2> (element);

  m_queueSizeBytesTrace (m_queueSizeBytes, GetE2EDestinationAddress (txParams->m_packetsInBurst));
  m_queueSizePacketsTrace (m_queueSizePackets, GetE2EDestinationAddress (txParams->m_packetsInBurst));

  // Add sent packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (txParams->m_packetsInBurst));

  Simulator::Schedule (txParams->m_duration + NanoSeconds (1), &SatGeoUserPhy::EndTx, this);

  m_phyTx->StartTx (txParams);
}

void
SatGeoUserPhy::EndTx ()
{
  m_isSending = false;
  if (!m_queue.empty ())
    {
      this->SendFromQueue ();
    }
}

void
SatGeoUserPhy::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  if (m_isStatisticsTagsEnabled)
    {
      SatSignalParameters::PacketsInBurst_t::iterator it1;
      for (it1 = packets.begin ();
           it1 != packets.end (); ++it1)
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

          SatPhyLinkTimeTag linkTimeTag;
          if ((*it1)->RemovePacketTag (linkTimeTag))
            {
              NS_LOG_DEBUG (this << " contains a SatPhyLinkTimeTag tag");
              Time delay = Simulator::Now () - linkTimeTag.GetSenderLinkTimestamp ();
              m_rxLinkDelayTrace (delay, addr);
              if (m_lastLinkDelay.IsZero() == false)
                {
                  Time jitter = Abs (delay - m_lastLinkDelay);
                  m_rxLinkJitterTrace (jitter, addr);
                }
              m_lastLinkDelay = delay;
            }

        } // end of `for (it1 = rxParams->m_packetsInBurst)`

    } // end of `if (m_isStatisticsTagsEnabled)`
}

void
SatGeoUserPhy::Receive (Ptr<SatSignalParameters> rxParams, bool phyError)
{
  NS_LOG_FUNCTION (this << rxParams);

  SatEnums::SatPacketEvent_t event = (phyError) ? SatEnums::PACKET_DROP : SatEnums::PACKET_RECV;

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 event,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (rxParams->m_packetsInBurst));

  if (phyError)
    {
      // If there was a PHY error, the packet is dropped here.
      NS_LOG_INFO (this << " dropped " << rxParams->m_packetsInBurst.size ()
                        << " packets because of PHY error.");
    }
  else
    {
      switch (m_returnLinkRegenerationMode)
        {
          case SatEnums::TRANSPARENT:
            break;
          case SatEnums::REGENERATION_LINK:
          case SatEnums::REGENERATION_NETWORK:
            {
              if (rxParams->m_txInfo.waveformId == 2)
                {
                  SatSignalParameters::PacketsInBurst_t::iterator it;
                  for (it = rxParams->m_packetsInBurst.begin (); it != rxParams->m_packetsInBurst.end (); it++ )
                    {
                      SatUplinkInfoTag satUplinkInfoTag;
                      (*it)->RemovePacketTag (satUplinkInfoTag);
                      satUplinkInfoTag.SetIsControl (true);
                      (*it)->AddPacketTag (satUplinkInfoTag);
                    }
                }
              // No break here: need to add additional fields SatUplinkInfoTag
            }
          case SatEnums::REGENERATION_PHY:
            {
              rxParams->m_txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;

              SatSignalParameters::PacketsInBurst_t::iterator it;
              for (it = rxParams->m_packetsInBurst.begin (); it != rxParams->m_packetsInBurst.end (); it++)
                {
                  SatUplinkInfoTag satUplinkInfoTag;
                  (*it)->RemovePacketTag (satUplinkInfoTag);
                  satUplinkInfoTag.SetBeamId (rxParams->m_beamId);
                  satUplinkInfoTag.SetSatelliteReceptionTime (Simulator::Now ());
                  (*it)->AddPacketTag (satUplinkInfoTag);
                }

              RxTraces (rxParams->m_packetsInBurst);

              break;
            }
          default:
            NS_FATAL_ERROR ("Unknown regeneration mode");
        }

      m_rxCallback ( rxParams->m_packetsInBurst, rxParams);
    }
}

double
SatGeoUserPhy::GetAdditionalInterference ()
{
  NS_LOG_FUNCTION (this);

  return 1 / ( (1 / m_aciInterferenceCOverI) + (1 / m_otherSysInterferenceCOverI) );
}

SatEnums::SatLinkDir_t
SatGeoUserPhy::GetSatLinkTxDir ()
{
  return SatEnums::LD_FORWARD;
}

SatEnums::SatLinkDir_t
SatGeoUserPhy::GetSatLinkRxDir ()
{
  return SatEnums::LD_RETURN;
}

Address
SatGeoUserPhy::GetE2EDestinationAddress (SatPhy::PacketContainer_t packets)
{
  SatSignalParameters::PacketsInBurst_t::iterator it1;
  for (it1 = packets.begin ();
       it1 != packets.end (); ++it1)
    {
      Address addr; // invalid address.
      SatAddressE2ETag satAddressE2ETag;
      if ((*it1)->PeekPacketTag (satAddressE2ETag))
        {
          return satAddressE2ETag.GetE2EDestAddress ();
        }
    }
  return Mac48Address ();
}

} // namespace ns3
