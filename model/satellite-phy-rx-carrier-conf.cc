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
#include "satellite-channel-estimation-error-container.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierConf);

SatPhyRxCarrierConf::SatPhyRxCarrierConf ()
  : m_daIfModel (),
    m_raIfModel (),
    m_errorModel (),
    m_daConstantErrorRate (0.0),
    m_rxTemperatureK (),
    m_rxAciIfWrtNoiseFactor (),
    m_rxMode (),
    m_carrierCount (),
    m_carrierBandwidthConverter (),
    m_channelType (),
    m_channelEstimationError (),
    m_sinrCalculate (),
    m_linkResults (),
    m_rxExtNoiseDensityWhz (0),
    m_enableIntfOutputTrace (false),
    m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize (10),
    m_raCollisionModel (RA_COLLISION_NOT_DEFINED),
    m_raConstantErrorRate (0.0),
    m_enableRandomAccessDynamicLoadControl (true),
		m_randomAccessModel ()
{
  NS_FATAL_ERROR ("SatPhyRxCarrierConf::SatPhyRxCarrierConf - Constructor not in use");
}

SatPhyRxCarrierConf::SatPhyRxCarrierConf (RxCarrierCreateParams_s createParams)
  : m_daIfModel (createParams.m_daIfModel),
    m_raIfModel (createParams.m_raIfModel),
    m_errorModel (createParams.m_errorModel),
    m_daConstantErrorRate (createParams.m_daConstantErrorRate),
    m_rxTemperatureK (createParams.m_rxTemperatureK),
    m_rxAciIfWrtNoiseFactor (createParams.m_aciIfWrtNoiseFactor),
    m_rxMode (createParams.m_rxMode),
    m_carrierCount (createParams.m_carrierCount),
    m_carrierBandwidthConverter (createParams.m_bwConverter),
    m_channelType (createParams.m_chType),
    m_channelEstimationError (createParams.m_cec),
    m_sinrCalculate (),
    m_linkResults (),
    m_rxExtNoiseDensityWhz (createParams.m_extNoiseDensityWhz),
    m_enableIntfOutputTrace (false),
    m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize (10),
    m_raCollisionModel (createParams.m_raCollisionModel),
    m_raConstantErrorRate (createParams.m_raConstantErrorRate),
    m_enableRandomAccessDynamicLoadControl (true),
		m_randomAccessModel (createParams.m_randomAccessModel)
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatPhyRxCarrierConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierConf")
    .SetParent<Object> ()
    .AddAttribute ("EnableIntfOutputTrace",
                   "Enable interference output trace.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatPhyRxCarrierConf::m_enableIntfOutputTrace),
                   MakeBooleanChecker ())
    .AddAttribute ("RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize",
                   "Random access average normalized offered load measurement window size",
                   UintegerValue (10),
                   MakeUintegerAccessor (&SatPhyRxCarrierConf::m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EnableRandomAccessDynamicLoadControl",
                   "Enable random access dynamic load control.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatPhyRxCarrierConf::m_enableRandomAccessDynamicLoadControl),
                   MakeBooleanChecker ())
    .AddConstructor<SatPhyRxCarrierConf> ()
    ;
  return tid;
}

void
SatPhyRxCarrierConf::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_linkResults = NULL;
  m_carrierBandwidthConverter.Nullify ();
  m_sinrCalculate.Nullify ();

  Object::DoDispose ();
}

void
SatPhyRxCarrierConf::SetLinkResults (Ptr<SatLinkResults> linkResults)
{
  m_linkResults = linkResults;
}

uint32_t
SatPhyRxCarrierConf::GetCarrierCount () const
{
  return m_carrierCount;
}


SatPhyRxCarrierConf::ErrorModel
SatPhyRxCarrierConf::GetErrorModel () const
{
  return m_errorModel;
}

double
SatPhyRxCarrierConf::GetConstantDaErrorRate () const
{
  return m_daConstantErrorRate;
}

SatPhyRxCarrierConf::InterferenceModel
SatPhyRxCarrierConf::GetInterferenceModel (bool isRandomAccessCarrier) const
{
  if (isRandomAccessCarrier)
    {
      return m_raIfModel;
    }
  else
    {
      return m_daIfModel;
    }
}

Ptr<SatLinkResults>
SatPhyRxCarrierConf::GetLinkResults () const
{
  return m_linkResults;
}

double
SatPhyRxCarrierConf::GetCarrierBandwidthHz ( uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType ) const
{
  return m_carrierBandwidthConverter ( m_channelType, carrierId, bandwidthType );
}

double
SatPhyRxCarrierConf::GetRxTemperatureK () const
{
  return m_rxTemperatureK;
}

double
SatPhyRxCarrierConf::GetExtPowerDensityWhz () const
{
  return m_rxExtNoiseDensityWhz;
}

double
SatPhyRxCarrierConf::GetRxAciInterferenceWrtNoiseFactor () const
{
  return m_rxAciIfWrtNoiseFactor;
}

SatPhyRxCarrierConf::RxMode
SatPhyRxCarrierConf::GetRxMode () const
{
  return m_rxMode;
}

SatEnums::ChannelType_t
SatPhyRxCarrierConf::GetChannelType () const
{
  return m_channelType;
}

bool
SatPhyRxCarrierConf::IsIntfOutputTraceEnabled () const
{
  return m_enableIntfOutputTrace;
}

Ptr<SatChannelEstimationErrorContainer>
SatPhyRxCarrierConf::GetChannelEstimatorErrorContainer () const
{
  return m_channelEstimationError;
}

SatPhyRxCarrierConf::RandomAccessCollisionModel
SatPhyRxCarrierConf::GetRandomAccessCollisionModel () const
{
  if (m_raIfModel == IF_PER_PACKET)
    {
      return m_raCollisionModel;
    }
  else
    {
      return RA_COLLISION_ALWAYS_DROP_ALL_COLLIDING_PACKETS;
    }
}

double
SatPhyRxCarrierConf::GetRandomAccessConstantErrorRate () const
{
  return m_raConstantErrorRate;
}


uint32_t
SatPhyRxCarrierConf::GetRandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize () const
{
  return m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize;
}

bool
SatPhyRxCarrierConf::IsRandomAccessDynamicLoadControlEnabled () const
{
  return m_enableRandomAccessDynamicLoadControl;
}

} // namespace ns3
