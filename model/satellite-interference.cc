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

SatInterference::SatInterference ()
{
}
SatInterference::~SatInterference ()
{

}

Ptr<SatInterference::InterferenceChangeEvent>
SatInterference::Add (Time duration, double power, Address rxAddress)
{
  return DoAdd(duration, power, rxAddress);
}

double
SatInterference::Calculate (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  return DoCalculate (event);
}

void
SatInterference::Reset (void)
{
  DoReset();
}

void
SatInterference::NotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  DoNotifyRxStart(event);
}

void
SatInterference::NotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  DoNotifyRxEnd(event);
}

}
// namespace ns3
