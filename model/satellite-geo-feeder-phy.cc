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
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"

#include "satellite-utils.h"
#include "satellite-geo-feeder-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"

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
    .AddAttribute( "RxTemperatureDbk",
                   "RX noise temperature in Geo Feeder in dBK.",
                    DoubleValue (28.4),
                    MakeDoubleAccessor (&SatPhy::GetRxNoiseTemperatureDbk, &SatPhy::SetRxNoiseTemperatureDbk),
                    MakeDoubleChecker<double>())
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
    .AddAttribute( "ExtNoisePowerDensityDbwhz",
                   "Other system interference, C over I in dB.",
                    DoubleValue (-207.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_extNoisePowerDensityDbwHz),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "ImIfCOverIDb",
                   "Adjacent channel interference, C over I in dB.",
                    DoubleValue (27.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_imInterferenceCOverIDb),
                    MakeDoubleChecker<double> ())
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
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoFeederPhy default constructor is not allowed to use");
}

SatGeoFeederPhy::SatGeoFeederPhy (SatPhy::CreateParam_t& params,
                                  InterferenceModel ifModel,
                                  CarrierBandwidthConverter converter,
                                  uint32_t carrierCount)
  : SatPhy (params)
{
  NS_LOG_FUNCTION (this);

  SatPhy::GetPhyTx()->SetAttribute("TxMode", EnumValue(SatPhyTx::TRANSPARENT));

  ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_imInterferenceCOverI = SatUtils::DbToLinear (m_imInterferenceCOverIDb);

  // Configure the SatPhyRxCarrier instances
  // Note, that in GEO satellite, there is no need for error modeling.

  Ptr<SatPhyRxCarrierConf> carrierConf =
          CreateObject<SatPhyRxCarrierConf> (SatPhy::GetRxNoiseTemperatureDbk(),
                                             SatPhyRxCarrierConf::EM_NONE,
                                             ifModel,
                                             SatPhyRxCarrierConf::TRANSPARENT,
                                             SatEnums::FORWARD_FEEDER_CH,
                                             converter,
                                             carrierCount);

  carrierConf->SetAttribute ("ExtNoiseDensityDbwhz", DoubleValue (m_extNoisePowerDensityDbwHz) );

  carrierConf->SetSinrCalculatorCb (MakeCallback (&SatGeoFeederPhy::CalculateSinr, this));

  SatPhy::ConfigureRxCarriers (carrierConf);
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
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now(),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (txParams->m_packetsInBurst));

  // copy as sender own PhyTx object (at satellite) to ensure right distance calculation
  // and antenna gain getting at receiver (UT or GW)
  // copy on tx power too.

  txParams->m_phyTx = m_phyTx;
  txParams->m_txPower_W = m_eirpWoGainW;
  m_phyTx->StartTx (txParams);
}

void
SatGeoFeederPhy::Receive (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now(),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 SatEnums::LD_FORWARD,
                 SatUtils::GetPacketInfo (rxParams->m_packetsInBurst));

  m_rxCallback ( rxParams->m_packetsInBurst, rxParams);
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

} // namespace ns3
