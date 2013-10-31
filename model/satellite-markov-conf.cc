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
SatMarkovConf::GetTypeId (void) //TODO: add attribute for m_markovElevations and m_markovProbabilities
{
  static TypeId tid = TypeId ("ns3::SatMarkovConf")
      .SetParent<Object> ()
      .AddConstructor<SatMarkovConf> ()
      .AddAttribute ("ElevationCount", "Number of elevation sets in the Markov model.",
                     UintegerValue (SatMarkovConf::DEFAULT_ELEVATION_COUNT),
                     MakeUintegerAccessor (&SatMarkovConf::m_elevationCount),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("StateCount", "Number of states in the Markov model.",
                     UintegerValue (SatMarkovConf::DEFAULT_STATE_COUNT),
                     MakeUintegerAccessor (&SatMarkovConf::m_stateCount),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute( "MinimumPositionChangeInMeters", "Minimum position change in meters for Markov model state change cooldown.",
                     DoubleValue (20.0),
                     MakeDoubleAccessor(&SatMarkovConf::m_minimumPositionChangeInMeters),
                     MakeDoubleChecker<double>())
      .AddAttribute ("InitialState", "The initial state of the Markov model.",
                     UintegerValue (0),
                     MakeUintegerAccessor (&SatMarkovConf::m_initialState),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute( "InitialElevation", "Initial elevation value.",
                     DoubleValue (45),
                     MakeDoubleAccessor(&SatMarkovConf::m_initialElevation),
                     MakeDoubleChecker<double>())
      .AddAttribute( "CooldownPeriodLength", "Cooldown period length for state change.",
                     TimeValue (Seconds (0.00005)),
                     MakeTimeAccessor(&SatMarkovConf::m_cooldownPeriodLength),
                     MakeTimeChecker());
  return tid;
}

SatMarkovConf::SatMarkovConf () :
    m_elevationCount (SatMarkovConf::DEFAULT_ELEVATION_COUNT),
    m_stateCount (SatMarkovConf::DEFAULT_STATE_COUNT),
    m_minimumPositionChangeInMeters (20.0),
    m_initialState (0),
    m_initialElevation (45),
    m_cooldownPeriodLength (Seconds (0.00005))
{
  NS_LOG_FUNCTION (this);

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
}

std::vector<std::vector<double> >
SatMarkovConf::GetElevationProbabilities (uint32_t set)
{
  NS_LOG_FUNCTION (this << set);

  NS_ASSERT( (set >= 0) && (set < m_elevationCount));
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatMarkovConf - Getting elevation probabilities for set ID " << set);
  return m_markovProbabilities[set];
}

uint32_t
SatMarkovConf::GetProbabilitySetID (double elevation)
{
  NS_LOG_FUNCTION (this << elevation);

  NS_ASSERT( (elevation >= 0.0) && (elevation <= 90.0));

  uint32_t smallestDifferenceIndex = 0;
  double smallestDifference = 360; /// elevation angle can never be this large
  double difference = 0;

  std::map<double, uint32_t>::iterator iter;

  for (iter = m_markovElevations.begin (); iter != m_markovElevations.end (); ++iter)
    {
      difference = fabs(iter->first - elevation);
      if (difference < smallestDifference)
        {
          smallestDifference = difference;
          smallestDifferenceIndex = iter->second;
        }
    }
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatMarkovConf - New ID for elevation " << elevation << " is " << smallestDifferenceIndex);

  return smallestDifferenceIndex;
}

uint32_t
SatMarkovConf::GetStateCount ()
{
  NS_LOG_FUNCTION (this);

  return m_stateCount;
}

Time
SatMarkovConf::GetCooldownPeriod ()
{
  NS_LOG_FUNCTION (this);

  return m_cooldownPeriodLength;
}

double
SatMarkovConf::GetMinimumPositionChange ()
{
  NS_LOG_FUNCTION (this);

  return m_minimumPositionChangeInMeters;
}

uint32_t
SatMarkovConf::GetNumOfSets ()
{
  NS_LOG_FUNCTION (this);

  return m_markovElevations.size();
}

uint32_t
SatMarkovConf::GetInitialState ()
{
  NS_LOG_FUNCTION (this);

  return m_initialState;
}

double
SatMarkovConf::GetInitialElevation ()
{
  NS_LOG_FUNCTION (this);

  return m_initialElevation;
}

} // namespace ns3
