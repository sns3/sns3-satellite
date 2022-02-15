/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/boolean.h>

#include "satellite-phy-rx-carrier-per-frame.h"

#include <algorithm>
#include <ostream>
#include <limits>
#include <utility>

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierPerFrame");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierPerFrame);

SatPhyRxCarrierPerFrame::SatPhyRxCarrierPerFrame (uint32_t carrierId,
                                                  Ptr<SatPhyRxCarrierConf> carrierConf,
                                                  Ptr<SatWaveformConf> waveformConf,
                                                  bool randomAccessEnabled)
  : SatPhyRxCarrierPerSlot (carrierId, carrierConf, waveformConf, randomAccessEnabled),
  m_frameEndSchedulingInitialized (false)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Constructor called with arguments " << carrierId << ", " << carrierConf << ", and " << randomAccessEnabled);

  NS_ASSERT (m_randomAccessEnabled == true);
}

void
SatPhyRxCarrierPerFrame::BeginEndScheduling ()
{
  NS_LOG_FUNCTION (this);
  if (!m_frameEndSchedulingInitialized)
    {
      Time nextSuperFrameRxTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (SatConstVariables::SUPERFRAME_SEQUENCE);

      if (Now () >= nextSuperFrameRxTime)
        {
          NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
        }

      Time schedulingDelay = nextSuperFrameRxTime - Now ();

      if (GetNodeInfo () == NULL)
        {
          NS_FATAL_ERROR ("m_nodeInfo not set");
        }

      m_frameEndSchedulingInitialized = true;

      Simulator::ScheduleWithContext (GetNodeInfo ()->GetNodeId (), schedulingDelay, &SatPhyRxCarrierPerFrame::DoFrameEnd, this);
    }
}


