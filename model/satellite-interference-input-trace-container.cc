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
#include "satellite-interference-input-trace-container.h"

NS_LOG_COMPONENT_DEFINE ("SatInterferenceInputTraceContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatInterferenceInputTraceContainer);

TypeId 
SatInterferenceInputTraceContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatInterferenceInputTraceContainer")
    .SetParent<SatBaseTraceContainer> ();
  return tid;
}

SatInterferenceInputTraceContainer::SatInterferenceInputTraceContainer () :
  m_index (0)
{
  NS_LOG_FUNCTION (this);
}

SatInterferenceInputTraceContainer::~SatInterferenceInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatInterferenceInputTraceContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();

  SatBaseTraceContainer::DoDispose();
}

void
SatInterferenceInputTraceContainer::Reset ()
{
  if ( !m_container.empty() )
    {
      m_container.clear();
    }
  m_index = 0;
}

void
SatInterferenceInputTraceContainer::AddNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;

  /// TODO: add simulator root folder to the path!

  filename << "data/interference_trace/input/nodeId_" << m_index << "_channelType_" + key.second;

  std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair(key, CreateObject<SatInputFileStreamDoubleContainer> (filename.str(), std::ios::in, DEFAULT_NUMBER_OF_COLUMNS)));

  if (result.second == false)
    {
      NS_FATAL_ERROR ("SatInterferenceInputTraceContainer::AddNode failed");
    }

  m_index++;
}

Ptr<SatInputFileStreamDoubleContainer>
SatInterferenceInputTraceContainer::FindNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  return m_container.at (key);
}

double
SatInterferenceInputTraceContainer::GetInterferenceDensity (key_t key)
{
  NS_LOG_FUNCTION (this);

  return FindNode (key)->ProceedToNextClosestTimeSample ().at (DEFAULT_INTF_DENSITY_INDEX);
}

double
SatInterferenceInputTraceContainer::GetRxPowerDensity (key_t key)
{
  NS_LOG_FUNCTION (this);

  return FindNode (key)->ProceedToNextClosestTimeSample ().at (DEFAULT_RX_POWER_DENSITY_INDEX);
}

} // namespace ns3
