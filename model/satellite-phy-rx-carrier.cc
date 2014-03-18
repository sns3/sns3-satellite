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
#include "ns3/singleton.h"
#include "satellite-composite-sinr-output-trace-container.h"
#include "satellite-phy-tx.h"
#include "satellite-crdsa-replica-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrier");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrier);


SatPhyRxCarrier::SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf)
  :m_state (IDLE),
   m_receivingDedicatedAccess (false),
   m_beamId (),
   m_carrierId (carrierId),
   m_satInterference (),
   m_channelType (carrierConf->GetChannelType ()),
   m_enableCompositeSinrOutputTrace (false),
   m_numOfOngoingRx (0),
   m_rxPacketCounter (0),
   m_dropCollidingRandomAccessPackets (false)
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

  // Configured channel estimation error
  m_channelEstimationError = carrierConf->GetChannelEstimatorErrorContainer ();
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
  m_uniformVariable = NULL;

  for (uint32_t i = 0; i < m_crdsaPacketContainer.size (); i++)
    {
      m_crdsaPacketContainer[i].rxParams = NULL;
    }

  m_crdsaPacketContainer.clear ();

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

  uint32_t key;

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

          for (SatSignalParameters::TransmitBuffer_t::const_iterator i = rxParams->m_packetsInBurst.begin ();
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
              if (m_receivingDedicatedAccess && rxParams->m_txInfo.packetType == SatEnums::DEDICATED_ACCESS_PACKET)
                {
                  NS_FATAL_ERROR ("Starting reception of a packet when receiving DA transmission!");
                }

              // Now, we are starting to receive a packet, set the source and
              // destination addresses of packet

              rxParams_s rxParamsStruct;

              rxParamsStruct.destAddress = dest;
              rxParamsStruct.sourceAddress = source;
              rxParamsStruct.interferenceEvent = interferenceEvent;
              rxParamsStruct.rxParams = rxParams;

              m_satInterference->NotifyRxStart (rxParamsStruct.interferenceEvent);

              key = m_rxPacketCounter;
              m_rxPacketCounter++;

              m_rxParamsMap.insert (std::make_pair (key, rxParamsStruct));

              NS_LOG_LOGIC (this << " scheduling EndRx with delay " << rxParams->m_duration.GetSeconds () << "s");
              Simulator::Schedule (rxParams->m_duration, &SatPhyRxCarrier::EndRxData, this, key);

              IncreaseNumOfRxState (rxParams->m_txInfo.packetType);
            }
          break;
        }
        default:
          {
            NS_FATAL_ERROR ("SatPhyRxCarrier::StartRx - Unknown state");
            break;
          }
      }
}

void
SatPhyRxCarrier::EndRxData (uint32_t key)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  if (m_rxMode == SatPhyRxCarrierConf::NORMAL)
    {
      EndRxDataNormal (key);
    }
  else
    {
      EndRxDataTransparent (key);
    }
}

void
SatPhyRxCarrier::EndRxDataTransparent (uint32_t key)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == RX);

  std::map<uint32_t,rxParams_s>::iterator iter = m_rxParamsMap.find (key);

  if (iter == m_rxParamsMap.end ())
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::EndRxData - No matching Rx params found");
    }

  DecreaseNumOfRxState (iter->second.rxParams->m_txInfo.packetType);

  double ifPower = m_satInterference->Calculate (iter->second.interferenceEvent);

  /// calculates sinr for 1st link
  double sinr = CalculateSinr ( iter->second.rxParams->m_rxPower_W, ifPower );

  /// initializes composite sinr with 1st link sinr
  double cSinr = sinr;

  NS_ASSERT (m_rxMode == SatPhyRxCarrierConf::TRANSPARENT && iter->second.rxParams->m_sinr == 0);

  /// PHY transmission decoded successfully. Note, that at transparent satellite,
  /// all the transmissions are not decoded.
  bool phyError (false);

  /// save 1st link sinr value for 2nd link composite sinr calculations
  iter->second.rxParams->m_sinr = sinr;

  /// uses 1st link sinr
  m_packetTrace (iter->second.rxParams, m_ownAddress, iter->second.destAddress, ifPower, cSinr);

  m_satInterference->NotifyRxEnd (iter->second.interferenceEvent);

  /// Send packet upwards
  m_rxCallback ( iter->second.rxParams, phyError );

  /// uses 1st link sinr
  if (!m_cnoCallback.IsNull ())
    {
      double cno = cSinr * m_rxBandwidthHz;
      m_cnoCallback (iter->second.rxParams->m_beamId, iter->second.sourceAddress, m_ownAddress, cno);
    }

  /// erase the used Rx params
  iter->second.rxParams = NULL;
  iter->second.interferenceEvent = NULL;
  m_rxParamsMap.erase (key);
}

