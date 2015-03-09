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
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/address.h>
#include <ns3/satellite-net-device.h>
#include <sstream>

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

SatIdMapper::SatIdMapper ()
  : m_traceIdIndex (1),
    m_utIdIndex (1),
    m_utUserIdIndex (1),
    m_gwUserIdIndex (1),
    m_enableMapPrint (false)
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

  if (m_enableMapPrint)
    {
      PrintTraceMap ();
    }

  // Trace ID maps

  if (!m_macToTraceIdMap.empty ())
    {
      m_macToTraceIdMap.clear ();
    }
  m_traceIdIndex = 1;

  // UT ID maps

  if (!m_macToUtIdMap.empty ())
    {
      m_macToUtIdMap.clear ();
    }
  m_utIdIndex = 1;

  // UT user ID maps

  if (!m_macToUtUserIdMap.empty ())
    {
      m_macToUtUserIdMap.clear ();
    }
  m_utUserIdIndex = 1;

  // Beam ID maps

  if (!m_macToBeamIdMap.empty ())
    {
      m_macToBeamIdMap.clear ();
    }

  // GW ID maps

  if (!m_macToGwIdMap.empty ())
    {
      m_macToGwIdMap.clear ();
    }

  // GW user ID maps

  if (!m_macToGwUserIdMap.empty ())
    {
      m_macToGwUserIdMap.clear ();
    }
  m_gwUserIdIndex = 1;

  m_enableMapPrint = false;
}

// ATTACH TO MAPS

uint32_t
SatIdMapper::AttachMacToTraceId (Address mac)
{
  NS_LOG_FUNCTION (this);

  const uint32_t ret = m_traceIdIndex;
  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToTraceId = m_macToTraceIdMap.insert (std::make_pair (mac, m_traceIdIndex));

  if (resultMacToTraceId.second == false)
    {
      NS_FATAL_ERROR ("SatIdMapper::AttachMacToTraceId - MAC to Trace ID failed");
    }

  NS_LOG_INFO ("SatIdMapper::AttachMacToTraceId - Added MAC " << mac << " with Trace ID " << m_traceIdIndex);

  m_traceIdIndex++;
  return ret;
}

uint32_t
SatIdMapper::AttachMacToUtId (Address mac)
{
  NS_LOG_FUNCTION (this);

  const uint32_t ret = m_utIdIndex;
  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToUtId = m_macToUtIdMap.insert (std::make_pair (mac, m_utIdIndex));

  if (resultMacToUtId.second == false)
    {
      NS_FATAL_ERROR ("SatIdMapper::AttachMacToUtId - MAC to UT ID failed");
    }

  NS_LOG_INFO ("SatIdMapper::AttachMacToUtId - Added MAC " << mac << " with UT ID " << m_utIdIndex);

  m_utIdIndex++;
  return ret;
}

uint32_t
SatIdMapper::AttachMacToUtUserId (Address mac)
{
  NS_LOG_FUNCTION (this);

  const uint32_t ret = m_utUserIdIndex;
  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToUtUserId = m_macToUtUserIdMap.insert (std::make_pair (mac, m_utUserIdIndex));

  if (resultMacToUtUserId.second == false)
    {
      NS_FATAL_ERROR ("SatIdMapper::AttachMacToUtUserId - MAC to UT user ID failed");
    }

  NS_LOG_INFO ("SatIdMapper::AttachMacToUtUserId - Added MAC " << mac << " with UT user ID " << m_utUserIdIndex);

  m_utUserIdIndex++;
  return ret;
}

void
SatIdMapper::AttachMacToBeamId (Address mac, uint32_t beamId)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToBeamId = m_macToBeamIdMap.insert (std::make_pair (mac, beamId));

  if (resultMacToBeamId.second == false)
    {
      NS_FATAL_ERROR ("SatIdMapper::AttachMacToBeamId - MAC to beam ID failed");
    }

  NS_LOG_INFO ("SatIdMapper::AttachMacToBeamId - Added MAC " << mac << " with beam ID " << beamId);
}