SatPhyRxCarrierPerFrame::~SatPhyRxCarrierPerFrame ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
SatPhyRxCarrierPerFrame::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierPerFrame")
    .SetParent<SatPhyRxCarrierPerSlot> ()
    .AddTraceSource ("CrdsaReplicaRx",
                     "Received a CRDSA packet replica through Random Access",
                     MakeTraceSourceAccessor (&SatPhyRxCarrierPerFrame::m_crdsaReplicaRxTrace),
                     "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
    .AddTraceSource ("CrdsaUniquePayloadRx",
                     "Received a unique CRDSA payload (after frame processing) "
                     "through Random Access CRDSA",
                     MakeTraceSourceAccessor (&SatPhyRxCarrierPerFrame::m_crdsaUniquePayloadRxTrace),
                     "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
  ;
  return tid;
}


void
SatPhyRxCarrierPerFrame::DoDispose ()
{
  SatPhyRxCarrierPerSlot::DoDispose ();
  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter;

  for (iter = m_crdsaPacketContainer.begin (); iter != m_crdsaPacketContainer.end (); iter++)
    {
      std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iterList;
      for (iterList = iter->second.begin (); iterList != iter->second.end (); iterList++)
        {
          iterList->rxParams = NULL;
        }
      iter->second.clear ();
    }
  m_crdsaPacketContainer.clear ();
}

void
SatPhyRxCarrierPerFrame::ReceiveSlot (SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets)
{
  NS_ASSERT (packetRxParams.rxParams->m_txInfo.packetType != SatEnums::PACKET_TYPE_DEDICATED_ACCESS);

  // If the received random access packet is of type slotted aloha, we
  // receive the packet with the base class ReceiveSlot method.
  if (packetRxParams.rxParams->m_txInfo.packetType == SatEnums::PACKET_TYPE_SLOTTED_ALOHA
        || packetRxParams.rxParams->m_txInfo.packetType == SatEnums::PACKET_TYPE_LOGON)
    {
      SatPhyRxCarrierPerSlot::ReceiveSlot (packetRxParams, nPackets);
      return;
    }

  NS_LOG_INFO ("CRDSA packet received");
  SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s params;

  params.destAddress = packetRxParams.destAddress;
  params.sourceAddress = packetRxParams.sourceAddress;
  params.rxParams = packetRxParams.rxParams;

  /// check for collisions
  params.hasCollision = GetInterferenceModel ()->HasCollision (packetRxParams.interferenceEvent);
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

void
SatPhyRxCarrierPerFrame::DoFrameEnd ()
{
  NS_LOG_FUNCTION (this);

  if (!m_crdsaPacketContainer.empty ())
    {
      // Update the CRDSA random access load for unique payloads!
      UpdateRandomAccessLoad ();

      NS_LOG_INFO ("Packets in container, will process the frame");

      std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> results = ProcessFrame ();

      if (!m_crdsaPacketContainer.empty ())
        {
          NS_FATAL_ERROR ("All CRDSA packets in the frame were not processed");
        }

      /// sort the results based on CRDSA packet IDs to make sure the packets are processed in correct order
      std::sort (results.begin (), results.end (), CompareCrdsaPacketId);

      for (crdsaPacketRxParams_s& crdsaPacket : results)
        {
          NS_LOG_INFO ("Sending a packet to the next layer, slot: " << crdsaPacket.ownSlotId <<
                       ", UT: " << crdsaPacket.sourceAddress <<
                       ", unique CRDSA packet ID: " << crdsaPacket.rxParams->m_txInfo.crdsaUniquePacketId <<
                       ", destination address: " << crdsaPacket.destAddress <<
                       ", error: " << crdsaPacket.phyError <<
                       ", SINR: " << crdsaPacket.cSinr);

          for (Ptr<Packet>& packetInBurst : crdsaPacket.rxParams->m_packetsInBurst)
            {
              NS_LOG_INFO ("Fragment (HL packet) UID: " << packetInBurst->GetUid ());
            }

          /// uses composite sinr
          m_linkBudgetTrace (crdsaPacket.rxParams,
                             GetOwnAddress (),
                             crdsaPacket.destAddress,
                             crdsaPacket.ifPower,
                             crdsaPacket.cSinr);
          /// CRDSA trace
          m_crdsaUniquePayloadRxTrace (crdsaPacket.rxParams->m_packetsInBurst.size (),  // number of packets
                                       crdsaPacket.sourceAddress,  // sender address
                                       crdsaPacket.phyError        // error flag
                                       );

          // Update composite SINR trace for CRDSA packet after combination
          m_sinrTrace (SatUtils::LinearToDb (crdsaPacket.cSinr), crdsaPacket.sourceAddress);

          /// send packet upwards
          m_rxCallback (crdsaPacket.rxParams,
                        crdsaPacket.phyError);

          crdsaPacket.rxParams = NULL;
        }

      results.clear ();
    }

  if (IsRandomAccessDynamicLoadControlEnabled ())
    {
      MeasureRandomAccessLoad ();
    }
  else
    {
      if (!m_crdsaPacketContainer.empty ())
        {
          NS_FATAL_ERROR ("CRDSA packets received by carrier which has random access disabled");
        }
    }

  Time nextSuperFrameRxTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (SatConstVariables::SUPERFRAME_SEQUENCE);

  if (Now () >= nextSuperFrameRxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameRxTime - Now ();

  Simulator::Schedule (schedulingDelay, &SatPhyRxCarrierPerFrame::DoFrameEnd, this);
}


void
SatPhyRxCarrierPerFrame::UpdateRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);

  std::vector<uint64_t> uniquePacketIds;
  uint32_t uniqueCrdsaBytes (0);

  // Go through all the received CRDSA packets
  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter;
  for (iter = m_crdsaPacketContainer.begin (); iter != m_crdsaPacketContainer.end (); iter++)
    {
      // Go through all the packets received in the same slot id
      std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iterList;
      for (iterList = iter->second.begin (); iterList != iter->second.end (); iterList++)
        {
          // It is sufficient to check the first packet Uid
          uint64_t uid = iterList->rxParams->m_packetsInBurst.front ()->GetUid ();

          // Check if we have already counted the bytes of this transmission
          std::vector<uint64_t>::iterator it = std::find (uniquePacketIds.begin (),
                                                          uniquePacketIds.end (),
                                                          uid);
          // Not found -> is unique
          if (it == uniquePacketIds.end ())
            {
              // Push this to accounted unique transmissions vector
              uniquePacketIds.push_back (uid);

              // Update the load with FEC block size!
              uniqueCrdsaBytes += iterList->rxParams->m_txInfo.fecBlockSizeInBytes;
            }
          // else, do nothing, i.e. this is a replica
        }
    }

  // Update with the unique FEC block sum of CRDSA frame
  m_randomAccessBitsInFrame = uniqueCrdsaBytes * SatConstVariables::BITS_PER_BYTE;
}

void
SatPhyRxCarrierPerFrame::MeasureRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);

  /// calculate the load for this frame
  double normalizedOfferedLoad = CalculateNormalizedOfferedRandomAccessLoad ();

  /// save the load for this frame
  SaveMeasuredRandomAccessLoad (normalizedOfferedLoad);

  /// calculate the average load over the measurement window
  double averageNormalizedOfferedLoad = CalculateAverageNormalizedOfferedRandomAccessLoad ();

  NS_LOG_INFO ("Average normalized offered load: " << averageNormalizedOfferedLoad);

  m_avgNormalizedOfferedLoadCallback (GetBeamId (), GetCarrierId (), GetRandomAccessAllocationChannelId (), averageNormalizedOfferedLoad);
}

