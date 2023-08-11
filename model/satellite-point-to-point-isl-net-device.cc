/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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
 * (Based on point-to-point network device)
 * Author: Andre Aguas    March 2020
 *         Simon          2020
 * Adapted to SNS-3 by: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#include "ns3/satellite-point-to-point-isl-net-device.h"

#include "ns3/drop-tail-queue.h"
#include "ns3/error-model.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/pointer.h"
#include "ns3/ppp-header.h"
#include "ns3/satellite-ground-station-address-tag.h"
#include "ns3/satellite-point-to-point-isl-channel.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE("PointToPointIslNetDevice");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(PointToPointIslNetDevice);

TypeId
PointToPointIslNetDevice::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::PointToPointIslNetDevice")
            .SetParent<NetDevice>()
            .AddConstructor<PointToPointIslNetDevice>()
            .AddAttribute("ReceiveErrorModel",
                          "The receiver error model used to simulate packet loss",
                          PointerValue(),
                          MakePointerAccessor(&PointToPointIslNetDevice::m_receiveErrorModel),
                          MakePointerChecker<ErrorModel>())
            .AddAttribute("InterframeGap",
                          "The time to wait between packet (frame) transmissions",
                          TimeValue(Seconds(0.0)),
                          MakeTimeAccessor(&PointToPointIslNetDevice::m_tInterframeGap),
                          MakeTimeChecker())
            .AddTraceSource(
                "PacketDropRateTrace",
                "A packet has been dropped or not",
                MakeTraceSourceAccessor(&PointToPointIslNetDevice::m_packetDropRateTrace),
                "ns3::SatTypedefs::PacketDropRateTrace");
    return tid;
}

PointToPointIslNetDevice::PointToPointIslNetDevice()
    : m_txMachineState(READY),
      m_channel(0),
      m_linkUp(false),
      m_currentPkt(0)
{
    NS_LOG_FUNCTION(this);
}

PointToPointIslNetDevice::~PointToPointIslNetDevice()
{
    NS_LOG_FUNCTION(this);
}

void
PointToPointIslNetDevice::AddHeader(Ptr<Packet> p, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << p << protocolNumber);
    PppHeader ppp;
    ppp.SetProtocol(EtherToPpp(protocolNumber));
    p->AddHeader(ppp);
}

bool
PointToPointIslNetDevice::ProcessHeader(Ptr<Packet> p, uint16_t& param)
{
    NS_LOG_FUNCTION(this << p << param);
    PppHeader ppp;
    p->RemoveHeader(ppp);
    param = PppToEther(ppp.GetProtocol());
    return true;
}

void
PointToPointIslNetDevice::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_node = 0;
    m_channel = 0;
    m_receiveErrorModel = 0;
    m_currentPkt = 0;
    m_queue = 0;
    NetDevice::DoDispose();
}

void
PointToPointIslNetDevice::SetDataRate(DataRate bps)
{
    NS_LOG_FUNCTION(this);
    m_dataRate = bps;
}

void
PointToPointIslNetDevice::SetInterframeGap(Time t)
{
    NS_LOG_FUNCTION(this << t.GetSeconds());
    m_tInterframeGap = t;
}

bool
PointToPointIslNetDevice::TransmitStart(Ptr<Packet> p)
{
    NS_LOG_FUNCTION(this << p);
    NS_LOG_LOGIC("UID is " << p->GetUid() << ")");

    //
    // This function is called to start the process of transmitting a packet.
    // We need to tell the channel that we've started wiggling the wire and
    // schedule an event that will be executed when the transmission is complete.
    //
    NS_ASSERT_MSG(m_txMachineState == READY, "Must be READY to transmit");
    m_txMachineState = BUSY;
    m_currentPkt = p;

    Time txTime = m_dataRate.CalculateBytesTxTime(p->GetSize());
    Time txCompleteTime = txTime + m_tInterframeGap;

    NS_LOG_LOGIC("Schedule TransmitCompleteEvent in " << txCompleteTime.GetSeconds() << "sec");
    Simulator::Schedule(txCompleteTime, &PointToPointIslNetDevice::TransmitComplete, this);

    bool result = m_channel->TransmitStart(p, this, m_destinationNode, txTime);
    return result;
}

