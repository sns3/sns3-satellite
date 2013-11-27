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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "satellite-utils.h"
#include "satellite-phy-rx-carrier-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierConf");

namespace ns3 {

SatPhyRxCarrierConf::SatPhyRxCarrierConf ()
: m_ifModel (),
  m_errorModel (),
  m_linkResults (),
  m_rxTemperature_K (),
  m_rxExtNoiseDensity_dbWHz (),
  m_rxOtherSysInterference_db (),
  m_rxImInterference_db (),
  m_rxAciInterference_db (),
  m_rxAciIfWrtNoise (),
  m_rxMode (),
  m_enableIntfOutputTrace (false),
  m_carrierCount (),
  m_carrierBandwidthConverter (),
  m_channelType ()
{
  NS_FATAL_ERROR ("SatPhyRxCarrierConf::SatPhyRxCarrierConf - Constructor not in use");
}

SatPhyRxCarrierConf::SatPhyRxCarrierConf ( double rxTemperature_dBK, ErrorModel errorModel, InterferenceModel ifModel,
                                           RxMode rxMode, SatEnums::ChannelType_t chType,
                                           CarrierBandwidthConverter converter, uint32_t carrierCount )
 : m_ifModel (ifModel),
   m_errorModel (errorModel),
   m_linkResults (),
   m_rxTemperature_K (SatUtils::DbToLinear (rxTemperature_dBK)),
   m_rxExtNoiseDensity_dbWHz (0),
   m_rxOtherSysInterference_db (0),
   m_rxImInterference_db (0),
   m_rxAciInterference_db (0),
   m_rxAciIfWrtNoise (0),
   m_rxMode (rxMode),
   m_enableIntfOutputTrace (false),
   m_carrierCount (carrierCount),
   m_carrierBandwidthConverter (converter),
   m_channelType (chType)
{

}

TypeId
SatPhyRxCarrierConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierConf")
    .SetParent<Object> ()
    .AddAttribute( "RxOtherSysIfDb",
                   "Other system interference.",
                    DoubleValue (0.0),
                    MakeDoubleAccessor (&SatPhyRxCarrierConf::m_rxOtherSysInterference_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "RxImIfDb",
                   "Intermodulation interference.",
                    DoubleValue (0.0),
                    MakeDoubleAccessor (&SatPhyRxCarrierConf::m_rxImInterference_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "RxAciIfDb",
                   "Adjacent channel interference.",
                    DoubleValue (0.0),
                    MakeDoubleAccessor (&SatPhyRxCarrierConf::m_rxAciInterference_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "RxAciIfWrtNoise",
                   "Adjacent channel interference wrt noise in percents.",
                    DoubleValue (0.0),
                    MakeDoubleAccessor (&SatPhyRxCarrierConf::m_rxAciIfWrtNoise),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "ExtNoiseDensityDbWHz",
                   "External noise power density.",
                    DoubleValue (SatUtils::MinDb<double> ()),
                    MakeDoubleAccessor (&SatPhyRxCarrierConf::m_rxExtNoiseDensity_dbWHz),
                    MakeDoubleChecker<double> ())
    .AddAttribute( "EnableIntfOutputTrace",
                   "Enable interference output trace.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatPhyRxCarrierConf::m_enableIntfOutputTrace),
                    MakeBooleanChecker ())
    .AddConstructor<SatPhyRxCarrierConf> ()
  ;
  return tid;
}


void
SatPhyRxCarrierConf::SetLinkResults (Ptr<SatLinkResults> linkResults)
{
  m_linkResults = linkResults;
}

uint32_t SatPhyRxCarrierConf::GetCarrierCount () const
{
  return m_carrierCount;
}


SatPhyRxCarrierConf::ErrorModel SatPhyRxCarrierConf::GetErrorModel () const
{
  return m_errorModel;
}

SatPhyRxCarrierConf::InterferenceModel SatPhyRxCarrierConf::GetInterferenceModel () const
{
  return m_ifModel;
}

Ptr<SatLinkResults> SatPhyRxCarrierConf::GetLinkResults () const
{
  return m_linkResults;
}

double SatPhyRxCarrierConf::GetCarrierBandwidth_Hz ( uint32_t carrierId ) const
{
  return m_carrierBandwidthConverter( m_channelType, carrierId );
}

double SatPhyRxCarrierConf::GetRxTemperature_K () const
{
  return m_rxTemperature_K;
}

double SatPhyRxCarrierConf::GetExtPowerDensity_dbWHz () const
{
  return m_rxExtNoiseDensity_dbWHz;
}

double SatPhyRxCarrierConf::GetRxOtherSystemInterference_dB () const
{
  return m_rxOtherSysInterference_db;
}

double SatPhyRxCarrierConf::GetRxImInterference_dB () const
{
  return m_rxImInterference_db;
}

double SatPhyRxCarrierConf::GetRxAciInterference_dB () const
{
  return m_rxAciInterference_db;
}

double SatPhyRxCarrierConf::GetRxAciInterferenceWrtNoise () const
{
  return m_rxAciIfWrtNoise;
}

SatPhyRxCarrierConf::RxMode SatPhyRxCarrierConf::GetRxMode () const
{
  return m_rxMode;
}

SatEnums::ChannelType_t SatPhyRxCarrierConf::GetChannelType () const
{
  return m_channelType;
}

bool SatPhyRxCarrierConf::IsIntfOutputTraceEnabled () const
{
  return m_enableIntfOutputTrace;
}

} // namespace ns3