void
SatPhyRxCarrier::EndRxDataNormal (uint32_t key)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == RX);

  std::map<uint32_t,rxParams_s>::iterator iter = m_rxParamsMap.find (key);

  if (iter == m_rxParamsMap.end ())
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::EndRxData - No matching Rx params found");
    }

  DecreaseNumOfRxState (iter->second.rxParams->m_txInfo.packetType);

  NS_ASSERT (m_rxMode == SatPhyRxCarrierConf::NORMAL && iter->second.rxParams->m_sinr != 0);

  double ifPower = m_satInterference->Calculate (iter->second.interferenceEvent);

  if (iter->second.rxParams->m_txInfo.packetType != SatEnums::CRDSA_PACKET)
    {
      /// calculates sinr for 2nd link
      double sinr = CalculateSinr ( iter->second.rxParams->m_rxPower_W, ifPower );

      /// initializes composite sinr with 2st link sinr. This value will be replaced in the following block
      double cSinr = sinr;

      /// PHY transmission decoded successfully. Note, that at transparent satellite,
      /// all the transmissions are not decoded.
      bool phyError (false);

      /// calculate composite SINR
      cSinr = CalculateCompositeSinr (sinr, iter->second.rxParams->m_sinr);

      /**
       * Channel estimation error. Channel estimation error works in dB domain, thus we need
       * to do linear-to-db and db-to-linear conversions here.
       */
      // Forward link
      if (m_nodeInfo->GetNodeType () == SatEnums::NT_UT)
        {
          cSinr = SatUtils::LinearToDb (m_channelEstimationError->AddError (SatUtils::DbToLinear(cSinr)));
        }
      // Return link
      else if (m_nodeInfo->GetNodeType () == SatEnums::NT_GW)
        {
          cSinr = SatUtils::LinearToDb (m_channelEstimationError->AddError (SatUtils::DbToLinear(cSinr), iter->second.rxParams->m_txInfo.waveformId));
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported node type for a NORMAL Rx model!");
        }

      /// composite sinr output trace
      if (m_enableCompositeSinrOutputTrace)
        {
          DoCompositeSinrOutputTrace (cSinr);
        }

      if (iter->second.rxParams->m_txInfo.packetType == SatEnums::SLOTTED_ALOHA_PACKET)
        {
          /// check for slotted aloha packet collisions
          phyError = ProcessSlottedAlohaCollisions (cSinr, iter->second.rxParams, iter->second.interferenceEvent);
        }
      else
        {
          /// check against link results
          phyError = CheckAgainstLinkResults (cSinr, iter->second.rxParams);
        }

      /// save 2nd link sinr value
      iter->second.rxParams->m_sinr = sinr;

      /// uses composite sinr
      m_packetTrace (iter->second.rxParams, m_ownAddress, iter->second.destAddress, ifPower, cSinr);

      /// send packet upwards
      m_rxCallback ( iter->second.rxParams, phyError );
      /// uses composite sinr

      if (!m_cnoCallback.IsNull ())
        {
          double cno = cSinr * m_rxBandwidthHz;
          m_cnoCallback (iter->second.rxParams->m_beamId, iter->second.sourceAddress, m_ownAddress, cno);
        }

      iter->second.rxParams = NULL;
     }
  else
    {
      NS_LOG_INFO ("SatPhyRxCarrier::EndRxDataNormal - CRDSA packet received");
      SatPhyRxCarrier::crdsaPacketRxParams_s params;

      params.destAddress = iter->second.destAddress;
      params.sourceAddress = iter->second.sourceAddress;
      params.rxParams = iter->second.rxParams;

      /// check for collisions
      params.hasCollision = m_satInterference->HasCollision (iter->second.interferenceEvent);

      AddCrdsaPacket (params);
    }

  m_satInterference->NotifyRxEnd (iter->second.interferenceEvent);

  /// erase the used Rx params
  iter->second.interferenceEvent = NULL;
  m_rxParamsMap.erase (key);
}

bool
SatPhyRxCarrier::ProcessSlottedAlohaCollisions (double cSinr, Ptr<SatSignalParameters> rxParams, Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions");

  bool phyError = false;

  if (m_dropCollidingRandomAccessPackets)
    {
      /// check whether the packet has collided. This mode is intended to be used with constant interference and traced interference
      phyError = m_satInterference->HasCollision (interferenceEvent);
    }
  else
    {
      /// check cSinr against link results
      phyError = CheckAgainstLinkResults (cSinr,rxParams);
    }
  return phyError;
}

