/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
#include "satellite-crdsa.h"

NS_LOG_COMPONENT_DEFINE ("SatCrdsa");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatCrdsa);

TypeId 
SatCrdsa::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatCrdsa")
    .SetParent<Object> ();
  return tid;
}

SatCrdsa::SatCrdsa () :
  m_randomAccessConf (),
  m_uniformVariable (),
  m_minRandomizationValue (0.0),
  m_maxRandomizationValue (0.0),
  m_numOfInstances (0),
  m_newData (true),
  m_backoffReleaseTime (0.0),
  m_backoffTime (0.0),
  m_backoffProbability (0.0),
  m_maxUniquePayloadPerBlock (0),
  m_maxConsecutiveBlocksAccessed (0),
  m_minIdleBlocks (0),
  m_numOfConsecutiveBlocksUsed (0),
  m_maxPacketSize (0)
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatCrdsa::SatCrdsa - Constructor not in use");
}

SatCrdsa::SatCrdsa (Ptr<SatRandomAccessConf> randomAccessConf) :
  m_randomAccessConf (randomAccessConf),
  m_uniformVariable (),
  m_minRandomizationValue (randomAccessConf->GetCrdsaDefaultMinRandomizationValue ()),
  m_maxRandomizationValue (randomAccessConf->GetCrdsaDefaultMaxRandomizationValue ()),
  m_numOfInstances (randomAccessConf->GetCrdsaDefaultNumOfInstances ()),
  m_newData (true),
  m_backoffReleaseTime (Now ().GetSeconds ()),
  m_backoffTime (randomAccessConf->GetCrdsaDefaultBackoffTime () / 1000),
  m_backoffProbability (randomAccessConf->GetCrdsaDefaultBackoffProbability ()),
  m_maxUniquePayloadPerBlock (randomAccessConf->GetCrdsaDefaultMaxUniquePayloadPerBlock ()),
  m_maxConsecutiveBlocksAccessed (randomAccessConf->GetCrdsaDefaultMaxConsecutiveBlocksAccessed()),
  m_minIdleBlocks (randomAccessConf->GetCrdsaDefaultMinIdleBlocks ()),
  m_numOfConsecutiveBlocksUsed (0),
  m_maxPacketSize (randomAccessConf->GetCrdsaDefaultMaxUniquePayloadPerBlock () * randomAccessConf->GetCrdsaDefaultPayloadBytes ())
{
  NS_LOG_FUNCTION (this);

  m_uniformVariable = CreateObject<UniformRandomVariable> ();

  DoVariableSanityCheck ();

  NS_LOG_INFO ("SatCrdsa::SatCrdsa - Module created");
}

SatCrdsa::~SatCrdsa ()
{
  NS_LOG_FUNCTION (this);
}

void
SatCrdsa::DoVariableSanityCheck ()
{
  NS_LOG_FUNCTION (this);

  if (m_minRandomizationValue < 0 || m_maxRandomizationValue < 0)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - min < 0 || max < 0");
    }

  if (m_minRandomizationValue > m_maxRandomizationValue)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - min > max");
    }

  /// TODO check this
  if (m_numOfInstances < 1 || m_numOfInstances > 3)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - instances < 1 || instances > 3");
    }

  if ( (m_maxRandomizationValue - m_minRandomizationValue) < m_numOfInstances)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - (max - min) < instances");
    }

  if (m_backoffTime < 0)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - m_backoffTime < 0");
    }

  if (m_backoffProbability < 0 || m_backoffProbability > 1.0)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - m_backoffProbability < 0.0 || m_backoffProbability > 1.0");
    }

  if (m_maxUniquePayloadPerBlock < 1)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - m_maxUniquePayloadPerBlock < 1");
    }

  if (m_maxConsecutiveBlocksAccessed < 1)
    {
      NS_FATAL_ERROR ("SatCrdsa::DoVariableSanityCheck - m_maxConsecutiveBlocksAccessed < 1");
    }

  NS_LOG_INFO ("SatCrdsa::DoVariableSanityCheck - Variable sanity check done");
}

