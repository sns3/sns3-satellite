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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "satellite-orbiter-net-device.h"

#include "satellite-address-tag.h"
#include "satellite-channel.h"
#include "satellite-ground-station-address-tag.h"
#include "satellite-id-mapper.h"
#include "satellite-mac.h"
#include "satellite-orbiter-feeder-mac.h"
#include "satellite-orbiter-feeder-phy.h"
#include "satellite-orbiter-user-mac.h"
#include "satellite-orbiter-user-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-phy.h"
#include "satellite-time-tag.h"
#include "satellite-uplink-info-tag.h"

#include "ns3/channel.h"
#include "ns3/error-model.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/object-map.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/singleton.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

NS_LOG_COMPONENT_DEFINE("SatOrbiterNetDevice");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatOrbiterNetDevice);

TypeId
SatOrbiterNetDevice::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatOrbiterNetDevice")
            .SetParent<NetDevice>()
            .AddAttribute("ReceiveErrorModel",
                          "The receiver error model used to simulate packet loss",
                          PointerValue(),
                          MakePointerAccessor(&SatOrbiterNetDevice::m_receiveErrorModel),
                          MakePointerChecker<ErrorModel>())
            .AddAttribute("UserPhy",
                          "The User Phy objects attached to this device.",
                          ObjectMapValue(),
                          MakeObjectMapAccessor(&SatOrbiterNetDevice::m_userPhy),
                          MakeObjectMapChecker<SatPhy>())
            .AddAttribute("FeederPhy",
                          "The Feeder Phy objects attached to this device.",
                          ObjectMapValue(),
                          MakeObjectMapAccessor(&SatOrbiterNetDevice::m_feederPhy),
                          MakeObjectMapChecker<SatPhy>())
            .AddAttribute("UserMac",
                          "The User MAC objects attached to this device.",
                          ObjectMapValue(),
                          MakeObjectMapAccessor(&SatOrbiterNetDevice::m_userMac),
                          MakeObjectMapChecker<SatMac>())
            .AddAttribute("FeederMac",
                          "The Feeder MAC objects attached to this device.",
                          ObjectMapValue(),
                          MakeObjectMapAccessor(&SatOrbiterNetDevice::m_feederMac),
                          MakeObjectMapChecker<SatMac>())
            .AddAttribute("EnableStatisticsTags",
                          "If true, some tags will be added to each transmitted packet to assist "
                          "with statistics computation",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatOrbiterNetDevice::m_isStatisticsTagsEnabled),
                          MakeBooleanChecker())
            .AddTraceSource("PacketTrace",
                            "Packet event trace",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_packetTrace),
                            "ns3::SatTypedefs::PacketTraceCallback")
            .AddTraceSource("Tx",
                            "A packet to be sent",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("SignallingTx",
                            "A signalling packet to be sent",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_signallingTxTrace),
                            "ns3::SatTypedefs::PacketDestinationAddressCallback")
            .AddTraceSource("RxFeeder",
                            "A packet received on feeder",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_rxFeederTrace),
                            "ns3::SatTypedefs::PacketSourceAddressCallback")
            .AddTraceSource("RxUser",
                            "A packet received on user",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_rxUserTrace),
                            "ns3::SatTypedefs::PacketSourceAddressCallback")
            .AddTraceSource("RxFeederLinkDelay",
                            "A packet is received with feeder link delay information",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_rxFeederLinkDelayTrace),
                            "ns3::SatTypedefs::PacketDelayAddressCallback")
            .AddTraceSource(
                "RxFeederLinkJitter",
                "A packet is received with feeder link jitter information",
                MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_rxFeederLinkJitterTrace),
                "ns3::SatTypedefs::PacketJitterAddressCallback")
            .AddTraceSource("RxUserLinkDelay",
                            "A packet is received with feeder link delay information",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_rxUserLinkDelayTrace),
                            "ns3::SatTypedefs::PacketDelayAddressCallback")
            .AddTraceSource("RxUserLinkJitter",
                            "A packet is received with feeder link jitter information",
                            MakeTraceSourceAccessor(&SatOrbiterNetDevice::m_rxUserLinkJitterTrace),
                            "ns3::SatTypedefs::PacketJitterAddressCallback");
    return tid;
}

