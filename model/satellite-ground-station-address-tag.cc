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

#include <ns3/log.h>

#include "satellite-ground-station-address-tag.h"


NS_LOG_COMPONENT_DEFINE ("SatGroundStationAddressTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGroundStationAddressTag);

SatGroundStationAddressTag::SatGroundStationAddressTag ()
{
  // Nothing to do here
}

SatGroundStationAddressTag::SatGroundStationAddressTag (Mac48Address groundStationAddress)
  : m_groundStationAddress (groundStationAddress)
{
  // Nothing to do here
}

TypeId
SatGroundStationAddressTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGroundStationAddressTag")
    .SetParent<Tag> ()
    .AddConstructor<SatGroundStationAddressTag> ();
  return tid;
}

TypeId
SatGroundStationAddressTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatGroundStationAddressTag::GetSerializedSize (void) const
{
  return ADDRESS_LENGHT;
}

void
SatGroundStationAddressTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);

  uint8_t buff[ADDRESS_LENGHT];

  m_groundStationAddress.CopyTo (buff);
  i.Write (buff, ADDRESS_LENGHT);
}

void
SatGroundStationAddressTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);

  uint8_t buff[ADDRESS_LENGHT];

  i.Read (buff, ADDRESS_LENGHT);
  m_groundStationAddress.CopyFrom (buff);
}

void
SatGroundStationAddressTag::Print (std::ostream &os) const
{
  os << m_groundStationAddress;
}

Mac48Address
SatGroundStationAddressTag::GetGroundStationAddress (void) const
{
  return m_groundStationAddress;
}

void
SatGroundStationAddressTag::SetGroundStationAddress (Mac48Address groundStationAddress)
{
  m_groundStationAddress = groundStationAddress;
}


} // namespace ns3

