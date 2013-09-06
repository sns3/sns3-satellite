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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/channel.h"
#include "ns3/virtual-channel.h"

#include "satellite-net-device.h"
#include "satellite-phy.h"
#include "satellite-mac.h"
#include "satellite-channel.h"


NS_LOG_COMPONENT_DEFINE ("SatNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatNetDevice);

TypeId 
SatNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<SatNetDevice> ()
    .AddAttribute ("ReceiveErrorModel",
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&SatNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&SatNetDevice::m_phyRxDropTrace))
    .AddAttribute ("SatMac", "The Satellite MAC layer attached to this device.",
                    PointerValue (),
                    MakePointerAccessor (&SatNetDevice::GetMac,
                                         &SatNetDevice::SetMac),
                    MakePointerChecker<SatMac> ())
    .AddAttribute ("SatPhy", "The Satellite Phy layer attached to this device.",
                    PointerValue (),
                    MakePointerAccessor (&SatNetDevice::GetPhy,
                                         &SatNetDevice::SetPhy),
                    MakePointerChecker<SatPhy> ())
  ;
  return tid;
}

SatNetDevice::SatNetDevice ()
  : m_phy (0),
    m_mac (0),
    m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0)
{
  NS_LOG_FUNCTION (this);
}

void
SatNetDevice::ReceiveMac (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_LOGIC ("Time " << Simulator::Now ().GetSeconds () << ": receiving a packet: " << packet->GetUid());

  m_rxCallback (this, packet, Ipv4L3Protocol::PROT_NUMBER, Address ());
}

void 
SatNetDevice::SetPhy (Ptr<SatPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phy = phy;
}
void
SatNetDevice::SetMac (Ptr<SatMac> mac)
{
  NS_LOG_FUNCTION (this << mac);
  m_mac = mac;
}
void
SatNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void 
SatNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}
uint32_t 
SatNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}
Ptr<SatPhy>
SatNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}
Ptr<SatMac>
SatNetDevice::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}
void
SatNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);

  NS_ASSERT( m_mac );
  m_mac->SetAddress (m_address);
}
Address 
SatNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  NS_LOG_FUNCTION (this);
  return m_address;
}
bool 
SatNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}
uint16_t 
SatNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
bool 
SatNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void 
SatNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
 NS_LOG_FUNCTION (this << &callback);
}
bool 
SatNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
Address
SatNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool 
SatNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
Address 
SatNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address SatNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool 
SatNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
SatNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
SatNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  /*
   * \todo The duration should be specified by the TBTP or the GW/NCC scheduler
   */

  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_LOG_LOGIC ("Time " << Simulator::Now ().GetSeconds () << ": sending a packet: " << packet->GetUid() << ", dest: " << dest);

  m_mac->Send (packet, dest);

  return true;
}
bool 
SatNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);

  m_mac->Send (packet, dest);

  return true;
}

Ptr<Node> 
SatNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
void 
SatNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}
bool 
SatNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void 
SatNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

void
SatNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_phy = 0;
  m_mac->Dispose();
  m_mac = 0;
  m_node = 0;
  m_receiveErrorModel = 0;
  NetDevice::DoDispose ();
}


void
SatNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

bool
SatNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
SatNetDevice::SetVirtualChannel (Ptr<VirtualChannel> vChannel)
{
  NS_LOG_FUNCTION (this);

  m_virtualChannel = vChannel;
}


Ptr<Channel>
SatNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);

  return m_virtualChannel;
}


} // namespace ns3
