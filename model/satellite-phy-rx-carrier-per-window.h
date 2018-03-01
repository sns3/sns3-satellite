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
};


//////////////////////////////////////////////////////////

}

#endif /* SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H */
