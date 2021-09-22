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
  NS_LOG_FUNCTION (this << packet);

  std::cout << "Packet received" << std::endl;

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
  m_rxCallback (this, packet, Ipv4L3Protocol::PROT_NUMBER, Address ());
}

bool
SatLorawanNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  // TODO

  return true;
}

bool
SatLorawanNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);

  // TODO

  return true;
}

bool
SatLorawanNetDevice::SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest)
{
  NS_LOG_FUNCTION (this << msg << dest);

  // TODO

  return true;
}

void
SatLorawanNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  SatNetDevice::DoDispose ();
}

} // namespace ns3
