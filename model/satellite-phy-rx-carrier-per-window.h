/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Joaquin Muguerza <jmuguerza@toulouse.viveris.fr>
 */

#ifndef SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H
#define SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H

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
 *                              and extended it with CRDSA functionality.
 */
class SatPhyRxCarrierPerWindow : public SatPhyRxCarrierPerSlot
{
public:
  /**
   * \brief Struct for storing the E-SSA packet specific Rx parameters
   */
  typedef struct
  {
    Ptr<SatSignalParameters> rxParams;
    Mac48Address destAddress;
    Mac48Address sourceAddress;
    bool packetHasBeenProcessed;
    double meanSinr;
    double preambleMeanSinr;
    std::vector< std::pair<double, double> > gamma;
    bool phyError;
    Time arrivalTime;
    Time duration;
  } essaPacketRxParams_s;

  /**
   * Constructor.
   * \param carrierId ID of the carrier
   * \param carrierConf Carrier configuration
   * \param waveformConf Waveform configuration
   * \param randomAccessEnabled Is this a RA carrier
   */
  SatPhyRxCarrierPerWindow (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatWaveformConf> waveformConf, bool randomAccessEnabled);

  /**
   * \brief Destructor
   */
  virtual ~SatPhyRxCarrierPerWindow ();

  /**
   * Get the TypeId of the class.
   * \return TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for initializing the window end scheduling
   */
  void BeginEndScheduling ();

  /**
   * \brief Method for querying the type of the carrier
   */
  inline virtual CarrierType GetCarrierType ()
  {
    return CarrierType::RA_ESSA;
  }

protected:
  /**
   * Receive a slot.
   */
  virtual void ReceiveSlot (SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets);

  ///////// TRACED CALLBACKS /////////
  /**
   * \brief `EssaRxCollision` trace source.
   *
   * Fired when a packet is received through Random Access ESSA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet.
   * - the MAC48 address of the sender; and
   * - whether a collision has occured.
   */
  TracedCallback<uint32_t, const Address &, bool> m_essaRxCollisionTrace;

  /**
   * \brief Dispose implementation
   */
  virtual void DoDispose ();

private:
  typedef std::list<SatPhyRxCarrierPerWindow::essaPacketRxParams_s> packetList_t;
  /**
   * \brief Function for storing the received E-SSA packets
   */
  void AddEssaPacket (SatPhyRxCarrierPerWindow::essaPacketRxParams_s essaPacketParams);

  /**
   * \brief Calculate gamma and Interference vectors for a single packet
   */
  void CalculatePacketInterferenceVectors (SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet);

  /**
   * \brief Remove old packets from packet container
   */
  void CleanOldPackets (const Time windowStartTime);

  /**
   * \brief Function called when a window ends
   */
  void DoWindowEnd ();

  /**
   * \brief Get the effective SNIR of the packet using the Mutual Information function
   */
  double GetEffectiveSnir (const SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet);

  /**
   * \brief Get the packet with the highst SNIR on the list
   * \return True if a packet is returned, False otherwise
   */
  packetList_t::iterator GetHighestSnirPacket (const std::pair<packetList_t::iterator, packetList_t::iterator> windowBounds);

  /**
   * \brief Get a pair of iterators, pointing to the first element in the window, and to the first after the window
   */
  std::pair<packetList_t::iterator, packetList_t::iterator> GetWindowBounds ();

  /**
   * \brief Function for processing a window
   */
  bool PacketCanBeDetected (const SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet);

  /**
   * \brief Function for processing a window
   */
  void ProcessWindow (Time startTime, Time endTime);

  /**
   * The duration of the sliding window
   */
  Time m_windowDuration;

  /**
   * The step of the sliding window
   */
  Time m_windowStep;

  /**
   * The number of SIC iterations performed on each window
   */
  uint32_t m_windowSicIterations;

  /**
   * \brief Has the window end scheduling been initialized
   */
  bool m_windowEndSchedulingInitialized;

  /**
   * \brief SNIR detection threshold for a packet (in magnitude)
   */
  // TODO: find a more appropiate place for this parameter
  double m_detectionThreshold;

  /**
   * \brief ESSA packet container
   */
  packetList_t m_essaPacketContainer;
};


//////////////////////////////////////////////////////////

}

#endif /* SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H */