SatOrbiterNetDevice::SatOrbiterNetDevice()
    : m_node(0),
      m_mtu(0xffff),
      m_ifIndex(0)
{
    NS_LOG_FUNCTION(this);
}

void
SatOrbiterNetDevice::ReceiveUser(SatPhy::PacketContainer_t /*packets*/,
                                 Ptr<SatSignalParameters> rxParams)
{
    NS_LOG_FUNCTION(this << rxParams->m_packetsInBurst.size() << rxParams);
    NS_LOG_INFO("Receiving a packet at the satellite from user link");

    switch (m_returnLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        DynamicCast<SatOrbiterFeederPhy>(m_feederPhy[rxParams->m_beamId])
            ->SendPduWithParams(rxParams);
        break;
    }
    case SatEnums::REGENERATION_LINK: {
        for (SatPhy::PacketContainer_t::const_iterator it = rxParams->m_packetsInBurst.begin();
             it != rxParams->m_packetsInBurst.end();
             ++it)
        {
            DynamicCast<SatOrbiterFeederMac>(m_feederMac[rxParams->m_beamId])->EnquePacket(*it);
        }

        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        NS_FATAL_ERROR(
            "SatOrbiterNetDevice::ReceiveUser should not be used in case of network regeneration");
    }
    default: {
        NS_FATAL_ERROR("Not implemented yet");
    }
    }
}

void
SatOrbiterNetDevice::ReceiveFeeder(SatPhy::PacketContainer_t /*packets*/,
                                   Ptr<SatSignalParameters> rxParams)
{
    NS_LOG_FUNCTION(this << rxParams->m_packetsInBurst.size() << rxParams);
    NS_LOG_INFO("Receiving a packet at the satellite from feeder link");

    switch (m_forwardLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        DynamicCast<SatOrbiterUserPhy>(m_userPhy[rxParams->m_beamId])->SendPduWithParams(rxParams);
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        NS_FATAL_ERROR("SatOrbiterNetDevice::ReceiveFeeder should not be used in case of network "
                       "regeneration");
    }
    default: {
        NS_FATAL_ERROR("Not implemented yet");
    }
    }
}

void
SatOrbiterNetDevice::SetReceiveErrorModel(Ptr<ErrorModel> em)
{
    NS_LOG_FUNCTION(this << em);
    m_receiveErrorModel = em;
}

void
SatOrbiterNetDevice::SetForwardLinkRegenerationMode(
    SatEnums::RegenerationMode_t forwardLinkRegenerationMode)
{
    m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
}

void
SatOrbiterNetDevice::SetReturnLinkRegenerationMode(
    SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    m_returnLinkRegenerationMode = returnLinkRegenerationMode;
}

void
SatOrbiterNetDevice::SetNodeId(uint32_t nodeId)
{
    m_nodeId = nodeId;
}

void
SatOrbiterNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this << index);
    m_ifIndex = index;
}

uint32_t
SatOrbiterNetDevice::GetIfIndex(void) const
{
    NS_LOG_FUNCTION(this);
    return m_ifIndex;
}

void
SatOrbiterNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = Mac48Address::ConvertFrom(address);
}

Address
SatOrbiterNetDevice::GetAddress(void) const
{
    //
    // Implicit conversion from Mac48Address to Address
    //
    NS_LOG_FUNCTION(this);
    return m_address;
}

bool
SatOrbiterNetDevice::SetMtu(const uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

uint16_t
SatOrbiterNetDevice::GetMtu(void) const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

bool
SatOrbiterNetDevice::IsLinkUp(void) const
{
    NS_LOG_FUNCTION(this);
    return true;
}

void
SatOrbiterNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(this << &callback);
}

