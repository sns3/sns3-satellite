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

#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/boolean.h>
#include <ns3/error-model.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/ipv4-header.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/channel.h>

#include "satellite-net-device.h"
#include <ns3/satellite-phy.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-llc.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-utils.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-address-tag.h>
#include <ns3/satellite-time-tag.h>
#include <ns3/satellite-typedefs.h>

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
    .AddAttribute ("SatLlc", "The Satellite Llc layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&SatNetDevice::GetLlc,
                                        &SatNetDevice::SetLlc),
                   MakePointerChecker<SatLlc> ())
    .AddAttribute ( "MaximumTransmissionUnit",
                    "Maximum transmission unit in Bytes",
                    UintegerValue (0xffff),
                    MakeUintegerAccessor (&SatNetDevice::m_mtu),
                    MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("EnableStatisticsTags",
                   "If true, some tags will be added to each transmitted packet to assist with statistics computation",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatNetDevice::m_isStatisticsTagsEnabled),
                   MakeBooleanChecker ())
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatNetDevice::m_packetTrace),
                     "ns3::SatTypedefs::PacketTraceCallback")
    .AddTraceSource ("Tx",
                     "A packet to be sent",
                     MakeTraceSourceAccessor (&SatNetDevice::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("SignallingTx",
                     "A signalling packet to be sent",
                     MakeTraceSourceAccessor (&SatNetDevice::m_signallingTxTrace),
                     "ns3::SatTypedefs::PacketDestinationAddressCallback")
    .AddTraceSource ("Rx",
                     "A packet received",
                     MakeTraceSourceAccessor (&SatNetDevice::m_rxTrace),
                     "ns3::SatTypedefs::PacketSourceAddressCallback")
    .AddTraceSource ("RxDelay",
                     "A packet is received with delay information",
                     MakeTraceSourceAccessor (&SatNetDevice::m_rxDelayTrace),
                     "ns3::SatTypedefs::PacketDelayAddressCallback")
  ;
  return tid;
}

SatNetDevice::SatNetDevice ()
  : m_phy (0),
    m_mac (0),
    m_llc (0),
    m_isStatisticsTagsEnabled (false),
    m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0)
{
  NS_LOG_FUNCTION (this);
}

void
SatNetDevice::Receive (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_INFO ("Time " << Simulator::Now ().GetSeconds () << ": receiving a packet: " << packet->GetUid ());

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld =
    (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_FORWARD : SatEnums::LD_RETURN;

  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_ND,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  /*
   * Invoke the `Rx` and `RxDelay` trace sources. We look at the packet's tags
   * for information, but cannot remove the tags because the packet is a const.
   */
  if (m_isStatisticsTagsEnabled)
    {
      Address addr; // invalid address.
      bool isTaggedWithAddress = false;
      ByteTagIterator it = packet->GetByteTagIterator ();

      while (!isTaggedWithAddress && it.HasNext ())
        {
          ByteTagIterator::Item item = it.Next ();

          if (item.GetTypeId () == SatAddressTag::GetTypeId ())
            {
              NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                 << " start=" << item.GetStart ()
                                 << " end=" << item.GetEnd ());
              SatAddressTag addrTag;
              item.GetTag (addrTag);
              addr = addrTag.GetSourceAddress ();
              isTaggedWithAddress = true; // this will exit the while loop.
            }
        }

      m_rxTrace (packet, addr);

      SatDevTimeTag timeTag;
      if (packet->PeekPacketTag (timeTag))
        {
          NS_LOG_DEBUG (this << " contains a SatDevTimeTag tag");
          m_rxDelayTrace (Simulator::Now () - timeTag.GetSenderTimestamp (),
                          addr);
        }
    }

  // Pass the packet to the upper layer.
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
SatNetDevice::SetLlc (Ptr<SatLlc> llc)
{
  NS_LOG_FUNCTION (this << llc);
  m_llc = llc;
}

void
SatNetDevice::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  m_nodeInfo = nodeInfo;
}

void
SatNetDevice::ToggleState (bool enabled)
{
  NS_LOG_FUNCTION (this << enabled);

  if (enabled)
    {
      m_mac->Enable ();
    }
  else
    {
      m_mac->Disable ();
    }
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

Ptr<SatLlc>
SatNetDevice::GetLlc (void) const
{
  NS_LOG_FUNCTION (this);
  return m_llc;
}

void
SatNetDevice::SetPacketClassifier (Ptr<SatPacketClassifier> classifier)
{
  NS_LOG_FUNCTION (this);
  m_classifier = classifier;
}

Ptr<SatPacketClassifier>
SatNetDevice::GetPacketClassifier () const
{
  NS_LOG_FUNCTION (this);
  return m_classifier;
}

void
SatNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
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
  return true;
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
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  if (m_isStatisticsTagsEnabled)
    {
      // Add a SatAddressTag tag with this device's address as the source address.
      packet->AddByteTag (SatAddressTag (m_nodeInfo->GetMacAddress ()));

      // Add a SatDevTimeTag tag for packet delay computation at the receiver end.
      packet->AddPacketTag (SatDevTimeTag (Simulator::Now ()));
    }

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld =
    (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_ND,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  m_txTrace (packet);

  uint8_t flowId = m_classifier->Classify (packet, dest, protocolNumber);
  m_llc->Enque (packet, dest, flowId);

  return true;
}
bool
SatNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);

  if (m_isStatisticsTagsEnabled)
    {
      // Add a SatAddressTag tag with this device's address as the source address.
      packet->AddByteTag (SatAddressTag (m_nodeInfo->GetMacAddress ()));

      // Add a SatDevTimeTag tag for packet delay computation at the receiver end.
      packet->AddPacketTag (SatDevTimeTag (Simulator::Now ()));
    }

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld =
    (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_ND,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  m_txTrace (packet);

  uint8_t flowId = m_classifier->Classify (packet, dest, protocolNumber);
  m_llc->Enque (packet, dest, flowId);

  return true;
}

bool
SatNetDevice::SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest)
{
  NS_LOG_FUNCTION (this << msg << dest);

  Ptr<Packet> packet = Create<Packet> (msg->GetSizeInBytes ());

  if (m_isStatisticsTagsEnabled)
    {
      // Add a SatAddressTag tag with this device's address as the source address.
      packet->AddByteTag (SatAddressTag (m_nodeInfo->GetMacAddress ()));

      // Add a SatDevTimeTag tag for packet delay computation at the receiver end.
      packet->AddPacketTag (SatDevTimeTag (Simulator::Now ()));
    }

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld =
    (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_ND,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  // Add control tag to message and write msg to container in MAC
  SatControlMsgTag tag;
  uint32_t id = m_mac->ReserveIdAndStoreCtrlMsgToContainer (msg);
  tag.SetMsgId (id);
  tag.SetMsgType (msg->GetMsgType ());
  packet->AddPacketTag (tag);

  uint8_t flowId = m_classifier->Classify (msg->GetMsgType (), dest);

  m_signallingTxTrace (packet, dest);

  m_llc->Enque (packet, dest, flowId);

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
  m_rxCallback.Nullify ();
  m_promiscCallback.Nullify ();
  m_phy = 0;
  m_mac->Dispose ();
  m_mac = 0;
  m_node = 0;
  m_receiveErrorModel = 0;
  m_llc->Dispose ();
  m_llc = 0;
  m_classifier = 0;

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

Ptr<Channel>
SatNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);

  /**
   * We cannot do anything here, since the SatNetDevice does not hold
   * directly any channels, but they are attached to Phy layers.
   */
  return 0;
}


} // namespace ns3
