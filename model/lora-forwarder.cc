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

#include <ns3/log.h>

#include <ns3/lora-beam-tag.h>

#include "ns3/lora-forwarder.h"

NS_LOG_COMPONENT_DEFINE ("LoraForwarder");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoraForwarder);

TypeId
LoraForwarder::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraForwarder")
    .SetParent<Application> ()
    .AddConstructor<LoraForwarder> ();
  return tid;
}

LoraForwarder::LoraForwarder ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

LoraForwarder::~LoraForwarder ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LoraForwarder::SetPointToPointNetDevice (Ptr<PointToPointNetDevice> pointToPointNetDevice)
{
  NS_LOG_FUNCTION (this << pointToPointNetDevice);

  m_pointToPointNetDevice = pointToPointNetDevice;
}

void
LoraForwarder::SetLoraNetDevice (uint8_t beamId, Ptr<SatLorawanNetDevice> loraNetDevice)
{
  NS_LOG_FUNCTION (this << loraNetDevice);

  m_satLorawanNetDevices[beamId] = loraNetDevice;
}

bool
LoraForwarder::ReceiveFromLora (Ptr<SatLorawanNetDevice> loraNetDevice, Ptr<const Packet> packet, uint16_t protocol, const Address& sender)
{
  NS_LOG_FUNCTION (this << packet << protocol << sender);

  Ptr<Packet> packetCopy = packet->Copy ();

  LoraBeamTag beamTag = LoraBeamTag (loraNetDevice->GetLorawanMac ()->GetBeamId ());
  packetCopy->AddPacketTag (beamTag);

  m_pointToPointNetDevice->Send (packetCopy, m_pointToPointNetDevice->GetBroadcast (), protocol);

  return true;
}

bool
LoraForwarder::ReceiveFromPointToPoint (Ptr<NetDevice> pointToPointNetDevice, Ptr<const Packet> packet, uint16_t protocol, const Address& sender)
{
  NS_LOG_FUNCTION (this << packet << protocol << sender);

  Ptr<Packet> packetCopy = packet->Copy ();

  LoraBeamTag tag;
  packetCopy->RemovePacketTag(tag);
  uint8_t beamId = tag.GetBeamId ();

  // TODO not sure address is correct...
  m_satLorawanNetDevices[beamId]->Send (packetCopy, sender, protocol);

  return true;
}

void
LoraForwarder::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // TODO Make sure we are connected to both needed devices
}

void
LoraForwarder::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // TODO Get rid of callbacks
}

}
