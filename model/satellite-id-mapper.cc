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

SatIdMapper::SatIdMapper () :
  m_traceIdIndex (0),
  m_utIdIndex (0),
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

  if (!m_traceIdToMacMap.empty ())
    {
      m_traceIdToMacMap.clear ();
    }
  m_traceIdIndex = 0;

  // UT ID maps

  if (!m_macToUtIdMap.empty ())
    {
      m_macToUtIdMap.clear ();
    }
  m_utIdIndex = 0;

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

  m_enableMapPrint = false;
}

void
SatIdMapper::AttachMacToTraceId (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToTraceId = m_macToTraceIdMap.insert (std::make_pair (mac, m_traceIdIndex));

  if (resultMacToTraceId.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AttachMacToTraceId - MAC to Trace ID failed");
    }

  std::pair < std::map<uint32_t, Address>::iterator, bool> resultTraceIdToMac = m_traceIdToMacMap.insert (std::make_pair (m_traceIdIndex, mac));

  if (resultTraceIdToMac.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AttachMacToTraceId - Trace ID to MAC failed");
    }

  NS_LOG_INFO ("SatMacIdMacMapper::AttachMacToTraceId - Added MAC " << mac << " with Trace ID " << m_traceIdIndex);

  m_traceIdIndex++;
}

void
SatIdMapper::AttachMacToUtId (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToUtId = m_macToUtIdMap.insert (std::make_pair (mac, m_utIdIndex));

  if (resultMacToUtId.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AttachMacToUtId - MAC to UT ID failed");
    }

  NS_LOG_INFO ("SatMacIdMacMapper::AttachMacToUtId - Added MAC " << mac << " with UT ID " << m_utIdIndex);

  m_utIdIndex++;
}

void
SatIdMapper::AttachMacToBeamId (Address mac, uint32_t beamId)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToBeamId = m_macToBeamIdMap.insert (std::make_pair (mac, beamId));

  if (resultMacToBeamId.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AttachMacToBeamId - MAC to beam ID failed");
    }

  NS_LOG_INFO ("SatMacIdMacMapper::AttachMacToBeamId - Added MAC " << mac << " with beam ID " << beamId);
}

void
SatIdMapper::AttachMacToGwId (Address mac, uint32_t gwId)
{
  NS_LOG_FUNCTION (this);

  std::pair < std::map<Address, uint32_t>::iterator, bool> resultMacToGwId = m_macToGwIdMap.insert (std::make_pair (mac, gwId));

  if (resultMacToGwId.second == false)
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::AttachMacToGwId - MAC to GW ID failed");
    }

  NS_LOG_INFO ("SatMacIdMacMapper::AttachMacToGwId - Added MAC " << mac << " with GW ID " << gwId);
}

// MAC GETTERS

Address
SatIdMapper::GetMacWithTraceId (uint32_t traceId)
{
  NS_LOG_FUNCTION (this);

  std::map<uint32_t, Address>::iterator iter = m_traceIdToMacMap.find (traceId);

  if (iter == m_traceIdToMacMap.end ())
    {
      NS_FATAL_ERROR ("GetMacWithTraceId::GetMacWithTraceIdv - Trace ID " << traceId << " not found");
    }

  return iter->second;
}

// ID GETTERS

uint32_t
SatIdMapper::GetTraceIdWithMac (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::iterator iter = m_macToTraceIdMap.find (mac);

  if (iter == m_macToTraceIdMap.end ())
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::GetTraceIdWithMac - MAC " << mac << " not found");
    }

  return iter->second;
}

uint32_t
SatIdMapper::GetUtIdWithMac (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::iterator iter = m_macToUtIdMap.find (mac);

  if (iter == m_macToUtIdMap.end ())
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::GetUtIdWithMac - MAC " << mac << " not found");
    }

  return iter->second;
}

uint32_t
SatIdMapper::GetBeamIdWithMac (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::iterator iter = m_macToBeamIdMap.find (mac);

  if (iter == m_macToBeamIdMap.end ())
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::GetBeamIdWithMac - MAC " << mac << " not found");
    }

  return iter->second;
}

uint32_t
SatIdMapper::GetGwIdWithMac (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::iterator iter = m_macToGwIdMap.find (mac);

  if (iter == m_macToGwIdMap.end ())
    {
      NS_FATAL_ERROR ("SatMacIdMacMapper::GetGwIdWithMac - MAC " << mac << " not found");
    }

  return iter->second;
}

std::string
SatIdMapper::GetMacInfo (Address mac)
{
  NS_LOG_FUNCTION (this);

  std::stringstream out;
  bool isInMap = false;

  out << mac << " ";

  std::map<Address, uint32_t>::iterator iterTrace = m_macToTraceIdMap.find (mac);

  if (!(iterTrace == m_macToTraceIdMap.end ()))
    {
      out << "trace ID: " << iterTrace->second << " ";
      isInMap = true;
    }

  std::map<Address, uint32_t>::iterator iterBeam = m_macToBeamIdMap.find (mac);

  if (!(iterBeam == m_macToBeamIdMap.end ()))
    {
      out << "beam ID: " << iterBeam->second << " ";
      isInMap = true;
    }

  std::map<Address, uint32_t>::iterator iterUt = m_macToUtIdMap.find (mac);

  if (!(iterUt == m_macToUtIdMap.end ()))
    {
      out << "UT ID: " << iterUt->second << " ";
      isInMap = true;
    }

  std::map<Address, uint32_t>::iterator iterGw = m_macToGwIdMap.find (mac);

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
SatIdMapper::PrintTraceMap ()
{
  NS_LOG_FUNCTION (this);

  std::map<Address, uint32_t>::iterator iter;

  for (iter = m_macToTraceIdMap.begin(); iter != m_macToTraceIdMap.end(); iter++)
    {
      std::cout << GetMacInfo (iter->first) << std::endl;
    }
}

} // namespace ns3