double
SatPhyRxCarrierPerFrame::CalculateNormalizedOfferedRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);

  Time superFrameDuration = Singleton<SatRtnLinkTime>::Get ()->GetSuperFrameDuration (SatConstVariables::SUPERFRAME_SEQUENCE);

  double normalizedOfferedLoad = (m_randomAccessBitsInFrame / superFrameDuration.GetSeconds ()) / m_rxBandwidthHz;

  NS_LOG_INFO ("Bits: " << m_randomAccessBitsInFrame <<
               ", frame length in seconds: " << superFrameDuration.GetSeconds () <<
               ", bandwidth in Hz: " << m_rxBandwidthHz <<
               ", normalized offered load (bps/Hz): " << normalizedOfferedLoad);

  /// reset the counter
  m_randomAccessBitsInFrame = 0;

  return normalizedOfferedLoad;
}

void
SatPhyRxCarrierPerFrame::AddCrdsaPacket (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s crdsaPacketParams)
{
  NS_LOG_FUNCTION (this);

  if (crdsaPacketParams.rxParams->m_packetsInBurst.size () > 0)
    {
      SatCrdsaReplicaTag replicaTag;

      /// check the first packet for tag
      bool result = crdsaPacketParams.rxParams->m_packetsInBurst[0]->PeekPacketTag (replicaTag);

      if (!result)
        {
          NS_FATAL_ERROR ("First packet did not contain a CRDSA replica tag");
        }

      std::vector<uint16_t> slotIds = replicaTag.GetSlotIds ();

      if (slotIds.size () < 1)
        {
          NS_FATAL_ERROR ("The tag did not contain any slot IDs");
        }

      /// the first slot ID is this replicas own slot ID
      crdsaPacketParams.ownSlotId = slotIds[0];

      if (crdsaPacketParams.slotIdsForOtherReplicas.size () > 0)
        {
          NS_FATAL_ERROR ("Vector for packet replicas should be empty at this point");
        }

      /// rest of the slot IDs are for the replicas
      for (uint32_t i = 1; i < slotIds.size (); i++)
        {
          crdsaPacketParams.slotIdsForOtherReplicas.push_back (slotIds[i]);
        }

      /// tags are not needed after this
      for (uint32_t i = 0; i < crdsaPacketParams.rxParams->m_packetsInBurst.size (); i++)
        {
          crdsaPacketParams.rxParams->m_packetsInBurst[i]->RemovePacketTag (replicaTag);
        }
    }
  else
    {
      NS_FATAL_ERROR ("CRDSA reception with 0 packets");
    }

  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator result;

  result = m_crdsaPacketContainer.find (crdsaPacketParams.ownSlotId);

  if (result == m_crdsaPacketContainer.end ())
    {
      std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> tempList;
      tempList.push_back (crdsaPacketParams);
      m_crdsaPacketContainer.insert (std::make_pair (crdsaPacketParams.ownSlotId, tempList));
    }
  else
    {
      result->second.push_back (crdsaPacketParams);
    }

  NS_LOG_INFO ("Packet in slot " << crdsaPacketParams.ownSlotId << " was added to the CRDSA packet container");

  for (uint32_t i = 0; i < crdsaPacketParams.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("A replica of the packet is in slot " << crdsaPacketParams.slotIdsForOtherReplicas[i]);
    }
}

