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

SatInterference::InterferenceChangeEvent::InterferenceChangeEvent (uint32_t id, Time rxDuration, double rxPower, Address satEarthStationAddress)
  : m_startTime (Simulator::Now ()),
    m_endTime (m_startTime + rxDuration),
    m_rxPower (rxPower),
    m_id (id),
    m_satEarthStationAddress (satEarthStationAddress)
{
}
SatInterference::InterferenceChangeEvent::~InterferenceChangeEvent ()
{
}

uint32_t
SatInterference::InterferenceChangeEvent::GetId () const
{
  NS_LOG_FUNCTION (this);

  return m_id;
}

Time
SatInterference::InterferenceChangeEvent::GetDuration () const
{
  NS_LOG_FUNCTION (this);

  return m_endTime - m_startTime;
}

Time
SatInterference::InterferenceChangeEvent::GetStartTime () const
{
  NS_LOG_FUNCTION (this);

  return m_startTime;
}

Time
SatInterference::InterferenceChangeEvent::GetEndTime () const
{
  NS_LOG_FUNCTION (this);

  return m_endTime;
}

double
SatInterference::InterferenceChangeEvent::GetRxPower () const
{
  NS_LOG_FUNCTION (this);

  return m_rxPower;
}

Address
SatInterference::InterferenceChangeEvent::GetSatEarthStationAddress () const
{
  NS_LOG_FUNCTION (this);

  return m_satEarthStationAddress;
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
  return GetTypeId ();
}

SatInterference::SatInterference ()
  : m_currentlyReceiving (0)
{

}

SatInterference::~SatInterference ()
{

}

Ptr<SatInterference::InterferenceChangeEvent>
SatInterference::Add (Time duration, double power, Address rxAddress)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << power << rxAddress);

  return DoAdd (duration, power, rxAddress);
}

double
SatInterference::Calculate (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  if (m_currentlyReceiving > 1)
    {
      std::map<Ptr<SatInterference::InterferenceChangeEvent>, bool>::iterator iter;
      bool wasCollisionReported = true;

      for (iter = m_packetCollisions.begin (); iter != m_packetCollisions.end (); iter++)
        {
          if (!iter->second)
            {
              wasCollisionReported = false;
            }
          iter->second = true;
        }
      if (!wasCollisionReported)
        {
          NS_LOG_INFO ("SatInterference::Calculate - Time: " << Now ().GetSeconds () << " - Packet collision!");
        }
    }

  return DoCalculate (event);
}

void
SatInterference::Reset ()
{
  NS_LOG_FUNCTION (this);

  m_packetCollisions.clear ();
  m_currentlyReceiving = 0;

  DoReset ();
}

void
SatInterference::NotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_currentlyReceiving++;

  std::pair<std::map<Ptr<SatInterference::InterferenceChangeEvent>, bool>::iterator,bool> result;
  result = m_packetCollisions.insert (std::make_pair (event,false));

  if (!result.second)
    {
      NS_FATAL_ERROR ("SatInterference::NotifyRxStart - Event already exists");
    }

  DoNotifyRxStart (event);
}

void
SatInterference::NotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

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
  NS_LOG_FUNCTION (this);

  std::map<Ptr<SatInterference::InterferenceChangeEvent>, bool>::iterator result = m_packetCollisions.find (event);

  if (result == m_packetCollisions.end ())
    {
      NS_FATAL_ERROR ("SatInterference::HasCollision - Event not found");
    }

  return result->second;
}

}
// namespace ns3
