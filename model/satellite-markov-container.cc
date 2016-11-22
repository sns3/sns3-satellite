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

#include "satellite-markov-container.h"
#include "satellite-utils.h"

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatMarkovContainer);
NS_LOG_COMPONENT_DEFINE ("SatMarkovContainer");

TypeId
SatMarkovContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMarkovContainer")
    .SetParent<SatBaseFading> ()
    .AddConstructor<SatMarkovContainer> ()
    .AddTraceSource ("FadingTrace",
                     "The trace for fading values",
                     MakeTraceSourceAccessor (&SatMarkovContainer::m_fadingTrace),
                     "ns3::SatMarkovContainer::FadingTraceCallback");
  return tid;
}

SatMarkovContainer::SatMarkovContainer ()
  : m_markovModel (NULL),
    m_markovConf (NULL),
    m_fader_up (NULL),
    m_fader_down (NULL),
    m_numOfStates (),
    m_numOfSets (),
    m_currentSet (),
    m_currentState (),
    m_cooldownPeriodLength (),
    m_minimumPositionChangeInMeters (),
    m_latestCalculatedFadingValue_up (),
    m_latestCalculatedFadingValue_down (),
    m_latestCalculationTime_up (),
    m_latestCalculationTime_down (),
    m_enableSetLock (false),
    m_enableStateLock (false),
    m_velocity (),
    m_latestStateChangeTime (),
    m_useDecibels (false)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatMarkovContainer::SatMarkovContainer - Constructor not in use");
}

SatMarkovContainer::SatMarkovContainer (Ptr<SatMarkovConf> markovConf, SatBaseFading::ElevationCallback elevation, SatBaseFading::VelocityCallback velocity)
  : m_markovModel (NULL),
    m_markovConf (markovConf),
    m_fader_up (NULL),
    m_fader_down (NULL),
    m_numOfStates (markovConf->GetStateCount ()),
    m_numOfSets (markovConf->GetNumOfSets ()),
    m_currentState (markovConf->GetInitialState ()),
    m_cooldownPeriodLength (markovConf->GetCooldownPeriod ()),
    m_minimumPositionChangeInMeters (markovConf->GetMinimumPositionChange ()),
    m_latestCalculatedFadingValue_up (0.0),
    m_latestCalculatedFadingValue_down (0.0),
    m_latestCalculationTime_up (Now ()),
    m_latestCalculationTime_down (Now ()),
    m_enableSetLock (false),
    m_enableStateLock (false),
    m_velocity (velocity),
    m_latestStateChangeTime (Now ()),
    m_currentElevation (elevation),
    m_useDecibels (markovConf->AreDecibelsUsed ())
{
  NS_LOG_FUNCTION (this);

  /// create Markov model
  m_markovModel = CreateObject<SatMarkovModel> (m_numOfStates,m_currentState);

  /// initialize Markov model
  m_currentSet = m_markovConf->GetProbabilitySetID (m_currentElevation ());
  UpdateProbabilities (m_currentSet);
  m_markovModel->DoTransition ();

  /// create faders
  CreateFaders (m_markovConf->GetFaderType ());

  /// initialize fading values
  CalculateFading (SatEnums::RETURN_USER_CH);
  CalculateFading (SatEnums::FORWARD_USER_CH);

  NS_LOG_INFO ("Time " << Now ().GetSeconds ()
                       << " SatMarkovContainer::SatMarkovContainer - Creating SatMarkovContainer, States: " << m_numOfStates
                       << " Elevation: " << m_currentElevation ()
                       << " Current Set ID: " << m_currentSet
                       << " Cool down Period Length In Seconds: " << m_cooldownPeriodLength.GetSeconds ()
                       << " Minimum Position Change In Meters: " << m_minimumPositionChangeInMeters
               );
}

SatMarkovContainer::~SatMarkovContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatMarkovContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  SatBaseFading::DoDispose ();
}

void
SatMarkovContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  m_markovConf = NULL;
  m_fader_up = NULL;
  m_fader_down = NULL;
  m_markovModel = NULL;

  m_currentElevation.Nullify ();
  m_velocity.Nullify ();
}

