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

#include "satellite-fading-container.h"
#include "satellite-channel.h"

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatFadingContainer);
NS_LOG_COMPONENT_DEFINE ("SatFadingContainer");

TypeId
SatFadingContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingContainer")
    .SetParent<Object> ()
    .AddConstructor<SatFadingContainer> ();
  return tid;
}

SatFadingContainer::SatFadingContainer ()
  : m_numOfStates (),
    m_currentElevation (),
    m_setId (),
    m_cooldownPeriodLength (),
    m_minimumPositionChangeInMeters (),
    m_currentPosition (),
    m_latestCalculationPosition (),
    m_latestCalculatedFadingValue_up (),
    m_latestCalculatedFadingValue_down (),
    m_latestCalculationTime_up (),
    m_latestCalculationTime_down (),
    m_dopplerFrequencyHz (),
    m_numOfOscillators (),
    m_enableStateLock (false),
    m_lockedState ()
{
  m_markovConf = NULL;
  m_markovModel = NULL;
  m_looModel_up = NULL;
  m_looModel_down = NULL;
  NS_ASSERT(0);
}

SatFadingContainer::SatFadingContainer (Ptr<SatMarkovConf> markovConf, GeoCoordinate currentPosition)
  : m_numOfStates(markovConf->GetStateCount ()),
    m_currentElevation(30),
    m_cooldownPeriodLength (markovConf->GetCooldownPeriod ()),
    m_minimumPositionChangeInMeters (markovConf->GetMinimumPositionChange ()),
    m_currentPosition (currentPosition),
    m_latestCalculationPosition (currentPosition),
    m_latestCalculatedFadingValue_up (0.0),
    m_latestCalculatedFadingValue_down (0.0),
    m_latestCalculationTime_up (Now()),
    m_latestCalculationTime_down (Now()),
    m_dopplerFrequencyHz (markovConf->GetDopplerFrequency ()),
    m_numOfOscillators (markovConf->GetNumOfOscillators ()),
    m_enableStateLock (false),
    m_lockedState (0)
{
  m_markovConf = markovConf;
  m_markovModel = CreateObject<SatMarkovModel> (m_numOfStates);
  m_setId = m_markovConf->GetProbabilitySetID (m_currentElevation);
  m_looModel_up = CreateObject<SatLooModel> (0.5,0.5,-8.0,m_dopplerFrequencyHz,m_numOfOscillators);
  m_looModel_down = CreateObject<SatLooModel> (0.5,0.5,-8.0,m_dopplerFrequencyHz,m_numOfOscillators);

  UpdateProbabilities (m_setId);
  UpdateLooParameters (m_setId);

  NS_LOG_INFO("Time " << Now ().GetSeconds ()
              << " SatFadingContainer - Creating SatFadingContainer, States: " << m_numOfStates
              << " Elevation: " << m_currentElevation
              << " Initial Set ID: " << m_setId
              << " Cooldown Period Length In Seconds: " << m_cooldownPeriodLength.GetSeconds ()
              << " Minimum Position Change In Meters: " << m_minimumPositionChangeInMeters
  );
}

SatFadingContainer::~SatFadingContainer ()
{
}

double
SatFadingContainer::GetFading (SatChannel::ChannelType channeltype)
{
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Getting fading");

  if (HasCooldownPeriodPassed (channeltype))
    {
      NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Cooldown period has passed, calculating new fading value");

      if (!m_enableStateLock)
        {
          EvaluateStateChange ();
        }

      return CalculateFading (channeltype);
    }
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Cooldown period in effect, using old fading value");
  return GetCachedFadingValue (channeltype);
}

double
SatFadingContainer::GetCachedFadingValue (SatChannel::ChannelType channeltype)
{
  switch (channeltype)
  {
    case SatChannel::RETURN_USER_CH:
    case SatChannel::FORWARD_FEEDER_CH:
      {
        return m_latestCalculatedFadingValue_up;
      }
    case SatChannel::FORWARD_USER_CH:
    case SatChannel::RETURN_FEEDER_CH:
      {
        return m_latestCalculatedFadingValue_down;
      }
    default :
      {
        NS_ASSERT(0);
      }
  }
  NS_ASSERT(0);
  return -1;
}

void
SatFadingContainer::EvaluateStateChange ()
{
  if (HasPositionChanged ())
    {
      uint32_t newSetId = m_markovConf->GetProbabilitySetID (m_currentElevation);

      if (m_setId != newSetId)
        {
          NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - elevation: " << m_currentElevation  << ", set ID [old,new]: [" << m_setId << "," << newSetId << "]");

          m_setId = newSetId;
          UpdateProbabilities (m_setId);
          UpdateLooParameters (m_setId);
        }
      m_markovModel->DoTransition ();
    }
}

