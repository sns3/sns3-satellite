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
   m_dropCollidingRandomAccessPackets (carrierConf->AreCollidingRandomAccessPacketsAlwaysDropped ()),
   m_randomAccessDynamicLoadControlMeasurementWindowSize (10), /// TODO change this to parameter
   m_isLowRandomAccessLoad (true), /// TODO change this to parameter
   m_highRandomAccessLoadThreshold (0.5), /// TODO change this to parameter
   m_isRandomAccessEnabledForThisCarrier (true) /// TODO change this to parameter
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
    .AddAttribute( "EnableCompositeSinrOutputTrace",
                   "Enable composite SINR output trace.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatPhyRxCarrier::m_enableCompositeSinrOutputTrace),
                    MakeBooleanChecker ())
    .AddTraceSource ("PacketTrace",
                     "The trace for calculated interferences of the received packets",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_packetTrace))
    .AddTraceSource ("Sinr",
                     "The trace for composite SINR in dB",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_sinrTrace))
    .AddTraceSource ("DaRx",
                     "Received a packet burst through Dedicated Channel",
                      MakeTraceSourceAccessor (&SatPhyRxCarrier::m_daRxTrace))
    .AddTraceSource ("SlottedAlohaRxCollision",
                     "Received a packet burst through Random Access Slotted ALOHA",
                      MakeTraceSourceAccessor (&SatPhyRxCarrier::m_slottedAlohaRxCollisionTrace))
    .AddTraceSource ("SlottedAlohaRxError",
                     "Received a packet burst through Random Access Slotted ALOHA",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_slottedAlohaRxErrorTrace))
    .AddTraceSource ("CrdsaReplicaRx",
                     "Received a CRDSA packet replica through Random Access",
                      MakeTraceSourceAccessor (&SatPhyRxCarrier::m_crdsaReplicaRxTrace))
    .AddTraceSource ("CrdsaUniquePayloadRx",
                     "Received a unique CRDSA payload (after frame processing) "
                     "through Random Access CRDSA",
                      MakeTraceSourceAccessor (&SatPhyRxCarrier::m_crdsaUniquePayloadRxTrace))
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

          for (SatSignalParameters::PacketsInBurst_t::const_iterator i = rxParams->m_packetsInBurst.begin ();
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

  const uint32_t nPackets = iter->second.rxParams->m_packetsInBurst.size ();

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
          cSinr = SatUtils::DbToLinear (m_channelEstimationError->AddError (SatUtils::LinearToDb(cSinr)));
        }
      // Return link
      else if (m_nodeInfo->GetNodeType () == SatEnums::NT_GW)
        {
          cSinr = SatUtils::DbToLinear (m_channelEstimationError->AddError (SatUtils::LinearToDb(cSinr), iter->second.rxParams->m_txInfo.waveformId));
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported node type for a NORMAL Rx model!");
        }

      m_sinrTrace (SatUtils::LinearToDb (cSinr), iter->second.sourceAddress);

      /// composite sinr output trace
      if (m_enableCompositeSinrOutputTrace)
        {
          DoCompositeSinrOutputTrace (cSinr);
        }

      if (iter->second.rxParams->m_txInfo.packetType == SatEnums::SLOTTED_ALOHA_PACKET)
        {
          NS_LOG_INFO ("SatPhyRxCarrier::EndRxDataNormal - Time: " << Now ().GetSeconds () << " - Slotted ALOHA packet received");
          /// check for slotted aloha packet collisions
          phyError = ProcessSlottedAlohaCollisions (cSinr, iter->second.rxParams, iter->second.interferenceEvent);

          if (nPackets > 0)
            {
              const bool hasCollision =
                m_satInterference->HasCollision (iter->second.interferenceEvent);
              m_slottedAlohaRxCollisionTrace (nPackets,                    // number of packets
                                              iter->second.sourceAddress,  // sender address
                                              hasCollision                 // collision flag
                                              );
              m_slottedAlohaRxErrorTrace (nPackets,                    // number of packets
                                          iter->second.sourceAddress,  // sender address
                                          phyError                     // error flag
                                          );
            }
        }
      else
        {
          /// check against link results
          phyError = CheckAgainstLinkResults (cSinr, iter->second.rxParams);

          if (nPackets > 0)
            {
              m_daRxTrace (nPackets,                    // number of packets
                           iter->second.sourceAddress,  // sender address
                           phyError                     // error flag
                           );
            }
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
      NS_LOG_INFO ("SatPhyRxCarrier::EndRxDataNormal - Time: " << Now ().GetSeconds () << " - CRDSA packet received");
      SatPhyRxCarrier::crdsaPacketRxParams_s params;

      params.destAddress = iter->second.destAddress;
      params.sourceAddress = iter->second.sourceAddress;
      params.rxParams = iter->second.rxParams;

      /// check for collisions
      params.hasCollision = m_satInterference->HasCollision (iter->second.interferenceEvent);
      params.packetHasBeenProcessed = false;

      if (nPackets > 0)
        {
          m_crdsaReplicaRxTrace (nPackets,              // number of packets
                                 params.sourceAddress,  // sender address
                                 params.hasCollision    // collision flag
                                 );
        }

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
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions - Time: " << Now ().GetSeconds () << " - Strict collision mode, phyError: " << phyError);
    }
  else
    {
      /// check cSinr against link results
      phyError = CheckAgainstLinkResults (cSinr,rxParams);
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions - Time: " << Now ().GetSeconds () << " - Composite SINR mode, phyError: " << phyError);
    }

  return phyError;
}

