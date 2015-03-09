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
#include "ns3/boolean.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-loo-conf.h"
#include "satellite-rayleigh-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief A configuration class for three state Markov model. This class provides
 * configuration parameters for the Markov-based fading. It also acts as an
 * interface for the configurations used by the different faders in Markov-based
 * fading.
 */
class SatMarkovConf : public Object
{
public:
  /**
   * Possible types of Markov state faders
   */
  typedef enum
  {
    LOO_FADER, RAYLEIGH_FADER
  } MarkovFaderType_t;

  /**
   * \brief Default elevation count
   */
  static const uint32_t DEFAULT_ELEVATION_COUNT = 1;

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
  ~SatMarkovConf ();

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
   * \brief Function for returning the initial state
   * \return initial state
   */
  uint32_t GetInitialState ();

  /**
   * \brief Function for returning the Loo's model configuration
   * \return Loo's model configuration
   */
  Ptr<SatLooConf> GetLooConf ();

  /**
   * \brief Function for returning the Loo's model configuration
   * \return Loo's model configuration
   */
  Ptr<SatRayleighConf> GetRayleighConf ();

  /**
   * \brief Function for returning the selected fader type
   * \return selected fader type
   */
  SatMarkovConf::MarkovFaderType_t GetFaderType ();

  /**
   * \brief Function for getting whether the calculations should return the fading value in decibels or not
   * \return are decibels used or not
   */
  bool AreDecibelsUsed ();

  /**
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

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
   * \brief Markov state change probabilities
   */
  std::vector<std::vector<std::vector<double> > > m_markovProbabilities;

  /**
   * \brief Initial Markov state probabilities
   */
  std::vector<double> m_initialProbabilities;

  /**
   * \brief Markov elevations
   */
  std::map<double, uint32_t> m_markovElevations;

  /**
   * \brief Minimum position change in meters
   */
  double m_minimumPositionChangeInMeters;

  /**
   * \brief Cooldown period lengthin seconds
   */
  Time m_cooldownPeriodLength;

  /**
   * \brief Defines whether the calculations should return the fading value in decibels or not
   */
  bool m_useDecibels;

  /**
   * \brief Loo configuration
   */
  Ptr<SatLooConf> m_looConf;

  /**
   * \brief Rayleigh configuration
   */
  Ptr<SatRayleighConf> m_rayleighConf;

  /**
   * \brief Selected fader type
   */
  SatMarkovConf::MarkovFaderType_t m_faderType;

  /**
   * \brief Clear used variables
   */
  void Reset ();
};

} // namespace ns3

#endif /* SAT_MARKOV_CONF_H */
