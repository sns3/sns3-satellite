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

#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-signal-parameters.h"
#include "ns3/satellite-channel.h"
#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cmath>
#include <ns3/simulator.h>
#include <ns3/boolean.h>
#include <ns3/double.h>

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
  m_device = 0;
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


Ptr<NetDevice>
SatPhyTx::GetDevice ()
{
  NS_LOG_FUNCTION (this);
  return m_device;
}


Ptr<SatPhy>
SatPhyTx::GetPhy ()
{
  return m_phy;
}

void
SatPhyTx::SetPhy (Ptr<SatPhy> phy)
{
  m_phy = phy;
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
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}


void
SatPhyTx::StartTx (Ptr<Packet> p, Time duration)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  switch (m_state)
  {
    case TX:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {

      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the BeamId).
      NS_ASSERT (m_channel);
      ChangeState (TX);
      Ptr<SatSignalParameters> txParams = Create<SatSignalParameters> ();
      txParams->m_duration = duration;
      txParams->m_phyTx = this;
      txParams->m_packet = p;
      txParams->m_beamId = m_beamId;
      m_channel->StartTx (txParams);
      Simulator::Schedule (duration, &SatPhyTx::EndTx, this);
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
SatPhyTx::SetBeamId (uint16_t beamId)
{
  m_beamId = beamId;
}




} // namespace ns3
