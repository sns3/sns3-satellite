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

#include "satellite-geo-net-device.h"
#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-mac.h"
#include "ns3/satellite-channel.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv4-header.h"
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/channel.h>

NS_LOG_COMPONENT_DEFINE ("SatGeoNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoNetDevice);

TypeId 
SatGeoNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<SatGeoNetDevice> ()
    .AddAttribute ("ReceiveErrorModel",
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&SatGeoNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&SatGeoNetDevice::m_phyRxDropTrace))
  ;
  return tid;
}

SatGeoNetDevice::SatGeoNetDevice ()
  : m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0)
{
  NS_LOG_FUNCTION (this);
  m_feederPhy.reserve(98);
  m_userPhy.reserve(98);

  //LogComponentEnable ("SatGeoNetDevice", LOG_LEVEL_INFO);
}

void
SatGeoNetDevice::ReceiveUser (Ptr<Packet> packet, uint16_t beamId)
{
  NS_LOG_FUNCTION (this << packet);
  m_feederPhy[beamId]->SendPdu(packet, Seconds(0));
}

void
SatGeoNetDevice::ReceiveFeeder (Ptr<Packet> packet, uint16_t beamId)
{
  NS_LOG_FUNCTION (this << packet);
  m_userPhy[beamId]->SendPdu(packet, Seconds(0));
}

void
SatGeoNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void 
SatGeoNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}
uint32_t 
SatGeoNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}
void
SatGeoNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}
Address 
SatGeoNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  NS_LOG_FUNCTION (this);
  return m_address;
}
bool 
SatGeoNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}
uint16_t 
SatGeoNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
bool 
SatGeoNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void 
SatGeoNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
 NS_LOG_FUNCTION (this << &callback);
}
bool 
SatGeoNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
Address
SatGeoNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool 
SatGeoNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
Address 
SatGeoNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address SatGeoNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool 
SatGeoNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
SatGeoNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
SatGeoNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  // upper layer functionality not supported, should not be called
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_ASSERT (false);
  return false;
}
bool 
SatGeoNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  // upper layer functionality not supported, should not be called
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  NS_ASSERT (false);
  return false;
}

Ptr<Node> 
SatGeoNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
void 
SatGeoNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}
bool 
SatGeoNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
void 
SatGeoNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
}

void
SatGeoNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_receiveErrorModel = 0;
  m_userPhy.clear();
  m_feederPhy.clear();
  NetDevice::DoDispose ();
}

void
SatGeoNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

bool
SatGeoNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Ptr<Channel>
SatGeoNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);

  return NULL;
}

void
SatGeoNetDevice::AttachChannels (Ptr<SatChannel> ff, Ptr<SatChannel> fr, Ptr<SatChannel> uf, Ptr<SatChannel> ur, uint16_t beamId )
{
  NS_LOG_FUNCTION (this << ff << fr << uf << ur);

  // Create the first needed SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> uPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> uPhyRx = CreateObject<SatPhyRx> ();
  Ptr<SatPhyTx> fPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> fPhyRx = CreateObject<SatPhyRx> ();

  // Set SatChannels to SatPhyTx/SatPhyRx
  uPhyTx->SetChannel (uf);
  uPhyRx->SetChannel (ur);
  uPhyRx->SetDevice (this);

  fPhyTx->SetChannel (fr);
  fPhyRx->SetChannel (ff);
  fPhyRx->SetDevice (this);

  SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoNetDevice::ReceiveUser, this);
  SatPhy::ReceiveCallback fCb = MakeCallback (&SatGeoNetDevice::ReceiveFeeder, this);

  // Create SatPhy modules
  Ptr<SatPhy> uPhy = CreateObject<SatPhy> (uPhyTx, uPhyRx, beamId, uCb);
  Ptr<SatPhy> fPhy = CreateObject<SatPhy> (fPhyTx, fPhyRx, beamId, fCb);

  m_userPhy[beamId] = uPhy;
  m_feederPhy[beamId] = fPhy;
}

} // namespace ns3
