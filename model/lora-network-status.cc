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

#include "lora-network-status.h"

#include "lora-beam-tag.h"
#include "lora-device-address.h"
#include "lora-end-device-status.h"
#include "lora-gateway-status.h"
#include "satellite-lorawan-net-device.h"
#include "satellite-topology.h"

#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/singleton.h"

#include <map>
#include <stdint.h>
#include <utility>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LoraNetworkStatus");

NS_OBJECT_ENSURE_REGISTERED(LoraNetworkStatus);

TypeId
LoraNetworkStatus::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::LoraNetworkStatus").SetParent<Object>().AddConstructor<LoraNetworkStatus>();
    return tid;
}

LoraNetworkStatus::LoraNetworkStatus()
{
    NS_LOG_FUNCTION_NOARGS();

    m_forwardLinkRegenerationMode = Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode();
    m_uniform = CreateObject<UniformRandomVariable>();
}

LoraNetworkStatus::~LoraNetworkStatus()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
LoraNetworkStatus::AddNode(Ptr<LorawanMacEndDeviceClassA> edMac)
{
    NS_LOG_FUNCTION(this << edMac);

    // Check whether this device already exists in our list
    LoraDeviceAddress edAddress = edMac->GetDeviceAddress();
    if (m_endDeviceStatuses.find(edAddress) == m_endDeviceStatuses.end())
    {
        // The device doesn't exist. Create new EndDeviceStatus
        Ptr<LoraEndDeviceStatus> edStatus =
            CreateObject<LoraEndDeviceStatus>(edAddress,
                                              edMac->GetObject<LorawanMacEndDeviceClassA>());

        // Add it to the map
        m_endDeviceStatuses.insert(
            std::pair<LoraDeviceAddress, Ptr<LoraEndDeviceStatus>>(edAddress, edStatus));
        NS_LOG_DEBUG("Added to the list a device with address " << edAddress.Print());
    }
}

void
LoraNetworkStatus::AddGateway(Ptr<Node> gw, Address& address, Ptr<LoraGatewayStatus> gwStatus)
{
    NS_LOG_FUNCTION(this << address << gwStatus);

    // Check whether this device already exists in the list
    if (m_gatewayStatuses.find(address) == m_gatewayStatuses.end())
    {
        // The device doesn't exist.

        // Add it to the map
        m_gatewayStatuses.insert(std::pair<Address, Ptr<LoraGatewayStatus>>(address, gwStatus));
        NS_LOG_DEBUG("Added to the list a gateway with address " << address);
    }

    // Get the PointToPointNetDevice
    Ptr<PointToPointNetDevice> p2pNetDevice;
    for (uint32_t i = 0; i < gw->GetNDevices(); i++)
    {
        p2pNetDevice = gw->GetDevice(i)->GetObject<PointToPointNetDevice>();
        if (p2pNetDevice != nullptr)
        {
            break;
        }
    }

    m_gws.insert(std::make_pair(gw, p2pNetDevice));
}

void
LoraNetworkStatus::OnReceivedPacket(Ptr<const Packet> packet, const Address& gwAddress)
{
    NS_LOG_FUNCTION(this << packet << gwAddress);

    // Create a copy of the packet
    Ptr<Packet> myPacket = packet->Copy();

    // Extract the headers
    LorawanMacHeader macHdr;
    myPacket->RemoveHeader(macHdr);
    LoraFrameHeader frameHdr;
    frameHdr.SetAsUplink();
    myPacket->RemoveHeader(frameHdr);

    // Update the correct EndDeviceStatus object
    LoraDeviceAddress edAddr = frameHdr.GetAddress();
    NS_LOG_DEBUG("Node address: " << edAddr);
    m_endDeviceStatuses.at(edAddr)->InsertReceivedPacket(packet, gwAddress);
}

bool
LoraNetworkStatus::NeedsReply(LoraDeviceAddress deviceAddress)
{
    // Throws out of range if no device is found
    return m_endDeviceStatuses.at(deviceAddress)->NeedsReply();
}

