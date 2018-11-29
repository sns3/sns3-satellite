/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
#include "satellite-simple-channel.h"
#include "satellite-simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SatSimpleChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSimpleChannel)
;

TypeId
SatSimpleChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSimpleChannel")
    .SetParent<Channel> ()
    .AddConstructor<SatSimpleChannel> ()
  ;
  return tid;
}

SatSimpleChannel::SatSimpleChannel ()
{
  NS_LOG_FUNCTION (this);
}

void
SatSimpleChannel::Send (Ptr<Packet> p, uint16_t protocol,
                        Mac48Address to, Mac48Address from,
                        Ptr<SatSimpleNetDevice> sender)
{
  NS_LOG_FUNCTION (this << p << protocol << to << from << sender)
  ;
  for (std::vector<Ptr<SatSimpleNetDevice> >::const_iterator i = m_devices.begin (); i != m_devices.end (); ++i)
    {
      Ptr<SatSimpleNetDevice> device = *i;

      if (device != sender)
        {
          Simulator::ScheduleWithContext (device->GetNode ()->GetId (), Seconds (0),
                                          &SatSimpleNetDevice::Receive, device, p->Copy (), protocol, to, from);
        }
    }
}

void
SatSimpleChannel::Add (Ptr<SatSimpleNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_devices.push_back (device);
}

std::size_t
SatSimpleChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION (this);
  return m_devices.size ();
}
Ptr<NetDevice>
SatSimpleChannel::GetDevice (std::size_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_devices[i];
}

} // namespace ns3
