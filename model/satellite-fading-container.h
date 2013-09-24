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

#include "satellite-rayleigh-model.h"
#include "satellite-loo-model.h"
#include "satellite-markov-model.h"
#include "satellite-markov-conf.h"
#include "geo-coordinate.h"
#include "ns3/vector.h"
#include "satellite-channel.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Fading model container
 */
class SatFadingContainer : public Object
{
public:
  static TypeId GetTypeId (void);

  SatFadingContainer ();

  SatFadingContainer (Ptr<SatMarkovConf> markovConf, GeoCoordinate currentPosition);

  ~SatFadingContainer ();

  double GetFading (SatChannel::ChannelType channeltype);

  void SetPosition (GeoCoordinate newPosition);

  void SetElevation (double newElevation);

  void UnlockSetAndState ();

  void LockToSetAndState (uint32_t setId, uint32_t stateId);

  void LockToSet (uint32_t setId);

  void LockToRandomSetAndState ();

private:

  Ptr<SatMarkovModel> m_markovModel;
  Ptr<SatMarkovConf> m_markovConf;
  Ptr<SatLooModel> m_looModel_up;
  Ptr<SatLooModel> m_looModel_down;

  uint32_t m_numOfStates;
  uint32_t m_numOfSets;
  double m_currentElevation;
  uint32_t m_setId;
  Time m_cooldownPeriodLength;
  double m_minimumPositionChangeInMeters;
  GeoCoordinate m_currentPosition;
  GeoCoordinate m_latestCalculationPosition;
  double m_latestCalculatedFadingValue_up;
  double m_latestCalculatedFadingValue_down;
  Time m_latestCalculationTime_up;
  Time m_latestCalculationTime_down;
  double m_dopplerFrequencyHz;
  uint32_t m_numOfOscillators;
  bool m_enableSetLock;
  bool m_enableStateLock;
  uint32_t m_lockedState;

  std::vector <std::vector <double> > m_looParameters;

  TracedCallback< double,                   // time
                  SatChannel::ChannelType,  // channel type
                  double                    // fading value
                  >
     m_fadingTrace;

  void UpdateProbabilities (uint32_t setId);
  void UpdateLooParameters (uint32_t setId);
  void EvaluateStateChange ();
  double CalculateFading (SatChannel::ChannelType channeltype);
  double CalculateElevation ();
  bool HasPositionChanged ();
  bool HasCooldownPeriodPassed (SatChannel::ChannelType channeltype);
  double GetCachedFadingValue (SatChannel::ChannelType channeltype);
};

} // namespace ns3

#endif /* SATELLITE_FADING_CONTAINER_H */
