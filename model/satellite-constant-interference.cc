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
#include "ns3/double.h"
#include "satellite-constant-interference.h"

NS_LOG_COMPONENT_DEFINE ("SatConstantInterference");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatConstantInterference);

TypeId
SatConstantInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatConstantInterference")
    .SetParent<SatInterference> ()
    .AddConstructor<SatConstantInterference> ()
    .AddAttribute ( "ConstantInterferencePower",
                    "Constant interference power in linear format.",
                    DoubleValue (0.0),
                    MakeDoubleAccessor (&SatConstantInterference::m_power),
                    MakeDoubleChecker<double_t> ())
  ;
  return tid;
}

TypeId
SatConstantInterference::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatConstantInterference::SatConstantInterference ()
  : m_power (0.0),
    m_rxing (false)
{

}

SatConstantInterference::~SatConstantInterference ()
{
  Reset ();
}

Ptr<SatInterference::InterferenceChangeEvent>
SatConstantInterference::DoAdd (Time duration, double power, Address rxAddress)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << power << rxAddress);

  Ptr<SatInterference::InterferenceChangeEvent> event;
  event = Create<SatInterference::InterferenceChangeEvent> (0, duration, power, rxAddress);

  return event;
}

double
SatConstantInterference::DoCalculate (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  if (m_rxing == false)
    {
      NS_LOG_WARN ("Most likely two overlapping receptions! With random access carrier"
          " this should be fine, but with dedicated access prohibited!");
    }

  return m_power;
}

void
SatConstantInterference::DoReset ()
{
  NS_LOG_FUNCTION (this);

  m_rxing = false;
}

void
SatConstantInterference::DoNotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_rxing = true;
}

void
SatConstantInterference::DoNotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_rxing = false;
}

}
// namespace ns3
