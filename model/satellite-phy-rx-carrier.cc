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
#include "satellite-rtn-link-time.h"

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
   m_dropCollidingRandomAccessPackets (carrierConf->AreCollidingRandomAccessPacketsAlwaysDropped ())
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

  Time nextSuperFrameRxTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (0);

  if (Now () >= nextSuperFrameRxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameRxTime - Now ();

  Simulator::Schedule (schedulingDelay, &SatPhyRxCarrier::DoFrameEnd, this);
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

  std::map<uint32_t,std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> >::iterator iter;

  for (iter = m_crdsaPacketContainer.begin (); iter != m_crdsaPacketContainer.end (); iter++)
    {
      std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iterList;
      for (iterList = iter->second.begin (); iterList != iter->second.end (); iterList++)
        {
          iterList->rxParams = NULL;
        }
      iter->second.clear ();
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

  iter->second.rxParams->m_ifPower_W = m_satInterference->Calculate (iter->second.interferenceEvent);

  /// save values for CRDSA receiver
  iter->second.rxParams->m_ifPowerInSatellite_W = iter->second.rxParams->m_ifPower_W;
  iter->second.rxParams->m_rxPowerInSatellite_W = iter->second.rxParams->m_rxPower_W;
  iter->second.rxParams->m_rxNoisePowerInSatellite_W = m_rxNoisePowerW;
  iter->second.rxParams->m_rxAciIfPowerInSatellite_W = m_rxAciIfPowerW;
  iter->second.rxParams->m_rxExtNoisePowerInSatellite_W = m_rxExtNoisePowerW;
  iter->second.rxParams->m_sinrCalculate = m_sinrCalculate;

  /// calculates sinr for 1st link
  double sinr = CalculateSinr ( iter->second.rxParams->m_rxPower_W,
                                iter->second.rxParams->m_ifPower_W,
                                m_rxNoisePowerW,
                                m_rxAciIfPowerW,
                                m_rxExtNoisePowerW,
                                m_sinrCalculate);

  NS_ASSERT (m_rxMode == SatPhyRxCarrierConf::TRANSPARENT && iter->second.rxParams->m_sinr == 0);

  /// PHY transmission decoded successfully. Note, that at transparent satellite,
  /// all the transmissions are not decoded.
  bool phyError (false);

  /// save 1st link sinr value for 2nd link composite sinr calculations
  iter->second.rxParams->m_sinr = sinr;

  /// uses 1st link sinr
  m_packetTrace (iter->second.rxParams, m_ownAddress, iter->second.destAddress, iter->second.rxParams->m_ifPower_W, sinr);

  m_satInterference->NotifyRxEnd (iter->second.interferenceEvent);

  /// Send packet upwards
  m_rxCallback ( iter->second.rxParams, phyError );

  /// uses 1st link sinr
  if (!m_cnoCallback.IsNull ())
    {
      double cno = sinr * m_rxBandwidthHz;
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

  iter->second.rxParams->m_ifPower_W = m_satInterference->Calculate (iter->second.interferenceEvent);

  if (iter->second.rxParams->m_txInfo.packetType != SatEnums::CRDSA_PACKET)
    {
      /// calculates sinr for 2nd link
      double sinr = CalculateSinr ( iter->second.rxParams->m_rxPower_W,
                                    iter->second.rxParams->m_ifPower_W,
                                    m_rxNoisePowerW,
                                    m_rxAciIfPowerW,
                                    m_rxExtNoisePowerW,
                                    m_sinrCalculate);

      /// PHY transmission decoded successfully. Note, that at transparent satellite,
      /// all the transmissions are not decoded.
      bool phyError (false);

      /// calculate composite SINR
      double cSinr = CalculateCompositeSinr (sinr, iter->second.rxParams->m_sinr);

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
      m_packetTrace (iter->second.rxParams, m_ownAddress, iter->second.destAddress, iter->second.rxParams->m_ifPower_W, cSinr);

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
      params.packetHasBeenProcessed = false;

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
      NS_LOG_INFO ("SatPhyRxCarrier::DoFrameEnd - Packets in container, will process the frame");

      std::vector<SatPhyRxCarrier::crdsaPacketRxParams_s> results = ProcessFrame ();

      if (m_crdsaPacketContainer.size () > 0)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::DoFrameEnd - All CRDSA packets in the frame were not processed");
        }

      for (uint32_t i = 0; i < results.size (); i++)
        {
          NS_LOG_INFO ("SatPhyRxCarrier::DoFrameEnd - Sending a packet to the next layer, slot: " << results[i].ownSlotId
                       << " error: " << results[i].phyError
                       << " SINR: " << results[i].cSinr);

          /// uses composite sinr
          m_packetTrace (results[i].rxParams,
                         m_ownAddress,
                         results[i].destAddress,
                         results[i].ifPower,
                         results[i].cSinr);

          /// send packet upwards
          m_rxCallback (results[i].rxParams,
                        results[i].phyError);

          /// uses composite sinr
          if (!m_cnoCallback.IsNull ())
            {
              double cno = results[i].cSinr * m_rxBandwidthHz;
              m_cnoCallback (results[i].rxParams->m_beamId,
                             results[i].sourceAddress,
                             m_ownAddress,
                             cno);
            }

          results[i].rxParams = NULL;
        }

      results.clear ();
    }

  Time nextSuperFrameRxTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (0);

  if (Now () >= nextSuperFrameRxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameRxTime - Now ();

  Simulator::Schedule (schedulingDelay, &SatPhyRxCarrier::DoFrameEnd, this);
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
SatPhyRxCarrier::CalculateSinr (double rxPowerW,
                                double ifPowerW,
                                double rxNoisePowerW,
                                double rxAciIfPowerW,
                                double rxExtNoisePowerW,
                                SatPhyRxCarrierConf::SinrCalculatorCallback sinrCalculate)
{
  NS_LOG_FUNCTION (this << rxPowerW <<  ifPowerW);

  if (rxNoisePowerW <= 0.0)
    {
      NS_FATAL_ERROR ("Noise power must be greater than zero!!!");
    }

  // Calculate first SINR based on co-channel interference, Adjacent channel interference, noise and external noise
  // NOTE! ACI noise power and Ext noise power are set 0 by default and given as attributes by PHY object when used.
  double sinr = rxPowerW / (ifPowerW +  rxNoisePowerW + rxAciIfPowerW + rxExtNoisePowerW);

  // Call PHY calculator to composite C over I interference configured to PHY.
  double finalSinr = sinrCalculate (sinr);

  return (finalSinr);
}

double
SatPhyRxCarrier::CalculateCompositeSinr (double sinr1, double sinr2)
{
  NS_LOG_FUNCTION (this << sinr1 << sinr2 );

  if (sinr1 <= 0.0)
    {
      NS_FATAL_ERROR ("SINR 1 must be greater than zero!!!");
    }

  if (sinr2 <= 0.0)
    {
      NS_FATAL_ERROR ("SINR 2 must be greater than zero!!!");
    }

  return 1.0 / ( (1.0 / sinr1) + (1.0 / sinr2) );
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
      crdsaPacket.ownSlotId = slotIds[0];

      if (crdsaPacket.slotIdsForOtherReplicas.size () > 0)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::AddCrdsaPacket - Vector for packet replicas should be empty at this point");
        }

      for (uint32_t i = 1; i < slotIds.size (); i++)
        {
          crdsaPacket.slotIdsForOtherReplicas.push_back (slotIds[i]);
        }
    }
  else
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::AddCrdsaPacket - CRDSA reception with 0 packets");
    }

  std::map<uint32_t, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> >::iterator result;

  result = m_crdsaPacketContainer.find (crdsaPacket.ownSlotId);

  if (result == m_crdsaPacketContainer.end ())
    {
      std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> tempList;
      tempList.push_back (crdsaPacket);
      m_crdsaPacketContainer.insert (std::make_pair (crdsaPacket.ownSlotId, tempList));
    }
  else
    {
      result->second.push_back (crdsaPacket);
    }

  NS_LOG_INFO ("SatPhyRxCarrier::AddCrdsaPacket - Packet in slot " << crdsaPacket.ownSlotId << " was added to the CRDSA packet container");

  /// TODO this is for debugging purposes, it should be removed at later point
  for (uint32_t i = 0; i < crdsaPacket.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::AddCrdsaPacket - A replica of the packet is in slot " << crdsaPacket.slotIdsForOtherReplicas[i]);
    }
}