bool
SatOrbiterNetDevice::IsBroadcast(void) const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
SatOrbiterNetDevice::GetBroadcast(void) const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool
SatOrbiterNetDevice::IsMulticast(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

Address
SatOrbiterNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    return Mac48Address::GetMulticast(multicastGroup);
}

Address
SatOrbiterNetDevice::GetMulticast(Ipv6Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    return Mac48Address::GetMulticast(addr);
}

bool
SatOrbiterNetDevice::IsPointToPoint(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
SatOrbiterNetDevice::IsBridge(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
SatOrbiterNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);

    /**
     * The satellite does not have higher protocol layers which
     * utilize the Send method! Thus, this method should not be used!
     */
    NS_ASSERT(false);
    return false;
}

bool
SatOrbiterNetDevice::SendFrom(Ptr<Packet> packet,
                              const Address& source,
                              const Address& dest,
                              uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << source << dest << protocolNumber);

    /**
     * The satellite does not have higher protocol layers which
     * utilize the SendFrom method! Thus, this method should not be used!
     */
    NS_ASSERT(false);
    return false;
}

Ptr<Node>
SatOrbiterNetDevice::GetNode(void) const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

void
SatOrbiterNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

bool
SatOrbiterNetDevice::NeedsArp(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

void
SatOrbiterNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
}

void
SatOrbiterNetDevice::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    m_node = 0;
    m_receiveErrorModel = 0;
    m_userPhy.clear();
    m_feederPhy.clear();
    m_userMac.clear();
    m_feederMac.clear();
    m_addressMapFeeder.clear();
    m_addressMapUser.clear();
    NetDevice::DoDispose();
}

void
SatOrbiterNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_promiscCallback = cb;
}

bool
SatOrbiterNetDevice::SupportsSendFrom(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

Ptr<Channel>
SatOrbiterNetDevice::GetChannel(void) const
{
    NS_LOG_FUNCTION(this);
    return nullptr;
}

void
SatOrbiterNetDevice::AddUserPhy(Ptr<SatPhy> phy, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << phy << beamId);
    m_userPhy.insert(std::pair<uint32_t, Ptr<SatPhy>>(beamId, phy));
}

void
SatOrbiterNetDevice::AddFeederPhy(Ptr<SatPhy> phy, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << phy << beamId);
    m_feederPhy.insert(std::pair<uint32_t, Ptr<SatPhy>>(beamId, phy));
}

Ptr<SatPhy>
SatOrbiterNetDevice::GetUserPhy(uint32_t beamId)
{
    if (m_userPhy.count(beamId))
    {
        return m_userPhy[beamId];
    }
    NS_FATAL_ERROR("User Phy does not exist for beam " << beamId);
}

Ptr<SatPhy>
SatOrbiterNetDevice::GetFeederPhy(uint32_t beamId)
{
    if (m_userPhy.count(beamId))
    {
        return m_feederPhy[beamId];
    }
    NS_FATAL_ERROR("User Phy does not exist for beam " << beamId);
}

std::map<uint32_t, Ptr<SatPhy>>
SatOrbiterNetDevice::GetUserPhy()
{
    return m_userPhy;
}

std::map<uint32_t, Ptr<SatPhy>>
SatOrbiterNetDevice::GetFeederPhy()
{
    return m_feederPhy;
}

void
SatOrbiterNetDevice::AddUserMac(Ptr<SatMac> mac, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << mac << beamId);
    m_userMac.insert(std::pair<uint32_t, Ptr<SatMac>>(beamId, mac));
}

void
SatOrbiterNetDevice::AddFeederMac(Ptr<SatMac> mac, Ptr<SatMac> macUsed, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << mac << macUsed << beamId);
    m_feederMac.insert(std::pair<uint32_t, Ptr<SatMac>>(beamId, macUsed));
    m_allFeederMac.insert(std::pair<uint32_t, Ptr<SatMac>>(beamId, mac));
}

