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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-lorawan-net-device.h"

#include "lora-frame-header.h"
#include "lora-tag.h"
#include "lorawan-mac-header.h"
#include "satellite-topology.h"

#include "ns3/ipv4-header.h"
#include "ns3/log.h"
#include "ns3/singleton.h"

NS_LOG_COMPONENT_DEFINE("SatLorawanNetDevice");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatLorawanNetDevice);

TypeId
SatLorawanNetDevice::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatLorawanNetDevice")
                            .SetParent<SatNetDevice>()
                            .AddConstructor<SatLorawanNetDevice>();
    return tid;
}

SatLorawanNetDevice::SatLorawanNetDevice()
{
    NS_LOG_FUNCTION(this);

    switch (Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode())
    {
    case SatEnums::TRANSPARENT: {
        m_isRegenerative = false;
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        m_isRegenerative = true;
        break;
    }
    default:
        NS_FATAL_ERROR("Incorrect regeneration mode for LORA");
    }
}

void
SatLorawanNetDevice::Receive(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    NS_LOG_INFO("Receiving a packet: " << packet->GetUid());

    // Add packet trace entry:
    SatEnums::SatLinkDir_t ld =
        (m_nodeInfo->GetNodeType() == SatEnums::NT_UT) ? SatEnums::LD_FORWARD : SatEnums::LD_RETURN;

    m_packetTrace(Simulator::Now(),
                  SatEnums::PACKET_RECV,
                  m_nodeInfo->GetNodeType(),
                  m_nodeInfo->GetNodeId(),
                  m_nodeInfo->GetMacAddress(),
                  SatEnums::LL_ND,
                  ld,
                  SatUtils::GetPacketInfo(packet));

    /*
     * Invoke the `Rx` and `RxDelay` trace sources. We look at the packet's tags
     * for information, but cannot remove the tags because the packet is a const.
     */
    if (m_isStatisticsTagsEnabled)
    {
        Address addr; // invalid address.
        bool isTaggedWithAddress = false;
        ByteTagIterator it = packet->GetByteTagIterator();

        while (!isTaggedWithAddress && it.HasNext())
        {
            ByteTagIterator::Item item = it.Next();

            if (item.GetTypeId() == SatAddressTag::GetTypeId())
            {
                NS_LOG_DEBUG(this << " contains a SatAddressTag tag:" << " start="
                                  << item.GetStart() << " end=" << item.GetEnd());
                SatAddressTag addrTag;
                item.GetTag(addrTag);
                addr = addrTag.GetSourceAddress();
                isTaggedWithAddress = true; // this will exit the while loop.
            }
        }

        m_rxTrace(packet, addr);

        SatDevTimeTag timeTag;
        if (packet->PeekPacketTag(timeTag))
        {
            NS_LOG_DEBUG(this << " contains a SatDevTimeTag tag");
            Time delay = Simulator::Now() - timeTag.GetSenderTimestamp();
            m_rxDelayTrace(delay, addr);
            if (m_lastDelay.IsZero() == false)
            {
                Time jitter = Abs(delay - m_lastDelay);
                m_rxJitterTrace(jitter, addr);
            }
            m_lastDelay = delay;
        }
    }

    // Forward to Network Server if on GW.
    if (m_nodeInfo->GetNodeType() == SatEnums::NT_GW)
    {
        Ptr<Packet> pktCopy = packet->Copy();
        m_rxNetworkServerCallback(this, pktCopy, Ipv4L3Protocol::PROT_NUMBER, Address());
    }

    // Pass the packet to the upper layer if IP header in packet (GW or UT side)
    LorawanMacHeader mHdr;
    LoraFrameHeader fHdr;
    Ipv4Header ipv4Header;
    Ptr<Packet> pktCopy = packet->Copy();
    pktCopy->RemoveHeader(mHdr);
    pktCopy->RemoveHeader(fHdr);
    if (pktCopy->PeekHeader(ipv4Header))
    {
        m_rxCallback(this, pktCopy, Ipv4L3Protocol::PROT_NUMBER, Address());
    }
}

bool
SatLorawanNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);

    // only send tagged Lora packets
    LoraTag tag;
    if ((!packet->PeekPacketTag(tag)) && (m_nodeInfo->GetNodeType() == SatEnums::NT_GW))
    {
        return false;
    }

    // Add packet trace entry:
    SatEnums::SatLinkDir_t ld =
        (m_nodeInfo->GetNodeType() == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

    m_packetTrace(Simulator::Now(),
                  SatEnums::PACKET_SENT,
                  m_nodeInfo->GetNodeType(),
                  m_nodeInfo->GetNodeId(),
                  m_nodeInfo->GetMacAddress(),
                  SatEnums::LL_ND,
                  ld,
                  SatUtils::GetPacketInfo(packet));

    m_txTrace(packet);

    Address addr; // invalid address.
    bool isTaggedWithAddress = false;
    ByteTagIterator it = packet->GetByteTagIterator();

    while (!isTaggedWithAddress && it.HasNext())
    {
        ByteTagIterator::Item item = it.Next();

        if (item.GetTypeId() == SatAddressTag::GetTypeId())
        {
            NS_LOG_DEBUG(this << " contains a SatAddressTag tag:" << " start=" << item.GetStart()
                              << " end=" << item.GetEnd());
            SatAddressTag addrTag;
            item.GetTag(addrTag);
            addr = addrTag.GetSourceAddress();
            isTaggedWithAddress = true; // this will exit the while loop.
        }
    }

    if (m_isRegenerative && m_nodeInfo->GetNodeType() == SatEnums::NT_GW)
    {
        uint8_t flowId = m_classifier->Classify(packet, dest, protocolNumber);

        m_llc->Enque(packet, dest, flowId);
    }
    else
    {
        DynamicCast<LorawanMac>(m_mac)->Send(packet, dest, protocolNumber);
    }

    return true;
}

bool
SatLorawanNetDevice::SendControlMsg(Ptr<SatControlMessage> msg, const Address& dest)
{
    // We send nothing in Lora Mode. Control is made via LorawanMacCommand
    return true;
}

void
SatLorawanNetDevice::SetReceiveNetworkServerCallback(SatLorawanNetDevice::ReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_rxNetworkServerCallback = cb;
}

void
SatLorawanNetDevice::DoDispose(void)
{
    NS_LOG_FUNCTION(this);

    SatNetDevice::DoDispose();
}

} // namespace ns3