void
SatIdMapper::AttachMacToGwId (Address mac, uint32_t gwId)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToGwId = m_macToGwIdMap.insert (std::make_pair (mac, gwId));

  if (resultMacToGwId.second == false)
    {
      NS_FATAL_ERROR ("SatIdMapper::AttachMacToGwId - MAC to GW ID failed");
    }

  NS_LOG_INFO ("SatIdMapper::AttachMacToGwId - Added MAC " << mac << " with GW ID " << gwId);
}

uint32_t
SatIdMapper::AttachMacToGwUserId (Address mac)
{
  NS_LOG_FUNCTION (this);

  const uint32_t ret = m_gwUserIdIndex;
  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToGwUserId = m_macToGwUserIdMap.insert (std::make_pair (mac, m_gwUserIdIndex));

  if (resultMacToGwUserId.second == false)
    {
      NS_FATAL_ERROR ("SatIdMapper::AttachMacToGwUserId - MAC to GW user ID failed");
    }

  NS_LOG_INFO ("SatIdMapper::AttachMacToGwUserId - Added MAC " << mac << " with GW user ID " << m_gwUserIdIndex);

  m_gwUserIdIndex++;
  return ret;
}

// ID GETTERS

int32_t
SatIdMapper::GetTraceIdWithMac (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter = m_macToTraceIdMap.find (mac);

  if (iter == m_macToTraceIdMap.end ())
    {
      return -1;
    }

  return iter->second;
}

int32_t
SatIdMapper::GetUtIdWithMac (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter = m_macToUtIdMap.find (mac);

  if (iter == m_macToUtIdMap.end ())
    {
      return -1;
    }

  return iter->second;
}

int32_t
SatIdMapper::GetUtUserIdWithMac (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter = m_macToUtUserIdMap.find (mac);

  if (iter == m_macToUtUserIdMap.end ())
    {
      return -1;
    }

  return iter->second;
}

int32_t
SatIdMapper::GetBeamIdWithMac (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter = m_macToBeamIdMap.find (mac);

  if (iter == m_macToBeamIdMap.end ())
    {
      return -1;
    }

  return iter->second;
}

int32_t
SatIdMapper::GetGwIdWithMac (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter = m_macToGwIdMap.find (mac);

  if (iter == m_macToGwIdMap.end ())
    {
      return -1;
    }

  return iter->second;
}

int32_t
SatIdMapper::GetGwUserIdWithMac (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter = m_macToGwUserIdMap.find (mac);

  if (iter == m_macToGwUserIdMap.end ())
    {
      return -1;
    }

  return iter->second;
}

// NODE GETTERS

Address
SatIdMapper::GetGwMacWithNode (Ptr<Node> gwNode) const
{
  NS_LOG_FUNCTION (this << gwNode->GetId ());

  if (gwNode->GetNDevices () >= 3)
    {
      /*
       * Assuming that device #0 is for loopback device, device #(N-1) is for
       * backbone network device, and devices #1 until #(N-2) are for satellite
       * beam device.
       */
      Ptr<NetDevice> dev = gwNode->GetDevice (1);

      if (dev->GetObject<SatNetDevice> () != 0)
        {
          if (Mac48Address::IsMatchingType (dev->GetAddress ()))
            {
              return dev->GetAddress ();
            }
          else
            {
              NS_LOG_WARN (this << " Device 1 of Node " << gwNode->GetId ()
                                << " is not have a valid Mac48Address");
              return Address (); // returns an invalid address
            }
        }
      else
        {
          NS_LOG_WARN (this << " Node " << gwNode->GetId ()
                            << " is not a valid GW");
          return Address (); // returns an invalid address
        }
    }
  else
    {
      NS_LOG_WARN (this << " Node " << gwNode->GetId ()
                        << " is not a valid GW");
      return Address (); // returns an invalid address
    }
}