bool
SatFadingContainer::HasCooldownPeriodPassed (SatChannel::ChannelType channeltype)
{
  switch (channeltype)
  {
    case SatChannel::RETURN_USER_CH:
    case SatChannel::FORWARD_FEEDER_CH:
      {
        if ( (Now().GetSeconds() - m_latestCalculationTime_up.GetSeconds ()) > m_cooldownPeriodLength.GetSeconds () )
          {
            return true;
          }
        break;
      }
    case SatChannel::FORWARD_USER_CH:
    case SatChannel::RETURN_FEEDER_CH:
      {
        if ( (Now().GetSeconds() - m_latestCalculationTime_down.GetSeconds ()) > m_cooldownPeriodLength.GetSeconds () )
          {
            return true;
          }
        break;
      }
    default :
      {
        NS_ASSERT(0);
      }
  }
  return false;
}

bool
SatFadingContainer::HasPositionChanged ()
{
  ns3::Vector3D a = m_currentPosition.ToVector ();
  ns3::Vector3D b = m_latestCalculationPosition.ToVector ();

  double dx = b.x - a.x;
  double dy = b.y - a.y;
  double dz = b.z - a.z;
  double distance = std::sqrt (dx * dx + dy * dy + dz * dz);

  if (distance > m_minimumPositionChangeInMeters)
    {
      return true;
    }
  return false;
}

void
SatFadingContainer::UpdateProbabilities (uint32_t setId)
{
  std::vector <std::vector <double> > probabilities = m_markovConf->GetElevationProbabilities (setId);

  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Updating probabilities...");

  for (uint32_t i = 0; i < m_numOfStates; ++i)
    {
    for (uint32_t j = 0; j < m_numOfStates; ++j)
      {
        m_markovModel->SetProbability (i,j,probabilities[i][j]);
      }
    NS_LOG_INFO("------");
    }
}

void
SatFadingContainer::UpdateLooParameters (uint32_t setId)
{
  std::vector <std::vector <double> > looParameters = m_markovConf->GetLooParameters (setId);

  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Updating Loo parameters...");

  for (uint32_t i = 0; i < m_numOfStates; ++i)
    {
    for (uint32_t j = 0; j < SatMarkovConf::DEFAULT_LOO_PARAMETER_COUNT; ++j)
      {
        m_looParameters[i][j] = looParameters[i][j];
      }
    NS_LOG_INFO("------");
    }
}

void
SatFadingContainer::SetPosition (GeoCoordinate newPosition)
{
  m_currentPosition = newPosition;
}

void
SatFadingContainer::SetElevation (double newElevation)
{
  NS_ASSERT( (newElevation >= 0.0) && (newElevation <= 90.0));
  m_currentElevation = newElevation;
}


double
SatFadingContainer::CalculateFading (SatChannel::ChannelType channeltype)
{
  uint32_t state;
  if (m_enableStateLock)
    {
      state = m_lockedState;
    }
  else
    {
      state = m_markovModel->GetState ();
    }

  NS_ASSERT( (state >= 0) && (state < m_numOfStates));

  m_latestCalculationPosition = m_currentPosition;

  switch (channeltype)
  {
    case SatChannel::RETURN_USER_CH:
    case SatChannel::FORWARD_FEEDER_CH:
      {
        m_looModel_up->UpdateParameters (m_looParameters[state][0], m_looParameters[state][1], m_looParameters[state][2]);
        m_latestCalculatedFadingValue_up = m_looModel_up->GetChannelGainDb ();
        NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Calculated feeder fading value " << m_latestCalculatedFadingValue_up);
        m_latestCalculationTime_up = Now ();
        return m_latestCalculatedFadingValue_up;
      }
    case SatChannel::FORWARD_USER_CH:
    case SatChannel::RETURN_FEEDER_CH:
      {
        m_looModel_down->UpdateParameters (m_looParameters[state][0], m_looParameters[state][1], m_looParameters[state][2]);
        m_latestCalculatedFadingValue_down = m_looModel_down->GetChannelGainDb ();
        NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Calculated return fading value " << m_latestCalculatedFadingValue_down);
        m_latestCalculationTime_down = Now ();
        return m_latestCalculatedFadingValue_down;
      }
    default :
      {
        NS_ASSERT(0);
      }
  }
  NS_ASSERT(0);
  return -1;
}

void
SatFadingContainer::LockToState (uint32_t state)
{
  NS_ASSERT( (state >= 0) && (state < m_numOfStates));
  m_lockedState = state;
  m_enableStateLock = true;
}

void
SatFadingContainer::LockToRandomState ()
{
  LockToState (rand() % (m_numOfStates-1));
}

void
SatFadingContainer::UnlockState ()
{
  m_lockedState = 0;
  m_enableStateLock = false;
}

} // namespace ns3