void
SatMarkovContainer::CreateFaders (SatMarkovConf::MarkovFaderType_t faderType)
{
  NS_LOG_FUNCTION (this << faderType);

  switch (faderType)
    {
    case SatMarkovConf::LOO_FADER:
      {
        m_fader_up = CreateObject<SatLooModel> (m_markovConf->GetLooConf (),m_numOfStates,m_currentSet,m_currentState);
        m_fader_down = CreateObject<SatLooModel> (m_markovConf->GetLooConf (),m_numOfStates,m_currentSet,m_currentState);
        break;
      }
    case SatMarkovConf::RAYLEIGH_FADER:
      {
        m_fader_up = CreateObject<SatRayleighModel> (m_markovConf->GetRayleighConf (),m_currentSet,m_currentState);
        m_fader_down = CreateObject<SatRayleighModel> (m_markovConf->GetRayleighConf (),m_currentSet,m_currentState);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatMarkovContainer::CreateFaders - Invalid fader type");
      }
    }
}

double
SatMarkovContainer::DoGetFading (Address macAddress, SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << channelType);

  double fadingValue;

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovContainer::DoGetFading - Getting fading");

  if (HasCooldownPeriodPassed (channelType))
    {
      NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovContainer::DoGetFading - Cool down period has passed, calculating new fading value");

      if (m_velocity () > 0)
        {
          EvaluateStateChange (channelType);
        }
      fadingValue = CalculateFading (channelType);
    }
  else
    {
      NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovContainer::DoGetFading - Cool down period in effect, using old fading value");
      fadingValue = GetCachedFadingValue (channelType);
    }

  m_fadingTrace (Now ().GetSeconds (), channelType, fadingValue);

  return fadingValue;
}

double
SatMarkovContainer::GetCachedFadingValue (SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << channelType);

  switch (channelType)
    {
    case SatEnums::RETURN_USER_CH:
    case SatEnums::FORWARD_FEEDER_CH:
      {
        return m_latestCalculatedFadingValue_up;
      }
    case SatEnums::FORWARD_USER_CH:
    case SatEnums::RETURN_FEEDER_CH:
      {
        return m_latestCalculatedFadingValue_down;
      }
    default:
      {
        NS_FATAL_ERROR ("SatMarkovContainer::GetCachedFadingValue - Invalid channel type");
      }
    }
  return 0;
}

void
SatMarkovContainer::EvaluateStateChange (SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this);

  if (CalculateDistanceSinceLastStateChange () > m_minimumPositionChangeInMeters)
    {
      uint32_t newSetId;

      if (!m_enableSetLock)
        {
          newSetId = m_markovConf->GetProbabilitySetID (m_currentElevation ());

          if (m_currentSet != newSetId)
            {
              NS_LOG_INFO ("Time " << Now ().GetSeconds ()
                                   << " SatMarkovContainer::EvaluateStateChange - elevation: " << m_currentElevation ()
                                   << ", set ID [old,new]: [" << m_currentSet << "," << newSetId << "]");

              m_currentSet = newSetId;
              UpdateProbabilities (m_currentSet);
            }
        }

      if (!m_enableStateLock)
        {
          m_latestStateChangeTime = Now ();
          m_markovModel->DoTransition ();
        }
    }
}

bool
SatMarkovContainer::HasCooldownPeriodPassed (SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << channelType);

  switch (channelType)
    {
    case SatEnums::RETURN_USER_CH:
    case SatEnums::FORWARD_FEEDER_CH:
      {
        if ( (Now ().GetSeconds () - m_latestCalculationTime_up.GetSeconds ()) > m_cooldownPeriodLength.GetSeconds () )
          {
            return true;
          }
        break;
      }
    case SatEnums::FORWARD_USER_CH:
    case SatEnums::RETURN_FEEDER_CH:
      {
        if ( (Now ().GetSeconds () - m_latestCalculationTime_down.GetSeconds ()) > m_cooldownPeriodLength.GetSeconds () )
          {
            return true;
          }
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatMarkovContainer::HasCooldownPeriodPassed - Invalid channel type");
      }
    }
  return false;
}

void
SatMarkovContainer::UpdateProbabilities (uint32_t set)
{
  NS_LOG_FUNCTION (this << set);

  std::vector <std::vector <double> > probabilities = m_markovConf->GetElevationProbabilities (set);

  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovContainer::UpdateProbabilities - Updating probabilities...");

  for (uint32_t i = 0; i < m_numOfStates; ++i)
    {
      for (uint32_t j = 0; j < m_numOfStates; ++j)
        {
          m_markovModel->SetProbability (i,j,probabilities[i][j]);
        }
      NS_LOG_INFO ("------");
    }
}