void
SatCrdsa::UpdateRandomizationVariables (uint32_t min, uint32_t max, uint32_t numOfInstances, uint32_t maxUniquePayloadPerBlock)
{
  NS_LOG_FUNCTION (this << " min: " << min << " max: " << max << " numOfInstances: " << numOfInstances << " maxUniquePayloadPerBlock: " << maxUniquePayloadPerBlock);

  m_minRandomizationValue = min;
  m_maxRandomizationValue = max;
  m_numOfInstances = numOfInstances;
  m_maxUniquePayloadPerBlock = maxUniquePayloadPerBlock;

  DoVariableSanityCheck ();

  NS_LOG_INFO ("SatCrdsa::UpdateVariables - min: " << min << " max: " << max << " numOfInstances: " << numOfInstances << " maxUniquePayloadPerBlock: " << maxUniquePayloadPerBlock);
}

void
SatCrdsa::SetBackoffProbability (double backoffProbability)
{
  NS_LOG_FUNCTION (this);

  m_backoffProbability = backoffProbability;
}

void
SatCrdsa::SetBackoffTime (double backoffTime)
{
  NS_LOG_FUNCTION (this);

  m_backoffTime = backoffTime;
}

bool
SatCrdsa::HasBackoffTimePassed ()
{
  NS_LOG_FUNCTION (this);

  bool hasBackoffTimePassed = false;

  if ((Now ().GetSeconds () >= m_backoffReleaseTime) && (m_idleBlocksLeft < 1))
    {
      hasBackoffTimePassed = true;
    }
  else
    {
      UpdateIdleBlocks ();
    }

  NS_LOG_INFO ("SatCrdsa::HasBackoffTimePassed: " << hasBackoffTimePassed);

  return hasBackoffTimePassed;
}

void
SatCrdsa::UpdateIdleBlocks ()
{
  NS_LOG_FUNCTION (this);

  if (m_idleBlocksLeft > 0)
    {
      NS_LOG_INFO ("SatCrdsa::UpdateIdleBlocks - Reducing idle blocks by one");
      m_idleBlocksLeft--;
    }
}

bool
SatCrdsa::DoBackoff ()
{
  NS_LOG_FUNCTION (this);

  bool doBackoff = false;

  if (m_uniformVariable->GetValue (0.0,1.0) < m_backoffProbability)
    {
      doBackoff = true;
    }

  NS_LOG_INFO ("SatCrdsa::DoBackoff: " << doBackoff);

  return doBackoff;
}

/// TODO: implement this
bool
SatCrdsa::IsDamaAvailable ()
{
  NS_LOG_FUNCTION (this);

  bool isDamaAvailable = false;

  NS_LOG_INFO ("SatCrdsa::IsDamaAvailable: " << isDamaAvailable);

  return isDamaAvailable;
}

/// TODO: implement this
bool
SatCrdsa::AreBuffersEmpty ()
{
  NS_LOG_FUNCTION (this);

  bool areBuffersEmpty = true;

  NS_LOG_INFO ("SatCrdsa::AreBuffersEmpty: " << areBuffersEmpty);

  return areBuffersEmpty;
}

void
SatCrdsa::SetInitialBackoffTimer ()
{
  NS_LOG_FUNCTION (this);

  m_backoffReleaseTime = Now ().GetSeconds () + m_backoffTime;

  UpdateIdleBlocks ();

  m_numOfConsecutiveBlocksUsed = 0;

  NS_LOG_INFO ("SatCrdsa::SetBackoffTimer - Setting backoff timer");
}

std::set<uint32_t>
SatCrdsa::PrepareToTransmit ()
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t> txOpportunities;

  NS_LOG_INFO ("SatCrdsa::PrepareToTransmit - Preparing for transmission...");

  txOpportunities = RandomizeTxOpportunities ();

  if (AreBuffersEmpty ())
    {
      m_newData = true;
    }

  IncreaseConsecutiveBlocksUsed ();

  return txOpportunities;
}

