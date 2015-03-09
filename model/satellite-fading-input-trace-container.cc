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
#include "satellite-fading-input-trace-container.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/singleton.h"
#include "satellite-id-mapper.h"

NS_LOG_COMPONENT_DEFINE ("SatFadingInputTraceContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFadingInputTraceContainer);

TypeId
SatFadingInputTraceContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingInputTraceContainer")
    .SetParent<SatBaseTraceContainer> ()
    .AddConstructor<SatFadingInputTraceContainer> ();
  return tid;
}

TypeId
SatFadingInputTraceContainer::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatFadingInputTraceContainer::SatFadingInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);
}

SatFadingInputTraceContainer::~SatFadingInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatFadingInputTraceContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();

  SatBaseTraceContainer::DoDispose ();
}

void
SatFadingInputTraceContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  if (!m_container.empty ())
    {
      m_container.clear ();
    }
}

Ptr<SatInputFileStreamTimeDoubleContainer>
SatFadingInputTraceContainer::AddNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory ();

  int32_t gwId = Singleton<SatIdMapper>::Get ()->GetGwIdWithMac (key.first);
  int32_t utId = Singleton<SatIdMapper>::Get ()->GetUtIdWithMac (key.first);
  int32_t beamId = Singleton<SatIdMapper>::Get ()->GetBeamIdWithMac (key.first);

  if (beamId < 0 || (utId < 0 && gwId < 0))
    {
      NS_FATAL_ERROR ("SatFadingInputTraceContainer::AddNode - No such MAC address in the trace ID mapper");
    }
  else
    {
      if (utId >= 0 && gwId < 0)
        {
          filename << dataPath << "/fadingtraces/input/BEAM_" << beamId << "_UT_" << utId << "_channelType_" << SatEnums::GetChannelTypeName (key.second);
        }

      if (gwId >= 0 && utId < 0)
        {
          filename << dataPath << "/fadingtraces/input/BEAM_" << beamId << "_GW_" << gwId << "_channelType_" << SatEnums::GetChannelTypeName (key.second);
        }
      std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair (key, CreateObject<SatInputFileStreamTimeDoubleContainer> (filename.str ().c_str (), std::ios::in, SatBaseTraceContainer::FADING_TRACE_DEFAULT_NUMBER_OF_COLUMNS)));

      if (result.second == false)
        {
          NS_FATAL_ERROR ("SatFadingInputTraceContainer::AddNode failed");
        }

      NS_LOG_INFO ("SatFadingInputTraceContainer::AddNode: Added node with MAC " << key.first << " channel type " << key.second);

      return result.first->second;
    }

  NS_FATAL_ERROR ("SatFadingInputTraceContainer::AddNode failed");
  return NULL;
}

Ptr<SatInputFileStreamTimeDoubleContainer>
SatFadingInputTraceContainer::FindNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  container_t::iterator iter = m_container.find (key);

  if (iter == m_container.end ())
    {
      return AddNode (key);
    }

  return iter->second;
}

double
SatFadingInputTraceContainer::GetFadingValue (key_t key)
{
  NS_LOG_FUNCTION (this);

  return FindNode (key)->ProceedToNextClosestTimeSample ().at (SatBaseTraceContainer::FADING_TRACE_DEFAULT_FADING_VALUE_INDEX);
}

} // namespace ns3