void
PointToPointIslNetDevice::TransmitComplete(void)
{
    NS_LOG_FUNCTION(this);

    //
    // This function is called to when we're all done transmitting a packet.
    // We try and pull another packet off of the transmit queue.  If the queue
    // is empty, we are done, otherwise we need to start transmitting the
    // next packet.
    //
    NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
    m_txMachineState = READY;

    NS_ASSERT_MSG(m_currentPkt != nullptr,
                  "PointToPointIslNetDevice::TransmitComplete(): m_currentPkt zero");

    m_currentPkt = nullptr;

    Ptr<Packet> p = m_queue->Dequeue();
    if (p == nullptr)
    {
        NS_LOG_LOGIC("No pending packets in device queue after tx complete");
        return;
    }

    TransmitStart(p);
}

bool
PointToPointIslNetDevice::Attach(Ptr<PointToPointIslChannel> ch)
{
    NS_LOG_FUNCTION(this << &ch);

    m_channel = ch;

    m_channel->Attach(this);

    //
    // This device is up whenever it is attached to a channel.  A better plan
    // would be to have the link come up when both devices are attached, but this
    // is not done for now.
    //
    NotifyLinkUp();
    return true;
}

void
PointToPointIslNetDevice::SetQueue(Ptr<DropTailQueue<Packet>> q)
{
    NS_LOG_FUNCTION(this << q);
    m_queue = q;
}

void
PointToPointIslNetDevice::SetReceiveErrorModel(Ptr<ErrorModel> em)
{
    NS_LOG_FUNCTION(this << em);
    m_receiveErrorModel = em;
}

void
PointToPointIslNetDevice::Receive(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    uint16_t protocol = 0;

    if (!m_receiveErrorModel || !m_receiveErrorModel->IsCorrupt(packet))
    {
        //
        // Strip off the point-to-point protocol header and forward this packet
        // up the protocol stack.  Since this is a simple point-to-point link,
        // there is no difference in what the promisc callback sees and what the
        // normal receive callback sees.
        //
        ProcessHeader(packet, protocol);

        if (!m_promiscCallback.IsNull())
        {
            m_promiscCallback(this,
                              packet,
                              protocol,
                              GetRemote(),
                              GetAddress(),
                              NetDevice::PACKET_HOST);
        }

        // m_rxCallback (this, packet, protocol, GetRemote ());
        SatGroundStationAddressTag groundStationAddressTag;
        if (!packet->PeekPacketTag(groundStationAddressTag))
        {
            NS_FATAL_ERROR("SatGroundStationAddressTag not found");
        }

        m_geoNetDevice->ReceiveFromIsl(packet, groundStationAddressTag.GetGroundStationAddress());
    }
}

void
PointToPointIslNetDevice::SetGeoNetDevice(Ptr<SatGeoNetDevice> geoNetDevice)
{
    NS_LOG_FUNCTION(this);

    m_geoNetDevice = geoNetDevice;
}

Ptr<DropTailQueue<Packet>>
PointToPointIslNetDevice::GetQueue(void) const
{
    NS_LOG_FUNCTION(this);
    return m_queue;
}

void
PointToPointIslNetDevice::NotifyLinkUp(void)
{
    NS_LOG_FUNCTION(this);
    m_linkUp = true;
}

void
PointToPointIslNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this);
    m_ifIndex = index;
}

uint32_t
PointToPointIslNetDevice::GetIfIndex(void) const
{
    return m_ifIndex;
}

Ptr<Channel>
PointToPointIslNetDevice::GetChannel(void) const
{
    return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

void
PointToPointIslNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = Mac48Address::ConvertFrom(address);
}

Address
PointToPointIslNetDevice::GetAddress(void) const
{
    return m_address;
}

