/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/periodic-sender.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/satellite-lorawan-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PeriodicSender");

NS_OBJECT_ENSURE_REGISTERED (PeriodicSender);

TypeId
PeriodicSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PeriodicSender")
    .SetParent<Application> ()
    .AddConstructor<PeriodicSender> ()
    .SetGroupName ("lorawan")
    .AddAttribute ("Interval", "The interval between packet sends of this app",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PeriodicSender::GetInterval,
                                     &PeriodicSender::SetInterval),
                   MakeTimeChecker ());
  // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of the packet size, in bytes",
  //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
  //                MakePointerAccessor (&PeriodicSender::m_pktSizeRV),
  //                MakePointerChecker <RandomVariableStream>());
  return tid;
}

PeriodicSender::PeriodicSender ()
  : m_interval (Seconds (10)),
  m_initialDelay (Seconds (1)),
  m_basePktSize (10),
  m_pktSizeRV (0)

{
  NS_LOG_FUNCTION_NOARGS ();
}

PeriodicSender::~PeriodicSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
PeriodicSender::SetInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_interval = interval;
}

Time
PeriodicSender::GetInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interval;
}

void
PeriodicSender::SetInitialDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_initialDelay = delay;
}


void
PeriodicSender::SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv)
{
  m_pktSizeRV = rv;
}


void
PeriodicSender::SetPacketSize (uint8_t size)
{
  m_basePktSize = size;
}


void
PeriodicSender::SendPacket (void)
{
  NS_LOG_FUNCTION (this);

  // Create and send a new packet
  Ptr<Packet> packet;
  if (m_pktSizeRV)
    {
      int randomsize = m_pktSizeRV->GetInteger ();
      packet = Create<Packet> (m_basePktSize + randomsize);
    }
  else
    {
      packet = Create<Packet> (m_basePktSize);
    }
  //m_mac->Send (packet);
  //SatNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
  //m_netDevice->Send (packet, );

  // Schedule the next SendPacket event
  m_sendEvent = Simulator::Schedule (m_interval, &PeriodicSender::SendPacket,
                                     this);

  NS_LOG_DEBUG ("Sent a packet of size " << packet->GetSize ());
}

void
PeriodicSender::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  std::cout << m_node << std::endl;

  // Make sure we have a MAC layer
  if (m_mac == 0)
    {
      // Assumes there's only one device
      Ptr<SatLorawanNetDevice> loraNetDevice1 = m_node->GetDevice (0)->GetObject<SatLorawanNetDevice> ();
      Ptr<SatLorawanNetDevice> loraNetDevice2 = m_node->GetDevice (1)->GetObject<SatLorawanNetDevice> ();
      Ptr<SatNetDevice> satNetDevice1 = m_node->GetDevice (0)->GetObject<SatNetDevice> ();
      Ptr<SatNetDevice> satNetDevice2 = m_node->GetDevice (1)->GetObject<SatNetDevice> ();
      Ptr<NetDevice> netDevice1 = m_node->GetDevice (0)->GetObject<NetDevice> ();
      Ptr<NetDevice> netDevice2 = m_node->GetDevice (1)->GetObject<NetDevice> ();

      std::cout << "m_node->GetNDevices () " << m_node->GetNDevices () << std::endl;
      std::cout << "m_node->GetDevice (0) " << m_node->GetDevice (0) << std::endl;
      std::cout << "m_node->GetDevice (1) " << m_node->GetDevice (1) << std::endl;
      std::cout << "loraNetDevice1 " << loraNetDevice1 << std::endl;
      std::cout << "loraNetDevice2 " << loraNetDevice2 << std::endl;
      std::cout << "satNetDevice1 " << satNetDevice1 << std::endl;
      std::cout << "satNetDevice2 " << satNetDevice2 << std::endl;
      std::cout << "netDevice1 " << netDevice1 << std::endl;
      std::cout << "netDevice2 " << netDevice2 << std::endl;
      std::cout << "cast netDevice1 " << DynamicCast<SatLorawanNetDevice> (netDevice1) << std::endl;
      std::cout << "cast netDevice2 " << DynamicCast<SatLorawanNetDevice> (netDevice2) << std::endl;
      //m_mac = loraNetDevice2->GetLorawanMac ();
      m_netDevice = netDevice2;
      //NS_ASSERT (m_mac != 0);
    }

  // Schedule the next SendPacket event
  Simulator::Cancel (m_sendEvent);
  NS_LOG_DEBUG ("Starting up application with a first event with a " <<
                m_initialDelay.GetSeconds () << " seconds delay");
  m_sendEvent = Simulator::Schedule (m_initialDelay,
                                     &PeriodicSender::SendPacket, this);
  NS_LOG_DEBUG ("Event Id: " << m_sendEvent.GetUid ());
}

void
PeriodicSender::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}

}
