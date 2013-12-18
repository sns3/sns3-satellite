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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#include "satellite-id-mapper.h"

NS_LOG_COMPONENT_DEFINE ("SatIdMapper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatIdMapper);

TypeId 
SatIdMapper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatIdMapper")
    .SetParent<Object> ()
    .AddConstructor<SatIdMapper> ();
  return tid;
}

SatIdMapper::SatIdMapper () :
  m_index (0)
{
  NS_LOG_FUNCTION (this);
}

SatIdMapper::~SatIdMapper ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatIdMapper::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();

  Object::DoDispose ();
}

void
SatIdMapper::Reset ()
{
  NS_LOG_FUNCTION (this);

  if (!m_macToIdMap.empty ())
    {
      m_macToIdMap.clear ();
    }

  if (!m_idToMacMap.empty ())
    {
      m_idToMacMap.clear ();
    }
  m_index = 0;
}

void
SatIdMapper::AddMacToMapper (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToId = m_macToIdMap.insert (std::make_pair (mac, m_index));

  if (resultMacToId.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AddMacToMap - MAC to ID failed");
    }

  std::pair < std::map<uint32_t, Address>::iterator, bool> resultIdToMac = m_idToMacMap.insert (std::make_pair (m_index, mac));

  if (resultIdToMac.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AddMacToMap - ID to MAC failed");
    }

  NS_LOG_INFO ("SatMacIdMacMapper::AddMacToMap - Added MAC " << mac << " with ID " << m_index);

  m_index++;
}

Address
SatIdMapper::GetMac (uint32_t id)
{
  NS_LOG_FUNCTION (this);

  std::map<uint32_t, Address>::iterator iter = m_idToMacMap.find (id);

  if (iter == m_idToMacMap.end ())
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::GetMac - ID " << id << " not found");
    }

  return iter->second;
}

uint32_t
SatIdMapper::GetDeviceId (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::iterator iter = m_macToIdMap.find (mac);

  if (iter == m_macToIdMap.end ())
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::GetId - MAC " << mac << " not found");
    }

  return iter->second;
}

void
SatIdMapper::AddBroadcastMac ()
{
  AddMacToMapper (Mac48Address::GetBroadcast ());
}

void
SatIdMapper::PrintMap ()
{
  std::map<uint32_t, Address>::iterator iter;

  for (iter = m_idToMacMap.begin(); iter != m_idToMacMap.end(); iter++)
    {
      std::cout << "ID: " << iter->first << " MAC: " << iter->second << std::endl;
    }
}

} // namespace ns3