void
SatPhyRxCarrier::DoFrameEnd ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::DoFrameEnd");

  if (m_isRandomAccessEnabledForThisCarrier)
    {
      MeasureRandomAccessLoad ();

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

              /// CRDSA trace
              m_crdsaUniquePayloadRxTrace (results[i].rxParams->m_packetsInBurst.size (),  // number of packets
                                           results[i].sourceAddress,  // sender address
                                           results[i].phyError        // error flag
                                       );
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
    }
  else
    {
      if (m_crdsaPacketContainer.size () > 0)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::DoFrameEnd  - CRDSA packets received when RA is disabled");
        }
    }

  /// TODO get rid of the hard coded 0
  Time nextSuperFrameRxTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (0);

  if (Now () >= nextSuperFrameRxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameRxTime - Now ();

  Simulator::Schedule (schedulingDelay, &SatPhyRxCarrier::DoFrameEnd, this);
}

void
SatPhyRxCarrier::MeasureRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::MeasureRandomAccessLoad");

  /// calculate the load for this frame
  double normalizedOfferedLoad = CalculateNormalizedOfferedRandomAccessLoad ();

  /// save the load for this frame
  SaveMeasuredRandomAccessLoad (normalizedOfferedLoad);

  /// calculate the average load over the measurement window
  double averageNormalizedOfferedLoad = CalculateAverageNormalizedOfferedRandomAccessLoad ();

  /// low RA load in effect
  if (m_isLowRandomAccessLoad)
    {
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad >= m_highRandomAccessLoadThreshold)
        {
          /// use high load back off value
          // trigger control packet creation with high load settings

          /// flag RA load as high load
          m_isLowRandomAccessLoad = false;
        }
    }
  /// high RA load in effect
  else
    {
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad < m_highRandomAccessLoadThreshold)
        {
          /// use low load back off value
          // trigger control packet creation with low load settings

          /// flag RA load as low load
          m_isLowRandomAccessLoad = true;
        }
    }
}

double
SatPhyRxCarrier::CalculateNormalizedOfferedRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::CalculateNormalizedOfferedRandomAccessLoad");

  return 0.5;
}

void
SatPhyRxCarrier::SaveMeasuredRandomAccessLoad (double measuredRandomAccessLoad)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::SaveMeasuredRandomAccessLoad");

  m_randomAccessDynamicLoadControlNormalizedOfferedLoad.push_back (measuredRandomAccessLoad);

  while (m_randomAccessDynamicLoadControlNormalizedOfferedLoad.size () > m_randomAccessDynamicLoadControlMeasurementWindowSize)
    {
      m_randomAccessDynamicLoadControlNormalizedOfferedLoad.pop_front ();
    }
}

double
SatPhyRxCarrier::CalculateAverageNormalizedOfferedRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::CalculateAverageNormalizedOfferedRandomAccessLoad");

  double sum = 0.0;
  double averageNormalizedOfferedLoad = 0.0;
  std::deque<double>::iterator it;

  for (it = m_randomAccessDynamicLoadControlNormalizedOfferedLoad.begin (); it != m_randomAccessDynamicLoadControlNormalizedOfferedLoad.end (); it++)
    {
      sum += (*it);
    }

  if (!m_randomAccessDynamicLoadControlNormalizedOfferedLoad.empty ())
    {
      averageNormalizedOfferedLoad = sum / m_randomAccessDynamicLoadControlNormalizedOfferedLoad.size ();
    }

  return averageNormalizedOfferedLoad;
}

void
SatPhyRxCarrier::DoCompositeSinrOutputTrace (double cSinr)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::DoCompositeSinrOutputTrace");

  std::vector<double> tempVector;
  tempVector.push_back (Now ().GetSeconds ());
  tempVector.push_back (cSinr);

  Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (m_ownAddress, m_channelType), tempVector);
}