void
SatPhyRxCarrier::DoFrameEnd ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::DoFrameEnd");

  if (m_crdsaPacketContainer.size () > 0)
    {
      std::vector<std::pair<SatPhyRxCarrier::crdsaCombinedPacketRxParams_s, bool> > results = ProcessFrame ();

      if (m_crdsaPacketContainer.size () > 0)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::DoFrameEnd - All CRDSA packets in the frame were not processed");
        }

      for (uint32_t i = 0; i < results.size (); i++)
        {
          /// uses composite sinr
          m_packetTrace (results[i].first.rxParams,
                         m_ownAddress,
                         results[i].first.destAddress,
                         results[i].first.ifPower,
                         results[i].first.cSinr);

          /// send packet upwards
          m_rxCallback (results[i].first.rxParams,
                        results[i].second);

          /// uses composite sinr
          if (!m_cnoCallback.IsNull ())
            {
              double cno = results[i].first.cSinr * m_rxBandwidthHz;
              m_cnoCallback (results[i].first.rxParams->m_beamId,
                             results[i].first.sourceAddress,
                             m_ownAddress,
                             cno);
            }

          results[i].first.rxParams = NULL;
        }

      results.clear ();
    }

  /// schedule the next frame end
  /// TODO this needs to be modified when a proper mobility model is
  /// added as the timing advance will not be constant. Additionally,
  /// a proper timing specific class should be implemented for a
  /// functionality like this
  /*
  Time nextStartTime = Now () - GetSuperFrameTxTime (0) + Seconds (m_superframeSeq->GetDurationInSeconds (0));

  if (nextStartTime > Now ())
    {
      Simulator::Schedule (nextStartTime, &SatPhyRxCarrier::DoFrameEnd, this);
    }
  else
    {
      nextStartTime += Seconds (m_superframeSeq->GetDurationInSeconds (0));
      Simulator::Schedule (nextStartTime, &SatPhyRxCarrier::DoFrameEnd, this);
    }
   */
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
SatPhyRxCarrier::CheckAgainstLinkResults (double cSinr, Ptr<SatSignalParameters> rxParams)
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

              double ber = (m_linkResults->GetObject <SatLinkResultsDvbS2> ())->GetBler (rxParams->m_txInfo.modCod,SatUtils::LinearToDb (cSinr));
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

              double ebNo = cSinr * (1/log2(SatUtils::GetModulatedBits (rxParams->m_txInfo.modCod)));
              double ber = (m_linkResults->GetObject <SatLinkResultsDvbRcs2> ())->GetBler (rxParams->m_txInfo.waveformId,SatUtils::LinearToDb (ebNo));
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

void
SatPhyRxCarrier::IncreaseNumOfRxState (SatEnums::PacketType_t packetType)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::IncreaseNumOfRxState - Time: " << Now ().GetSeconds ());

  m_numOfOngoingRx++;
  ChangeState (RX);

  if (packetType == SatEnums::DEDICATED_ACCESS_PACKET)
    {
      m_receivingDedicatedAccess = true;
    }

  CheckRxStateSanity ();
}

void
SatPhyRxCarrier::DecreaseNumOfRxState (SatEnums::PacketType_t packetType)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::DecreaseNumOfRxState - Time: " << Now ().GetSeconds ());

  if (m_numOfOngoingRx > 0)
    {
      m_numOfOngoingRx--;
    }

  if (m_numOfOngoingRx < 1)
    {
      ChangeState (IDLE);
    }

  if (packetType == SatEnums::DEDICATED_ACCESS_PACKET)
    {
      m_receivingDedicatedAccess = false;
    }

  CheckRxStateSanity ();
}

void
SatPhyRxCarrier::CheckRxStateSanity ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::CheckRxStateSanity - Time: " << Now ().GetSeconds ());

  if (m_numOfOngoingRx > 0 && m_state == IDLE)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::CheckStateSanity - State mismatch");
    }

  if (m_numOfOngoingRx < 1 && m_state == RX)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::CheckStateSanity - State mismatch");
    }
}

/// CRDSA receiver functionality

