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
#include "ns3/satellite-env-variables.h"

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
  m_index (0),
  m_currentWorkingDirectory ("")
{
  NS_LOG_FUNCTION (this);

  Ptr<SatEnvVariables> envVariables = CreateObject<SatEnvVariables> ();
  m_currentWorkingDirectory = envVariables->GetCurrentWorkingDirectory ();
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
  NS_LOG_FUNCTION (this);

  if ( !m_container.empty() )
    {
      m_container.clear();
    }
  m_index = 0;
  m_currentWorkingDirectory = "";
}

void
SatInterferenceInputTraceContainer::AddNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;

  filename << m_currentWorkingDirectory << "/data/interference_trace/input/nodeId_" << m_index << "_channelType_" << key.second;

  std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair(key, CreateObject<SatInputFileStreamDoubleContainer> (filename.str().c_str(), std::ios::in, SatBaseTraceContainer::INTF_TRACE_DEFAULT_NUMBER_OF_COLUMNS)));

  if (result.second == false)
    {
      NS_FATAL_ERROR ("SatInterferenceInputTraceContainer::AddNode failed");
    }

  NS_LOG_INFO ("SatInterferenceInputTraceContainer::AddNode: Added node with ID " << m_index);

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

  return FindNode (key)->ProceedToNextClosestTimeSample ().at (SatBaseTraceContainer::INTF_TRACE_DEFAULT_INTF_DENSITY_INDEX);
}

} // namespace ns3
