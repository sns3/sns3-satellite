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
  /**
   * Parameters
   * from state X to state Y transition probabilities
   *
   * State 1 {{State 1, State 2, State 3}
   * State 2  {State 1, State 2, State 3}
   * State 3  {State 1, State 2, State 3}}
   */

  /* Elevation 30 degrees */
  {{0.9684, 0.0316, 0.0000},
   {0.4155, 0.5845, 0.0000},
   {0.0000, 0.0000, 0.0000}}
};

static const double g_MarkovInitialStateProbabilities[SatMarkovConf::DEFAULT_STATE_COUNT] =
{
  /**
   * Parameters
   * initial state probabilities
   *
   * State 1, State 2, State 3
   */
  0.9293, 0.0707, 0.0000
};

TypeId
SatMarkovConf::GetTypeId (void)
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
    .AddAttribute ( "MinimumPositionChangeInMeters", "Minimum position change in meters for Markov model state change cooldown.",
                    DoubleValue (100.0),
                    MakeDoubleAccessor (&SatMarkovConf::m_minimumPositionChangeInMeters),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "CooldownPeriodLength", "Cooldown period length for state change.",
                    TimeValue (Seconds (0.0001)),
                    MakeTimeAccessor (&SatMarkovConf::m_cooldownPeriodLength),
                    MakeTimeChecker ())
    .AddAttribute ( "UseDecibels", "Defines whether the fading value should be in decibels or not.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatMarkovConf::m_useDecibels),
                    MakeBooleanChecker ());
  return tid;
}

SatMarkovConf::SatMarkovConf ()
  : m_elevationCount (SatMarkovConf::DEFAULT_ELEVATION_COUNT),
    m_stateCount (SatMarkovConf::DEFAULT_STATE_COUNT),
    m_minimumPositionChangeInMeters (1000.0),
    m_cooldownPeriodLength (Seconds (0.00005)),
    m_useDecibels (false),
    m_looConf (NULL),
    m_rayleighConf (NULL),
    m_faderType (SatMarkovConf::LOO_FADER)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovConf::SatMarkovConf - Creating SatMarkovConf");

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

  for (uint32_t i = 0; i < m_stateCount; i++)
    {
      m_initialProbabilities.push_back (g_MarkovInitialStateProbabilities[i]);
    }

  std::pair<double, uint32_t> elevation;

  elevation.first = 30.0;
  elevation.second = 0;
  m_markovElevations.insert (elevation);

  m_looConf = CreateObject<SatLooConf> ();
  m_rayleighConf = CreateObject<SatRayleighConf> ();

  if (m_markovElevations.size () != m_elevationCount)
    {
      NS_FATAL_ERROR ("SatMarkovConf::SatMarkovConf - Markov elevations does not match");
    }
}

SatMarkovConf::~SatMarkovConf ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatMarkovConf::Reset ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_elevationCount; i++)
    {
      for (uint32_t j = 0; j < m_stateCount; j++)
        {
          m_markovProbabilities[i][j].clear ();
        }
    }

  m_looConf = NULL;
  m_rayleighConf = NULL;

  m_initialProbabilities.clear ();
  m_markovElevations.clear ();
}

void
SatMarkovConf::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  Object::DoDispose ();
}

std::vector<std::vector<double> >
SatMarkovConf::GetElevationProbabilities (uint32_t set)
{
  NS_LOG_FUNCTION (this << set);

  if (set >= m_elevationCount)
    {
      NS_FATAL_ERROR ("SatMarkovConf::GetElevationProbabilities - Invalid set");
    }

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovConf::GetElevationProbabilities - Getting elevation probabilities for set ID " << set);
  return m_markovProbabilities[set];
}

uint32_t
SatMarkovConf::GetProbabilitySetID (double elevation)
{
  NS_LOG_FUNCTION (this << elevation);

  if (elevation < 0.0 && elevation > 90.0)
    {
      NS_FATAL_ERROR ("SatMarkovConf::GetProbabilitySetID - Invalid elevation");
    }

  uint32_t smallestDifferenceIndex = 0;
  double smallestDifference = 360; /// elevation angle can never be this large
  double difference = 0;

  std::map<double, uint32_t>::iterator iter;

  for (iter = m_markovElevations.begin (); iter != m_markovElevations.end (); iter++)
    {
      difference = fabs (iter->first - elevation);
      if (difference < smallestDifference)
        {
          smallestDifference = difference;
          smallestDifferenceIndex = iter->second;
        }
    }
  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovConf::GetProbabilitySetID - New ID for elevation " << elevation << " is " << smallestDifferenceIndex);

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

  return m_markovElevations.size ();
}

uint32_t
SatMarkovConf::GetInitialState ()
{
  NS_LOG_FUNCTION (this);

  double total = 0;

  for (uint32_t i = 0; i < m_stateCount; ++i)
    {
      total += m_initialProbabilities[i];
    }

  if (total != 1)
    {
      NS_FATAL_ERROR ("SatMarkovConf::GetInitialState - Total sum doesn not match");
    }

  double r = total * (std::rand () / double (RAND_MAX));
  double acc = 0.0;

  for (uint32_t i = 0; i < m_stateCount; ++i)
    {
      acc += m_initialProbabilities[i];

      if (r <= acc)
        {
          return i;
        }
    }

  return m_stateCount - 1;
}

Ptr<SatLooConf>
SatMarkovConf::GetLooConf ()
{
  NS_LOG_FUNCTION (this);

  return m_looConf;
}

Ptr<SatRayleighConf>
SatMarkovConf::GetRayleighConf ()
{
  NS_LOG_FUNCTION (this);

  return m_rayleighConf;
}

SatMarkovConf::MarkovFaderType_t
SatMarkovConf::GetFaderType ()
{
  NS_LOG_FUNCTION (this);

  return m_faderType;
}

bool
SatMarkovConf::AreDecibelsUsed ()
{
  NS_LOG_FUNCTION (this);

  return m_useDecibels;
}

} // namespace ns3
