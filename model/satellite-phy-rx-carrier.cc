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

#include <math.h>

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
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
#include "ns3/singleton.h"
#include "satellite-composite-sinr-output-trace-container.h"
#include "satellite-phy-tx.h"

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
   m_bitsToContainByte (8),
   m_enableCompositeSinrOutputTrace (false)
{
  NS_LOG_FUNCTION (this << carrierId);

  m_rxBandwidthHz = carrierConf->GetCarrierBandwidthHz (carrierId, SatEnums::EFFECTIVE_BANDWIDTH);

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
          m_satInterference = CreateObject<SatPerPacketInterference> (m_channelType, m_rxBandwidthHz);
        }
      else
        {
          m_satInterference = CreateObject<SatPerPacketInterference> ();
        }
      break;

    case SatPhyRxCarrierConf::IF_TRACE:
      NS_LOG_LOGIC (this << " Traced interference model created for carrier: " << carrierId);
      m_satInterference = CreateObject<SatTracedInterference> (m_channelType, m_rxBandwidthHz);
      break;

    default:
      NS_LOG_ERROR (this << " Not a valid interference model!");
      break;
  }

  m_rxMode = carrierConf->GetRxMode ();

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

  /**
   * Uniform random variable used for checking whether a packet
   * was received successfully or not
   */
  m_uniformVariable = CreateObject<UniformRandomVariable> ();
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
    .AddAttribute( "EnableCompositeSinrOutputTrace",
                   "Enable composite SINR output trace.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatPhyRxCarrier::m_enableCompositeSinrOutputTrace),
                    MakeBooleanChecker ())
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

  NS_LOG_LOGIC ("Node: " << m_nodeInfo->GetMacAddress () << " starts receiving packet at: " << Simulator::Now().GetSeconds () << " in carrier: " << rxParams->m_carrierId);  NS_LOG_LOGIC ("Sender: " << rxParams->m_phyTx);

  switch (m_state)
    {
      case IDLE:
      case RX:
        {
          bool receivePacket = false;
          bool ownAddressFound = false;
          Mac48Address source;
          Mac48Address dest;
          Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent;

          for ( SatSignalParameters::TransmitBuffer_t::const_iterator i = rxParams->m_packetsInBurst.begin ();
                ((i != rxParams->m_packetsInBurst.end ()) && (ownAddressFound == false) ); i++)
            {
              SatMacTag tag;
              (*i)->PeekPacketTag (tag);

              dest = Mac48Address::ConvertFrom (tag.GetDestAddress ());
              source = Mac48Address::ConvertFrom (tag.GetSourceAddress ());

              if (( dest == m_ownAddress ))
                {
                  NS_LOG_LOGIC ("Packet intended for this specific receiver: " << dest);

                  receivePacket = true;
                  ownAddressFound = true;
                }
              else if ( dest.IsBroadcast () )
                {
                  NS_LOG_LOGIC ("Destination is broadcast address: " << dest);

                  receivePacket = true;
                }
              else if ( dest.IsGroup () )
                {
                  NS_LOG_LOGIC ("Destination is multicast address: " << dest);

                  receivePacket = true;
                }
            }

          if ( m_rxMode == SatPhyRxCarrierConf::TRANSPARENT )
            {
              NS_LOG_LOGIC ("Receiver is attached to satellite node");
              receivePacket = true;
            }

          // add interference in any case
          switch (m_channelType)
          {
            case SatEnums::FORWARD_FEEDER_CH:
            case SatEnums::RETURN_USER_CH:
              {
                interferenceEvent = m_satInterference->Add (rxParams->m_duration, rxParams->m_rxPower_W, source);
                break;
              }
            case SatEnums::FORWARD_USER_CH:
            case SatEnums::RETURN_FEEDER_CH:
              {
                interferenceEvent = m_satInterference->Add (rxParams->m_duration, rxParams->m_rxPower_W, m_ownAddress);
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
              if (m_state != IDLE)
                {
                  NS_FATAL_ERROR ("Starting reception of a packet when not in IDLE state!");
                }

              // Now, we are starting to receive a packet, set the source and
              // destination addresses of packet
              m_destAddress = dest;
              m_sourceAddress = source;
              m_interferenceEvent = interferenceEvent;

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

  // PHY transmission decoded successfully. Note, that at transparent satellite,
  // all the transmissions are not decoded.
  bool phyError (false);

  /// in 1st link: does not enter this block
  /// in 2nd link: calculates composite sinr
  if ( m_rxMode == SatPhyRxCarrierConf::NORMAL )
    {
      /// calculate composite SINR
      cSinr = CalculateCompositeSinr (sinr, m_rxParams->m_sinr);

      /// composite sinr output trace
      if (m_enableCompositeSinrOutputTrace)
        {
          DoCompositeSinrOutputTrace (cSinr);
        }

      /// check against link results
      phyError = CheckAgainstLinkResults (cSinr);
    }

  /// in 1st link: save 1st link sinr value for 2nd link composite sinr calculations
  /// in 2nd link: save 2nd link sinr value
  m_rxParams->m_sinr = sinr;

  /// in 1st link: uses 1st link sinr
  /// in 2nd link: uses composite sinr
  m_packetTrace (m_rxParams, m_ownAddress, m_destAddress, ifPower, cSinr);

  m_satInterference->NotifyRxEnd (m_interferenceEvent);

  /// Send packet upwards
  m_rxCallback ( m_rxParams, phyError );

  /// in 1st link: uses 1st link sinr
  /// in 2nd link: uses composite sinr
  if (!m_cnoCallback.IsNull ())
    {
      double cno = cSinr * m_rxBandwidthHz;
      m_cnoCallback (m_rxParams->m_beamId, m_sourceAddress, m_ownAddress, cno);
    }
}

void
SatPhyRxCarrier::DoCompositeSinrOutputTrace (double cSinr)
{
  std::vector<double> tempVector;
  tempVector.push_back (Now ().GetSeconds ());
  tempVector.push_back (cSinr);

  Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (m_ownAddress, m_channelType), tempVector);
}

bool
SatPhyRxCarrier::CheckAgainstLinkResults (double cSinr)
{
  /// Initialize with no errors
  bool error = false;

  switch (m_errorModel)
    {
    case SatPhyRxCarrierConf::EM_AVI:
      {
        switch (m_channelType)
          {
          case SatEnums::FORWARD_USER_CH:
            {
              /**
               * In forward link the link results are in Es/No format, thus here we need
               * to convert the SINR into Es/No:
               * Es/No = (C*Ts)/No = C/No * (1/fs) = C/N
              */

              double ber = (m_linkResults->GetObject <SatLinkResultsDvbS2> ())->GetBler (m_rxParams->m_modCod,SatUtils::LinearToDb (cSinr));
              double r = m_uniformVariable->GetValue (0, 1);

              if ( r < ber )
                {
                  error = true;
                }

              NS_LOG_INFO ("FORWARD cSinr (dB): " << SatUtils::LinearToDb (cSinr)
                        << " esNo (dB): " << SatUtils::LinearToDb (cSinr)
                        << " rand: " << r
                        << " ber: " << ber
                        << " error: " << error);
              break;
            }
          case SatEnums::RETURN_FEEDER_CH:
            {
              /**
               * In return link the link results are in Eb/No format, thus here we need
               * to convert the SINR into Eb/No:
               * Eb/No = (Es/log2M)/No = (Es/No)*(1/log2M)  = C/N * (1/log2M) = C/No * (1/fs) * (1/log2M)
              */

              double ebNo = cSinr * (1/log2(SatUtils::GetModulatedBits (m_rxParams->m_modCod)));
              double ber = (m_linkResults->GetObject <SatLinkResultsDvbRcs2> ())->GetBler (m_rxParams->m_waveformId,SatUtils::LinearToDb (ebNo));
              double r = m_uniformVariable->GetValue (0, 1);

              if ( r < ber )
                {
                  error = true;
                }

              NS_LOG_INFO ("RETURN cSinr (dB): " << SatUtils::LinearToDb (cSinr)
                        << " ebNo (dB): " << SatUtils::LinearToDb (ebNo)
                        << " rand: " << r
                        << " ber: " << ber
                        << " error: " << error);
              break;
            }
          case SatEnums::FORWARD_FEEDER_CH:
          case SatEnums::RETURN_USER_CH:
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
        double r = m_uniformVariable->GetValue (0, 1);
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
SatPhyRxCarrier::SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);

  m_ownAddress = nodeInfo->GetMacAddress ();
  m_nodeInfo = nodeInfo;
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