std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>
SatPhyRxCarrierPerFrame::ProcessFrame ()
{
  NS_LOG_FUNCTION (this);

  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter;
  std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> combinedPacketsForFrame;

  // Perform SIC in its entirety, until no more packets can be decoded
  PerformSicCycles (combinedPacketsForFrame);

  NS_LOG_INFO ("All successfully received packets processed, packets left in container: " << m_crdsaPacketContainer.size ());

  // Cleanup: remove remaining packets from the receive container
  // and add them to the resulting vector by ensuring that their
  // phyError is set to true.
  do
    {
      /// go through the packets
      iter = m_crdsaPacketContainer.begin ();

      if (iter != m_crdsaPacketContainer.end ())
        {
          std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iterList = iter->second.begin ();

          if (iterList != iter->second.end ())
            {
              NS_LOG_INFO ("Processing unsuccessfully received packet in slot: " << iterList->ownSlotId <<
                           " packet phy error: " << iterList->phyError <<
                           " packet has been processed: " << iterList->packetHasBeenProcessed);

              if (!iterList->packetHasBeenProcessed || !iterList->phyError)
                {
                  NS_FATAL_ERROR ("All successfully received packets should have been processed by now");
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
          else
            {
              /// remove the empty slot container
              m_crdsaPacketContainer.erase (iter);
            }
        }
    }
  while (!m_crdsaPacketContainer.empty ());

  NS_LOG_INFO ("Container processed, packets left: " << m_crdsaPacketContainer.size ());

  return combinedPacketsForFrame;
}

void
SatPhyRxCarrierPerFrame::PerformSicCycles (
  std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>& combinedPacketsForFrame)
{
  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter;
  SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s processedPacket;
  bool nothingToProcess = true;

  NS_LOG_INFO ("Packets to process: " << m_crdsaPacketContainer.size ());

  do
    {
      NS_LOG_INFO ("Searching for the next successfully received packet");

      /// reset the flag
      nothingToProcess = true;

      /// go through the packets
      for (iter = m_crdsaPacketContainer.begin (); iter != m_crdsaPacketContainer.end (); ++iter)
        {
          NS_LOG_INFO ("Iterating slot: " << iter->first);
          std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>& slotContent = iter->second;

          if (slotContent.size () < 1)
            {
              NS_FATAL_ERROR ("No packet in slot! This should not happen");
            }

          std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator currentPacket;

          for (currentPacket = slotContent.begin (); currentPacket != slotContent.end (); currentPacket++)
            {
              NS_LOG_INFO ("Iterating packet in slot: " << currentPacket->ownSlotId);

              if (!currentPacket->packetHasBeenProcessed)
                {
                  NS_LOG_INFO ("Found a packet ready for processing");

                  /// process the received packet
                  *currentPacket = ProcessReceivedCrdsaPacket (*currentPacket, slotContent.size ());

                  NS_LOG_INFO ("Packet error: " << currentPacket->phyError);

                  /// packet successfully received
                  if (!currentPacket->phyError)
                    {
                      NS_LOG_INFO ("Packet successfully received, breaking the slot iteration");

                      nothingToProcess = false;

                      /// save packet for processing outside the loop
                      processedPacket = *currentPacket;

                      /// remove the successfully received packet from the container
                      slotContent.erase (currentPacket);

                      /// eliminate the interference caused by this packet to other packets in this slot
                      EliminateInterference (iter, processedPacket);

                      /// break the cycle
                      break;
                    }
                }
              else
                {
                  NS_LOG_INFO ("This packet has already been processed");
                }
            }

          /// successfully received packet found
          if (!nothingToProcess)
            {
              NS_LOG_INFO ("Packet successfully received, breaking the container iteration");
              break;
            }
        }

      if (!nothingToProcess)
        {
          NS_LOG_INFO ("Packet successfully received, processing the replicas");

          /// find and remove replicas of the received packet
          FindAndRemoveReplicas (processedPacket);

          /// save the the received packet
          combinedPacketsForFrame.push_back (processedPacket);
        }
    }
  while (!nothingToProcess);
}

SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s
SatPhyRxCarrierPerFrame::ProcessReceivedCrdsaPacket (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s packet,
                                                     uint32_t numOfPacketsForThisSlot)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Processing a packet in slot: " << packet.ownSlotId <<
               " number of packets in this slot: " << numOfPacketsForThisSlot);

  for (uint32_t i = 0; i < packet.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("Replica in slot: " << packet.slotIdsForOtherReplicas[i]);
    }

  NS_LOG_INFO ("SINR CALCULATION, RX sat: " << packet.rxParams->GetRxPowerInSatellite () <<
               " IF sat: " << packet.rxParams->GetInterferencePowerInSatellite () <<
               " RX gnd: " << packet.rxParams->m_rxPower_W <<
               " IF gnd: " << packet.rxParams->GetInterferencePower ());

  double sinr = CalculatePacketCompositeSinr (packet);

  /*
   * Update link specific SINR trace for the RETURN_FEEDER link. The RETURN_USER
   * link SINR is already updated at the SatPhyRxCarrierUplink::EndRxData ()
   * method!
   */
  m_linkSinrTrace (SatUtils::LinearToDb (sinr));

  if (GetRandomAccessCollisionModel () == SatPhyRxCarrierConf::RA_COLLISION_ALWAYS_DROP_ALL_COLLIDING_PACKETS)
    {
      NS_LOG_INFO ("Strict collision detection is ENABLED");

      /// there is a collision
      if (numOfPacketsForThisSlot > 1)
        {
          NS_LOG_INFO ("Multiple packets in this slot, successful reception is not possible");
          /// not possible to have a successful reception
          packet.phyError = true;
        }
      else
        {
          NS_LOG_INFO ("Only packet in this slot, checking against link results");
          /// check against link results
          packet.phyError = CheckAgainstLinkResults (packet.cSinr, packet.rxParams);
        }
      NS_LOG_INFO ("Strict collision detection, phy error: " << packet.phyError);
    }
  else if (GetRandomAccessCollisionModel () == SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR)
    {
      /// check against link results
      packet.phyError = CheckAgainstLinkResults (packet.cSinr, packet.rxParams);
      NS_LOG_INFO ("Check against link results, phy error: " << packet.phyError);
    }
  else
    {
      NS_FATAL_ERROR ("Random access collision model not defined");
    }

  /// mark the packet as processed
  packet.packetHasBeenProcessed = true;

  return packet;
}

