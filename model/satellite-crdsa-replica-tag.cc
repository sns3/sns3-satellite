/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

#include "ns3/log.h"
#include "satellite-crdsa-replica-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatCrdsaReplicaTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatCrdsaReplicaTag);


SatCrdsaReplicaTag::SatCrdsaReplicaTag ()
  : m_numOfIds (0)
{
  NS_LOG_FUNCTION (this);
}

SatCrdsaReplicaTag::~SatCrdsaReplicaTag ()
{
  NS_LOG_FUNCTION (this);

  m_slotIds.clear ();
}

TypeId
SatCrdsaReplicaTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatCrdsaReplicaTag")
    .SetParent<Tag> ()
    .AddConstructor<SatCrdsaReplicaTag> ()
  ;
  return tid;
}
TypeId
SatCrdsaReplicaTag::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

void
SatCrdsaReplicaTag::AddSlotId (uint16_t slotId)
{
  NS_LOG_FUNCTION (this);
  m_slotIds.push_back (slotId);
  m_numOfIds = m_slotIds.size ();
}

std::vector<uint16_t>
SatCrdsaReplicaTag::GetSlotIds (void)
{
  NS_LOG_FUNCTION (this);
  return m_slotIds;
}

uint32_t
SatCrdsaReplicaTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);

  return ( sizeof (uint8_t) + m_slotIds.size () * sizeof (uint16_t) );
}
void
SatCrdsaReplicaTag::Serialize (TagBuffer buffer) const
{
  NS_LOG_FUNCTION (this << &buffer);

  buffer.WriteU8 (m_numOfIds);

  for (uint8_t i = 0; i < m_numOfIds; i++)
    {
      buffer.WriteU16 (m_slotIds[i]);
    }
}

void
SatCrdsaReplicaTag::Deserialize (TagBuffer buffer)
{
  NS_LOG_FUNCTION (this << &buffer);

  m_numOfIds = buffer.ReadU8 ();

  for (uint8_t i = 0; i < m_numOfIds; i++)
    {
      m_slotIds.push_back (buffer.ReadU16 ());
    }
}

void
SatCrdsaReplicaTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "id=";

  for (uint8_t i = 0; i < m_numOfIds; i++)
    {
      os << m_slotIds[i] << " ";
    }
}


} // namespace ns3