std::vector<SatPhyRxCarrier::crdsaPacketRxParams_s>
SatPhyRxCarrier::ProcessFrame ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::ProcessFrame - Time: " << Now ().GetSeconds ());

  std::map<uint32_t,std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> >::iterator iter;
  std::vector<SatPhyRxCarrier::crdsaPacketRxParams_s> combinedPacketsForFrame;
  SatPhyRxCarrier::crdsaPacketRxParams_s processedPacket;

  bool nothingToProcess = true;

  NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Packets to process: " << m_crdsaPacketContainer.size ());

  do
    {
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Searching for the next successfully received packet");

      /// reset the flag
      nothingToProcess = true;

      /// go through the packets
      for (iter = m_crdsaPacketContainer.begin (); iter != m_crdsaPacketContainer.end (); iter++)
        {
          NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Iterating packets in slot: " << iter->first);

          if (iter->second.size () < 1)
            {
              NS_FATAL_ERROR ("SatPhyRxCarrier::ProcessFrame - This should not happen");
            }

          std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iterList;

          for (iterList = iter->second.begin (); iterList != iter->second.end (); iterList++)
            {
              NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Iterating packet in slot: " << iterList->ownSlotId);

              if (!iterList->packetHasBeenProcessed)
                {
                  NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Found a packet ready for processing");

                  /// process the received packet
                  *iterList = ProcessReceivedCrdsaPacket (*iterList, iter->second.size ());

                  NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Packet error: " << iterList->phyError);

                  /// packet successfully received
                  if (!iterList->phyError)
                    {
                      NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Packet successfully received, breaking the slot iteration");

                      nothingToProcess = false;

                      /// save packet for processing outside the loop
                      processedPacket = *iterList;

                      /// remove the successfully received packet from the container
                      iter->second.erase (iterList);

                      /// break the cycle
                      break;
                    }
                }
            }

          /// successfully received packet found
          if (!nothingToProcess)
            {
              NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Packet successfully received, breaking the container iteration");

              if (iter->second.empty ())
                {
                  m_crdsaPacketContainer.erase (iter);
                }

              break;
            }
        }

      if (!nothingToProcess)
        {
          NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Packet successfully received, processing the replicas");

          /// find and remove replicas of the received packet
          FindAndRemoveReplicas (processedPacket);

          /// save the the received packet
          combinedPacketsForFrame.push_back (processedPacket);
        }
    } while (!nothingToProcess);

  NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - All successfully received packets processed, packets left in container: " << m_crdsaPacketContainer.size ());

  do
    {
      /// go through the packets
      iter = m_crdsaPacketContainer.begin ();

      if (iter != m_crdsaPacketContainer.end ())
        {
          std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iterList = iter->second.begin ();

          NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Processing unsuccessfully received packet in slot: " << iterList->ownSlotId
                       << " packet phy error: " << iterList->phyError
                       << " packet has been processed: " << iterList->packetHasBeenProcessed);

          if (!iterList->packetHasBeenProcessed || !iterList->phyError)
            {
               NS_FATAL_ERROR ("SatPhyRxCarrier::ProcessFrame - All packets should have been processed by now");
            }

          /// find and remove replicas of the received packet
          FindAndRemoveReplicas (*iterList);

          /// save the the received packet
          combinedPacketsForFrame.push_back (*iterList);

          /// remove the packet from the container
          iter->second.erase (iterList);

          /// remove the empty slot container
          if (iter->second.empty ())
            {
              m_crdsaPacketContainer.erase (iter);
            }
        }
    } while (m_crdsaPacketContainer.size () > 0);

  NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Container processed, packets left: " << m_crdsaPacketContainer.size ());

  return combinedPacketsForFrame;
}

