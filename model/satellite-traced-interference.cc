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

NS_LOG_COMPONENT_DEFINE ("SatTracedInterference");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTracedInterference);

TypeId
SatTracedInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTracedInterference")
    .SetParent<SatInterference> ()
    .AddConstructor<SatTracedInterference>();

  return tid;
}

TypeId
SatTracedInterference::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatTracedInterference::SatTracedInterference () :
    m_rxing (false)
{
}

SatTracedInterference::~SatTracedInterference ()
{
  Reset ();
}

Ptr<SatInterference::Event>
SatTracedInterference::DoAdd (Time duration, double power)
{
  Ptr<SatInterference::Event> event;
  event = Create<SatInterference::Event> (0, duration, power);

  return event;
}

double
SatTracedInterference::DoCalculate (Ptr<SatInterference::Event> event, double* finalPower)
{
  return 0;
}

void
SatTracedInterference::DoReset (void)
{

}
void
SatTracedInterference::DoNotifyRxStart (Ptr<SatInterference::Event> event)
{
  m_rxing = true;
}
void
SatTracedInterference::DoNotifyRxEnd (Ptr<SatInterference::Event> event)
{
  m_rxing = false;
}

}
// namespace ns3
