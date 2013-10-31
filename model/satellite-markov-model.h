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

  /**
   *
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   *
   */
  SatMarkovModel ();

  /**
   *
   * \param numOfStates
   */
  SatMarkovModel (uint32_t numOfStates);

  /**
   *
   */
  ~SatMarkovModel ();

  /**
   *
   * \param from
   * \param to
   * \param probability
   */
  void SetProbability (uint32_t from,
                       uint32_t to,
                       double probability);

  /**
   *
   * \return
   */
  uint32_t DoTransition ();

  /**
   *
   * \return
   */
  uint32_t GetState () const;

  /**
   *
   * \param newState
   */
  void SetState (uint32_t newState);

private:

  /**
   *
   */
  double* m_probabilities;
  uint32_t m_numOfStates;
  uint32_t m_currentState;

};

} // namespace ns3

#endif /* SATELLITE_MARKOV_MODEL_H */
