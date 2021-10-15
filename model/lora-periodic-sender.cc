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
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/pointer.h>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/string.h>
#include <ns3/satellite-lorawan-net-device.h>

#include "lora-periodic-sender.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraPeriodicSender");

NS_OBJECT_ENSURE_REGISTERED (LoraPeriodicSender);

TypeId
LoraPeriodicSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraPeriodicSender")
    .SetParent<Application> ()
    .AddConstructor<LoraPeriodicSender> ()
    .SetGroupName ("lorawan")
    .AddAttribute ("Interval", "The interval between packet sends of this app",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&LoraPeriodicSender::GetInterval,
                                     &LoraPeriodicSender::SetInterval),
                   MakeTimeChecker ());
  // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of the packet size, in bytes",
  //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
  //                MakePointerAccessor (&LoraPeriodicSender::m_pktSizeRV),
  //                MakePointerChecker <RandomVariableStream>());
  return tid;
}

LoraPeriodicSender::LoraPeriodicSender ()
  : m_interval (Seconds (10)),
  m_initialDelay (Seconds (1)),
  m_basePktSize (10),
  m_pktSizeRV (0)

{
  NS_LOG_FUNCTION_NOARGS ();
}

LoraPeriodicSender::~LoraPeriodicSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LoraPeriodicSender::SetInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_interval = interval;
}

Time
LoraPeriodicSender::GetInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interval;
}

void
LoraPeriodicSender::SetInitialDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_initialDelay = delay;
}


void
LoraPeriodicSender::SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv)
{
  m_pktSizeRV = rv;
}


void
LoraPeriodicSender::SetPacketSize (uint8_t size)
{
  m_basePktSize = size;
}


void
LoraPeriodicSender::SendPacket (void)
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
  m_mac->Send (packet);

  // Schedule the next SendPacket event
  m_sendEvent = Simulator::Schedule (m_interval, &LoraPeriodicSender::SendPacket,
                                     this);

  NS_LOG_DEBUG ("Sent a packet of size " << packet->GetSize ());
}

void
LoraPeriodicSender::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // Make sure we have a MAC layer
  if (m_mac == 0)
    {
      Ptr<SatLorawanNetDevice> loraNetDevice;
      for (uint32_t i = 0; i < m_node->GetNDevices (); i++)
        {
          loraNetDevice = DynamicCast<SatLorawanNetDevice> (m_node->GetDevice (i));
          if (loraNetDevice)
            {
              break;
            }
        }

      m_mac = DynamicCast<LorawanMac> (loraNetDevice->GetMac ());
      NS_ASSERT (m_mac != 0);
    }

  // Schedule the next SendPacket event
  Simulator::Cancel (m_sendEvent);
  NS_LOG_DEBUG ("Starting up application with a first event with a " <<
                m_initialDelay.GetSeconds () << " seconds delay");
  m_sendEvent = Simulator::Schedule (m_initialDelay,
                                     &LoraPeriodicSender::SendPacket, this);
  NS_LOG_DEBUG ("Event Id: " << m_sendEvent.GetUid ());
}

void
LoraPeriodicSender::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}

}
