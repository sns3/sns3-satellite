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
#include "ns3/antenna-model.h"
#include "ns3/object-factory.h"

#include "satellite-net-device.h"
#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-rx-carrier.h"
#include "satellite-channel.h"
#include "satellite-signal-parameters.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRx");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatPhyRx);

SatPhyRx::SatPhyRx ()
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
  Object::DoDispose ();
} 

TypeId
SatPhyRx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRx")
    .SetParent<Object> ()
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
SatPhyRx::SetPhy (Ptr<SatPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin();
      it != m_rxCarriers.end();
      ++it)
    {
      (*it)->SetPhy (phy);
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
SatPhyRx::SetChannel (Ptr<SatChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  c->AddRx (this);
}


void
SatPhyRx::ConfigurePhyRxCarriers (uint32_t maxRxCarriers)
{
    NS_LOG_FUNCTION (this << maxRxCarriers);
    NS_ASSERT (maxRxCarriers > 0);
    NS_ASSERT (m_rxCarriers.empty());

    for ( uint32_t i = 0; i < maxRxCarriers; ++i )
      {
        NS_LOG_LOGIC(this << " Create carrier: " << i);
        Ptr<SatPhyRxCarrier> rxc = CreateObject<SatPhyRxCarrier> (i);
        m_rxCarriers.push_back (rxc);
      }
}

void
SatPhyRx::SetBeamId (uint32_t beamId)
{
    NS_LOG_FUNCTION (this << beamId);
    NS_ASSERT (beamId >= 0);
    NS_ASSERT (!m_rxCarriers.empty());

    for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin(); it != m_rxCarriers.end(); ++it)
      {
        (*it)->SetBeamId (beamId);
      }
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