bool
SatPhyRxCarrier::CheckAgainstLinkResults (double cSinr, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::CheckAgainstLinkResults");

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
          NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Iterating slot: " << iter->first);

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

                      /// eliminate the interference caused by this packet to other packets in this slot
                      EliminateInterference (iter,processedPacket);

                      /// break the cycle
                      break;
                    }
                }
              else
                {
                  NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - This packet has already been processed");
                }
            }

          /// successfully received packet found
          if (!nothingToProcess)
            {
              NS_LOG_INFO ("SatPhyRxCarrier::ProcessFrame - Packet successfully received, breaking the container iteration");
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

  /// TODO this is for debugging and can be removed later
  for (uint32_t i = 0; i < packet.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Replica in slot: " << packet.slotIdsForOtherReplicas[i]);
    }

  NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - SINR CALCULATION, RX sat: " << packet.rxParams->m_rxPowerInSatellite_W <<
               " IF sat: " << packet.rxParams->m_ifPowerInSatellite_W <<
               " RX gnd: " << packet.rxParams->m_rxPower_W <<
               " IF gnd: " << packet.rxParams->m_ifPower_W);

  double sinrSatellite = CalculateSinr ( packet.rxParams->m_rxPowerInSatellite_W,
                                         packet.rxParams->m_ifPowerInSatellite_W,
                                         packet.rxParams->m_rxExtNoisePowerInSatellite_W,
                                         packet.rxParams->m_rxAciIfPowerInSatellite_W,
                                         packet.rxParams->m_rxExtNoisePowerInSatellite_W,
                                         packet.rxParams->m_sinrCalculate);

  double sinr = CalculateSinr ( packet.rxParams->m_rxPower_W,
                                packet.rxParams->m_ifPower_W,
                                m_rxExtNoisePowerW,
                                m_rxAciIfPowerW,
                                m_rxExtNoisePowerW,
                                m_sinrCalculate);

  double cSinr = CalculateCompositeSinr (sinr, sinrSatellite);

  /**
   * Channel estimation error. Channel estimation error works in dB domain, thus we need
   * to do linear-to-db and db-to-linear conversions here.
   */
  // Forward link
  if (m_nodeInfo->GetNodeType () == SatEnums::NT_UT)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Random access is not used in forward link");
    }
  // Return link
  else if (m_nodeInfo->GetNodeType () == SatEnums::NT_GW)
    {
      cSinr = SatUtils::DbToLinear (m_channelEstimationError->AddError (SatUtils::LinearToDb (cSinr), packet.rxParams->m_txInfo.waveformId));
    }
  else
    {
      NS_FATAL_ERROR ("Unsupported node type for a NORMAL Rx model!");
    }

  packet.cSinr = cSinr;
  packet.ifPower = packet.rxParams->m_ifPower_W;

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
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessReceivedCrdsaPacket - Strict collision detection, phy error: " << packet.phyError);
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
      SatPhyRxCarrier::crdsaPacketRxParams_s removedPacket;

      bool replicaFound = false;

      for (iterList = iter->second.begin (); iterList != iter->second.end (); )
        {
          /// check for the same UT & same slots
          if (IsReplica (packet, iterList))
            {
              /// replica found for removal
              replicaFound = true;
              removedPacket = *iterList;
              iter->second.erase (iterList++);
            }
          else
            {
              ++iterList;
            }
        }

      if (!replicaFound)
        {
          NS_FATAL_ERROR ("SatPhyRxCarrier::FindAndRemoveReplicas - Replica not found");
        }

      EliminateInterference (iter, removedPacket);
    }
}

void
SatPhyRxCarrier::EliminateInterference (std::map<uint32_t,std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> >::iterator iter, SatPhyRxCarrier::crdsaPacketRxParams_s processedPacket)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("SatPhyRxCarrier::EliminateInterference");

  if (iter->second.empty ())
    {
      NS_LOG_INFO ("SatPhyRxCarrier::EliminateInterference - No other packets in this slot, erasing the slot container");
      m_crdsaPacketContainer.erase (iter);
    }
  else
    {
      std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iterList;

      for (iterList = iter->second.begin (); iterList != iter->second.end (); iterList++)
        {
          /// release packets in this slot for re-processing
          iterList->packetHasBeenProcessed = false;

          NS_LOG_INFO ("SatPhyRxCarrier::EliminateInterference- BEFORE INTERFERENCE ELIMINATION, RX sat: " << iterList->rxParams->m_rxPowerInSatellite_W <<
                       " IF sat: " << iterList->rxParams->m_ifPowerInSatellite_W <<
                       " RX gnd: " << iterList->rxParams->m_rxPower_W <<
                       " IF gnd: " << iterList->rxParams->m_ifPower_W);

          /// reduce interference power for the colliding packets
          /// TODO more novel way to eliminate partially overlapping interference should be considered. Inaccuracies with double needs to be taken into account
          iterList->rxParams->m_ifPowerInSatellite_W -= processedPacket.rxParams->m_rxPowerInSatellite_W;
          iterList->rxParams->m_ifPower_W -= processedPacket.rxParams->m_rxPower_W;

          if (iterList->rxParams->m_ifPowerInSatellite_W < 0)
            {
              iterList->rxParams->m_ifPowerInSatellite_W = 0;
            }

          if (iterList->rxParams->m_ifPower_W < 0)
            {
              iterList->rxParams->m_ifPower_W = 0;
            }

          NS_LOG_INFO ("SatPhyRxCarrier::EliminateInterference- AFTER INTERFERENCE ELIMINATION, RX sat: " << iterList->rxParams->m_rxPowerInSatellite_W <<
                       " IF sat: " << iterList->rxParams->m_ifPowerInSatellite_W <<
                       " RX gnd: " << iterList->rxParams->m_rxPower_W <<
                       " IF gnd: " << iterList->rxParams->m_ifPower_W);
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
