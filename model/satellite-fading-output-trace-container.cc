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
#include "satellite-fading-output-trace-container.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/singleton.h"
#include "satellite-id-mapper.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

NS_LOG_COMPONENT_DEFINE ("SatFadingOutputTraceContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFadingOutputTraceContainer);

TypeId
SatFadingOutputTraceContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingOutputTraceContainer")
    .SetParent<SatBaseTraceContainer> ()
    .AddConstructor<SatFadingOutputTraceContainer> ();
  return tid;
}

TypeId
SatFadingOutputTraceContainer::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatFadingOutputTraceContainer::SatFadingOutputTraceContainer ()
  : m_enableFigureOutput (true)
{
  NS_LOG_FUNCTION (this);
}

SatFadingOutputTraceContainer::~SatFadingOutputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatFadingOutputTraceContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();

  SatBaseTraceContainer::DoDispose ();
}

void
SatFadingOutputTraceContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  if (!m_container.empty ())
    {
      WriteToFile ();

      m_container.clear ();
    }
  m_enableFigureOutput = true;
}

Ptr<SatOutputFileStreamDoubleContainer>
SatFadingOutputTraceContainer::AddNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->GetOutputPath ();

  int32_t gwId = Singleton<SatIdMapper>::Get ()->GetGwIdWithMac (key.first);
  int32_t utId = Singleton<SatIdMapper>::Get ()->GetUtIdWithMac (key.first);
  int32_t beamId = Singleton<SatIdMapper>::Get ()->GetBeamIdWithMac (key.first);

  if (beamId < 0 || (utId < 0 && gwId < 0))
    {
      return NULL;
    }
  else
    {
      if (utId >= 0 && gwId < 0)
        {
          filename << dataPath << "/fading_output_trace_BEAM_" << beamId << "_UT_" << utId << "_channelType_" << SatEnums::GetChannelTypeName (key.second);
        }

      if (gwId >= 0 && utId < 0)
        {
          filename << dataPath << "/fading_output_trace_BEAM_" << beamId << "_GW_" << gwId << "_channelType_" << SatEnums::GetChannelTypeName (key.second);
        }

      std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair (key, CreateObject<SatOutputFileStreamDoubleContainer> (filename.str ().c_str (), std::ios::out, SatBaseTraceContainer::FADING_TRACE_DEFAULT_NUMBER_OF_COLUMNS)));

      if (result.second == false)
        {
          NS_FATAL_ERROR ("SatFadingOutputTraceContainer::AddNode failed");
        }

      NS_LOG_INFO ("SatFadingOutputTraceContainer::AddNode: Added node with MAC " << key.first << " channel type " << key.second);

      return result.first->second;
    }
}

Ptr<SatOutputFileStreamDoubleContainer>
SatFadingOutputTraceContainer::FindNode (key_t key)
{
  NS_LOG_FUNCTION (this);

  container_t::iterator iter = m_container.find (key);

  if (iter == m_container.end ())
    {
      return AddNode (key);
    }

  return iter->second;
}

void
SatFadingOutputTraceContainer::WriteToFile ()
{
  NS_LOG_FUNCTION (this);

  container_t::iterator iter;

  for (iter = m_container.begin (); iter != m_container.end (); iter++)
    {
      if (m_enableFigureOutput)
        {
          iter->second->EnableFigureOutput ("Fading trace",
                                            "Time (s)",
                                            "Fading (dB)",
                                            "set key top right",
                                            SatOutputFileStreamDoubleContainer::DECIBEL_AMPLITUDE,
                                            Gnuplot2dDataset::LINES);
        }
      iter->second->WriteContainerToFile ();
    }
}

void
SatFadingOutputTraceContainer::AddToContainer (key_t key, std::vector<double> newItem)
{
  NS_LOG_FUNCTION (this);

  if (newItem.size () != SatBaseTraceContainer::FADING_TRACE_DEFAULT_NUMBER_OF_COLUMNS)
    {
      NS_FATAL_ERROR ("SatFadingOutputTraceContainer::AddToContainer - Incorrect vector size");
    }

  Ptr<SatOutputFileStreamDoubleContainer> node = FindNode (key);

  if (node != NULL)
    {
      node->AddToContainer (newItem);
    }
}

} // namespace ns3