void
SatCrdsa::IncreaseConsecutiveBlocksUsed ()
{
  NS_LOG_FUNCTION (this);

  m_numOfConsecutiveBlocksUsed++;

  NS_LOG_INFO ("SatCrdsa::IncreaseConsecutiveBlocksUsed - Increasing the number of used consecutive blocks");

  if (m_numOfConsecutiveBlocksUsed >= m_maxConsecutiveBlocksAccessed)
    {
      NS_LOG_INFO ("SatCrdsa::IncreaseConsecutiveBlocksUsed - Maximum number of consecutive blocks reached, forcing idle blocks");
      m_idleBlocksLeft = m_minIdleBlocks;
      m_numOfConsecutiveBlocksUsed = 0;
    }
}

std::set<uint32_t>
SatCrdsa::DoCrdsa ()
{
  NS_LOG_FUNCTION (this);

  /// TODO: what to return in the case CRDSA is not used?
  std::set<uint32_t> txOpportunities;

  NS_LOG_INFO ("-------------------------------------");
  NS_LOG_INFO ("------ Running CRDSA algorithm ------");
  NS_LOG_INFO ("-------------------------------------");

  PrintVariables ();

  NS_LOG_INFO ("-------------------------------------");

  NS_LOG_INFO ("SatCrdsa::DoCrdsa - Checking backoff period status...");

  if (HasBackoffTimePassed ())
    {
      NS_LOG_INFO ("SatCrdsa::DoCrdsa - Backoff period over, checking DAMA status...");

      if (!IsDamaAvailable ())
        {
          NS_LOG_INFO ("SatCrdsa::DoCrdsa - No DAMA, checking buffer status...");

          if (m_newData)
            {
              m_newData = false;

              NS_LOG_INFO ("SatCrdsa::DoCrdsa - Evaluating backoff...");

              if (DoBackoff ())
                {
                  SetInitialBackoffTimer ();
                }
              else
                {
                  txOpportunities = PrepareToTransmit ();
                }
            }
          else
            {
              txOpportunities = PrepareToTransmit ();
            }
        }
      else
        {
          UpdateIdleBlocks ();
        }
    }

  NS_LOG_INFO ("SatCrdsa::DoCrdsa - Algorithm finished.");

  return txOpportunities;
}


std::set<uint32_t>
SatCrdsa::RandomizeTxOpportunities ()
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t> txOpportunities;
  std::pair<std::set<uint32_t>::iterator,bool> result;

  NS_LOG_INFO ("SatCrdsa::RandomizeTxOpportunities - Randomizing TX opportunities");

  while (txOpportunities.size () < (m_numOfInstances * m_maxUniquePayloadPerBlock))
    {
      uint32_t slot = m_uniformVariable->GetInteger (m_minRandomizationValue, m_maxRandomizationValue);

      result = txOpportunities.insert (slot);

      NS_LOG_INFO ("SatCrdsa::RandomizeTxOpportunities - Insert successful: " << result.second << " for TX opportunity slot: " << (*result.first));
    }

  NS_LOG_INFO ("SatCrdsa::RandomizeTxOpportunities - Randomizing done");

  return txOpportunities;
}

void
SatCrdsa::PrintVariables ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Simulation time: " << Now ().GetSeconds () << " seconds");
  NS_LOG_INFO ("Backoff release time: " << m_backoffReleaseTime << " seconds");
  NS_LOG_INFO ("Backoff time: " << m_backoffTime << " seconds");
  NS_LOG_INFO ("Backoff probability: " << m_backoffProbability * 100 << " %");
  NS_LOG_INFO ("New data status: " << m_newData);
  NS_LOG_INFO ("Slot randomization: " << m_numOfInstances * m_maxUniquePayloadPerBlock << " Tx opportunities with range from "<< m_minRandomizationValue << " to " << m_maxRandomizationValue);
  NS_LOG_INFO ("Number of instances: " << m_numOfInstances);
  NS_LOG_INFO ("Number of unique payloads per block: " << m_maxUniquePayloadPerBlock);
  NS_LOG_INFO ("Number of consecutive blocks accessed: " << m_numOfConsecutiveBlocksUsed << "/" << m_maxConsecutiveBlocksAccessed);
  NS_LOG_INFO ("Number of idle blocks left: " << m_idleBlocksLeft << "/" << m_minIdleBlocks);
}

} // namespace ns3