double
SatMarkovContainer::CalculateFading (SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << channelType);

  double fadingValue;

  if (!m_enableStateLock)
    {
      m_currentState = m_markovModel->GetState ();
    }

  NS_ASSERT ( (m_currentState >= 0) && (m_currentState < m_numOfStates));

  switch (channelType)
    {
    case SatEnums::RETURN_USER_CH:
    case SatEnums::FORWARD_FEEDER_CH:
      {
        m_fader_up->UpdateParameters (m_currentSet, m_currentState);

        if (m_useDecibels)
          {
            m_latestCalculatedFadingValue_up = m_fader_down->GetChannelGainDb ();
          }
        else
          {
            m_latestCalculatedFadingValue_up = m_fader_down->GetChannelGain ();
          }

        NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovContainer::CalculateFading - Calculated feeder fading value " << m_latestCalculatedFadingValue_up);

        m_latestCalculationTime_up = Now ();

        fadingValue = m_latestCalculatedFadingValue_up;
        break;
      }
    case SatEnums::FORWARD_USER_CH:
    case SatEnums::RETURN_FEEDER_CH:
      {
        m_fader_down->UpdateParameters (m_currentSet, m_currentState);

        if (m_useDecibels)
          {
            m_latestCalculatedFadingValue_down = m_fader_down->GetChannelGainDb ();
          }
        else
          {
            m_latestCalculatedFadingValue_down = m_fader_down->GetChannelGain ();
          }

        NS_LOG_INFO ("Time " << Now ().GetSeconds () << " SatMarkovContainer::CalculateFading - Calculated return fading value " << m_latestCalculatedFadingValue_down);

        m_latestCalculationTime_down = Now ();

        fadingValue = m_latestCalculatedFadingValue_down;
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatMarkovContainer::CalculateFading - Invalid channel type");
      }
    }
  return fadingValue;
}

void
SatMarkovContainer::LockToSetAndState (uint32_t newSet, uint32_t newState)
{
  NS_LOG_FUNCTION (this << newSet << " " << newState);

  if (newState >= m_numOfStates)
    {
      NS_FATAL_ERROR ("SatMarkovContainer::LockToSetAndState - Invalid state");
    }
  if (newSet >= m_numOfSets)
    {
      NS_FATAL_ERROR ("SatMarkovContainer::LockToSetAndState - Invalid set");
    }

  m_currentSet = newSet;
  m_currentState = newState;

  UpdateProbabilities (m_currentSet);

  m_enableSetLock = true;
  m_enableStateLock = true;
}

void
SatMarkovContainer::LockToSet (uint32_t newSet)
{
  NS_LOG_FUNCTION (this << newSet);

  if (newSet >= m_numOfSets)
    {
      NS_FATAL_ERROR ("SatMarkovContainer::LockToSet - Invalid set");
    }

  m_currentSet = newSet;

  UpdateProbabilities (m_currentSet);

  m_enableSetLock = true;
  m_enableStateLock = false;
}

void
SatMarkovContainer::RandomizeLockedSetAndState ()
{
  NS_LOG_FUNCTION (this);

  uint32_t newSet = 0;
  uint32_t newState = 0;

  if (m_numOfSets > 1)
    {
      newSet = (rand () % (m_numOfSets - 1));
    }

  if (m_numOfStates > 1)
    {
      newState = (rand () % (m_numOfStates - 1));
    }

  LockToSetAndState (newSet,newState);
}

void
SatMarkovContainer::RandomizeLockedState (uint32_t set)
{
  NS_LOG_FUNCTION (this);

  LockToSet (set);

  uint32_t newState = 0;

  if (m_numOfStates > 1)
    {
      newState = (rand () % (m_numOfStates - 1));
    }

  m_currentState = newState;

  m_enableStateLock = true;
}

void
SatMarkovContainer::UnlockSetAndState ()
{
  NS_LOG_FUNCTION (this);

  m_enableSetLock = false;
  m_enableStateLock = false;
}

double
SatMarkovContainer::CalculateDistanceSinceLastStateChange ()
{
  NS_LOG_FUNCTION (this);

  return (Now ().GetSeconds () - m_latestStateChangeTime.GetSeconds ()) * m_velocity ();
}

} // namespace ns3