Ptr<SatMac>
SatOrbiterNetDevice::GetUserMac(uint32_t beamId)
{
    if (m_userMac.count(beamId))
    {
        return m_userMac[beamId];
    }
    NS_FATAL_ERROR("User MAC does not exist for beam " << beamId);
}

Ptr<SatMac>
SatOrbiterNetDevice::GetFeederMac(uint32_t beamId)
{
    if (m_feederMac.count(beamId))
    {
        return m_feederMac[beamId];
    }
    NS_FATAL_ERROR("User MAC does not exist for beam " << beamId);
}

std::map<uint32_t, Ptr<SatMac>>
SatOrbiterNetDevice::GetUserMac()
{
    return m_userMac;
}

std::map<uint32_t, Ptr<SatMac>>
SatOrbiterNetDevice::GetFeederMac()
{
    return m_feederMac;
}

std::map<uint32_t, Ptr<SatMac>>
SatOrbiterNetDevice::GetAllFeederMac()
{
    return m_allFeederMac;
}

void
SatOrbiterNetDevice::AddFeederPair(uint32_t beamId, Mac48Address satelliteFeederAddress)
{
    NS_LOG_FUNCTION(this << beamId << satelliteFeederAddress);

    m_addressMapFeeder.insert(std::pair<uint32_t, Mac48Address>(beamId, satelliteFeederAddress));
}

void
SatOrbiterNetDevice::AddUserPair(uint32_t beamId, Mac48Address satelliteUserAddress)
{
    NS_LOG_FUNCTION(this << beamId << satelliteUserAddress);

    m_addressMapUser.insert(std::pair<uint32_t, Mac48Address>(beamId, satelliteUserAddress));
}

Mac48Address
SatOrbiterNetDevice::GetSatelliteFeederAddress(uint32_t beamId)
{
    NS_LOG_FUNCTION(this << beamId);

    if (m_addressMapFeeder.count(beamId))
    {
        return m_addressMapFeeder[beamId];
    }
    NS_FATAL_ERROR("Satellite MAC does not exist for beam " << beamId);
}

Mac48Address
SatOrbiterNetDevice::GetSatelliteUserAddress(uint32_t beamId)
{
    NS_LOG_FUNCTION(this << beamId);

    if (m_addressMapUser.count(beamId))
    {
        return m_addressMapUser[beamId];
    }
    NS_FATAL_ERROR("Satellite MAC does not exist for beam " << beamId);
}

Address
SatOrbiterNetDevice::GetRxUtAddress(Ptr<Packet> packet, SatEnums::SatLinkDir_t ld)
{
    NS_LOG_FUNCTION(this << packet);

    Address utAddr; // invalid address.

    SatAddressE2ETag addressE2ETag;
    if (packet->PeekPacketTag(addressE2ETag))
    {
        NS_LOG_DEBUG(this << " contains a SatE2E tag");
        if (ld == SatEnums::LD_FORWARD)
        {
            utAddr = addressE2ETag.GetE2EDestAddress();
        }
        else if (ld == SatEnums::LD_RETURN)
        {
            utAddr = addressE2ETag.GetE2ESourceAddress();
        }
    }

    return utAddr;
}

void
SatOrbiterNetDevice::ConnectGw(Mac48Address gwAddress, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << gwAddress << beamId);

    NS_ASSERT_MSG(m_gwConnected.find(gwAddress) == m_gwConnected.end(),
                  "Cannot add same GW twice to map");

    m_gwConnected.insert({gwAddress, beamId});
    Singleton<SatIdMapper>::Get()->AttachMacToSatIdIsl(gwAddress, m_nodeId);

    if (m_feederMac.find(beamId) != m_feederMac.end())
    {
        Ptr<SatOrbiterFeederMac> orbiterFeederMac =
            DynamicCast<SatOrbiterFeederMac>(GetFeederMac(beamId));
        NS_ASSERT(orbiterFeederMac != nullptr);
        {
            orbiterFeederMac->AddPeer(gwAddress);
        }
    }
}

