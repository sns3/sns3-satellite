/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef LORA_FORWARDER_H
#define LORA_FORWARDER_H

#include <map>

#include <ns3/application.h>
#include <ns3/nstime.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/attribute.h>

#include "satellite-lorawan-net-device.h"


namespace ns3 {

/**
 * This application forwards packets between NetDevices:
 * SatLorawanNetDevice -> PointToPointNetDevice and vice versa.
 */
class LoraForwarder : public Application
{
public:
  LoraForwarder ();
  ~LoraForwarder ();

  static TypeId GetTypeId (void);

  /**
   * Sets the device to use to communicate with the EDs.
   *
   * \param beamId The beam ID of the device.
   * \param loraNetDevice The LoraNetDevice on this node.
   */
  void SetLoraNetDevice (uint8_t beamId, Ptr<SatLorawanNetDevice> loraNetDevice);

  /**
   * Sets the P2P device to use to communicate with the NS.
   *
   * \param pointToPointNetDevice The P2PNetDevice on this node.
   */
  void SetPointToPointNetDevice (Ptr<PointToPointNetDevice> pointToPointNetDevice);

  /**
   * Receive a packet from the LoraNetDevice.
   *
   * \param loraNetDevice The LoraNetDevice we received the packet from.
   * \param packet The packet we received.
   * \param protocol The protocol number associated to this packet.
   * \param sender The address of the sender.
   * \returns True if we can handle the packet, false otherwise.
   */
  bool ReceiveFromLora (Ptr<SatLorawanNetDevice> loraNetDevice, Ptr<const Packet> packet, uint16_t protocol, const Address& sender);

  /**
   * Receive a packet from the PointToPointNetDevice
   */
  bool ReceiveFromPointToPoint (Ptr<NetDevice> pointToPointNetDevice, Ptr<const Packet> packet, uint16_t protocol, const Address& sender);

  /**
   * Start the application
   */
  void StartApplication (void);

  /**
   * Stop the application
   */
  void StopApplication (void);

private:
  std::map<uint8_t, Ptr<SatLorawanNetDevice> > m_satLorawanNetDevices; //!< Map between beam ID and pointer to the node's SatLorawanNetDevice

  Ptr<PointToPointNetDevice> m_pointToPointNetDevice; //!< Pointer to the
  //!P2PNetDevice we use to
  //!communicate with the NS
};

} //namespace ns3

#endif /* LORA_FORWARDER_H */
