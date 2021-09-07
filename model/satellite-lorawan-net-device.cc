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

  // TODO
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

} // namespace ns3
