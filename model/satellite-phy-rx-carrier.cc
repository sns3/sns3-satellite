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

#include "satellite-utils.h"
#include "satellite-phy-rx-carrier.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel.h"
#include "satellite-interference.h"
#include "satellite-constant-interference.h"
#include "satellite-per-packet-interference.h"
#include "satellite-traced-interference.h"
#include "satellite-mac-tag.h"
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

  m_rxMode = carrierConf->GetRxMode ();
  m_rxBandwidth_Hz = carrierConf->GetCarrierBandwidth_Hz (carrierId);

  m_rxOtherSysNoise_W = SatUtils::DbWToW(carrierConf->GetExtPowerDensity_dbWHz ()) * m_rxBandwidth_Hz;

  if (carrierConf->GetErrorModel () == SatPhyRxCarrierConf::EM_AVI)
    {
      NS_LOG_LOGIC (this << " link results in use in carrier: " << carrierId);
      m_linkResults = carrierConf->GetLinkResults ();
    }

  m_rxTemperature_K = carrierConf->GetRxTemperature_K ();

  // calculate RX noise
  m_rxNoise_W = BoltzmannConstant * m_rxTemperature_K * m_rxBandwidth_Hz;

  // calculate RX Aci wrt noise
  m_rxAciIf_W = m_rxNoise_W * carrierConf->GetRxAciInterferenceWrtNoise () / 100;

  m_rxOtherSysInterference = SatUtils::DbToLinear (carrierConf->GetRxOtherSystemInterference_dB ());
  m_rxImInterference = SatUtils::DbToLinear (carrierConf->GetRxImInterference_dB ());
  m_rxAciInterference = SatUtils::DbToLinear (carrierConf->GetRxAciInterference_dB ());
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
    .AddTraceSource ("PacketTrace",
                     "The trace for calculated interferencies of the received packets",
                      MakeTraceSourceAccessor (&SatPhyRxCarrier::m_packetTrace))
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
SatPhyRxCarrier::SetReceiveCb (SatPhyRx::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_rxCallback = cb;
}

void
SatPhyRxCarrier::SetCnoCb (SatPhyRx::CnoCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_cnoCallback = cb;
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
          // add interference in any case
          m_interferenceEvent = m_satInterference->Add(rxParams->m_duration, rxParams->m_rxPower_W);

          SatMacTag tag;
          rxParams->m_packet->PeekPacketTag (tag);

          m_destAddress = Mac48Address::ConvertFrom (tag.GetDestAddress ());
          m_sourceAddress = Mac48Address::ConvertFrom (tag.GetSourceAddress ());

          // Check whether the packet is sent to our beam.
          // In case that RX mode is something else than transparent
          // additionally check that whether the packet was intended for this specific receiver

          if ( ( rxParams->m_beamId == m_beamId ) &&
               ( ( m_rxMode == SatPhyRxCarrierConf::TRANSPARENT ) ||
                 ( m_destAddress == m_ownAddress ) ||
                 ( m_destAddress.IsBroadcast () ) ) )
            {
              NS_ASSERT (m_state == IDLE);

              m_satInterference->NotifyRxStart (m_interferenceEvent);

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

  double ifPower = m_satInterference->Calculate ( m_interferenceEvent );

  double sinr = CalculateSinr ( m_rxParams->m_rxPower_W, ifPower );
  double cSinr = sinr;

  NS_ASSERT( ( m_rxMode == SatPhyRxCarrierConf::TRANSPARENT && m_rxParams->m_sinr == 0  ) ||
             ( m_rxMode == SatPhyRxCarrierConf::NORMAL && m_rxParams->m_sinr != 0  ) );

  if ( m_rxMode == SatPhyRxCarrierConf::NORMAL )
    {
      // calculate composite SINR
      // TODO: just calculated now, needed to check against link results later
      cSinr = CalculateCompositeSinr (sinr, m_rxParams->m_sinr);
    }

  m_rxParams->m_sinr = sinr;
  
  m_packetTrace (m_rxParams, m_ownAddress, m_destAddress, ifPower, cSinr);

  m_satInterference->NotifyRxEnd ( m_interferenceEvent );

  // Send packet upwards
  m_rxCallback ( m_rxParams );

  if ( m_cnoCallback.IsNull() == false )
    {
      double cno = cSinr * m_rxBandwidth_Hz;
      m_cnoCallback (m_rxParams->m_beamId, m_sourceAddress, cno);
    }
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
  NS_LOG_FUNCTION (this << ownAddress);

  m_ownAddress = ownAddress;
}

double
SatPhyRxCarrier::CalculateSinr (double rxPower_W, double ifPower_W)
{
  NS_LOG_FUNCTION (this << rxPower_W <<  ifPower_W);

  NS_ASSERT( m_rxNoise_W >= SatUtils::MinLin<double> () );

  // caluclate first sinr based on co-channel interference, Adjacent channel interference, noise and other sys noise
  double sinr = rxPower_W / (ifPower_W + m_rxAciIf_W + m_rxNoise_W + m_rxOtherSysNoise_W);

  double inverseSinr = 1 / sinr;

  // calculate final sinr taken into account configured interferencies (C over I)
  // interference ratio 1 means that it is not configured and is not calculated

  if ( m_rxOtherSysInterference != 1 )
    {
      inverseSinr += 1 / m_rxOtherSysInterference;
    }

  if ( m_rxImInterference != 1 )
    {
      inverseSinr += 1 / m_rxImInterference;
    }

  if ( m_rxAciInterference != 1 )
    {
      inverseSinr += 1 / m_rxAciInterference;
    }

  sinr = 1 / inverseSinr;

  return (sinr);
}

double
SatPhyRxCarrier::CalculateCompositeSinr (double sinr1, double sinr2)
{
  NS_LOG_FUNCTION (this << sinr1 << sinr2 );

  NS_ASSERT (sinr1);
  NS_ASSERT (sinr2);

  return 1 / ( (1 / sinr1) + (1 / sinr2) );
}

}