Address
SatIdMapper::GetUtMacWithNode (Ptr<Node> utNode) const
{
  NS_LOG_FUNCTION (this << utNode->GetId ());

  if (utNode->GetNDevices () >= 3)
    {
      /*
       * Assuming that device #0 is for loopback device, device #1 is for
       * subscriber network device, and device #2 is for satellite beam device.
       */
      Ptr<NetDevice> dev = utNode->GetDevice (2);

      if (dev->GetObject<SatNetDevice> () != 0)
        {
          if (Mac48Address::IsMatchingType (dev->GetAddress ()))
            {
              return dev->GetAddress ();
            }
          else
            {
              NS_LOG_WARN (this << " Device 2 of Node " << utNode->GetId ()
                                << " is not have a valid Mac48Address");
              return Address (); // returns an invalid address
            }
        }
      else
        {
          NS_LOG_WARN (this << " Node " << utNode->GetId ()
                            << " is not a valid UT");
          return Address (); // returns an invalid address
        }
    }
  else
    {
      NS_LOG_WARN (this << " Node " << utNode->GetId ()
                        << " is not a valid UT");
      return Address (); // returns an invalid address
    }
}

Address
SatIdMapper::GetUtUserMacWithNode (Ptr<Node> utUserNode) const
{
  NS_LOG_FUNCTION (this << utUserNode->GetId ());

  if (utUserNode->GetNDevices () >= 2)
    {
      /*
       * Assuming that #0 is for loopback device and #1 is for subscriber
       * network device.
       */
      Ptr<NetDevice> dev = utUserNode->GetDevice (1);
      NS_ASSERT (dev != 0);

      if (Mac48Address::IsMatchingType (dev->GetAddress ()))
        {
          return dev->GetAddress ();
        }
      else
        {
          NS_LOG_WARN (this << " Device 1 of Node " << utUserNode->GetId ()
                            << " is not have a valid Mac48Address");
          return Address (); // returns an invalid address
        }
    }
  else
    {
      NS_LOG_WARN (this << " Node " << utUserNode->GetId ()
                        << " is not a valid UT user");
      return Address (); // returns an invalid address
    }
}

std::string
SatIdMapper::GetMacInfo (Address mac) const
{
  NS_LOG_FUNCTION (this);

  std::stringstream out;
  bool isInMap = false;

  out << mac << " ";

  std::map<Address, uint32_t>::const_iterator iterTrace = m_macToTraceIdMap.find (mac);

  if (!(iterTrace == m_macToTraceIdMap.end ()))
    {
      out << "trace ID: " << iterTrace->second << " ";
      isInMap = true;
    }

  std::map<Address, uint32_t>::const_iterator iterBeam = m_macToBeamIdMap.find (mac);

  if (!(iterBeam == m_macToBeamIdMap.end ()))
    {
      out << "beam ID: " << iterBeam->second << " ";
      isInMap = true;
    }

  std::map<Address, uint32_t>::const_iterator iterUt = m_macToUtIdMap.find (mac);

  if (!(iterUt == m_macToUtIdMap.end ()))
    {
      out << "UT ID: " << iterUt->second << " ";
      isInMap = true;
    }

  std::map<Address, uint32_t>::const_iterator iterGw = m_macToGwIdMap.find (mac);

  if (!(iterGw == m_macToGwIdMap.end ()))
    {
      out << "GW ID: " << iterGw->second << " ";
      isInMap = true;
    }

  std::string infoString = out.str ();

  // if the string is empty, the MAC was not found in any of the maps
  if (!isInMap)
    {
      out << "not found in the mapper";
      infoString = out.str ();
    }

  return infoString;
}

void
SatIdMapper::PrintTraceMap () const
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::const_iterator iter;

  for (iter = m_macToTraceIdMap.begin (); iter != m_macToTraceIdMap.end (); ++iter)
    {
      std::cout << GetMacInfo (iter->first) << std::endl;
    }
}

} // namespace ns3