SatPhyRxCarrier::crdsaPacketRxParams_s
SatPhyRxCarrier::ProcessReceivedCrdsaPacket (SatPhyRxCarrier::crdsaPacketRxParams_s packet, uint32_t numOfPacketsForThisSlot)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::ProcessSuccessfullyReceivedCrdsaPacket - Time: " << Now ().GetSeconds ());

  NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Processing a packet in slot: " << packet.ownSlotId <<
               " number of packets in this slot: " << numOfPacketsForThisSlot);

  for (uint32_t i = 0; i < packet.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Replica in slot: " << packet.slotIdsForOtherReplicas[i]);
    }

  /// TODO these need to be calculated
  packet.cSinr = 10;
  packet.ifPower = 5;

  if (m_dropCollidingRandomAccessPackets)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Strict collision detection is ENABLED");

      /// there is a collision
      if (numOfPacketsForThisSlot > 1)
        {
          NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Multiple packets in this slot, successful reception is not possible");
          /// not possible to have a successful reception
          packet.phyError = true;
        }
      else
        {
          NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Only packet in this slot, checking against link results");
          /// check against link results
          packet.phyError = CheckAgainstLinkResults (packet.cSinr,packet.rxParams);
        }
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Strict collision detection error: " << packet.phyError);
    }
  else
    {
      /// check against link results
      packet.phyError = CheckAgainstLinkResults (packet.cSinr,packet.rxParams);
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Check against link results, phy error: " << packet.phyError);
    }

  /// mark the packet as processed
  packet.packetHasBeenProcessed = true;

  return packet;
}

