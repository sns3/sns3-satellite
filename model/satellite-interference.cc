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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "satellite-interference.h"

NS_LOG_COMPONENT_DEFINE ("SatInterference");

namespace ns3 {

SatInterference::InterferenceChangeEvent::InterferenceChangeEvent (uint32_t id, Time rxDuration, double rxPower, Address terrestrialAddress)
  : m_startTime (Simulator::Now ()),
    m_endTime (m_startTime + rxDuration),
    m_rxPower (rxPower),
    m_id (id),
    m_terrestrialAddress (terrestrialAddress)
{
}
SatInterference::InterferenceChangeEvent::~InterferenceChangeEvent ()
{
}

uint32_t
SatInterference::InterferenceChangeEvent::GetId (void) const
{
  return m_id;
}

Time
SatInterference::InterferenceChangeEvent::GetDuration (void) const
{
  return m_endTime - m_startTime;
}

Time
SatInterference::InterferenceChangeEvent::GetStartTime (void) const
{
  return m_startTime;
}

Time
SatInterference::InterferenceChangeEvent::GetEndTime (void) const
{
  return m_endTime;
}

double
SatInterference::InterferenceChangeEvent::GetRxPower (void) const
{
  return m_rxPower;
}

Address
SatInterference::InterferenceChangeEvent::GetTerrestrialNodeAddress (void) const
{
  return m_terrestrialAddress;
}
/****************************************************************
 *       The actual SatInterference
 ****************************************************************/

NS_OBJECT_ENSURE_REGISTERED (SatInterference);

TypeId
SatInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatInterference")
    .SetParent<Object> ();

  return tid;
}

TypeId
SatInterference::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatInterference::SatInterference () :
  m_currentlyReceiving (0)
{

}

SatInterference::~SatInterference ()
{

}

Ptr<SatInterference::InterferenceChangeEvent>
SatInterference::Add (Time duration, double power, Address rxAddress)
{
  return DoAdd (duration, power, rxAddress);
}

double
SatInterference::Calculate (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  if (m_currentlyReceiving > 1)
    {
      std::map<Ptr<SatInterference::InterferenceChangeEvent>, bool>::iterator iter;

      for (iter = m_packetCollisions.begin (); iter != m_packetCollisions.end (); iter++)
        {
          iter->second = true;
        }
    }

  return DoCalculate (event);
}

void
SatInterference::Reset (void)
{
  m_packetCollisions.clear ();
  m_currentlyReceiving = 0;

  DoReset ();
}

void
SatInterference::NotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  m_currentlyReceiving++;

  std::pair<std::map<Ptr<SatInterference::InterferenceChangeEvent>, bool>::iterator,bool> result;
  result = m_packetCollisions.insert (std::make_pair(event,false));

  if (!result.second)
    {
      NS_FATAL_ERROR ("SatConstantInterference::DoAdd - Event already exists");
    }

  DoNotifyRxStart (event);
}

void
SatInterference::NotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  if (m_currentlyReceiving > 0)
    {
      m_currentlyReceiving--;
    }

  m_packetCollisions.erase (event);

  DoNotifyRxEnd (event);
}

bool
SatInterference::HasCollision (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  std::map<Ptr<SatInterference::InterferenceChangeEvent>, bool>::iterator result = m_packetCollisions.find (event);

  if (result == m_packetCollisions.end ())
    {
      NS_FATAL_ERROR ("SatConstantInterference::DoHasCollision - Event not found");
    }

  return result->second;
}

}
// namespace ns3
