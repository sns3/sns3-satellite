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

#include "ns3/point-to-point-net-device.h"
#include "ns3/packet.h"
#include "ns3/node-container.h"
#include "ns3/net-device.h"

#include "ns3/satellite-lorawan-net-device.h"

#include "ns3/lora-network-server.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-network-status.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lorawan-mac-command.h"
#include "ns3/lorawan-mac-end-device-class-a.h"
#include "ns3/lorawan-mac-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraNetworkServer");

NS_OBJECT_ENSURE_REGISTERED (LoraNetworkServer);

TypeId
LoraNetworkServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraNetworkServer")
    .SetParent<Application> ()
    .AddConstructor<LoraNetworkServer> ()
    .AddTraceSource ("ReceivedPacket",
                     "Trace source that is fired when a packet arrives at the Network Server",
                     MakeTraceSourceAccessor (&LoraNetworkServer::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .SetGroupName ("lorawan");
  return tid;
}

LoraNetworkServer::LoraNetworkServer () :
  m_status (Create<LoraNetworkStatus> ()),
  m_controller (Create<LoraNetworkController> (m_status)),
  m_scheduler (Create<LoraNetworkScheduler> (m_status, m_controller))
{
  NS_LOG_FUNCTION_NOARGS ();
}

LoraNetworkServer::~LoraNetworkServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LoraNetworkServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LoraNetworkServer::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LoraNetworkServer::AddGateway (Ptr<Node> gateway, Ptr<NetDevice> netDevice)
{
  NS_LOG_FUNCTION (this << gateway);

  // Get the PointToPointNetDevice
  Ptr<PointToPointNetDevice> p2pNetDevice;
  for (uint32_t i = 0; i < gateway->GetNDevices (); i++)
    {
      p2pNetDevice = gateway->GetDevice (i)->GetObject<PointToPointNetDevice> ();
      if (p2pNetDevice != 0)
        {
          // We found a p2pNetDevice on the gateway
          break;
        }
    }

  // Get the gateway's LoRa MAC layer (assumes gateway's MAC is configured as first device)
  Ptr<SatLorawanNetDevice> satLoraNetDevice = DynamicCast<SatLorawanNetDevice> (gateway->GetDevice (1));
  Ptr<LorawanMacGateway> gwMac = DynamicCast<LorawanMacGateway> (satLoraNetDevice->GetMac ());
  NS_ASSERT (gwMac != 0);

  // Get the Address
  Address gatewayAddress = p2pNetDevice->GetAddress ();

  // Create new gatewayStatus
  Ptr<LoraGatewayStatus> gwStatus = Create<LoraGatewayStatus> (gatewayAddress,
                                                       netDevice,
                                                       gwMac);

  m_status->AddGateway (gatewayAddress, gwStatus);
}

void
LoraNetworkServer::AddNodes (NodeContainer nodes)
{
  NS_LOG_FUNCTION_NOARGS ();

  // For each node in the container, call the function to add that single node
  NodeContainer::Iterator it;
  for (it = nodes.Begin (); it != nodes.End (); it++)
    {
      AddNode (*it);
    }
}

void
LoraNetworkServer::AddNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);

  // Get the SatLorawanNetDevice
  Ptr<SatLorawanNetDevice> loraNetDevice;
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      loraNetDevice = DynamicCast<SatLorawanNetDevice> (node->GetDevice (i));
      if (loraNetDevice != 0)
        {
          // We found a SatLorawanNetDevice on the node
          break;
        }
    }

  // Get the MAC
  Ptr<LorawanMacEndDeviceClassA> edLorawanMac = DynamicCast<LorawanMacEndDeviceClassA> (loraNetDevice->GetMac ());

  // Update the NetworkStatus about the existence of this node
  m_status->AddNode (edLorawanMac);
}

bool
LoraNetworkServer::Receive (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& address)
{
  NS_LOG_FUNCTION (this << packet << protocol << address);

  // Fire the trace source
  m_receivedPacket (packet);

  // Inform the scheduler of the newly arrived packet
  m_scheduler->OnReceivedPacket (packet);

  // Inform the status of the newly arrived packet
  m_status->OnReceivedPacket (packet, address);

  // Inform the controller of the newly arrived packet
  m_controller->OnNewPacket (packet);

  return true;
}

void
LoraNetworkServer::AddComponent (Ptr<LoraNetworkControllerComponent> component)
{
  NS_LOG_FUNCTION (this << component);

  m_controller->Install (component);
}

Ptr<LoraNetworkStatus>
LoraNetworkServer::GetNetworkStatus (void)
{
  return m_status;
}

}
