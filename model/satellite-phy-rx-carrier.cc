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
   m_beamId (),
   m_carrierId (carrierId),
   m_satInterference (),
   m_channelType (carrierConf->GetChannelType ())
{
  NS_LOG_FUNCTION (this << carrierId);

  // Create proper interference object for carrier i
  switch (carrierConf->GetInterferenceModel ())
  {
    case SatPhyRxCarrierConf::IF_CONSTANT:
      NS_LOG_LOGIC (this << " Constant interference model created for carrier: " << carrierId);
      m_satInterference = CreateObject<SatConstantInterference> ();
      break;

    case SatPhyRxCarrierConf::IF_PER_PACKET:
      NS_LOG_LOGIC (this << " Per packet interference model created for carrier: " << carrierId);

      if (carrierConf->IsIntfOutputTraceEnabled ())
        {
          m_satInterference = CreateObject<SatPerPacketInterference> (m_channelType, carrierConf->GetCarrierBandwidthHz (carrierId));
        }
      else
        {
          m_satInterference = CreateObject<SatPerPacketInterference> ();
        }
      break;

    case SatPhyRxCarrierConf::IF_TRACE:
      NS_LOG_LOGIC (this << " Traced interference model created for carrier: " << carrierId);
      m_satInterference = CreateObject<SatTracedInterference> (m_channelType, carrierConf->GetCarrierBandwidthHz (carrierId));
      break;

    default:
      NS_LOG_ERROR (this << " Not a valid interference model!");
      break;
  }

  m_rxMode = carrierConf->GetRxMode ();
  m_rxBandwidthHz = carrierConf->GetCarrierBandwidthHz (carrierId);

  m_rxExtNoisePowerW = SatUtils::DbWToW(carrierConf->GetExtPowerDensityDbwhz ()) * m_rxBandwidthHz;

  if (carrierConf->GetErrorModel () == SatPhyRxCarrierConf::EM_AVI)
    {
      NS_LOG_LOGIC (this << " link results in use in carrier: " << carrierId);
      m_linkResults = carrierConf->GetLinkResults ();
    }

  m_rxTemperatureK = carrierConf->GetRxTemperatureK ();

  // calculate RX noise
  m_rxNoisePowerW = BoltzmannConstant * m_rxTemperatureK * m_rxBandwidthHz;

  // calculate RX ACI power with percent wrt noise
  m_rxAciIfPowerW = m_rxNoisePowerW * carrierConf->GetRxAciInterferenceWrtNoise () / 100;

  m_sinrCalculate = carrierConf->GetSinrCalculatorCb ();

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
                     "The trace for calculated interferences of the received packets",
                      MakeTraceSourceAccessor (&SatPhyRxCarrier::m_packetTrace))
  ;
  return tid;
}

void
SatPhyRxCarrier::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_rxCallback.Nullify();
  m_cnoCallback.Nullify();
  m_sinrCalculate.Nullify();
  m_satInterference = NULL;
  m_interferenceEvent = NULL;

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
          SatMacTag tag;
          rxParams->m_packet->PeekPacketTag (tag);

          m_destAddress = Mac48Address::ConvertFrom (tag.GetDestAddress ());
          m_sourceAddress = Mac48Address::ConvertFrom (tag.GetSourceAddress ());

          // add interference in any case
          switch (m_channelType)
          {
            case SatEnums::FORWARD_FEEDER_CH :
            case SatEnums::RETURN_USER_CH :
              {
                m_interferenceEvent = m_satInterference->Add (rxParams->m_duration, rxParams->m_rxPower_W, m_sourceAddress);
                break;
              }
            case SatEnums::FORWARD_USER_CH :
            case SatEnums::RETURN_FEEDER_CH :
              {
                m_interferenceEvent = m_satInterference->Add (rxParams->m_duration, rxParams->m_rxPower_W, m_ownAddress);
                break;
              }
            default :
              {
                NS_FATAL_ERROR ("SatPhyRxCarrier::StartRx - Invalid channel type");
                break;
              }
          }

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
          NS_FATAL_ERROR ("SatPhyRxCarrier::StartRx - Unknown state");
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
      double cno = cSinr * m_rxBandwidthHz;
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
SatPhyRxCarrier::CalculateSinr (double rxPowerW, double ifPowerW)
{
  NS_LOG_FUNCTION (this << rxPowerW <<  ifPowerW);

  if (  m_rxNoisePowerW <= 0 )
    {
      NS_FATAL_ERROR ("Noise power must be greater than zero!!!");
    }

  // Calculate first SINR based on co-channel interference, Adjacent channel interference, noise and external noise
  // NOTE! ACI noise power and Ext noise power are set 0 by default and given as attributes by PHY object when used.
  double sinr = rxPowerW / (ifPowerW +  m_rxNoisePowerW + m_rxAciIfPowerW + m_rxExtNoisePowerW);

  // Call PHY calculator to composite C over I interference configured to PHY.
  double finalSinr = m_sinrCalculate (sinr);

  return (finalSinr);
}

double
SatPhyRxCarrier::CalculateCompositeSinr (double sinr1, double sinr2)
{
  NS_LOG_FUNCTION (this << sinr1 << sinr2 );

  if (  sinr1 <= 0 )
    {
      NS_FATAL_ERROR ("SINR 1 must be greater than zero!!!");
    }

  if (  sinr2 <= 0 )
    {
      NS_FATAL_ERROR ("SINR 2 must be greater than zero!!!");
    }

  return 1 / ( (1 / sinr1) + (1 / sinr2) );
}

}
