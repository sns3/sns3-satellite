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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_PHY_RX_CARRIER_MARSALA_H
#define SATELLITE_PHY_RX_CARRIER_MARSALA_H

#include <ns3/satellite-phy-rx-carrier-per-frame.h>

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Bite
 */
class SatPhyRxCarrierMarsala : public SatPhyRxCarrierPerFrame
{
public:
  /**
   * \brief Constructor.
   * \param carrierId ID of the carrier
   * \param carrierConf Carrier configuration
   * \param waveformConf Waveform configuration
   * \param randomAccessEnabled Is this a RA carrier
   */
  SatPhyRxCarrierMarsala (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatWaveformConf> waveformConf, bool randomAccessEnabled);

  /**
   * \brief Destructor
   */
  ~SatPhyRxCarrierMarsala ();

  /**
   * Get the TypeId of the class.
   * \return TypeId
   */
  static TypeId GetTypeId (void);

protected:
  /**
   * \brief Function for receiving decodable packets and removing their
   * interference from the other packets in the slots they’re in; perform
   * as many cycles as needed to try to decode each packet.
   * \param combinedPacketsForFrame  container to store packets
   * as they are decoded and removed from the frame
   */
  void PerformSicCycles (std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>& combinedPacketsForFrame);

private:
  /**
   * \brief Function for performing MARSALA corelation on remaining packets in the frame
   * \param combinedPacketsForFrame  container to store packets as they are decoded and removed from the frame
   * \return Whether or not a packet was decoded
   */
  bool PerformMarsala (std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>& combinedPacketsForFrame);
};

}  // namespace ns3

#endif /* SATELLITE_PHY_RX_CARRIER_MARSALA_H */
