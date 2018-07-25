/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include <ns3/satellite-env-variables.h>
#include <ns3/singleton.h>
#include "satellite-position-input-trace-container.h"
#include "satellite-id-mapper.h"

NS_LOG_COMPONENT_DEFINE ("SatPositionInputTraceContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPositionInputTraceContainer);

TypeId
SatPositionInputTraceContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPositionInputTraceContainer")
    .SetParent<SatBaseTraceContainer> ()
    .AddConstructor<SatPositionInputTraceContainer> ();
  return tid;
}

TypeId
SatPositionInputTraceContainer::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatPositionInputTraceContainer::SatPositionInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);
}

SatPositionInputTraceContainer::~SatPositionInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatPositionInputTraceContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();

  SatBaseTraceContainer::DoDispose ();
}

void
SatPositionInputTraceContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  if (!m_container.empty ())
    {
      m_container.clear ();
    }
}

Ptr<SatInputFileStreamTimeDoubleContainer>
SatPositionInputTraceContainer::AddNode (Address key)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory ();

  int32_t gwId = Singleton<SatIdMapper>::Get ()->GetGwIdWithMac (key);
  NS_ASSERT_MSG (gwId < 0, "Gateways can't be assigned mobile positions");

  int32_t utId = Singleton<SatIdMapper>::Get ()->GetUtIdWithMac (key);
  int32_t beamId = Singleton<SatIdMapper>::Get ()->GetBeamIdWithMac (key);

  if (beamId >= 0 && utId >= 0)
    {
      filename << dataPath << "/utpositions/input/BEAM_" << beamId << "_UT_" << utId;

      std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair (key, CreateObject<SatInputFileStreamTimeDoubleContainer> (filename.str ().c_str (), std::ios::in, SatBaseTraceContainer::POSITION_TRACE_DEFAULT_NUMBER_OF_COLUMNS)));

      if (result.second == false)
        {
          NS_FATAL_ERROR ("SatPositionInputTraceContainer::AddNode failed");
        }

      NS_LOG_INFO ("Added node with MAC " << key);

      return result.first->second;
    }

  NS_FATAL_ERROR ("SatPositionInputTraceContainer::AddNode failed");
  return NULL;
}

Ptr<SatInputFileStreamTimeDoubleContainer>
SatPositionInputTraceContainer::FindNode (Address key)
{
  NS_LOG_FUNCTION (this);

  container_t::iterator iter = m_container.find (key);

  if (iter == m_container.end ())
    {
      return AddNode (key);
    }

  return iter->second;
}

GeoCoordinate
SatPositionInputTraceContainer::GetPosition (Address key, GeoCoordinate::ReferenceEllipsoid_t refEllipsoid)
{
  NS_LOG_FUNCTION (this);

  std::vector<double> row = FindNode (key)->ProceedToNextClosestTimeSample ();
  return GeoCoordinate (
    row.at (SatBaseTraceContainer::POSITION_TRACE_DEFAULT_LATITUDE_INDEX),
    row.at (SatBaseTraceContainer::POSITION_TRACE_DEFAULT_LONGITUDE_INDEX),
    row.at (SatBaseTraceContainer::POSITION_TRACE_DEFAULT_ALTITUDE_INDEX),
    refEllipsoid);
}

} // namespace ns3
