/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#include "ns3/log.h"
#include "ns3/node.h"
#include "ideal-net-device.h"

NS_LOG_COMPONENT_DEFINE ("IdealNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (IdealNetDevice);

TypeId 
IdealNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdealNetDevice")
    .SetParent<SimpleNetDevice> ()
    .AddConstructor<IdealNetDevice> ()
  ;
  return tid;
}

IdealNetDevice::IdealNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

bool
IdealNetDevice::Receive (Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t protocol,
                          const Address &from)
{
  return true;
}

bool
IdealNetDevice::ReceivePromisc (Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t protocol,
                          const Address &from, const Address &to, NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << packet << protocol << to << from);

  if (packetType != NetDevice::PACKET_OTHERHOST)
    {
      m_rxCallback (this, packet, protocol, from);

      if (!m_promiscCallback.IsNull ())
        {
          m_promiscCallback (this, packet, protocol, from, to, packetType);
        }
    }

  return true;
}
bool
IdealNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void 
IdealNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;

  NetDevice::ReceiveCallback receiveCb = MakeCallback (&IdealNetDevice::Receive, this);
  NetDevice::PromiscReceiveCallback promiscReceiveCb = MakeCallback (&IdealNetDevice::ReceivePromisc, this);

  SimpleNetDevice::SetReceiveCallback(receiveCb);
  SimpleNetDevice::SetPromiscReceiveCallback(promiscReceiveCb);
}

void
IdealNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

} // namespace ns3
