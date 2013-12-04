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
#include "satellite-rx-error-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrier");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrier);


SatPhyRxCarrier::SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf)
  :m_state (IDLE),
   m_beamId (),
   m_carrierId (carrierId),
   m_satInterference (),
   m_channelType (carrierConf->GetChannelType ()),
   m_startRxTime (),
   m_bitsToContainByte (8)
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

  m_errorModel = carrierConf->GetErrorModel ();

  if ( m_errorModel == SatPhyRxCarrierConf::EM_AVI)
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

  m_constantErrorRate = carrierConf->GetConstantErrorRate ();
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

  m_startRxTime = Now ();

  switch (m_state)
    {
      case IDLE:
      case RX:
        {
          bool receivePacket = false;
          bool ownAddressFound = false;

          for ( SatSignalParameters::TransmitBuffer_t::const_iterator i = rxParams->m_packetBuffer.begin ();
                ((i != rxParams->m_packetBuffer.end ()) && (ownAddressFound == false) ); i++)
            {
              SatMacTag tag;
              (*i)->PeekPacketTag (tag);

              m_destAddress = Mac48Address::ConvertFrom (tag.GetDestAddress ());
              m_sourceAddress = Mac48Address::ConvertFrom (tag.GetSourceAddress ());

              if (( m_destAddress == m_ownAddress ))
                {
                  receivePacket = true;
                  ownAddressFound = true;
                }
              else if ( m_destAddress.IsBroadcast () )
                {
                  receivePacket = true;
                }
            }

          if ( m_rxMode == SatPhyRxCarrierConf::TRANSPARENT )
            {
              receivePacket = true;
            }

          // add interference in any case
          switch (m_channelType)
          {
            case SatEnums::FORWARD_FEEDER_CH:
            case SatEnums::RETURN_USER_CH:
              {
                m_interferenceEvent = m_satInterference->Add (rxParams->m_duration, rxParams->m_rxPower_W, m_sourceAddress);
                break;
              }
            case SatEnums::FORWARD_USER_CH:
            case SatEnums::RETURN_FEEDER_CH:
              {
                m_interferenceEvent = m_satInterference->Add (rxParams->m_duration, rxParams->m_rxPower_W, m_ownAddress);
                break;
              }
            case SatEnums::UNKNOWN_CH:
            default :
              {
                NS_FATAL_ERROR ("SatPhyRxCarrier::StartRx - Invalid channel type");
                break;
              }
          }

          // Check whether the packet is sent to our beam.
          // In case that RX mode is something else than transparent
          // additionally check that whether the packet was intended for this specific receiver

          if ( receivePacket && ( rxParams->m_beamId == m_beamId ) )
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
  /**
   * For code comments:
   * 1st link is the link to satellite
   * 2nd link is the link from satellite to ground
   */

  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == RX);
  ChangeState (IDLE);

  double ifPower = m_satInterference->Calculate (m_interferenceEvent);

  /// in 1st link: calculates sinr for 1st link
  /// in 2nd link: calculates sinr for 2nd link
  double sinr = CalculateSinr ( m_rxParams->m_rxPower_W, ifPower );

  /// in 1st link: initializes composite sinr with 1st link sinr
  /// in 2nd link: initializes composite sinr with 2st link sinr. This value will be replaced in the following block
  double cSinr = sinr;

  NS_ASSERT( ( m_rxMode == SatPhyRxCarrierConf::TRANSPARENT && m_rxParams->m_sinr == 0  ) ||
             ( m_rxMode == SatPhyRxCarrierConf::NORMAL && m_rxParams->m_sinr != 0  ) );

  /// in 1st link: does not enter this block
  /// in 2nd link: calculates composite sinr
  if ( m_rxMode == SatPhyRxCarrierConf::NORMAL )
    {
      /// calculate composite SINR
      cSinr = CalculateCompositeSinr (sinr, m_rxParams->m_sinr);

      /// check against link results
      if (CheckAgainstLinkResults (cSinr))
        {
          for ( SatSignalParameters::TransmitBuffer_t::const_iterator i = m_rxParams->m_packetBuffer.begin ();
                i != m_rxParams->m_packetBuffer.end (); i++)
            {
              SatRxErrorTag tag;
              tag.SetError (true);
              (*i)->AddPacketTag (tag);
            }
        }
    }

  /// in 1st link: save 1st link sinr value for 2nd link composite sinr calculations
  /// in 2nd link: save 2nd link sinr value
  m_rxParams->m_sinr = sinr;

  /// in 1st link: uses 1st link sinr
  /// in 2nd link: uses composite sinr
  m_packetTrace (m_rxParams, m_ownAddress, m_destAddress, ifPower, cSinr);

  m_satInterference->NotifyRxEnd (m_interferenceEvent);

  /// Send packet upwards
  m_rxCallback ( m_rxParams );

  /// in 1st link: uses 1st link sinr
  /// in 2nd link: uses composite sinr
  if (!m_cnoCallback.IsNull ())
    {
      double cno = cSinr * m_rxBandwidthHz;
      m_cnoCallback (m_rxParams->m_beamId, m_sourceAddress, cno);
    }
}

