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
#include "sat-channel.h"
#include "sat-net-device.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SatChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatChannel);

TypeId 
SatChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatChannel")
    .SetParent<Channel> ()
    .AddConstructor<SatChannel> ()
    .AddAttribute ("Delay", "Transmission delay through the channel",
                       TimeValue (Seconds (0)),
                       MakeTimeAccessor (&SatChannel::m_delay),
                       MakeTimeChecker ())
    .AddTraceSource ("TxRxPointToPoint",
                     "Trace source indicating transmission of packet from the SatChannel, used by the Animation interface.",
                     MakeTraceSourceAccessor (&SatChannel::m_txrxPointToPoint))
  ;
  return tid;
}

SatChannel::SatChannel ()
{
  m_delay = Seconds (0.);
  NS_LOG_FUNCTION (this);
}

void
SatChannel::Send (Ptr<Packet> p, uint16_t protocol,
                     Mac48Address to, Mac48Address from,
                     Ptr<SatNetDevice> sender,
                     Time txTime)
{
  NS_LOG_FUNCTION (this << p << protocol << to << from << sender);
  for (std::vector<Ptr<SatNetDevice> >::const_iterator i = m_devices.begin (); i != m_devices.end (); ++i)
    {
      Ptr<SatNetDevice> tmp = *i;
      if (tmp != sender)
        {

          Simulator::ScheduleWithContext (tmp->GetNode ()->GetId (), txTime + m_delay,
                                      &SatNetDevice::Receive, tmp, p->Copy (), protocol, to, from);

          // Call the tx anim callback on the net device
          m_txrxPointToPoint (p, sender, tmp, txTime, txTime + m_delay);
        }
    }
}

void
SatChannel::Add (Ptr<SatNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_devices.push_back (device);
}

uint32_t
SatChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION (this);
  return m_devices.size ();
}
Ptr<NetDevice>
SatChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_devices[i];
}

} // namespace ns3