void
PointToPointIslNetDevice::SetDestinationNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_destinationNode = node;
}

Ptr<Node>
PointToPointIslNetDevice::GetDestinationNode(void) const
{
    return m_destinationNode;
}

bool
PointToPointIslNetDevice::IsLinkUp(void) const
{
    NS_LOG_FUNCTION(this);
    return m_linkUp;
}

void
PointToPointIslNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(this);
}

//
// This is a point-to-point device, so every transmission is a broadcast to
// all of the devices on the network.
//
bool
PointToPointIslNetDevice::IsBroadcast(void) const
{
    NS_LOG_FUNCTION(this);
    return true;
}

//
// We don't really need any addressing information since this is a
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
Address
PointToPointIslNetDevice::GetBroadcast(void) const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool
PointToPointIslNetDevice::IsMulticast(void) const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
PointToPointIslNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address("01:00:5e:00:00:00");
}

Address
PointToPointIslNetDevice::GetMulticast(Ipv6Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    return Mac48Address("33:33:00:00:00:00");
}

bool
PointToPointIslNetDevice::IsPointToPoint(void) const
{
    NS_LOG_FUNCTION(this);
    return true;
}

bool
PointToPointIslNetDevice::IsBridge(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
PointToPointIslNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);
    NS_LOG_LOGIC("p=" << packet << ", dest=" << &dest);
    NS_LOG_LOGIC("UID is " << packet->GetUid());

    // If IsLinkUp() is false it means there is no channel to send any packet
    // over so we just return an error.
    if (IsLinkUp() == false)
    {
        return false;
    }

    AddHeader(packet, protocolNumber);

    NS_ASSERT(m_queue != nullptr);

    if (m_queue->Enqueue(packet))
    {
        // Packet is enqued
        m_packetDropRateTrace(1, m_node, m_destinationNode, false);

        // If the channel is ready for transition we send the packet right now
        if (m_txMachineState == READY)
        {
            packet = m_queue->Dequeue();
            bool ret = TransmitStart(packet);
            return ret;
        }
        return true;
    }

    // Packet is dropped
    m_packetDropRateTrace(1, m_node, m_destinationNode, true);

    return false;
}

bool
PointToPointIslNetDevice::SendFrom(Ptr<Packet> packet,
                                   const Address& source,
                                   const Address& dest,
                                   uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << source << dest << protocolNumber);
    return false;
}

Ptr<Node>
PointToPointIslNetDevice::GetNode(void) const
{
    return m_node;
}

void
PointToPointIslNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this);
    m_node = node;
}

bool
PointToPointIslNetDevice::NeedsArp(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

void
PointToPointIslNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    m_rxCallback = cb;
}

void
PointToPointIslNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
    m_promiscCallback = cb;
}

bool
PointToPointIslNetDevice::SupportsSendFrom(void) const
{
    NS_LOG_FUNCTION(this);
    return false;
}

Address
PointToPointIslNetDevice::GetRemote(void) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_channel->GetNDevices() == 2);
    for (std::size_t i = 0; i < m_channel->GetNDevices(); ++i)
    {
        Ptr<NetDevice> tmp = m_channel->GetDevice(i);
        if (tmp != this)
        {
            return tmp->GetAddress();
        }
    }
    NS_ASSERT(false);
    // quiet compiler.
    return Address();
}

bool
PointToPointIslNetDevice::SetMtu(uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

uint16_t
PointToPointIslNetDevice::GetMtu(void) const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

uint16_t
PointToPointIslNetDevice::PppToEther(uint16_t proto)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (proto)
    {
    case 0x0021:
        return 0x0800; // IPv4
    case 0x0057:
        return 0x86DD; // IPv6
    default:
        NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    return 0;
}

uint16_t
PointToPointIslNetDevice::EtherToPpp(uint16_t proto)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (proto)
    {
    case 0x0800:
        return 0x0021; // IPv4
    case 0x86DD:
        return 0x0057; // IPv6
    default:
        NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
    }
    return 0;
}

} // namespace ns3
