/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Padova
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
 * Authors: Davide Magrin <magrinda@dei.unipd.it>
 *          Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef LORA_NETWORK_SERVER_H
#define LORA_NETWORK_SERVER_H

#include "lora-device-address.h"
#include "lora-gateway-status.h"
#include "lora-network-controller.h"
#include "lora-network-scheduler.h"
#include "lora-network-status.h"
#include "lorawan-mac-end-device-class-a.h"

#include <ns3/application.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/point-to-point-net-device.h>

namespace ns3
{

/**
 * The LoraNetworkServer is an application standing on top of a node equipped with
 * links that connect it with the gateways.
 *
 * This version of the LoraNetworkServer attempts to closely mimic an actual
 * Network Server, by providing as much functionality as possible.
 */
class LoraNetworkServer : public Application
{
  public:
    static TypeId GetTypeId(void);

    LoraNetworkServer();
    virtual ~LoraNetworkServer();

    /**
     * Start the NS application.
     */
    void StartApplication(void);

    /**
     * Stop the NS application.
     */
    void StopApplication(void);

    /**
     * Inform the LoraNetworkServer that these nodes are connected to the network.
     *
     * This method will create a DeviceStatus object for each new node, and add
     * it to the list.
     */
    void AddNodes(NodeContainer nodes);

    /**
     * Inform the LoraNetworkServer that this node is connected to the network.
     * This method will create a DeviceStatus object for the new node (if it
     * doesn't already exist).
     */
    void AddNode(Ptr<Node> node);

    /**
     * Add this gateway to the list of gateways connected to this NS.
     * Each GW is identified by its Address in the NS-GWs network.
     */
    void AddGateway(Ptr<Node> gateway, Ptr<NetDevice> netDevice);

    /**
     * A NetworkControllerComponent to this LoraNetworkServer instance.
     */
    void AddComponent(Ptr<LoraNetworkControllerComponent> component);

    /**
     * Receive a packet from a gateway.
     * \param packet the received packet
     */
    bool Receive(Ptr<NetDevice> device,
                 Ptr<const Packet> packet,
                 uint16_t protocol,
                 const Address& address);

    Ptr<LoraNetworkStatus> GetNetworkStatus(void);

  protected:
    Ptr<LoraNetworkStatus> m_status;
    Ptr<LoraNetworkController> m_controller;
    Ptr<LoraNetworkScheduler> m_scheduler;

    TracedCallback<Ptr<const Packet>> m_receivedPacket;
};

} // namespace ns3
#endif /* LORA_NETWORK_SERVER_H */