Address
LoraNetworkStatus::GetBestGatewayForDevice(LoraDeviceAddress deviceAddress, int window)
{
    NS_LOG_FUNCTION(this << deviceAddress << window);

    // Get the endDeviceStatus we are interested in
    Ptr<LoraEndDeviceStatus> edStatus = m_endDeviceStatuses.at(deviceAddress);
    double replyFrequency;
    if (window == 1)
    {
        replyFrequency = edStatus->GetFirstReceiveWindowFrequency();
    }
    else if (window == 2)
    {
        replyFrequency = edStatus->GetSecondReceiveWindowFrequency();
    }
    else
    {
        NS_ABORT_MSG("Invalid window value");
    }

    // Get the list of gateways that this device can reach
    // NOTE: At this point, we could also take into account the whole network to
    // identify the best gateway according to various metrics. For now, we just
    // ask the EndDeviceStatus to pick the best gateway for us via its method.
    std::map<double, Address> gwAddresses = edStatus->GetPowerGatewayMap();

    if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        uint8_t beamToUse = edStatus->GetBeamId();
        Ptr<Node> gateway;
        std::vector<Address> possibleAddresses;
        Ptr<SatLorawanNetDevice> lorawanNetDevice;
        for (std::map<Ptr<Node>, Ptr<PointToPointNetDevice>>::iterator it = m_gws.begin();
             it != m_gws.end();
             it++)
        {
            gateway = it->first;
            for (uint32_t i = 0; i < gateway->GetNDevices(); i++)
            {
                lorawanNetDevice = gateway->GetDevice(i)->GetObject<SatLorawanNetDevice>();
                if (lorawanNetDevice != nullptr)
                {
                    uint8_t beam = lorawanNetDevice->GetMac()->GetBeamId();
                    if (beam == beamToUse)
                    {
                        possibleAddresses.push_back(it->second->GetAddress());
                    }
                }
            }
        }
        if (possibleAddresses.size() > 0)
        {
            return possibleAddresses[m_uniform->GetInteger() % possibleAddresses.size()];
        }
        else
        {
            return gwAddresses.begin()->second;
        }
    }

    // By iterating on the map in reverse, we go from the 'best'
    // gateway, i.e. the one with the highest received power, to the
    // worst.
    Address bestGwAddress;
    for (auto it = gwAddresses.rbegin(); it != gwAddresses.rend(); it++)
    {
        bool isAvailable =
            m_gatewayStatuses.find(it->second)->second->IsAvailableForTransmission(replyFrequency);
        if (isAvailable)
        {
            bestGwAddress = it->second;
            break;
        }
    }

    return bestGwAddress;
}

void
LoraNetworkStatus::SendThroughGateway(Ptr<Packet> packet, Address gwAddress)
{
    NS_LOG_FUNCTION(packet << gwAddress);

    m_gatewayStatuses.find(gwAddress)->second->GetNetDevice()->Send(packet, gwAddress, 0x0800);
}

Ptr<Packet>
LoraNetworkStatus::GetReplyForDevice(LoraDeviceAddress edAddress, int windowNumber)
{
    // Get the reply packet
    Ptr<LoraEndDeviceStatus> edStatus = m_endDeviceStatuses.find(edAddress)->second;
    Ptr<Packet> packet = edStatus->GetCompleteReplyPacket();

    // Apply the appropriate tag
    LoraTag tag;
    tag.SetModcod(edStatus->GetModcod());
    switch (windowNumber)
    {
    case 1: {
        tag.SetDataRate(edStatus->GetMac()->GetFirstReceiveWindowDataRate());
        tag.SetFrequency(edStatus->GetFirstReceiveWindowFrequency());
        break;
    }
    case 2: {
        tag.SetDataRate(edStatus->GetMac()->GetSecondReceiveWindowDataRate());
        tag.SetFrequency(edStatus->GetSecondReceiveWindowFrequency());
        break;
    }
    }

    packet->AddPacketTag(tag);

    // Apply the appropriate tag
    LoraBeamTag beamTag;
    beamTag.SetBeamId(edStatus->GetBeamId());
    packet->AddPacketTag(beamTag);
    return packet;
}

Ptr<LoraEndDeviceStatus>
LoraNetworkStatus::GetEndDeviceStatus(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // Get the address
    LorawanMacHeader mHdr;
    LoraFrameHeader fHdr;
    Ptr<Packet> myPacket = packet->Copy();
    myPacket->RemoveHeader(mHdr);
    myPacket->RemoveHeader(fHdr);
    auto it = m_endDeviceStatuses.find(fHdr.GetAddress());
    if (it != m_endDeviceStatuses.end())
    {
        return (*it).second;
    }
    else
    {
        NS_LOG_ERROR("EndDeviceStatus not found");
        return 0;
    }
}

Ptr<LoraEndDeviceStatus>
LoraNetworkStatus::GetEndDeviceStatus(LoraDeviceAddress address)
{
    NS_LOG_FUNCTION(this << address);

    auto it = m_endDeviceStatuses.find(address);
    if (it != m_endDeviceStatuses.end())
    {
        return (*it).second;
    }
    else
    {
        NS_LOG_ERROR("EndDeviceStatus not found");
        return 0;
    }
}

int
LoraNetworkStatus::CountEndDevices(void)
{
    NS_LOG_FUNCTION(this);

    return m_endDeviceStatuses.size();
}
} // namespace ns3
