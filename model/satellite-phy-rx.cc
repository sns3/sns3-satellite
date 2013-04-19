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

#include "ns3/satellite-net-device.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-signal-parameters.h"
#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/antenna-model.h>


NS_LOG_COMPONENT_DEFINE ("SatPhyRx");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatPhyRx);

SatPhyRx::SatPhyRx ()
  : m_state (IDLE)
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

std::ostream& operator<< (std::ostream& os, SatPhyRx::State s)
{
  switch (s)
    {
    case SatPhyRx::IDLE:
      os << "IDLE";
      break;
    case SatPhyRx::RX:
      os << "RX";
      break;
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

TypeId
SatPhyRx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRx")
    .SetParent<Object> ()
  ;
  return tid;
}


Ptr<SatNetDevice>
SatPhyRx::GetDevice ()
{
  NS_LOG_FUNCTION (this);
  return m_device;
}

void
SatPhyRx::SetDevice (Ptr<SatNetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_device = d;
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
SatPhyRx::ChangeState (State newState)
{
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}


void
SatPhyRx::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);
  NS_LOG_LOGIC (this << " state: " << m_state);

  switch (m_state)
    {
      case IDLE:
      case RX:
        {
          // Check whether the packet is sent to our beam. In addition we should check
          // that whether the packet was intended for this specific receiver.
          if (rxParams->m_beamId == m_beamId)
            {
              NS_ASSERT (m_state == IDLE);
              // first transmission, i.e., we're IDLE and we
              // start RX
              m_firstRxStart = Simulator::Now ();
              m_firstRxDuration = rxParams->m_duration;

              m_packet = rxParams->m_packet->Copy();

              NS_LOG_LOGIC (this << " scheduling EndRx with delay " << rxParams->m_duration.GetSeconds () << "s");
              Simulator::Schedule (rxParams->m_duration, &SatPhyRx::EndRxData, this);

              ChangeState (RX);
            }
        }
        break;
        
        default:
          NS_FATAL_ERROR ("unknown state");
          break;
      }

}


void
SatPhyRx::EndRxData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == RX);
  ChangeState (IDLE);
  
  // Send packet upwards
  m_device->Receive ( m_packet );
}


void 
SatPhyRx::SetBeamId (uint16_t beamId)
{
  m_beamId = beamId;
}


} // namespace ns3