void
SatPhyRxCarrier::ReduceCrdsaPacketInterference (Ptr<SatSignalParameters> packet, Ptr<SatSignalParameters> successfullyReceivedPacket)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::ReduceCrdsaPacketInterference - Time: " << Now ().GetSeconds ());

  NS_LOG_INFO ("SatPhyRxCarrier::ReduceCrdsaPacketInterference");

  packet->m_ifPowerInSatellite_W -= successfullyReceivedPacket->m_ifPowerInSatellite_W;
  packet->m_ifPower_W -= successfullyReceivedPacket->m_ifPower_W;

  if (packet->m_ifPower_W < 0 || packet->m_ifPowerInSatellite_W < 0)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::ReducePacketInterference - Negative interference power");
    }
}

void
SatPhyRxCarrier::FindAndRemoveReplicas (SatPhyRxCarrier::crdsaPacketRxParams_s packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::FindAndRemoveReplicas - Time: " << Now ().GetSeconds ());

  for (uint32_t i = 0; i < packet.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::FindAndRemoveReplicas - Processing replica in slot: " << packet.slotIdsForOtherReplicas[i]);

      /// get the vector of packets for processing
      std::map<uint32_t,std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> >::iterator iter;
      iter = m_crdsaPacketContainer.find (packet.slotIdsForOtherReplicas[i]);

      if (iter == m_crdsaPacketContainer.end ())
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::FindAndRemoveReplicas - This should not happen");
        }

      std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iterList;

      bool replicaFound = false;

      /// TODO this functionality needs to be checked
      for (iterList = iter->second.begin (); iterList != iter->second.end (); )
        {
          /// release packets in this slot for re-processing
          iterList->packetHasBeenProcessed = false;

          /// check for the same UT & same slots
          if (IsReplica (packet, iterList))
            {
              /// replica found for removal
              replicaFound = true;
              iter->second.erase (iterList++);
            }
          else
            {
              ++iterList;
            }
        }

      if (iter->second.empty ())
        {
          NS_LOG_INFO ("SatPhyRxCarrier::FindAndRemoveReplicas - No other packets in this slot, erasing the slot container");
          m_crdsaPacketContainer.erase (iter);
        }

      if (!replicaFound)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::FindAndRemoveReplicas - Replica not found");
        }
    }
}

bool
SatPhyRxCarrier::IsReplica (SatPhyRxCarrier::crdsaPacketRxParams_s packet, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::IsReplica - Time: " << Now ().GetSeconds ());

  NS_LOG_INFO ("SatPhyRxCarrier::IsReplica - Checking the source addresses");

  bool isReplica = false;

  if (iter->sourceAddress == packet.sourceAddress)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::IsReplica - Same source addresses, checking slot IDs");

      if (HaveSameSlotIds (packet, iter))
        {
          NS_LOG_INFO ("SatPhyRxCarrier::IsReplica - Same slot IDs, replica found");
          isReplica = true;
        }
    }
  return isReplica;
}

bool
SatPhyRxCarrier::HaveSameSlotIds (SatPhyRxCarrier::crdsaPacketRxParams_s packet, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatPhyRxCarrier::HaveSameSlotIds - Time: " << Now ().GetSeconds ());

  std::set<uint16_t> firstSet;
  std::set<uint16_t> secondSet;
  std::set<uint16_t>::iterator firstSetIterator;
  std::set<uint16_t>::iterator secondSetIterator;
  bool HaveSameSlotIds = true;

  firstSet.insert (packet.ownSlotId);
  secondSet.insert (iter->ownSlotId);

  /// sanity check
  if (iter->slotIdsForOtherReplicas.size () != packet.slotIdsForOtherReplicas.size ())
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::HaveSameSlotIds - The amount of replicas does not match");
    }

  NS_LOG_INFO ("SatPhyRxCarrier::HaveSameSlotIds - Comparing slot IDs");

  /// form sets
  for (uint32_t i = 0; i < iter->slotIdsForOtherReplicas.size (); i++)
    {
      firstSet.insert (packet.slotIdsForOtherReplicas[i]);
      secondSet.insert (iter->slotIdsForOtherReplicas[i]);
    }

  uint32_t numOfMatchingSlots = 0;

  /// compare sets
  for (firstSetIterator = firstSet.begin (); firstSetIterator != firstSet.end (); firstSetIterator++)
    {
      secondSetIterator = secondSet.find (*firstSetIterator);

      if (secondSetIterator == secondSet.end ())
        {
          HaveSameSlotIds = false;
        }
      else
        {
          numOfMatchingSlots++;
        }
    }

  NS_LOG_INFO ("SatPhyRxCarrier::HaveSameSlotIds - Are slot IDs identical: " << HaveSameSlotIds
               << ", number of matching slots: " << numOfMatchingSlots);

  /// sanity check
  if (!HaveSameSlotIds && numOfMatchingSlots > 0)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::HaveSameSlotIds - Partially overlapping CRDSA slots");
    }

  return HaveSameSlotIds;
}

}
