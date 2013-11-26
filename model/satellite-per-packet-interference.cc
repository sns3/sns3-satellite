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
#include "ns3/satellite-helper.h"

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
  return GetTypeId();
}

SatPerPacketInterference::SatPerPacketInterference ()
  : m_firstPower (0.0),
    m_rxing (false),
    m_nextEventId (0),
    m_enableTraceOutput (false),
    m_channelType (),
    m_rxBandwidth_Hz ()
{

}

SatPerPacketInterference::SatPerPacketInterference (SatEnums::ChannelType_t channeltype, double rxBandwidth)
  : m_firstPower (0.0),
    m_rxing (false),
    m_nextEventId (0),
    m_enableTraceOutput (true),
    m_channelType (channeltype),
    m_rxBandwidth_Hz (rxBandwidth)
{
  if (!m_rxBandwidth_Hz > 0)
    {
      NS_FATAL_ERROR ("SatPerPacketInterference::SatPerPacketInterference - Invalid value");
    }
}


SatPerPacketInterference::~SatPerPacketInterference ()
{
  Reset ();
}

Ptr<SatInterference::Event>
SatPerPacketInterference::DoAdd (Time duration, double power, Address rxAddress)
{
  Ptr<SatInterference::Event> event;
  event = Create<SatInterference::Event> (m_nextEventId++, duration, power, rxAddress);
  Time now = event->GetStartTime ();

  if (!m_rxing)
    {
      InterferenceChanges::iterator nowIterator = m_changes.upper_bound (now);

      for (InterferenceChanges::iterator i = m_changes.begin (); i != nowIterator; i++)
        {
          m_firstPower += i->second.second;
        }

      m_changes.erase (m_changes.begin (), nowIterator);
      m_changes.insert (std::make_pair (now, InterferenceChange (event->GetId (), power)));
    }
  else
    {
      m_changes.insert (std::make_pair (now, InterferenceChange (event->GetId (), power)));
    }

  m_changes.insert (std::make_pair (event->GetEndTime (), InterferenceChange (event->GetId (), -power)));

  return event;
}

double
SatPerPacketInterference::DoCalculate (Ptr<SatInterference::Event> event)
{
  bool calcReady = false;
  bool increaseFirstPower = true;

  NS_ASSERT (m_rxing);

  double finalPower = 0;
  double firstPower = m_firstPower;

  double rxDuration = event->GetDuration ().GetDouble ();
  double rxEndTime = event->GetEndTime ().GetDouble ();

  for (InterferenceChanges::iterator item = m_changes.begin(); item != m_changes.end () && calcReady == false; item++)
    {
      if ( event->GetId () == item->second.first && event->GetRxPower () == -item->second.second)
        {
          calcReady = true; // receivers own 'end' event reached
        }
      else if (event->GetId () == item->second.first)
        {
          // stop first power increasing when own 'start' event is reached
          // needed to support multiple simultaneous receiving
          increaseFirstPower = false;
        }
      else if (increaseFirstPower)
        {
          // increase first power (until own 'start' event is reached)
          firstPower += item->second.second;
        }
      else
        {
          // increase/descrease final power according to change in list
          double itemTime = item->first.GetDouble();
          finalPower += ((rxEndTime - itemTime) / rxDuration) * item->second.second;
        }
    }

  finalPower += firstPower;

  if (m_enableTraceOutput)
    {
      std::vector<double> tempVector;
      tempVector.push_back (Now ().GetDouble());
      tempVector.push_back (finalPower / m_rxBandwidth_Hz);
      SatHelper::m_satIntfOutputTraceContainer->AddToContainer (std::make_pair (event->GetRxAddress (), m_channelType), tempVector);
    }

  return finalPower;
}

void
SatPerPacketInterference::DoReset (void)
{
  m_changes.clear ();
  m_rxing = false;
  m_firstPower = 0.0;
}

void
SatPerPacketInterference::DoNotifyRxStart (Ptr<SatInterference::Event> event)
{
  std::pair<std::set<uint32_t>::iterator, bool> result = m_events.insert (event->GetId ());

  NS_ASSERT (result.second);
  m_rxing = true;
}

void
SatPerPacketInterference::DoNotifyRxEnd (Ptr<SatInterference::Event> event)
{
  m_events.erase (event->GetId ());

  if (m_events.empty ())
    {
      m_rxing = false;
    }
}

void
SatPerPacketInterference::DoDispose ()
{
  SatInterference::DoDispose ();
}

void
SatPerPacketInterference::SetRxBandwidth (double rxBandwidth)
{
  if (!m_rxBandwidth_Hz > 0)
    {
      NS_FATAL_ERROR ("SatPerPacketInterference::SetRxBandwidth - Invalid value");
    }

  m_rxBandwidth_Hz = rxBandwidth;
}

}
// namespace ns3
