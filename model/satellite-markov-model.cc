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

#include "satellite-markov-model.h"
#include <cstdlib>
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMarkovModel);
NS_LOG_COMPONENT_DEFINE ("SatMarkovModel");

TypeId SatMarkovModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMarkovModel")
      .SetParent<Object> ()
      .AddConstructor<SatMarkovModel> ();
  return tid;
}

SatMarkovModel::SatMarkovModel () :
    m_probabilities (new double[3 * 3]),
    m_numOfStates (3),
    m_currentState (0)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT(0);
}

SatMarkovModel::SatMarkovModel (uint32_t numOfStates) :
    m_probabilities (new double[numOfStates * numOfStates]),
    m_numOfStates (numOfStates),
    m_currentState (0)
{
  NS_LOG_FUNCTION (this << numOfStates);

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - Creating Markov model for " << numOfStates << " states, initial state: " << m_currentState);

  for (uint32_t i = 0; i < m_numOfStates; ++i)
    {
      for (uint32_t j = 0; j < m_numOfStates; ++j)
        {
          m_probabilities[i * m_numOfStates + j] = 1;
        }
    }
}

SatMarkovModel::~SatMarkovModel ()
{
  NS_LOG_FUNCTION (this);

  delete[] m_probabilities;
}

uint32_t SatMarkovModel::GetState () const
{
  NS_LOG_FUNCTION (this);

  return m_currentState;
}

void SatMarkovModel::SetState (uint32_t newState)
{
  NS_LOG_FUNCTION (this << newState);

  NS_ASSERT ((newState >= 0 ) && (newState < m_numOfStates));
  m_currentState = newState;
}

uint32_t SatMarkovModel::DoTransition ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - Doing transition, current state: " << m_currentState);

  double total = 0;
  for (uint32_t i = 0; i < m_numOfStates; ++i)
    {
      total += m_probabilities[m_currentState * m_numOfStates + i];
    }

  NS_ASSERT(total == 1);

  double r = total * (std::rand () / double (RAND_MAX));

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - random value: " << r);

  double acc = 0.0;
  for (uint32_t i = 0; i < m_numOfStates; ++i)
    {
      acc += m_probabilities[m_currentState * m_numOfStates + i];

      NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - state " << i << " accumulated value: " << acc);

      if (r <= acc)
        {
          NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - Transition done, new state: " << i);
          m_currentState = i;
          return i;
        }
    }
  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - Transition done, new state: " << m_numOfStates - 1);
  m_currentState = m_numOfStates - 1;
  return m_numOfStates - 1;
}

void SatMarkovModel::SetProbability (uint32_t from,
                                     uint32_t to,
                                     double probability)
{
  NS_LOG_FUNCTION (this << from << " " << to << " " << probability);

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovModel - Setting probability, from: " << from << " to: " << to << " probability: " << probability);
  m_probabilities[from * m_numOfStates + to] = probability;
}

} // namespace ns3
