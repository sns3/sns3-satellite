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
   *
   */
  static const uint32_t DEFAULT_ELEVATION_COUNT = 4;
  static const uint32_t DEFAULT_STATE_COUNT = 3;

  /**
   *
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   *
   */
  SatMarkovConf ();

  /**
   *
   */
  virtual ~SatMarkovConf () {}

  /**
   *
   * \param elevation
   * \return
   */
  uint32_t GetProbabilitySetID (double elevation);

  /**
   *
   * \param setId
   * \return
   */
  std::vector<std::vector<double> > GetElevationProbabilities (uint32_t set);

  /**
   *
   * \return
   */
  uint32_t GetStateCount ();

  /**
   *
   * \return
   */
  Time GetCooldownPeriod ();

  /**
   *
   * \return
   */
  double GetMinimumPositionChange ();

  /**
   *
   * \return
   */
  uint32_t GetNumOfSets ();

  /**
   *
   * \return
   */
  double GetInitialElevation ();

  /**
   *
   * \return
   */
  uint32_t GetInitialState ();

private:

  /**
   *
   */
  uint32_t m_elevationCount;
  uint32_t m_stateCount;
  std::vector<std::vector<std::vector<double> > > m_markovProbabilities;
  std::map<double, uint32_t> m_markovElevations;
  double m_minimumPositionChangeInMeters;
  uint32_t m_initialState;
  double m_initialElevation;
  Time m_cooldownPeriodLength;
};

} // namespace ns3

#endif /* SAT_MARKOV_CONF_H */
