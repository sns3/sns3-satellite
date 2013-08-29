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
#include "ns3/object.h"
#include "ns3/simulator.h"

#include "satellite-phy-rx-carrier.h"
#include "satellite-signal-parameters.h"
#include "satellite-interference.h"
#include "satellite-constant-interference.h"
#include "satellite-per-packet-interference.h"
#include "satellite-traced-interference.h"
#include "satellite-mac.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrier");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrier);


SatPhyRxCarrier::SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf)
  :m_state (IDLE),
   m_carrierId (carrierId)
{
  NS_LOG_FUNCTION (this << carrierId);

  // Create proper interference object for carrier i
  switch ( carrierConf->GetInterferenceModel () )
  {
    case SatPhyRxCarrierConf::IF_CONSTANT:
      NS_LOG_LOGIC(this << " Constant interference model created for carrier: " << carrierId);
      m_satInterference = CreateObject<SatConstantInterference> ();
      break;

    case SatPhyRxCarrierConf::IF_PER_PACKET:
      NS_LOG_LOGIC(this << " Per packet interference model created for carrier: " << carrierId);
      m_satInterference = CreateObject<SatPerPacketInterference> ();
      break;

    case SatPhyRxCarrierConf::IF_TRACE:
      NS_LOG_LOGIC(this << " Traced interference model created for carrier: " << carrierId);
      m_satInterference = CreateObject<SatTracedInterference> ();
      break;

    default:
      NS_LOG_ERROR (this << " Not a valid interference model!");
      break;
  }

  m_rxMode = carrierConf->GetRxMode();
  m_rxOtherSysNoise_W = carrierConf->GetRxOtherSystemNoise_W();

  if (carrierConf->GetErrorModel() == SatPhyRxCarrierConf::EM_AVI)
    {
      NS_LOG_LOGIC(this << " link results in use in carrier: " << carrierId);
      m_linkResults = carrierConf->GetLinkResults ();
    }

  m_rxBandwidth_Hz = carrierConf->GetBandwidth_Hz();
  m_rxTemperature_K = carrierConf->GetRxTemperature_K();

  // calculate RX noise
  m_rxNoise_W = BoltzmannConstant * m_rxTemperature_K * m_rxBandwidth_Hz;
}


SatPhyRxCarrier::~SatPhyRxCarrier ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatPhyRxCarrier::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrier")
    .SetParent<Object> ()
  ;
  return tid;
}

void
SatPhyRxCarrier::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

std::ostream& operator<< (std::ostream& os, SatPhyRxCarrier::State s)
{
  switch (s)
    {
    case SatPhyRxCarrier::IDLE:
      os << "IDLE";
      break;
    case SatPhyRxCarrier::RX:
      os << "RX";
      break;
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

void
SatPhyRxCarrier::SetCb (SatPhyRx::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_rxCallback = cb;
}

void
SatPhyRxCarrier::ChangeState (State newState)
{
  NS_LOG_FUNCTION (this << newState);
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}

void
SatPhyRxCarrier::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);
  NS_LOG_LOGIC (this << " state: " << m_state);
  NS_ASSERT (rxParams->m_carrierId == m_carrierId);

  switch (m_state)
    {
      case IDLE:
      case RX:
        {
          // Check whether the packet is sent to our beam.
          // In case that RX mode is something else than transparent
          // additionally check that whether the packet was intended for this specific receiver
          bool receivePacket = false;

          if (rxParams->m_beamId == m_beamId )
            {
              if ( m_rxMode == SatPhyRxCarrierConf::TRANSPARENT )
                {
                  receivePacket = true;
                }
              else
                {
                  MacAddressTag tag;

                  // just check tag here, upper layer (MAC) is repsonsible for removing tag
                  if (rxParams->m_packet->PeekPacketTag(tag))
                    {
                      // If the packet is intended for this receiver
                      Mac48Address addr = Mac48Address::ConvertFrom (tag.GetAddress());

                      if ( addr == m_ownAddress ||  addr.IsBroadcast() )
                        {
                          receivePacket = true;
                        }
                    }
                }
            }

          // add interference in any case
          m_interferenceEvent = m_satInterference->Add(rxParams->m_duration, rxParams->m_rxPower_W);

          if ( receivePacket )
            {
              NS_ASSERT (m_state == IDLE);

              m_satInterference->NotifyRxStart(m_interferenceEvent);

              m_rxParams = rxParams->Copy ();

              NS_LOG_LOGIC (this << " scheduling EndRx with delay " << rxParams->m_duration.GetSeconds () << "s");
              Simulator::Schedule (rxParams->m_duration, &SatPhyRxCarrier::EndRxData, this);

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
SatPhyRxCarrier::EndRxData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == RX);
  ChangeState (IDLE);

  double iPower = 0.0;
  m_satInterference->Calculate(m_interferenceEvent, &iPower);

  double sinr = CalculateSinr(m_rxParams->m_rxPower_W, iPower);

  if ( m_rxMode == SatPhyRxCarrierConf::NORMAL )
    {
      // calculate composite SINR
      // TODO: just calculated now, needed to check against link results later
      sinr = 1 / ( (1 / sinr) + (1 / m_rxParams->m_sinr) );
    }
  else
    {
      m_rxParams->m_sinr = sinr;
    }

  m_satInterference->NotifyRxEnd(m_interferenceEvent);

  // Send packet upwards
  m_rxCallback (m_rxParams);
}

void
SatPhyRxCarrier::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  m_beamId = beamId;
}

void
SatPhyRxCarrier::SetAddress (Mac48Address ownAddress)
{
  m_ownAddress = ownAddress;
}

double
SatPhyRxCarrier::CalculateSinr(double rxPower_W, double iPower_W)
{
  return (rxPower_W / (iPower_W + m_rxNoise_W + m_rxOtherSysNoise_W));
}

}
