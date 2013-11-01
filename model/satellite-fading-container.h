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
#ifndef SATELLITE_FADING_CONTAINER_H
#define SATELLITE_FADING_CONTAINER_H

#include "satellite-loo-model.h"
#include "satellite-markov-model.h"
#include "satellite-markov-conf.h"
#include "geo-coordinate.h"
#include "ns3/vector.h"
#include "satellite-fading.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Fading model container
 */
class SatFadingContainer : public SatFading
{
public:

  /**
   * \brief NS-3 function for type id
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  SatFadingContainer ();

  /**
   * \brief Constructor
   * \param markovConf Markov configuration object
   * \param looConf Loo configuration object
   * \param currentPosition current position
   */
  SatFadingContainer (Ptr<SatMarkovConf> markovConf, GeoCoordinate currentPosition);

  /**
   * \brief Destructor
   */
  ~SatFadingContainer ();

  /**
   * \brief Function for getting the fading
   * \param channeltype channel type
   * \return fading value
   */
  double GetFading (SatChannel::ChannelType_t channeltype);

  /**
   * \brief Function for setting the position
   * \param newPosition new position
   */
  void SetPosition (GeoCoordinate newPosition);

  /**
   * \brief Function for setting the elevation
   * \param newElevation new elevation
   */
  void SetElevation (double newElevation);

  /**
   * \brief Function for unlocking the parameter set and state
   */
  void UnlockSetAndState ();

  /**
   * \brief Function for locking the parameter set and state
   * \param newSet new set
   * \param newState new state
   */
  void LockToSetAndState (uint32_t newSet, uint32_t newState);

  /**
   * \brief Function for locking the parameter set
   * \param newSet new set
   */
  void LockToSet (uint32_t newSet);

  /**
   * \brief Function for locking the parameter set and state to random values
   */
  void LockToRandomSetAndState ();

private:

  /**
   * \brief Markov model object
   */
  Ptr<SatMarkovModel> m_markovModel;

  /**
   * \brief Markoc model configuration
   */
  Ptr<SatMarkovConf> m_markovConf;

  /**
   * \brief Uplink fader
   */
  Ptr<SatFader> m_fader_up;

  /**
   * \brief Downlink fader
   */
  Ptr<SatFader> m_fader_down;

  /**
   * \brief Number of states available
   */
  uint32_t m_numOfStates;

  /**
   * \brief Number of parameter sets available
   */
  uint32_t m_numOfSets;

  /**
   * \brief Current elevation value
   */
  double m_currentElevation;

  /**
   * \brief Current parameter set
   */
  uint32_t m_currentSet;

  /**
   * \brief Current state
   */
  uint32_t m_currentState;

  /**
   * \brief Cooldown period length in time
   */
  Time m_cooldownPeriodLength;

  /**
   * \brief Minimum state change distance in meters
   */
  double m_minimumPositionChangeInMeters;

  /**
   * \brief Current position
   */
  GeoCoordinate m_currentPosition;

  /**
   * \brief Latest position where calculations were done
   */
  GeoCoordinate m_latestCalculationPosition;

  /**
   * \brief Latest calculated uplink fading value
   */
  double m_latestCalculatedFadingValue_up;

  /**
   * \brief Latest calculated downlink fading value
   */
  double m_latestCalculatedFadingValue_down;

  /**
   * \brief Latest calculation time for uplink fading value
   */
  Time m_latestCalculationTime_up;

  /**
   * \brief Latest calculation time for downlink fading value
   */
  Time m_latestCalculationTime_down;

  /**
   * \brief Variable for disabling the parameter set change
   */
  bool m_enableSetLock;

  /**
   * \brief Variable for disabling state changes
   */
  bool m_enableStateLock;

  /**
   * \brief Fading trace function
   */
  TracedCallback< double,                     // time
                  SatChannel::ChannelType_t,  // channel type
                  double                      // fading value
                  >
     m_fadingTrace;

  /**
   * \brief Function for updating the state change probabilities
   * \param set parameter set
   */
  void UpdateProbabilities (uint32_t set);

  /**
   * \brief Function for evaluating state change
   */
  void EvaluateStateChange ();

  /**
   * \brief Function for calculating the fadign value
   * \param channelType channel type
   * \return fading value
   */
  double CalculateFading (SatChannel::ChannelType_t channelType);

  /**
   * \brief Function for calculating the elevation
   * \return elevation value
   */
  double CalculateElevation ();

  /**
   * \brief Function for checking the position change
   * \return has position changed more that the defined parameter value
   */
  bool HasPositionChanged ();

  /**
   * \brief Function for checking whether the cooldown period has passed
   * \param channelType channel type
   * \return has cooldown period passed
   */
  bool HasCooldownPeriodPassed (SatChannel::ChannelType_t channelType);

  /**
   * \brief Function for getting the cached fadign values
   * \param channelType channel type
   * \return cached fading value
   */
  double GetCachedFadingValue (SatChannel::ChannelType_t channelType);

  /**
   * \brief Function for creating the Markov state faders
   */
  void CreateFaders (SatMarkovConf::MarkovFaderType_t faderType);

};

} // namespace ns3

#endif /* SATELLITE_FADING_CONTAINER_H */
