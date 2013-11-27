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
 * Author: Sami Rantanen <sami.rantanennen@magister.fi>
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
    .AddAttribute( "ExtNoisePowerDensityDbWHz",
                   "Other system interference, C over I in dB.",
                    DoubleValue (-207.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_extNoisePowerDensityDbWHz),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "ImIfCOverIDb",
                   "Adjacent channel interference, C over I in dB.",
                    DoubleValue (27.0),
                    MakeDoubleAccessor (&SatGeoFeederPhy::m_imInterferenceCiDb),
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

SatGeoFeederPhy::SatGeoFeederPhy (SatPhy::CreateParam_t& params, InterferenceModel ifModel,
                                  CarrierBandwidthConverter converter, uint32_t carrierCount )
  : SatPhy (params)
{
  NS_LOG_FUNCTION (this);

  SatPhy::GetPhyTx()->SetAttribute("TxMode", EnumValue(SatPhyTx::TRANSPARENT));

  ObjectBase::ConstructSelf(AttributeConstructionList ());

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

  carrierConf->SetAttribute ("ExtNoiseDensityDbWHz", DoubleValue (m_extNoisePowerDensityDbWHz) );
  carrierConf->SetAttribute ("RxImIfDb", DoubleValue (m_imInterferenceCiDb) );

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
SatGeoFeederPhy::DoStart ()
{
  NS_LOG_FUNCTION (this);
  Object::DoStart ();
}

} // namespace ns3
