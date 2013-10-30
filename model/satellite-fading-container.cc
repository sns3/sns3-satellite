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
#include "satellite-utils.h"

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatFadingContainer);
NS_LOG_COMPONENT_DEFINE ("SatFadingContainer");

TypeId
SatFadingContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingContainer")
    .SetParent<SatFading> ()
    .AddConstructor<SatFadingContainer> ()
    .AddTraceSource ("FadingTrace",
                     "The trace for fading values",
                     MakeTraceSourceAccessor (&SatFadingContainer::m_fadingTrace));

  return tid;
}

SatFadingContainer::SatFadingContainer () :
    m_markovModel (NULL),
    m_markovConf (NULL),
    m_looConf (NULL),
    m_fader_up (NULL),
    m_fader_down (NULL),
    m_numOfStates (),
    m_numOfSets (),
    m_currentElevation (),
    m_currentSet (),
    m_currentState (),
    m_cooldownPeriodLength (),
    m_minimumPositionChangeInMeters (),
    m_currentPosition (),
    m_latestCalculationPosition (),
    m_latestCalculatedFadingValue_up (),
    m_latestCalculatedFadingValue_down (),
    m_latestCalculationTime_up (),
    m_latestCalculationTime_down (),
    m_enableSetLock (false),
    m_enableStateLock (false)
{
  NS_ASSERT(0);
}

SatFadingContainer::SatFadingContainer (Ptr<SatMarkovConf> markovConf, Ptr<SatLooConf> looConf, GeoCoordinate currentPosition) :
    m_markovModel (NULL),
    m_markovConf (markovConf),
    m_looConf (looConf),
    m_fader_up (NULL),
    m_fader_down (NULL),
    m_numOfStates(markovConf->GetStateCount ()),
    m_numOfSets(markovConf->GetNumOfSets ()),
    m_currentElevation (markovConf->GetInitialElevation()),
    m_currentState (markovConf->GetInitialState()),
    m_cooldownPeriodLength (markovConf->GetCooldownPeriod ()),
    m_minimumPositionChangeInMeters (markovConf->GetMinimumPositionChange ()),
    m_currentPosition (currentPosition),
    m_latestCalculationPosition (currentPosition),
    m_latestCalculatedFadingValue_up (0.0),
    m_latestCalculatedFadingValue_down (0.0),
    m_latestCalculationTime_up (Now()),
    m_latestCalculationTime_down (Now()),
    m_enableSetLock (false),
    m_enableStateLock (false)
{
  m_markovModel = CreateObject<SatMarkovModel> (m_numOfStates);

  m_currentSet = m_markovConf->GetProbabilitySetID (m_currentElevation);

  UpdateProbabilities (m_currentSet);

  m_markovModel->DoTransition ();

  m_fader_up = CreateObject<SatLooModel> (m_looConf,m_numOfStates,m_currentSet,m_currentState);
  m_fader_down = CreateObject<SatLooModel> (m_looConf,m_numOfStates,m_currentSet,m_currentState);

  NS_LOG_INFO("Time " << Now ().GetSeconds ()
              << " SatFadingContainer - Creating SatFadingContainer, States: " << m_numOfStates
              << " Elevation: " << m_currentElevation
              << " Current Set ID: " << m_currentSet
              << " Cooldown Period Length In Seconds: " << m_cooldownPeriodLength.GetSeconds ()
              << " Minimum Position Change In Meters: " << m_minimumPositionChangeInMeters
  );
}

SatFadingContainer::~SatFadingContainer ()
{
}

double
SatFadingContainer::GetFading (SatChannel::ChannelType_t channeltype)
{
  double fadingValue;

  NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Getting fading");

  if (HasCooldownPeriodPassed (channeltype))
    {
      NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Cooldown period has passed, calculating new fading value");
      EvaluateStateChange ();
      fadingValue = CalculateFading (channeltype);
    }
  else
    {
      NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Cooldown period in effect, using old fading value");
      fadingValue = GetCachedFadingValue (channeltype);
    }

  m_fadingTrace ( Now ().GetSeconds (), channeltype, fadingValue);

  return fadingValue;
}

double
SatFadingContainer::GetCachedFadingValue (SatChannel::ChannelType_t channeltype)
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
      uint32_t newSetId;

      if (!m_enableSetLock)
        {
          newSetId = m_markovConf->GetProbabilitySetID (m_currentElevation);

          if (m_currentSet != newSetId)
            {
              NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - elevation: " << m_currentElevation  << ", set ID [old,new]: [" << m_currentSet << "," << newSetId << "]");

              m_currentSet = newSetId;
              UpdateProbabilities (m_currentSet);
            }
        }

      if (!m_enableStateLock)
        {
          m_markovModel->DoTransition ();
        }
    }
}

bool
SatFadingContainer::HasCooldownPeriodPassed (SatChannel::ChannelType_t channeltype)
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
SatFadingContainer::CalculateFading (SatChannel::ChannelType_t channeltype)
{
  if (!m_enableStateLock)
    {
      m_currentState = m_markovModel->GetState ();
    }

  NS_ASSERT( (m_currentState >= 0) && (m_currentState < m_numOfStates));

  m_latestCalculationPosition = m_currentPosition;

  switch (channeltype)
  {
    case SatChannel::RETURN_USER_CH:
    case SatChannel::FORWARD_FEEDER_CH:
      {
        m_fader_up->UpdateParameters (m_currentSet, m_currentState);
        m_latestCalculatedFadingValue_up = m_fader_up->GetChannelGain ();
        NS_LOG_INFO("Time " << Now ().GetSeconds () << " SatFadingContainer - Calculated feeder fading value " << m_latestCalculatedFadingValue_up);
        m_latestCalculationTime_up = Now ();
        return m_latestCalculatedFadingValue_up;
      }
    case SatChannel::FORWARD_USER_CH:
    case SatChannel::RETURN_FEEDER_CH:
      {
        m_fader_down->UpdateParameters (m_currentSet, m_currentState);
        m_latestCalculatedFadingValue_down = m_fader_down->GetChannelGain ();
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
SatFadingContainer::LockToSetAndState (uint32_t newSet, uint32_t newState)
{
  NS_ASSERT( (newState >= 0) && (newState < m_numOfStates));
  NS_ASSERT( (newSet >= 0) && (newSet < m_numOfSets));

  m_currentSet = newSet;
  m_currentState = newState;

  UpdateProbabilities (m_currentSet);

  m_enableSetLock = true;
  m_enableStateLock = true;
}

void
SatFadingContainer::LockToSet (uint32_t setId)
{
  NS_ASSERT( (setId >= 0) && (setId < m_numOfSets));

  m_currentSet = setId;

  UpdateProbabilities (m_currentSet);

  m_enableSetLock = true;
  m_enableStateLock = false;
}

void
SatFadingContainer::LockToRandomSetAndState ()
{
  LockToSetAndState ((rand() % (m_numOfSets-1)),(rand() % (m_numOfStates-1)));
}

void
SatFadingContainer::UnlockSetAndState ()
{
  m_enableSetLock = false;
  m_enableStateLock = false;
}

} // namespace ns3
