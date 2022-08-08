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
#include <ns3/enum.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>

#include "satellite-utils.h"
#include "satellite-geo-feeder-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel-estimation-error-container.h"
#include "satellite-address-tag.h"
#include "satellite-time-tag.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoFeederPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoFeederPhy);

TypeId
SatGeoFeederPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoFeederPhy")
    .SetParent<SatPhy> ()
    .AddConstructor<SatGeoFeederPhy> ()
    .AddAttribute ("PhyRx", "The PhyRx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatPhy::GetPhyRx, &SatPhy::SetPhyRx),
                   MakePointerChecker<SatPhyRx> ())
    .AddAttribute ("PhyTx", "The PhyTx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatPhy::GetPhyTx, &SatPhy::SetPhyTx),
                   MakePointerChecker<SatPhyTx> ())
    .AddAttribute ( "RxTemperatureDbk",
                    "RX noise temperature in Geo Feeder in dBK.",
                    DoubleValue (28.4),
                    MakeDoubleAccessor (&SatPhy::GetRxNoiseTemperatureDbk, &SatPhy::SetRxNoiseTemperatureDbk),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("RxMaxAntennaGainDb", "Maximum RX gain in dB",
                   DoubleValue (54.00),
                   MakeDoubleAccessor (&SatPhy::GetRxAntennaGainDb, &SatPhy::SetRxAntennaGainDb),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxAntennaGainDb", "Maximum TX gain in dB",
                   DoubleValue (54.00),
                   MakeDoubleAccessor (&SatPhy::GetTxAntennaGainDb, &SatPhy::SetTxAntennaGainDb),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxPowerDbw", "Maximum TX power in dB",
                   DoubleValue (-4.38),
                   MakeDoubleAccessor (&SatPhy::GetTxMaxPowerDbw, &SatPhy::SetTxMaxPowerDbw),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxOutputLossDb", "TX Output loss in dB",
                   DoubleValue (1.75),
                   MakeDoubleAccessor (&SatPhy::GetTxOutputLossDb, &SatPhy::SetTxOutputLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPointingLossDb", "TX Pointing loss in dB",
                   DoubleValue (0.00),
                   MakeDoubleAccessor (&SatPhy::GetTxPointingLossDb, &SatPhy::SetTxPointingLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxOboLossDb", "TX OBO loss in dB",
                   DoubleValue (4.00),
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
    .AddAttribute ( "ExtNoisePowerDensityDbwhz",
                    "Other system interference, C over I in dB.",
                    DoubleValue (-207.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_extNoisePowerDensityDbwHz),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "ImIfCOverIDb",
                    "Adjacent channel interference, C over I in dB.",
                    DoubleValue (27.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_imInterferenceCOverIDb),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "FixedAmplificationGainDb",
                    "Fixed amplification gain used in RTN link at the satellite.",
                    DoubleValue (82.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_fixedAmplificationGainDb),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "QueueSize",
                    "Maximum size of FIFO m_queue in bursts.",
                    UintegerValue (100),
                    MakeUintegerAccessor (&SatGeoFeederPhy::m_queueSizeMax),
                    MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TypeId
SatGeoFeederPhy::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGeoFeederPhy::SatGeoFeederPhy (void)
  : m_extNoisePowerDensityDbwHz (-207.0),
  m_imInterferenceCOverIDb (27.0),
  m_imInterferenceCOverI (SatUtils::DbToLinear (m_imInterferenceCOverIDb)),
  m_fixedAmplificationGainDb (82.0)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoFeederPhy default constructor is not allowed to use");
}

SatGeoFeederPhy::SatGeoFeederPhy (SatPhy::CreateParam_t& params,
                                  Ptr<SatLinkResults> linkResults,
                                  SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                                  Ptr<SatSuperframeConf> superFrameConf,
                                  SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                  SatEnums::RegenerationMode_t returnLinkRegenerationMode)
  : SatPhy (params)
{
  NS_LOG_FUNCTION (this);

  m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
  m_returnLinkRegenerationMode = returnLinkRegenerationMode;
  m_isSending = false;

  if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_PHY)
    {
      m_queue = std::queue<Ptr<SatSignalParameters>> ();
    }

  if (m_returnLinkRegenerationMode == SatEnums::TRANSPARENT)
    {
      SatPhy::GetPhyTx ()->SetAttribute ("TxMode", EnumValue (SatPhyTx::TRANSPARENT));
    }
  else
    {
      SatPhy::GetPhyTx ()->SetAttribute ("TxMode", EnumValue (SatPhyTx::NORMAL));
    }

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  m_imInterferenceCOverI = SatUtils::DbToLinear (m_imInterferenceCOverIDb);

  // Configure the SatPhyRxCarrier instances
  // Note, that in GEO satellite, there is no need for error modeling.

  parameters.m_rxTemperatureK = SatUtils::DbToLinear ( SatPhy::GetRxNoiseTemperatureDbk ());
  parameters.m_extNoiseDensityWhz = SatUtils::DbToLinear ( m_extNoisePowerDensityDbwHz );
  parameters.m_aciIfWrtNoiseFactor = 0.0;
  if (forwardLinkRegenerationMode == SatEnums::TRANSPARENT)
    {
      parameters.m_rxMode = SatPhyRxCarrierConf::TRANSPARENT;
    }
  else
    {
      parameters.m_rxMode = SatPhyRxCarrierConf::NORMAL;
    }
  parameters.m_linkRegenerationMode = forwardLinkRegenerationMode;
  parameters.m_chType = SatEnums::FORWARD_FEEDER_CH;

  Ptr<SatPhyRxCarrierConf> carrierConf = CreateObject<SatPhyRxCarrierConf> (parameters);

  if (linkResults)
    {
      carrierConf->SetLinkResults (linkResults);
    }

  carrierConf->SetSinrCalculatorCb (MakeCallback (&SatGeoFeederPhy::CalculateSinr, this));

  SatPhy::ConfigureRxCarriers (carrierConf, superFrameConf);
}

SatGeoFeederPhy::~SatGeoFeederPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoFeederPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatGeoFeederPhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatGeoFeederPhy::SendPduWithParams (Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << txParams);
  NS_LOG_INFO (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

  if (m_returnLinkRegenerationMode != SatEnums::TRANSPARENT)
    {
      SetTimeTag (txParams->m_packetsInBurst);
    }

  // copy as sender own PhyTx object (at satellite) to ensure right distance calculation
  // and antenna gain getting at receiver (UT or GW)
  // copy on tx power too.

  txParams->m_phyTx = m_phyTx;

  /**
   * In return link, at the satellite, instead of using a constant EIRP (without gain), we are
   * using a fixed amplifier gain amplifying the received signal. With this fixed gain, all bursts
   * in a slot are amplified by the same amplification gain before being transmitted on the feeder
   * link downlink. So, the tx power will be weak for the weak burst, and strong for the strong burst.
   * This approach shall be used for:
   * 1) RTN link only.
   * 2) For all CRDSA, SA, and DA.
   */

  txParams->m_txPower_W = txParams->m_rxPower_W * SatUtils::DbToLinear (m_fixedAmplificationGainDb);
  //txParams->m_txPower_W = m_eirpWoGainW;

  NS_LOG_INFO ("Amplified Tx power: " << SatUtils::LinearToDb (txParams->m_txPower_W));
  NS_LOG_INFO ("Statically configured tx power: " << SatUtils::LinearToDb (m_eirpWoGainW));

  SatEnums::SatPacketEvent_t event;

  if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_PHY)
    {
      if (m_queue.size () < m_queueSizeMax)
        {
          event = SatEnums::PACKET_ENQUE;
          m_queue.push (txParams);
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
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (txParams->m_packetsInBurst));
}

void
SatGeoFeederPhy::SendFromQueue ()
{
  if (m_queue.empty ())
    {
      NS_FATAL_ERROR ("Trying to deque an empty queue");
    }
  m_isSending = true;
  Ptr<SatSignalParameters> txParams = m_queue.front ();
  m_queue.pop ();

  // Add sent packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (txParams->m_packetsInBurst));

  Simulator::Schedule (txParams->m_duration + NanoSeconds (1), &SatGeoFeederPhy::EndTx, this);
  m_phyTx->StartTx (txParams);
}

void
SatGeoFeederPhy::EndTx ()
{
  m_isSending = false;
  if (!m_queue.empty ())
    {
      this->SendFromQueue ();
    }
}

void
SatGeoFeederPhy::RxTraces (SatPhy::PacketContainer_t packets)
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

          SatMacTag macTag;
          if ((*it1)->PeekPacketTag (macTag))
            {
              NS_LOG_DEBUG (this << " contains a SatMac tag");
              addr = macTag.GetDestAddress ();
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
SatGeoFeederPhy::Receive (Ptr<SatSignalParameters> rxParams, bool phyError)
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
                 SatEnums::LD_FORWARD,
                 SatUtils::GetPacketInfo (rxParams->m_packetsInBurst));

  if (phyError)
    {
      // If there was a PHY error, the packet is dropped here.
      NS_LOG_INFO (this << " dropped " << rxParams->m_packetsInBurst.size ()
                        << " packets because of PHY error.");
    }
  else
    {
      // In regenerative mode, we do not need to keep SINR of uplink when handling packet in satellite
      // We put infinite to SINR stored so that it has no impact on composite SINR: composite_sinr(inf,sinr_donwlink)=sinr_downlink
      if (m_forwardLinkRegenerationMode != SatEnums::TRANSPARENT)
        {
          rxParams->m_txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
          rxParams->SetSinr (std::numeric_limits<double>::infinity(), rxParams->GetSinrCalculator ());

          RxTraces (rxParams->m_packetsInBurst);
        }

      m_rxCallback ( rxParams->m_packetsInBurst, rxParams);
    }
}

double
SatGeoFeederPhy::CalculateSinr (double sinr)
{
  NS_LOG_FUNCTION (this << sinr);

  if ( sinr <= 0  )
    {
      NS_FATAL_ERROR ( "Calculated own SINR is expected to be greater than zero!!!");
    }

  // calculate final SINR taken into account configured additional interferences (C over I)
  // in addition to CCI which is included in given SINR

  double finalSinr = 1 / ( (1 / sinr) + (1 / m_imInterferenceCOverI) );

  return (finalSinr);
}

SatEnums::SatLinkDir_t
SatGeoFeederPhy::GetSatLinkTxDir ()
{
  return SatEnums::LD_RETURN;
}

SatEnums::SatLinkDir_t
SatGeoFeederPhy::GetSatLinkRxDir ()
{
  return SatEnums::LD_FORWARD;
}

} // namespace ns3
