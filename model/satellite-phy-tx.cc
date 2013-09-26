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

#include <cmath>

#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/object-factory.h"
#include "ns3/log.h"

#include "satellite-utils.h"
#include "satellite-phy.h"
#include "satellite-phy-tx.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel.h"
#include "satellite-antenna-gain-pattern.h"


NS_LOG_COMPONENT_DEFINE ("SatPhyTx");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatPhyTx);


SatPhyTx::SatPhyTx ()
  :m_state (IDLE)
{
  NS_LOG_FUNCTION (this);
}


SatPhyTx::~SatPhyTx ()
{
  NS_LOG_FUNCTION (this);
}

void SatPhyTx::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_mobility = 0;
  Object::DoDispose ();
} 

std::ostream& operator<< (std::ostream& os, SatPhyTx::State s)
{
  switch (s)
    {
    case SatPhyTx::IDLE:
      os << "IDLE";
      break;
    case SatPhyTx::TX:
      os << "TX";
      break;
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

TypeId
SatPhyTx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyTx")
    .SetParent<Object> ()
  ;
  return tid;
}

void
SatPhyTx::SetMaxAntennaGain_Db(double gain_db)
{
  NS_LOG_FUNCTION (this);
  m_maxAntennaGain = SatUtils::DbToLinear (gain_db);
}

double
SatPhyTx::GetAntennaGain_W (Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this);

  double gain_W (m_maxAntennaGain);

  // Get the transmit antenna gain at the receiver position.
  // E.g. GEO satellite transmits to the UT receiver.
  if (m_antennaGainPattern)
    {
      Ptr<SatMobilityModel> m = DynamicCast<SatMobilityModel> (mobility);
      gain_W = m_antennaGainPattern->GetAntennaGain_lin (m->GetGeoPosition ());
    }

  return gain_W;
}

Ptr<MobilityModel>
SatPhyTx::GetMobility ()
{
  NS_LOG_FUNCTION (this);
  return m_mobility;
}

void
SatPhyTx::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}

void
SatPhyTx::SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  m_antennaGainPattern = agp;
}

void
SatPhyTx::SetChannel (Ptr<SatChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}

Ptr<SatChannel>
SatPhyTx::GetChannel ()
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

void
SatPhyTx::ChangeState (State newState)
{
  NS_LOG_FUNCTION (this << newState);
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}

void
SatPhyTx::StartTx (Ptr<Packet> p, Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this << p << txParams);
  NS_LOG_LOGIC (this << " state: " << m_state);

  switch (m_state)
  {
    case TX:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      NS_ASSERT (m_channel);
      ChangeState (TX);
      m_channel->StartTx (txParams);
      Simulator::Schedule (txParams->m_duration, &SatPhyTx::EndTx, this);
    }
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      break;
  }
}

void
SatPhyTx::EndTx ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);
  NS_ASSERT (m_state == TX);

  ChangeState (IDLE);
}

void 
SatPhyTx::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  m_beamId = beamId;
}

} // namespace ns3
