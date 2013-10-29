/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#include "satellite-markov-conf.h"
#include <map>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMarkovConf);
NS_LOG_COMPONENT_DEFINE ("SatMarkovConf");

static const double g_MarkovElevationStateChangeProbabilities[SatMarkovConf::DEFAULT_ELEVATION_COUNT][SatMarkovConf::DEFAULT_STATE_COUNT][SatMarkovConf::DEFAULT_STATE_COUNT] =
    {
      /* Elevation 40 */
      {{0.9530, 0.0431, 0.0039},
       {0.0515, 0.9347, 0.0138},
       {0.0334, 0.0238, 0.9428}},

      /* Elevation 60 */
      {{0.9643, 0.0255, 0.0102},
       {0.0628, 0.9171, 0.0201},
       {0.0447, 0.0062, 0.9491}},

      /* Elevation 70 */
      {{0.9538, 0.0393, 0.0069},
       {0.0523, 0.9309, 0.0168},
       {0.0342, 0.0200, 0.9458}},

      /* Elevation 80 */
      {{0.9307, 0.0590, 0.0103},
       {0.0292, 0.9506, 0.0202},
       {0.0111, 0.0397, 0.9492}}
    };

TypeId
SatMarkovConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMarkovConf").SetParent<Object> ().AddConstructor<SatMarkovConf> ();
  return tid;
}

SatMarkovConf::SatMarkovConf () :
    m_elevationCount (SatMarkovConf::DEFAULT_ELEVATION_COUNT),
    m_stateCount (SatMarkovConf::DEFAULT_STATE_COUNT),
    m_minimumPositionChangeInMeters (20.0)
{
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatMarkovConf - Creating SatMarkovConf...");
  for (uint32_t i = 0; i < m_elevationCount; i++)
    {
      std::vector<std::vector<double> > states;

      for (uint32_t j = 0; j < m_stateCount; j++)
        {
          std::vector<double> probabilities;

          for (uint32_t k = 0; k < m_stateCount; k++)
            {
              probabilities.push_back (g_MarkovElevationStateChangeProbabilities[i][j][k]);
            }
          states.push_back (probabilities);
        }
      m_markovProbabilities.push_back (states);
    }

  std::pair<double, uint32_t> elevation;

  elevation.first = 40.0;
  elevation.second = 0;

  m_markovElevations.insert (elevation);

  elevation.first = 60.0;
  elevation.second = 1;

  m_markovElevations.insert (elevation);

  elevation.first = 70.0;
  elevation.second = 2;

  m_markovElevations.insert (elevation);

  elevation.first = 80.0;
  elevation.second = 3;

  m_markovElevations.insert (elevation);

  m_cooldownPeriodLength = Seconds (0.0005);
}

std::vector<std::vector<double> >
SatMarkovConf::GetElevationProbabilities (uint32_t setId)
{
  NS_ASSERT( (setId >= 0) && (setId < m_elevationCount));
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatMarkovConf - Getting elevation probabilities for set ID " << setId);
  return m_markovProbabilities[setId];
}

uint32_t
SatMarkovConf::GetProbabilitySetID (double elevation)
{
  NS_ASSERT( (elevation >= 0.0) && (elevation <= 90.0));

  uint32_t index = 0;
  std::map<double, uint32_t>::iterator iter;

  for (iter = m_markovElevations.begin (); iter != m_markovElevations.end (); ++iter)
    {
      if (elevation >= iter->first)
        {
          index = iter->second;
        }
      else
        {
          break;
        }
    }
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatMarkovConf - New ID for elevation " << elevation << " is " << index);

  return index;
}

uint32_t
SatMarkovConf::GetStateCount ()
{
  return m_stateCount;
}

Time
SatMarkovConf::GetCooldownPeriod ()
{
  return m_cooldownPeriodLength;
}

double
SatMarkovConf::GetMinimumPositionChange ()
{
  return m_minimumPositionChangeInMeters;
}

uint32_t
SatMarkovConf::GetNumOfSets ()
{
  return m_markovElevations.size();
}

} // namespace ns3