void
SatPhyRxCarrier::AddCrdsaPacket (SatPhyRxCarrier::crdsaPacketRxParams_s crdsaPacket)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::AddCrdsaPacket - Time: " << Now ().GetSeconds ());

  uint32_t slotId = 0;

  if (crdsaPacket.rxParams->m_packetsInBurst.size () > 0)
    {
      SatCrdsaReplicaTag replicaTag;

      /// the tag is in the first packet
      bool result = crdsaPacket.rxParams->m_packetsInBurst[0]->PeekPacketTag (replicaTag);

      if (!result)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::AddCrdsaPacket - First packet did not contain a CRDSA replica tag");
        }

      std::vector<uint16_t> slotIds = replicaTag.GetSlotIds ();

      if (slotIds.size () < 1)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::AddCrdsaPacket - The tag did not contain any slot IDs");
        }

      /// The first slot ID is this replicas own slot ID
      slotId = slotIds[0];

      if (crdsaPacket.otherReplicas.size () > 0)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::AddCrdsaPacket - Vector for packet replicas should be empty at this point");
        }

      for (uint32_t i = 1; i < slotIds.size (); i++)
        {
          crdsaPacket.otherReplicas.push_back (slotIds[i]);
        }
    }
  else
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::AddCrdsaPacket - CRDSA reception with 0 packets");
    }

  std::pair<std::map<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator,bool> result;

  result = m_crdsaPacketContainer.insert (std::make_pair(slotId,crdsaPacket));

  NS_LOG_INFO ("SatPhyRxCarrier::AddCrdsaPacket - Packet in slot " << slotId << " was added to the CRDSA packet container");

  /// TODO this is for debugging purposes, it should be removed at later point
  for (uint32_t i = 0; i < crdsaPacket.otherReplicas.size (); i++)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::AddCrdsaPacket - A replica of the packet is in slot " << crdsaPacket.otherReplicas[i]);
    }
}

std::vector<std::pair<SatPhyRxCarrier::crdsaCombinedPacketRxParams_s,bool> >
SatPhyRxCarrier::ProcessFrame ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::ProcessFrame - Time: " << Now ().GetSeconds ());

  std::vector<std::pair<SatPhyRxCarrier::crdsaCombinedPacketRxParams_s,bool> > combinedPacketsForFrame;

  while (m_crdsaPacketContainer.size () > 0)
    {
      /// find all replicas for a single unique packet
      std::vector<std::pair<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s> > replicas = FindReplicas ();

      /// process replicas
      SatPhyRxCarrier::crdsaCombinedPacketRxParams_s combinedPacket = ProcessReplicas (replicas);

      /// add combined packet to the result vector
      std::pair<SatPhyRxCarrier::crdsaCombinedPacketRxParams_s,bool> processedPacket = std::make_pair (combinedPacket, combinedPacket.phyError);

      combinedPacketsForFrame.push_back (processedPacket);
    }

  return combinedPacketsForFrame;
}

std::vector<std::pair<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s> >
SatPhyRxCarrier::FindReplicas ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::FindReplicas - Time: " << Now ().GetSeconds ());

  std::vector<std::pair<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s> > replicas;
  std::map<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator crdsaPacket = m_crdsaPacketContainer.begin ();

  if (crdsaPacket != m_crdsaPacketContainer.end ())
    {
      replicas.push_back (*crdsaPacket);

      m_crdsaPacketContainer.erase (crdsaPacket);
    }

  for (uint32_t i = 0; i < crdsaPacket->second.otherReplicas.size (); i++)
    {
      std::map<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter;
      iter = m_crdsaPacketContainer.find (crdsaPacket->second.otherReplicas[i]);

      if (iter != m_crdsaPacketContainer.end ())
        {
          replicas.push_back (*iter);

          m_crdsaPacketContainer.erase (iter);
        }
      else
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::FindReplicas - Replica was not found");
        }
    }

  return replicas;
}

SatPhyRxCarrier::crdsaCombinedPacketRxParams_s
SatPhyRxCarrier::ProcessReplicas (std::vector<std::pair<uint32_t,SatPhyRxCarrier::crdsaPacketRxParams_s> > replicas)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::ProcessReplicas - Time: " << Now ().GetSeconds ());

  SatPhyRxCarrier::crdsaCombinedPacketRxParams_s combinedPacket;

  /// TODO these need to be calculated
  combinedPacket.cSinr = 10;
  combinedPacket.ifPower = 5;
  combinedPacket.destAddress = replicas[0].second.destAddress;
  combinedPacket.sourceAddress = replicas[0].second.sourceAddress;
  combinedPacket.rxParams = replicas[0].second.rxParams;
  combinedPacket.phyError = true;

  if (m_dropCollidingRandomAccessPackets)
    {
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          if (!replicas[i].second.hasCollision)
            {
              combinedPacket.phyError = false;
            }
        }
    }

  for (uint32_t i = 0; i < replicas.size (); i++)
    {
      replicas[replicas.size () - 1].second.rxParams = NULL;
      replicas.pop_back ();
    }

  return combinedPacket;
}

}
