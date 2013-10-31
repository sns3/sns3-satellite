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

#ifndef SAT_MARKOV_CONF_H
#define SAT_MARKOV_CONF_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \brief A configuration class for three state Markov model
 *
 */
class SatMarkovConf : public Object
{
public:

  /**
   * \brief Default elevation count
   */
  static const uint32_t DEFAULT_ELEVATION_COUNT = 4;

  /**
   * \brief Default state count
   */
  static const uint32_t DEFAULT_STATE_COUNT = 3;

  /**
   * \brief NS-3 function for type id
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  SatMarkovConf ();

  /**
   * \brief Destructor
   */
  virtual ~SatMarkovConf () {}

  /**
   * \brief Function for returning the parameter set
   * \param elevation elevation
   * \return parameter set
   */
  uint32_t GetProbabilitySetID (double elevation);

  /**
   * \brief Function for returning the probabilities
   * \param set parameter set
   * \return probabilities
   */
  std::vector<std::vector<double> > GetElevationProbabilities (uint32_t set);

  /**
   * \brief Function for returning the number of states
   * \return number of states
   */
  uint32_t GetStateCount ();

  /**
   * \brief Function for returning the cooldown period
   * \return cooldown period
   */
  Time GetCooldownPeriod ();

  /**
   * \brief Function for returning the minimum position change distance
   * \return minimum position change distance value
   */
  double GetMinimumPositionChange ();

  /**
   * \brief Function for returning the number of parameter sets
   * \return number of parameter sets
   */
  uint32_t GetNumOfSets ();

  /**
   * \brief Function for returning the initial elevation value
   * \return initial elevation value
   */
  double GetInitialElevation ();

  /**
   * \brief Function for returning the initial state
   * \return initial state
   */
  uint32_t GetInitialState ();

private:

  /**
   * \brief Number of parameter sets
   */
  uint32_t m_elevationCount;

  /**
   * \brief Number of states
   */
  uint32_t m_stateCount;

  /**
   * \brief Markov probabilities
   */
  std::vector<std::vector<std::vector<double> > > m_markovProbabilities;

  /**
   * \brief Markov elevations
   */
  std::map<double, uint32_t> m_markovElevations;

  /**
   * \brief Minimum position change in meters
   */
  double m_minimumPositionChangeInMeters;

  /**
   * \brief Initial state
   */
  uint32_t m_initialState;

  /**
   * \brief Initial elevation in degrees
   */
  double m_initialElevation;

  /**
   * \brief Cooldown period lengthin seconds
   */
  Time m_cooldownPeriodLength;
};

} // namespace ns3

#endif /* SAT_MARKOV_CONF_H */
