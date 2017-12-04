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


#ifndef SATELLITE_PHY_RX_CARRIER_PER_SLOT_H
#define SATELLITE_PHY_RX_CARRIER_PER_SLOT_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include <ns3/mac48-address.h>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <ns3/satellite-phy-rx-carrier.h>

namespace ns3 {

class SatPhyRxCarrier;

/**
 * \ingroup satellite
 * \brief Inherited the base functionality of SatPhyRxCarriers for ground
 *        station nodes.
 */
class SatPhyRxCarrierPerSlot : public SatPhyRxCarrier
{
public:

	/**
	 * Constructor.
	 * \param carrierId ID of the carrier
	 * \param carrierConf Carrier configuration
	 */
	SatPhyRxCarrierPerSlot (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, bool randomAccessEnabled);

  /**
   * \brief Destructor
   */
  virtual ~SatPhyRxCarrierPerSlot ();

  /**
   * Get the TypeId of the class.
   * \return TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for setting the random access allocation channel ID
   * \param randomAccessAllocationChannelId
   */
  inline void SetRandomAccessAllocationChannelId (uint8_t randomAccessAllocationChannelId) { m_randomAccessAllocationChannelId = randomAccessAllocationChannelId; };

  /**
   * \brief Function for getting the random access allocation channel ID
   * \return randomAccessAllocationChannelId
   */
  inline uint8_t GetRandomAccessAllocationChannelId () const { return m_randomAccessAllocationChannelId; };

  /**
   * \brief Method for querying the type of the carrier
   */
  inline virtual CarrierType GetCarrierType ()
  {
  	return m_randomAccessEnabled ? CarrierType::RA_SLOTTED_ALOHA : CarrierType::DEDICATED_ACCESS;
  }

protected:

  /**
   * \brief Function for ending the packet reception from the SatChannel
   * \param key Key for Rx params map
   */
	virtual void EndRxData (uint32_t key);

	/**
	 * \brief Dispose implementation.
	 */
	virtual void DoDispose ();

	/**
	 * Receive a slot. Overridable by child classes.
	 */
	virtual void ReceiveSlot (SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets);

	/**
	 * \brief Get the random access collision model of the carrier
	 */
	inline SatPhyRxCarrierConf::RandomAccessCollisionModel GetRandomAccessCollisionModel () const
	{
		return m_randomAccessCollisionModel;
	};

	/**
	 * \brief Check if random access dynamic load control is enabled.
	 */
	inline bool IsRandomAccessDynamicLoadControlEnabled () const { return m_enableRandomAccessDynamicLoadControl; };

  /**
   * \brief Function for calculating the average normalized offered random access load
   * \return Average normalized offered load
   */
  double CalculateAverageNormalizedOfferedRandomAccessLoad ();

  /**
   * \brief Function for saving the measured random access load
   * \param measuredRandomAccessLoad Measured random access load
   */
  void SaveMeasuredRandomAccessLoad (double measuredRandomAccessLoad);

  /**
   * \brief Create an interference event based on Rx parameters and address.
   *
   * \return Pointer to the interference event.
   */
	virtual Ptr<SatInterference::InterferenceChangeEvent>
	  CreateInterference (Ptr<SatSignalParameters> rxParams, Address rxAddress);

	/**
	 * \brief The number of random access bits in current frame.
	 * 				Protected so that child classes are able to alter this.
	 */
	uint32_t m_randomAccessBitsInFrame;

	////////// TRACED CALLBACKS /////////
  /**
   * \brief `SlottedAlohaRxCollision` trace source.
   *
   * Fired when a packet burst is received through Random Access Slotted ALOHA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a collision has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_slottedAlohaRxCollisionTrace;

  /**
   * \brief `SlottedAlohaRxError` trace source.
   *
   * Fired when a packet burst is received through Random Access Slotted ALOHA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a PHY error has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_slottedAlohaRxErrorTrace;

private:

  /**
   * \brief Function for processing the Slotted ALOHA collisions
   * \param cSinr Composite SINR
   * \param rxParams Rx parameters of the packet
   * \param interferenceEvent Interference event details
   * \return PHY error
   */
  bool ProcessSlottedAlohaCollisions (double cSinr,
                                      Ptr<SatSignalParameters> rxParams,
                                      Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent);

  /// PRIVATE MEMBER VARIABLES

	uint8_t m_randomAccessAllocationChannelId; 		//< Random access allocation ID
  SatPhyRxCarrierConf::RandomAccessCollisionModel m_randomAccessCollisionModel;	//< Random access collision model
  double m_randomAccessConstantErrorRate; 			//< Constant error rate if collision model is RA_CONSTANT_COLLISION_PROBABILITY
  uint32_t m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize;	//< Random access average normalized offered load measurement window size
  bool m_enableRandomAccessDynamicLoadControl;	//< Is random access dynamic load control enabled
  std::deque<double> m_randomAccessDynamicLoadControlNormalizedOfferedLoad; //< Container for calculated normalized offered loads


};

}

#endif /* SATELLITE_PHY_RX_CARRIER_PER_SLOT_H */
