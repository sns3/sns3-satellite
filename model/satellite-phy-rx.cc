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
#include "ns3/object-vector.h"
#include "ns3/antenna-model.h"
#include "ns3/object-factory.h"
#include "satellite-utils.h"
#include "satellite-net-device.h"
#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-rx-carrier.h"
#include "satellite-phy-rx-carrier-conf.h"
#include "satellite-signal-parameters.h"
#include "satellite-antenna-gain-pattern.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRx");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatPhyRx);

SatPhyRx::SatPhyRx () :
  m_beamId (),
  m_maxAntennaGain (),
  m_antennaLoss (),
  m_defaultFadingValue ()
{
  NS_LOG_FUNCTION (this);
}

SatPhyRx::~SatPhyRx ()
{
  NS_LOG_FUNCTION (this);
}

void SatPhyRx::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_mobility = 0;
  m_device = 0;
  m_fadingContainer = 0;
  m_rxCarriers.clear();
  Object::DoDispose ();
} 

TypeId
SatPhyRx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRx")
    .SetParent<Object> ()
    .AddAttribute ("RxCarrierList", "The list of RX carriers associated to this Phy RX.",
                    ObjectVectorValue (),
                    MakeObjectVectorAccessor (&SatPhyRx::m_rxCarriers),
                    MakeObjectVectorChecker<SatPhyRxCarrier> ())
  ;
  return tid;
}

Ptr<NetDevice>
SatPhyRx::GetDevice ()
{
  NS_LOG_FUNCTION (this);
  return m_device;
}

void
SatPhyRx::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_device = d;
}

void
SatPhyRx::SetMaxAntennaGain_Db (double gain_Db)
{
  NS_LOG_FUNCTION (this);
  m_maxAntennaGain = SatUtils::DbWToW (gain_Db);
}

double
SatPhyRx::GetAntennaGain (Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this);

  double gain_W (m_maxAntennaGain);

  // Get the receive antenna gain at the transmitter position.
  // E.g. UT transmits to the satellite receiver.
  if (m_antennaGainPattern)
    {
      Ptr<SatMobilityModel> m = DynamicCast<SatMobilityModel> (mobility);
      gain_W = m_antennaGainPattern->GetAntennaGain_lin (m->GetGeoPosition ());
    }

  return gain_W;
}

void
SatPhyRx::SetDefaultFadingValue (double fadingValue)
{
  NS_LOG_FUNCTION (this);
  m_defaultFadingValue = fadingValue;
}

double
SatPhyRx::GetFadingValue (Address macAddress, SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this);

  double fadingValue = m_defaultFadingValue;

  if (m_fadingContainer)
    {
      fadingValue = m_fadingContainer->GetFading (macAddress, channelType);
    }
  // Returns value 1 if fading is not set, as fading value is used as multiplier
  return fadingValue;
}

void
SatPhyRx::SetFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);

  m_fadingContainer = fadingContainer;
}

void
SatPhyRx::SetAntennaLoss_Db (double gain_Db)
{
  NS_LOG_FUNCTION (this);

  m_antennaLoss = SatUtils::DbToLinear (gain_Db);
}

double
SatPhyRx::GetLosses ()
{
  NS_LOG_FUNCTION (this);

  return m_antennaLoss;
}

Mac48Address
SatPhyRx::GetAddress () const
{
  return m_macAddress;
}

void
SatPhyRx::SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo)
{
  NS_ASSERT (nodeInfo);

  m_macAddress = nodeInfo->GetMacAddress ();

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin();
        it != m_rxCarriers.end();
        ++it)
    {
      (*it)->SetNodeInfo(nodeInfo);
    }
}

void
SatPhyRx::BeginFrameEndScheduling ()
{
  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin();
        it != m_rxCarriers.end();
        ++it)
    {
      (*it)->BeginFrameEndScheduling ();
    }
}

void
SatPhyRx::SetReceiveCallback (SatPhyRx::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin();
      it != m_rxCarriers.end();
      ++it)
    {
      (*it)->SetReceiveCb(cb);
    }
}

void
SatPhyRx::SetCnoCallback (SatPhyRx::CnoCallback cb)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin();
      it != m_rxCarriers.end ();
      ++it)
    {
      (*it)->SetCnoCb (cb);
    }
}

void
SatPhyRx::SetAverageNormalizedOfferedLoadCallback (SatPhyRx::AverageNormalizedOfferedLoadCallback cb)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin();
      it != m_rxCarriers.end ();
      ++it)
    {
      (*it)->SetAverageNormalizedOfferedLoadCallback (cb);
    }
}

Ptr<MobilityModel>
SatPhyRx::GetMobility ()
{
  NS_LOG_FUNCTION (this);
  return m_mobility;
}

void
SatPhyRx::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}

void
SatPhyRx::SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  m_antennaGainPattern = agp;
}

void
SatPhyRx::ConfigurePhyRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatSuperframeConf> superFrameConf, bool isRandomAccessEnabled)
{
  NS_ASSERT (m_rxCarriers.empty());

  Ptr<SatPhyRxCarrier> rxc;

  for ( uint32_t i = 0; i < carrierConf->GetCarrierCount(); ++i )
    {
      NS_LOG_LOGIC(this << " Create carrier: " << i);

      if (isRandomAccessEnabled)
        {
          rxc = CreateObject<SatPhyRxCarrier> (i, carrierConf, superFrameConf->IsRandomAccessCarrier (i));
        }
      else
        {
          rxc = CreateObject<SatPhyRxCarrier> (i, carrierConf, false);
        }
      m_rxCarriers.push_back (rxc);
    }
}

void
SatPhyRx::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  NS_ASSERT (beamId >= 0);
  NS_ASSERT (!m_rxCarriers.empty());

  m_beamId = beamId;

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin(); it != m_rxCarriers.end(); ++it)
    {
      (*it)->SetBeamId (beamId);
    }
}

uint32_t
SatPhyRx::GetBeamId () const
{
  return m_beamId;
}

void
SatPhyRx::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);

  uint32_t cId = rxParams->m_carrierId;
  NS_ASSERT (cId < m_rxCarriers.size());

  m_rxCarriers[cId]->StartRx (rxParams);
}

} // namespace ns3