void
SatOrbiterNetDevice::DisconnectGw(Mac48Address gwAddress, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << gwAddress << beamId);

    NS_ASSERT_MSG(m_gwConnected.find(gwAddress) != m_gwConnected.end(), "GW not in map");

    m_gwConnected.erase(gwAddress);
    Singleton<SatIdMapper>::Get()->RemoveMacToSatIdIsl(gwAddress);

    if (m_feederMac.find(beamId) != m_feederMac.end())
    {
        Ptr<SatOrbiterFeederMac> orbiterFeederMac =
            DynamicCast<SatOrbiterFeederMac>(GetFeederMac(beamId));
        NS_ASSERT(orbiterFeederMac != nullptr);
        {
            orbiterFeederMac->RemovePeer(gwAddress);
        }
    }
}

std::set<Mac48Address>
SatOrbiterNetDevice::GetGwConnected()
{
    NS_LOG_FUNCTION(this);

    std::set<Mac48Address> gws;
    std::map<Mac48Address, uint32_t>::iterator it;
    for (it = m_gwConnected.begin(); it != m_gwConnected.end(); it++)
    {
        gws.insert(it->first);
    }

    return gws;
}

std::set<Mac48Address>
SatOrbiterNetDevice::GetUtConnected()
{
    NS_LOG_FUNCTION(this);

    std::set<Mac48Address> uts;
    std::map<Mac48Address, uint32_t>::iterator it;
    for (it = m_utConnected.begin(); it != m_utConnected.end(); it++)
    {
        uts.insert(it->first);
    }

    return uts;
}

void
SatOrbiterNetDevice::AddIslsNetDevice(Ptr<PointToPointIslNetDevice> islNetDevices)
{
    NS_LOG_FUNCTION(this);

    m_islNetDevices.push_back(islNetDevices);
}

std::vector<Ptr<PointToPointIslNetDevice>>
SatOrbiterNetDevice::GetIslsNetDevices()
{
    NS_LOG_FUNCTION(this);

    return m_islNetDevices;
}

void
SatOrbiterNetDevice::SetArbiter(Ptr<SatIslArbiter> arbiter)
{
    NS_LOG_FUNCTION(this << arbiter);

    m_arbiter = arbiter;
}

Ptr<SatIslArbiter>
SatOrbiterNetDevice::GetArbiter()
{
    NS_LOG_FUNCTION(this);

    return m_arbiter;
}

void
SatOrbiterNetDevice::SendToIsl(Ptr<Packet> packet, Mac48Address destination)
{
    NS_LOG_FUNCTION(this << packet << destination);

    // If ISLs, arbiter must be set
    NS_ASSERT_MSG(m_arbiter != nullptr, "Arbiter not set");

    if (destination.IsBroadcast())
    {
        // Send to all interfaces
        for (uint32_t i = 0; i < m_islNetDevices.size(); i++)
        {
            m_islNetDevices[i]->Send(packet->Copy(), Address(), 0x0800);
        }
        return;
    }

    int32_t islInterfaceIndex = m_arbiter->BaseDecide(packet, destination);

    if (islInterfaceIndex < 0)
    {
        NS_LOG_INFO("Cannot route packet form node " << m_nodeId << " to " << destination);
    }
    else if (uint32_t(islInterfaceIndex) >= m_islNetDevices.size())
    {
        NS_FATAL_ERROR("Incorrect interface index from arbiter: "
                       << islInterfaceIndex << ". Max is " << m_islNetDevices.size() - 1);
    }
    else
    {
        m_islNetDevices[islInterfaceIndex]->Send(packet, Address(), 0x0800);
    }
}

} // namespace ns3