bool
SatPhyRxCarrier::CheckAgainstLinkResults (double cSinr)
{
  /// Init with no error
  bool error = false;

  switch (m_errorModel)
    {
    case SatPhyRxCarrierConf::EM_AVI:
      {
        switch (m_channelType)
          {
          case SatEnums::FORWARD_FEEDER_CH:
          case SatEnums::FORWARD_USER_CH:
            {
              /// TODO check this! (cSinr -> esN0)
              double ber = (m_linkResults->GetObject <SatLinkResultsDvbS2> ())->GetBler (m_rxParams->m_modCod,SatUtils::LinearToDb (cSinr));

              /// TODO make proper version without rand
              double r = ((double) rand () / (RAND_MAX));

              if ( r < ber )
                {
                  error = true;
                }

              NS_LOG_INFO ("FORWARD cSinr (dB): " << SatUtils::LinearToDb (cSinr)
                        << " Rx bandwidth (Hz): " << m_rxBandwidthHz
                        << " esNo (dB): " << SatUtils::LinearToDb (cSinr)
                        << " rand: " << r
                        << " ber: " << ber
                        << " error: " << error);
              break;
            }
          case SatEnums::RETURN_FEEDER_CH:
          case SatEnums::RETURN_USER_CH:
            {
              uint32_t bytes = 0;

              for ( SatSignalParameters::TransmitBuffer_t::const_iterator i = m_rxParams->m_packetBuffer.begin ();
                    i != m_rxParams->m_packetBuffer.end (); i++)
                {
                  bytes += (*i)->GetSize ();
                }

              double duration = Now ().GetSeconds () - m_startRxTime.GetSeconds ();
              double bitrate = (bytes * m_bitsToContainByte) / duration;

              /// TODO check this!
              double ebNo = cSinr * (m_rxBandwidthHz / bitrate);
              double ber = (m_linkResults->GetObject <SatLinkResultsDvbRcs2> ())->GetBler (m_rxParams->m_waveformId,SatUtils::LinearToDb (ebNo));

              /// TODO make proper version without rand
              double r = ((double) rand () / (RAND_MAX));

              if ( r < ber )
                {
                  error = true;
                }

              NS_LOG_INFO ("RETURN cSinr (dB): " << SatUtils::LinearToDb (cSinr)
                        << " Rx bandwidth (Hz): " << m_rxBandwidthHz
                        << " bytes: " << bytes
                        << " duration (s): " << duration
                        << " bitrate (bps): " << bitrate
                        << " ebNo (dB): " << SatUtils::LinearToDb (ebNo)
                        << " rand: " << r
                        << " ber: " << ber
                        << " error: " << error);
              break;
            }
          case SatEnums::UNKNOWN_CH:
          default :
            {
              NS_FATAL_ERROR ("SatPhyRxCarrier::CheckAgainstLinkResults - Invalid channel type");
              break;
            }
          }
        break;
      }
    case SatPhyRxCarrierConf::EM_CONSTANT:
      {
        /// TODO make proper version without rand
        double r = ((double) rand () / (RAND_MAX));

        if (r <  m_constantErrorRate)
          {
            error = true;
          }
        break;
      }
    case SatPhyRxCarrierConf::EM_NONE:
      {
        /// No errors i.e. error = false;
        break;
      }
    default:
      {
        NS_FATAL_ERROR("SatPhyRxCarrier::EndRxData - Error model not defined");
        break;
      }
    }
  return error;
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