double
SatPhyRxCarrierPerFrame::CalculatePacketCompositeSinr (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s& packet)
{
  NS_LOG_FUNCTION (this);

  double sinrSatellite = CalculateSinr ( packet.rxParams->GetRxPowerInSatellite (),
                                         packet.rxParams->GetInterferencePowerInSatellite (),
                                         packet.rxParams->GetRxNoisePowerInSatellite (),
                                         packet.rxParams->GetRxAciIfPowerInSatellite (),
                                         packet.rxParams->GetRxExtNoisePowerInSatellite (),
                                         packet.rxParams->GetSinrCalculator ());

  double sinr = CalculateSinr ( packet.rxParams->m_rxPower_W,
                                packet.rxParams->GetInterferencePower (),
                                m_rxNoisePowerW,
                                m_rxAciIfPowerW,
                                m_rxExtNoisePowerW,
                                m_sinrCalculate);

  double cSinr = CalculateCompositeSinr (sinr, sinrSatellite);

  NS_LOG_INFO ("Computed cSINR for packet: " << cSinr);

  packet.cSinr = cSinr;
  packet.ifPower = packet.rxParams->GetInterferencePower ();

  return sinr;
}

void
SatPhyRxCarrierPerFrame::FindAndRemoveReplicas (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s packet)
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < packet.slotIdsForOtherReplicas.size (); i++)
    {
      NS_LOG_INFO ("Processing replica in slot: " << packet.slotIdsForOtherReplicas[i]);

      /// get the vector of packets for processing
      std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter;
      iter = m_crdsaPacketContainer.find (packet.slotIdsForOtherReplicas[i]);

      if (iter == m_crdsaPacketContainer.end ())
        {
          NS_FATAL_ERROR ("This should not happen");
        }

      std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iterList;
      SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s removedPacket;

      bool replicaFound = false;

      for (iterList = iter->second.begin (); iterList != iter->second.end (); )
        {
          /// check for the same UT & same slots
          if (IsReplica (packet, *iterList))
            {
              if (replicaFound)
                {
                  NS_FATAL_ERROR ("Found two replica of the same packet in the same slot");
                }
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
          NS_FATAL_ERROR ("Replica not found");
        }

      if (!packet.phyError)
        {
          CalculatePacketCompositeSinr (removedPacket);
          EliminateInterference (iter, removedPacket);
        }
    }
}

