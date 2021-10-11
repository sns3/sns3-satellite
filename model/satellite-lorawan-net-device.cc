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

NS_LOG_COMPONENT_DEFINE ("SatLorawanNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLorawanNetDevice);

TypeId
SatLorawanNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLorawanNetDevice")
    .SetParent<SatNetDevice> ()
    .AddConstructor<SatLorawanNetDevice> ()
  ;
  return tid;
}

SatLorawanNetDevice::SatLorawanNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
SatLorawanNetDevice::Receive (Ptr<const Packet> packet)
{
  // TODO edit

  NS_LOG_FUNCTION (this << packet);
  NS_LOG_INFO ("Receiving a packet: " << packet->GetUid ());

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
          NS_LOG_DEBUG (this << " contains a SatMacTimeTag tag");
          Time delay = Simulator::Now () - timeTag.GetSenderTimestamp ();
          m_rxDelayTrace (delay, addr);
          if (m_lastDelay.IsZero() == false)
            {
              Time jitter = Abs (delay - m_lastDelay);
              m_rxJitterTrace (jitter, addr);
            }
          m_lastDelay = delay;
        }
    }

  // Pass the packet to the upper layer.
  // m_rxCallback (this, packet, Ipv4L3Protocol::PROT_NUMBER, Address ());
}

bool
SatLorawanNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  // TODO edit

  NS_LOG_FUNCTION (this << packet);

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

  m_lorawanMac->Send (packet, dest, protocolNumber);

  return true;
}

bool
SatLorawanNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  // TODO edit

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

/*void
SatLorawanNetDevice::OnReceivedPacket (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (packet);

  // Get the current packet's frame counter
  Ptr<Packet> packetCopy = packet->Copy ();
  LorawanMacHeader receivedMacHdr;
  packetCopy->RemoveHeader (receivedMacHdr);
  LoraFrameHeader receivedFrameHdr;
  receivedFrameHdr.SetAsUplink ();
  packetCopy->RemoveHeader (receivedFrameHdr);

  // Need to decide whether to schedule a receive window
  if (!m_status->GetEndDeviceStatus (packet)->HasReceiveWindowOpportunityScheduled ())
  {
    // Extract the address
    LoraDeviceAddress deviceAddress = receivedFrameHdr.GetAddress ();

    // Schedule OnReceiveWindowOpportunity event
    m_status->GetEndDeviceStatus (packet)->SetReceiveWindowOpportunity (
      Simulator::Schedule (Seconds (1),
                           &SatLorawanNetDevice::OnReceiveWindowOpportunity,
                           this,
                           deviceAddress,
                           1)); // This will be the first receive window
  }
}

void
SatLorawanNetDevice::OnReceiveWindowOpportunity (LoraDeviceAddress deviceAddress, int window)
{
  NS_LOG_FUNCTION (deviceAddress);

  NS_LOG_DEBUG ("Opening receive window number " << window << " for device "
                                                 << deviceAddress);

  // Check whether we can send a reply to the device, again by using
  // NetworkStatus
  Address gwAddress = m_status->GetBestGatewayForDevice (deviceAddress, window);

  if (gwAddress == Address () && window == 1)
    {
      NS_LOG_DEBUG ("No suitable gateway found for first window.");

      // No suitable GW was found, but there's still hope to find one for the
      // second window.
      // Schedule another OnReceiveWindowOpportunity event
      m_status->GetEndDeviceStatus (deviceAddress)->SetReceiveWindowOpportunity (
        Simulator::Schedule (Seconds (1),
                             &NetworkScheduler::OnReceiveWindowOpportunity,
                             this,
                             deviceAddress,
                             2));     // This will be the second receive window
    }
  else if (gwAddress == Address () && window == 2)
    {
      // No suitable GW was found and this was our last opportunity
      // Simply give up.
      NS_LOG_DEBUG ("Giving up on reply: no suitable gateway was found " <<
                   "on the second receive window");

      // Reset the reply
      // XXX Should we reset it here or keep it for the next opportunity?
      m_status->GetEndDeviceStatus (deviceAddress)->RemoveReceiveWindowOpportunity();
      m_status->GetEndDeviceStatus (deviceAddress)->InitializeReply ();
    }
  else
    {
      // A gateway was found

      NS_LOG_DEBUG ("Found available gateway with address: " << gwAddress);

      m_controller->BeforeSendingReply (m_status->GetEndDeviceStatus
                                          (deviceAddress));

      // Check whether this device needs a response by querying m_status
      bool needsReply = m_status->NeedsReply (deviceAddress);

      if (needsReply)
        {
          NS_LOG_INFO ("A reply is needed");

          // Send the reply through that gateway
          m_status->SendThroughGateway (m_status->GetReplyForDevice
                                          (deviceAddress, window),
                                        gwAddress);

          // Reset the reply
          m_status->GetEndDeviceStatus (deviceAddress)->RemoveReceiveWindowOpportunity();
          m_status->GetEndDeviceStatus (deviceAddress)->InitializeReply ();
        }
    }
}*/

bool
SatLorawanNetDevice::SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest)
{
  // We send nothing in Lora Mode. Control is made via LorawanMacCommand

  return true;
}

Ptr<LorawanMac>
SatLorawanNetDevice::GetLorawanMac ()
{
  return m_lorawanMac;
}

void
SatLorawanNetDevice::SetLorawanMac (Ptr<LorawanMac> lorawanMac)
{
  SetMac(lorawanMac);
  m_lorawanMac = lorawanMac;
}

void
SatLorawanNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  SatNetDevice::DoDispose ();
}

} // namespace ns3
