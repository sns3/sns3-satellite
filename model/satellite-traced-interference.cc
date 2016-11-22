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
#include "satellite-traced-interference.h"
#include "ns3/singleton.h"

NS_LOG_COMPONENT_DEFINE ("SatTracedInterference");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTracedInterference);

TypeId
SatTracedInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTracedInterference")
    .SetParent<SatInterference> ()
    .AddConstructor<SatTracedInterference> ();

  return tid;
}

TypeId
SatTracedInterference::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatTracedInterference::SatTracedInterference (SatEnums::ChannelType_t channeltype, double rxBandwidth)
  : m_rxing (false),
    m_power (0),
    m_channelType (channeltype),
    m_rxBandwidth_Hz (rxBandwidth)
{
  NS_LOG_FUNCTION (this);

  if (m_rxBandwidth_Hz <= std::numeric_limits<double>::epsilon ())
    {
      NS_FATAL_ERROR ("SatTracedInterference::SatTracedInterference - Invalid value");
    }
}

SatTracedInterference::SatTracedInterference ()
  : m_rxing (false),
    m_power (),
    m_channelType (),
    m_rxBandwidth_Hz ()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatTracedInterference::SatTracedInterference - Constructor not in use");
}

SatTracedInterference::~SatTracedInterference ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

Ptr<SatInterference::InterferenceChangeEvent>
SatTracedInterference::DoAdd (Time duration, double power, Address rxAddress)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << power << rxAddress);

  Ptr<SatInterference::InterferenceChangeEvent> event;
  event = Create<SatInterference::InterferenceChangeEvent> (0, duration, power, rxAddress);

  return event;
}

double
SatTracedInterference::DoCalculate (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_power = m_rxBandwidth_Hz * Singleton<SatInterferenceInputTraceContainer>::Get ()->GetInterferenceDensity (std::make_pair (event->GetSatEarthStationAddress (),m_channelType));

  return m_power;
}

void
SatTracedInterference::DoReset ()
{
  NS_LOG_FUNCTION (this);
}

void
SatTracedInterference::DoNotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_rxing = true;
}

void
SatTracedInterference::DoNotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_rxing = false;
}

void
SatTracedInterference::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  SatInterference::DoDispose ();
}

void
SatTracedInterference::SetRxBandwidth (double rxBandwidth)
{
  NS_LOG_FUNCTION (this);

  if (rxBandwidth <= std::numeric_limits<double>::epsilon ())
    {
      NS_FATAL_ERROR ("SatTracedInterference::SetRxBandwidth - Invalid value");
    }

  m_rxBandwidth_Hz = rxBandwidth;
}

}
// namespace ns3
