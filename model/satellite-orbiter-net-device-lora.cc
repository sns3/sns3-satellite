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

#include "satellite-orbiter-net-device-lora.h"

#include "lorawan-mac-gateway.h"
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

NS_LOG_COMPONENT_DEFINE("SatOrbiterNetDeviceLora");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatOrbiterNetDeviceLora);

TypeId
SatOrbiterNetDeviceLora::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatOrbiterNetDeviceLora")
                            .SetParent<SatOrbiterNetDevice>()
                            .AddConstructor<SatOrbiterNetDeviceLora>();
    return tid;
}

SatOrbiterNetDeviceLora::SatOrbiterNetDeviceLora()
    : SatOrbiterNetDevice()
{
    NS_LOG_FUNCTION(this);
}

void
SatOrbiterNetDeviceLora::ReceivePacketUser(Ptr<Packet> packet, const Address& userAddress)
{
    NS_LOG_FUNCTION(this << packet);
    NS_LOG_INFO("Receiving a packet: " << packet->GetUid());

    Mac48Address macUserAddress = Mac48Address::ConvertFrom(userAddress);

    m_packetTrace(Simulator::Now(),
                  SatEnums::PACKET_RECV,
                  SatEnums::NT_SAT,
                  m_nodeId,
                  macUserAddress,
                  SatEnums::LL_ND,
                  SatEnums::LD_RETURN,
                  SatUtils::GetPacketInfo(packet));

    /*
     * Invoke the `Rx` and `RxDelay` trace sources. We look at the packet's tags
     * for information, but cannot remove the tags because the packet is a const.
     */
    if (m_isStatisticsTagsEnabled)
    {
        Address addr = GetRxUtAddress(packet, SatEnums::LD_RETURN);

        m_rxUserTrace(packet, addr);

        SatDevLinkTimeTag linkTimeTag;
        if (packet->RemovePacketTag(linkTimeTag))
        {
            NS_LOG_DEBUG(this << " contains a SatDevLinkTimeTag tag");
            Time delay = Simulator::Now() - linkTimeTag.GetSenderTimestamp();
            m_rxUserLinkDelayTrace(delay, addr);
            if (m_lastDelays[macUserAddress].IsZero() == false)
            {
                Time jitter = Abs(delay - m_lastDelays[macUserAddress]);
                m_rxUserLinkJitterTrace(jitter, addr);
            }
            m_lastDelays[macUserAddress] = delay;
        }
    }

    SatGroundStationAddressTag groundStationAddressTag;
    if (!packet->PeekPacketTag(groundStationAddressTag))
    {
        NS_FATAL_ERROR("SatGroundStationAddressTag not found");
    }
    Mac48Address destination = groundStationAddressTag.GetGroundStationAddress();

    SatUplinkInfoTag satUplinkInfoTag;
    if (!packet->PeekPacketTag(satUplinkInfoTag))
    {
        NS_FATAL_ERROR("SatUplinkInfoTag not found");
    }

    if (m_gwConnected.count(destination))
    {
        if (m_isStatisticsTagsEnabled)
        {
            // Add a SatDevLinkTimeTag tag for packet link delay computation at the receiver end.
            packet->AddPacketTag(SatDevLinkTimeTag(Simulator::Now()));
        }

        DynamicCast<SatOrbiterFeederMac>(m_feederMac[satUplinkInfoTag.GetBeamId()])
            ->EnquePacket(packet);
    }
    else
    {
        if (m_islNetDevices.size() > 0)
        {
            SendToIsl(packet, destination);
        }
    }
}

