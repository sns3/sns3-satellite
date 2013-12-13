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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "satellite-per-packet-interference.h"
#include "ns3/singleton.h"

NS_LOG_COMPONENT_DEFINE ("SatPerPacketInterference");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPerPacketInterference);

TypeId
SatPerPacketInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPerPacketInterference")
    .SetParent<SatInterference> ()
    .AddConstructor<SatPerPacketInterference>();

  return tid;
}

TypeId
SatPerPacketInterference::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatPerPacketInterference::SatPerPacketInterference ()
  : m_firstPowerW (0.0),
    m_rxing (false),
    m_nextEventId (0),
    m_enableTraceOutput (false),
    m_channelType (),
    m_rxBandwidth_Hz ()
{
  NS_LOG_FUNCTION (this);
}

SatPerPacketInterference::SatPerPacketInterference (SatEnums::ChannelType_t channeltype, double rxBandwidth)
  : m_firstPowerW (0.0),
    m_rxing (false),
    m_nextEventId (0),
    m_enableTraceOutput (true),
    m_channelType (channeltype),
    m_rxBandwidth_Hz (rxBandwidth)
{
  NS_LOG_FUNCTION (this);

  if (!m_rxBandwidth_Hz > 0)
    {
      NS_FATAL_ERROR ("SatPerPacketInterference::SatPerPacketInterference - Invalid value");
    }
}


SatPerPacketInterference::~SatPerPacketInterference ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

Ptr<SatInterference::Event>
SatPerPacketInterference::DoAdd (Time duration, double power, Address rxAddress)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatInterference::Event> event;
  event = Create<SatInterference::Event> (m_nextEventId++, duration, power, rxAddress);
  Time now = event->GetStartTime ();

  // do update and clean-ups, if we are not receiving
  if (!m_rxing)
    {
      InterferenceChanges::iterator nowIterator = m_changes.upper_bound (now);

      for (InterferenceChanges::iterator i = m_changes.begin (); i != nowIterator; i++)
        {
          m_firstPowerW += i->second.second;
        }

      m_changes.erase (m_changes.begin (), nowIterator);

    }

  m_changes.insert (std::make_pair (now, InterferenceChange (event->GetId (), power)));
  m_changes.insert (std::make_pair (event->GetEndTime (), InterferenceChange (event->GetId (), -power)));

  return event;
}

double
SatPerPacketInterference::DoCalculate (Ptr<SatInterference::Event> event)
{
  NS_LOG_FUNCTION (this);

  if ( m_rxing == false )
    {
      NS_FATAL_ERROR ("Receiving is not set on!!!");
    }

  double ifPowerW = m_firstPowerW;
  double rxDuration = event->GetDuration ().GetDouble ();
  double rxEndTime = event->GetEndTime ().GetDouble ();
  bool updatePartialPower = false;
  InterferenceChanges::iterator currentItem = m_changes.begin();

  // calculate power values until own "stop" event found (own negative power event)
  while ( (currentItem != m_changes.end ()) &&
         !( (event->GetId () == currentItem->second.first) && (event->GetRxPower () == -currentItem->second.second)) )
    {
      if (event->GetId () == currentItem->second.first)
        {
          // stop increasing power value fully, when own 'start' event is reached
          // needed to support multiple simultaneous receiving (currently not supported)
          // own event is not updated to ifPower
          updatePartialPower = true;
        }
      else if (updatePartialPower)
        {
          // increase/decrease interference power with relative part of duration of power change in list
          double itemTime = currentItem->first.GetDouble();
          ifPowerW += ((rxEndTime - itemTime) / rxDuration) * currentItem->second.second;
        }
      else
        {
          // increase/decrease interference power with full power change in list
          ifPowerW += currentItem->second.second;
        }

      currentItem++;
    }

  if (m_enableTraceOutput)
    {
      std::vector<double> tempVector;
      tempVector.push_back (Now ().GetSeconds());
      tempVector.push_back (ifPowerW / m_rxBandwidth_Hz);
      Singleton<SatInterferenceOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (event->GetTerrestrialNodeAddress (), m_channelType), tempVector);
    }

  return ifPowerW;
}

void
SatPerPacketInterference::DoReset (void)
{
  NS_LOG_FUNCTION (this);

  m_changes.clear ();
  m_rxing = false;
  m_firstPowerW = 0.0;
}

void
SatPerPacketInterference::DoNotifyRxStart (Ptr<SatInterference::Event> event)
{
  NS_LOG_FUNCTION (this);

  std::pair<std::set<uint32_t>::iterator, bool> result = m_events.insert (event->GetId ());

  NS_ASSERT (result.second);
  m_rxing = true;
}

void
SatPerPacketInterference::DoNotifyRxEnd (Ptr<SatInterference::Event> event)
{
  NS_LOG_FUNCTION (this);

  m_events.erase (event->GetId ());

  if (m_events.empty ())
    {
      m_rxing = false;
    }
}

void
SatPerPacketInterference::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  SatInterference::DoDispose ();
}

void
SatPerPacketInterference::SetRxBandwidth (double rxBandwidth)
{
  NS_LOG_FUNCTION (this);

  if (!m_rxBandwidth_Hz > 0)
    {
      NS_FATAL_ERROR ("SatPerPacketInterference::SetRxBandwidth - Invalid value");
    }

  m_rxBandwidth_Hz = rxBandwidth;
}

}
// namespace ns3