void
SatPhyRxCarrierPerFrame::EliminateInterference (
  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter,
  SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s processedPacket)
{
  NS_LOG_FUNCTION (this);

  if (iter->second.empty ())
    {
      NS_LOG_INFO ("No other packets in this slot, erasing the slot container");
      m_crdsaPacketContainer.erase (iter);
    }
  else
    {
      std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iterList;

      for (iterList = iter->second.begin (); iterList != iter->second.end (); iterList++)
        {
          /// release packets in this slot for re-processing
          iterList->packetHasBeenProcessed = false;

          NS_LOG_INFO ("BEFORE INTERFERENCE ELIMINATION, RX sat: " <<
                       iterList->rxParams->GetRxPowerInSatellite () <<
                       " IF sat: " << iterList->rxParams->GetInterferencePowerInSatellite () <<
                       " RX gnd: " << iterList->rxParams->m_rxPower_W <<
                       " IF gnd: " << iterList->rxParams->GetInterferencePower ());

          /// Reduce interference power for the colliding packets. Note, that the interference is
          /// eliminated only from the user link interference power at the satellite! The intra-beam
          /// interference is not handled in the return feeder link so that the intra-beam interference
          /// is not taken into account twice!
          /// TODO A more novel way to eliminate partially overlapping interference should be considered!
          /// In addition, as the interference values are extremely small, the use of long double (instead
          /// of double) should be considered to improve the accuracy.

          if (iterList->rxParams->GetInterferencePower () < 0)
            {
              NS_FATAL_ERROR ("Negative interference");
            }

          GetInterferenceEliminationModel ()->EliminateInterferences (iterList->rxParams, processedPacket.rxParams, processedPacket.cSinr);

          NS_LOG_INFO ("AFTER INTERFERENCE ELIMINATION, RX sat: " <<
                       iterList->rxParams->GetRxPowerInSatellite () <<
                       " IF sat: " << iterList->rxParams->GetInterferencePowerInSatellite () <<
                       " RX gnd: " << iterList->rxParams->m_rxPower_W <<
                       " IF gnd: " << iterList->rxParams->GetInterferencePower ());
        }
    }
}

bool
SatPhyRxCarrierPerFrame::IsReplica (const SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s& packet,
                                    const SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s& otherPacket) const
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Checking the source addresses");

  bool isReplica = false;

  if (otherPacket.sourceAddress == packet.sourceAddress)
    {
      NS_LOG_INFO ("Same source addresses, checking slot IDs");

      if (HaveSameSlotIds (packet, otherPacket))
        {
          NS_LOG_INFO ("Same slot IDs, replica found");
          isReplica = true;
        }
    }
  return isReplica;
}

bool
SatPhyRxCarrierPerFrame::HaveSameSlotIds (const SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s& packet,
                                          const SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s& otherPacket) const
{
  NS_LOG_FUNCTION (this);

  std::set<uint16_t> firstSet;
  std::set<uint16_t> secondSet;
  std::set<uint16_t>::iterator firstSetIterator;
  std::set<uint16_t>::iterator secondSetIterator;
  bool haveSameSlotIds = true;

  firstSet.insert (packet.ownSlotId);
  secondSet.insert (otherPacket.ownSlotId);

  /// sanity check
  if (otherPacket.slotIdsForOtherReplicas.size () != packet.slotIdsForOtherReplicas.size ())
    {
      NS_FATAL_ERROR ("SatPhyRxCarrierUt::HaveSameSlotIds - The amount of replicas does not match");
    }

  NS_LOG_INFO ("Comparing slot IDs");

  /// form sets
  for (uint32_t i = 0; i < otherPacket.slotIdsForOtherReplicas.size (); i++)
    {
      firstSet.insert (packet.slotIdsForOtherReplicas[i]);
      secondSet.insert (otherPacket.slotIdsForOtherReplicas[i]);
    }

  uint32_t numOfMatchingSlots = 0;

  /// compare sets
  for (firstSetIterator = firstSet.begin (); firstSetIterator != firstSet.end (); firstSetIterator++)
    {
      secondSetIterator = secondSet.find (*firstSetIterator);

      if (secondSetIterator == secondSet.end ())
        {
          haveSameSlotIds = false;
        }
      else
        {
          numOfMatchingSlots++;
        }
    }

  NS_LOG_INFO ("Are slot IDs identical: " << haveSameSlotIds <<
               ", number of matching slots: " << numOfMatchingSlots);

  /// sanity check
  if (!haveSameSlotIds && numOfMatchingSlots > 0)
    {
      NS_FATAL_ERROR ("Partially overlapping CRDSA slots");
    }

  return haveSameSlotIds;
}

bool
SatPhyRxCarrierPerFrame::CompareCrdsaPacketId (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s obj1,
                                               SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s obj2)
{
  return (bool) (obj1.rxParams->m_txInfo.crdsaUniquePacketId < obj2.rxParams->m_txInfo.crdsaUniquePacketId);
}


}