void
SatOrbiterNetDeviceLora::ReceivePacketFeeder(Ptr<Packet> packet, const Address& feederAddress)
{
    NS_LOG_FUNCTION(this << packet);
    NS_LOG_INFO("Receiving a packet: " << packet->GetUid());

    Mac48Address macFeederAddress = Mac48Address::ConvertFrom(feederAddress);

    m_packetTrace(Simulator::Now(),
                  SatEnums::PACKET_RECV,
                  SatEnums::NT_SAT,
                  m_nodeId,
                  macFeederAddress,
                  SatEnums::LL_ND,
                  SatEnums::LD_FORWARD,
                  SatUtils::GetPacketInfo(packet));

    /*
     * Invoke the `Rx` and `RxDelay` trace sources. We look at the packet's tags
     * for information, but cannot remove the tags because the packet is a const.
     */
    if (m_isStatisticsTagsEnabled)
    {
        Address addr = GetRxUtAddress(packet, SatEnums::LD_FORWARD);

        m_rxFeederTrace(packet, addr);

        SatDevLinkTimeTag linkTimeTag;
        if (packet->RemovePacketTag(linkTimeTag))
        {
            NS_LOG_DEBUG(this << " contains a SatDevLinkTimeTag tag");
            Time delay = Simulator::Now() - linkTimeTag.GetSenderTimestamp();
            m_rxFeederLinkDelayTrace(delay, addr);
            if (m_lastDelays[macFeederAddress].IsZero() == false)
            {
                Time jitter = Abs(delay - m_lastDelays[macFeederAddress]);
                m_rxFeederLinkJitterTrace(jitter, addr);
            }
            m_lastDelays[macFeederAddress] = delay;
        }
    }

    SatGroundStationAddressTag groundStationAddressTag;
    if (!packet->PeekPacketTag(groundStationAddressTag))
    {
        NS_FATAL_ERROR("SatGroundStationAddressTag not found");
    }
    Mac48Address destination = groundStationAddressTag.GetGroundStationAddress();

    if (destination.IsBroadcast())
    {
        m_broadcastReceived.insert(packet->GetUid());
    }

    SatUplinkInfoTag satUplinkInfoTag;
    if (!packet->PeekPacketTag(satUplinkInfoTag))
    {
        NS_FATAL_ERROR("SatUplinkInfoTag not found");
    }

    if (m_utConnected.count(destination) > 0 || destination.IsBroadcast())
    {
        if (m_isStatisticsTagsEnabled)
        {
            // Add a SatDevLinkTimeTag tag for packet link delay computation at the receiver end.
            packet->AddPacketTag(SatDevLinkTimeTag(Simulator::Now()));
        }

        DynamicCast<LorawanMacGateway>(m_userMac[satUplinkInfoTag.GetBeamId()])->Send(packet);
    }
    if ((m_utConnected.count(destination) == 0 || destination.IsBroadcast()) &&
        m_islNetDevices.size() > 0)
    {
        SendToIsl(packet, destination);
    }
}

void
SatOrbiterNetDeviceLora::ReceiveFromIsl(Ptr<Packet> packet, Mac48Address destination)
{
    NS_LOG_FUNCTION(this << packet << destination);

    if (destination.IsBroadcast())
    {
        if (m_broadcastReceived.count(packet->GetUid()) > 0)
        {
            // Packet already received, drop it
            return;
        }
        else
        {
            // Insert in list of receuived broadcast
            m_broadcastReceived.insert(packet->GetUid());
        }
    }

    if (m_gwConnected.count(destination) > 0)
    {
        SatUplinkInfoTag satUplinkInfoTag;
        if (!packet->PeekPacketTag(satUplinkInfoTag))
        {
            NS_FATAL_ERROR("SatUplinkInfoTag not found");
        }

        if (m_isStatisticsTagsEnabled)
        {
            // Add a SatDevLinkTimeTag tag for packet link delay computation at the receiver end.
            packet->AddPacketTag(SatDevLinkTimeTag(Simulator::Now()));
        }

        DynamicCast<SatOrbiterFeederMac>(m_feederMac[satUplinkInfoTag.GetBeamId()])
            ->EnquePacket(packet);
    }
    else
    {
        if (m_utConnected.count(destination) > 0 || destination.IsBroadcast())
        {
            SatUplinkInfoTag satUplinkInfoTag;
            if (!packet->PeekPacketTag(satUplinkInfoTag))
            {
                NS_FATAL_ERROR("SatUplinkInfoTag not found");
            }

            if (m_isStatisticsTagsEnabled && !destination.IsBroadcast())
            {
                // Add a SatDevLinkTimeTag tag for packet link delay computation at the receiver
                // end.
                packet->AddPacketTag(SatDevLinkTimeTag(Simulator::Now()));
            }

            DynamicCast<LorawanMacGateway>(m_userMac[satUplinkInfoTag.GetBeamId()])->Send(packet);
        }
        if ((m_utConnected.count(destination) == 0 || destination.IsBroadcast()) &&
            m_islNetDevices.size() > 0)
        {
            SendToIsl(packet, destination);
        }
    }
}

bool
SatOrbiterNetDeviceLora::SendControlMsgToFeeder(Ptr<SatControlMessage> msg,
                                                const Address& dest,
                                                Ptr<SatSignalParameters> rxParams)
{
    NS_LOG_FUNCTION(this << msg << dest);

    return true;
}

void
SatOrbiterNetDeviceLora::ConnectUt(Mac48Address utAddress, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << utAddress << beamId);

    NS_ASSERT_MSG(m_utConnected.find(utAddress) == m_utConnected.end(),
                  "Cannot add same UT twice to map");

    m_utConnected.insert({utAddress, beamId});
    Singleton<SatIdMapper>::Get()->AttachMacToSatIdIsl(utAddress, m_nodeId);
}

void
SatOrbiterNetDeviceLora::DisconnectUt(Mac48Address utAddress, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << utAddress << beamId);

    NS_ASSERT_MSG(m_utConnected.find(utAddress) != m_utConnected.end(), "UT not in map");

    m_utConnected.erase(utAddress);
    Singleton<SatIdMapper>::Get()->RemoveMacToSatIdIsl(utAddress);
}

} // namespace ns3
