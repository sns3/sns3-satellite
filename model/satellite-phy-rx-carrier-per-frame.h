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

#ifndef SATELLITE_PHY_RX_CARRIER_PER_FRAME_H
#define SATELLITE_PHY_RX_CARRIER_PER_FRAME_H

#include <ns3/singleton.h>
#include <ns3/satellite-rtn-link-time.h>
#include <ns3/satellite-crdsa-replica-tag.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-phy-rx-carrier-per-slot.h>

namespace ns3 {

class Address;
class SatPhy;
class SatSignalParameters;
class SatLinkResults;
class SatChannelEstimationErrorContainer;
class SatNodeInfo;
class SatPhyRxCarrier;
class SatPhyRxCarrierPerSlot;
class SatCrdsaReplicaTag;

/**
 * \ingroup satellite
 * \brief Inherited the functionality of ground station SatPhyRxCarriers
 * 				and extended it with CRDSA functionality.
 */
class SatPhyRxCarrierPerFrame : public SatPhyRxCarrierPerSlot
{
public:
  /**
   * \brief Struct for storing the CRDSA packet specific Rx parameters
   */
  typedef struct
  {
    Ptr<SatSignalParameters> rxParams;
    Mac48Address destAddress;
    Mac48Address sourceAddress;
    uint16_t ownSlotId;
    std::vector<uint16_t> slotIdsForOtherReplicas;
    bool hasCollision;
    bool packetHasBeenProcessed;
    double cSinr;
    double ifPower;
    bool phyError;
  } crdsaPacketRxParams_s;

	/**
	 * Constructor.
	 * \param carrierId ID of the carrier
	 * \param carrierConf Carrier configuration
	 */
	SatPhyRxCarrierPerFrame (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, bool randomAccessEnabled);

  /**
   * \brief Destructor
   */
  virtual ~SatPhyRxCarrierPerFrame ();

  /**
   * Get the TypeId of the class.
   * \return TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for comparing the CRDSA unique packet IDs
   * \param obj1 Comparison object 1
   * \param obj2 Comparison object 2
   * \return Comparison result
   */
  static bool CompareCrdsaPacketId (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s obj1,
  		SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s obj2);

  /**
   * \brief Function for initializing the frame end scheduling
   */
  void BeginFrameEndScheduling ();

  /**
   * \brief Method for querying the type of the carrier
   */
  inline virtual CarrierType GetCarrierType ()
  {
  	return CarrierType::RA_CRDSA;
  }

protected:

	/**
	 * Receive a slot.
	 */
  virtual void ReceiveSlot (SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets);

	/**
	 * \brief Dispose implementation
	 */
  virtual void DoDispose ();

private:

  /**
   * \brief Function for eliminating the interference to other packets in the slot from the correctly received packet
   * \param iter Packets in the slot
   * \param processedPacket Correctly received processed packet
   */
  void EliminateInterference (std::map<uint32_t,std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> >::iterator iter,
  		SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s processedPacket);

  /**
   * \brief Function for storing the received CRDSA packets
   * \param Rx parameters of the packet
   */
  void AddCrdsaPacket (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s crdsaPacketParams);

  /**
   * \brief `CrdsaReplicaRx` trace source.
   *
   * Fired when a CRDSA packet replica is received through Random Access CRDSA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a collision has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_crdsaReplicaRxTrace;

  /**
   * \brief `CrdsaUniquePayloadRx` trace source.
   *
   * Fired when a unique CRDSA payload is received (after frame processing)
   * through Random Access CRDSA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a PHY error has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_crdsaUniquePayloadRxTrace;

  /**
   * \brief Function for processing the CRDSA frame
   * \return Processed packets
   */
  std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> ProcessFrame ();

  /**
   * \brief Function for finding and removing the replicas of the CRDSA packet
   * \param packet CRDSA packet
   */
  void FindAndRemoveReplicas (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s packet);

  /**
   * \brief Function for identifying whether the packet is a replica of another packet
   * \param packet Packet
   * \param iter A packet in certain slot
   * \return Is the packet a replica
   */
  bool IsReplica (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s packet,
  		std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iter);

  /**
   * \brief Function for checking do the packets have identical slots
   * \param packet Packet
   * \param iter A packet in certain slot
   * \return Have the packets identical slots
   */
  bool HaveSameSlotIds (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s packet,
  		std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>::iterator iter);

  /**
   * \brief Function for calculating the normalized offered random access load
   * \return Normalized offered load
   */
  double CalculateNormalizedOfferedRandomAccessLoad ();

  /**
   * \brief Function for processing the frame interval operations
   */
  void DoFrameEnd ();

  /**
   * \brief Function for measuring the random access load
   */
  void MeasureRandomAccessLoad ();

  /**
   * Update the random access load for CRDSA. Count only the
   * received unique payloads.
   */
  void UpdateRandomAccessLoad ();

  /**
   * Process received CRDSA packet.
   */
  SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s
	ProcessReceivedCrdsaPacket (SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s packet,
                              uint32_t numOfPacketsForThisSlot);


  /**
   * \brief CRDSA packet container
   */
  std::map<uint32_t, std::list<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s> > m_crdsaPacketContainer;

  /**
   * \brief Has the frame end scheduling been initialized
   */
  bool m_frameEndSchedulingInitialized;
};


//////////////////////////////////////////////////////////

}

#endif /* SATELLITE_PHY_RX_CARRIER_PER_FRAME_H */
