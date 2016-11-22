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

#include "satellite-loo-conf.h"
#include "satellite-markov-conf.h"
#include <map>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLooConf);
NS_LOG_COMPONENT_DEFINE ("SatLooConf");

static const double g_LooParameters[SatMarkovConf::DEFAULT_ELEVATION_COUNT][SatMarkovConf::DEFAULT_STATE_COUNT][SatLooConf::DEFAULT_LOO_PARAMETER_COUNT] =
{
  /**
   * Parameters
   * {State 1 {direct signal mean in dB, direct signal std deviation in dB, rms squared multipath power in dB, number of direct signal oscillators, number of multipath oscillators, direct signal Doppler in Hz, multipath Doppler in Hz}
   *  State 2 {direct signal mean in dB, direct signal std deviation in dB, rms squared multipath power in dB, number of direct signal oscillators, number of multipath oscillators, direct signal Doppler in Hz, multipath Doppler in Hz}
   *  State 3 {direct signal mean in dB, direct signal std deviation in dB, rms squared multipath power in dB, number of direct signal oscillators, number of multipath oscillators, direct signal Doppler in Hz, multipath Doppler in Hz}}
   */

  /* Elevation 30 degrees */
  {{  0.0, 0.5, -25.0, 10, 10, 2, 30},
   {-10.0, 3.0, -25.0, 10, 10, 2, 30},
   {-21.0, 4.0, -25.0, 10, 10, 2, 30}}
};

TypeId
SatLooConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLooConf")
    .SetParent<SatBaseFaderConf> ()
    .AddConstructor<SatLooConf> ()
    .AddAttribute ("ElevationCount", "Number of elevation sets in the Markov model.",
                   UintegerValue (SatMarkovConf::DEFAULT_ELEVATION_COUNT),
                   MakeUintegerAccessor (&SatLooConf::m_elevationCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("StateCount", "Number of states in the Markov model.",
                   UintegerValue (SatMarkovConf::DEFAULT_STATE_COUNT),
                   MakeUintegerAccessor (&SatLooConf::m_stateCount),
                   MakeUintegerChecker<uint32_t> ());
  return tid;
}

SatLooConf::SatLooConf ()
  : m_elevationCount (SatMarkovConf::DEFAULT_ELEVATION_COUNT),
    m_stateCount (SatMarkovConf::DEFAULT_STATE_COUNT)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatLooConf::SatLooConf - Creating SatLooConf...");

  for (uint32_t i = 0; i < m_elevationCount; i++)
    {
      std::vector<std::vector<double> > states;

      for (uint32_t j = 0; j < m_stateCount; j++)
        {
          std::vector<double> parameters;

          for (uint32_t k = 0; k < SatLooConf::DEFAULT_LOO_PARAMETER_COUNT; k++)
            {
              parameters.push_back (g_LooParameters[i][j][k]);
            }
          states.push_back (parameters);
        }
      m_looParameters.push_back (states);
    }
}

SatLooConf::~SatLooConf ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

std::vector<std::vector<double> >
SatLooConf::GetParameters (uint32_t set)
{
  NS_LOG_FUNCTION (this << set);

  if (set >= m_elevationCount)
    {
      NS_FATAL_ERROR ("SatLooConf::GetParameters - Invalid set");
    }

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatLooConf::GetParameters - Getting Loo parameters for set ID " << set);
  return m_looParameters[set];
}

void
SatLooConf::Reset ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_elevationCount; i++)
    {
      for (uint32_t j = 0; j < m_stateCount; j++)
        {
          m_looParameters[i][j].clear ();
        }
    }
}

void
SatLooConf::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  SatBaseFaderConf::DoDispose ();
}

} // namespace ns3

