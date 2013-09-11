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
#ifndef SATELLITE_MARKOV_MODEL_H
#define SATELLITE_MARKOV_MODEL_H

#include "ns3/object.h"
#include "ns3/log.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Markov model
 */
class SatMarkovModel : public Object
{
public:
  static TypeId GetTypeId (void);

  SatMarkovModel ();

  /** Create a new Markov process with nstates states. */
  SatMarkovModel (uint32_t numOfStates);

  /** Destroy the Markov process. */
  ~SatMarkovModel ();

  /** Set the probability of transition from one state to another. */
  void SetProbability (uint32_t from,
                       uint32_t to,
                       double prob);

  /** Go to next (nondeterministic) state. */
  uint32_t DoTransition ();

  /** Get the current state. */
  uint32_t GetState () const;

  /** Set the current state. */
  void SetState (uint32_t newState);

private:

  /** Probability matrix. */
  double* m_probabilities;

  /** Number of states. */
  uint32_t m_numOfStates;

  /** Current state. */
  uint32_t m_currentState;

};

} // namespace ns3

#endif /* SATELLITE_MARKOV_MODEL_H */
