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
#include "satellite-random-access-container.h"
#include <set>

NS_LOG_COMPONENT_DEFINE ("SatRandomAccess");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRandomAccess);

TypeId 
SatRandomAccess::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomAccess")
    .SetParent<Object> ();
  return tid;
}

SatRandomAccess::SatRandomAccess () :
  m_uniformVariable (),
  m_randomAccessModel (RA_OFF),
  m_randomAccessConf (),

  /// Slotted ALOHA
  m_slottedAlohaMinRandomizationValue (),
  m_slottedAlohaMaxRandomizationValue (),

  /// CRDSA
  m_crdsaBackoffProbability (),
  m_crdsaMaximumBackoffProbability (),
  m_crdsaMinRandomizationValue (),
  m_crdsaMaxRandomizationValue (),
  m_crdsaNumOfInstances (),
  m_crdsaNewData (),
  m_crdsaBackoffReleaseTime (),
  m_crdsaBackoffTime (),
  m_crdsaMaxUniquePayloadPerBlock (),
  m_crdsaMaxConsecutiveBlocksAccessed (),
  m_crdsaMinIdleBlocks (),
  m_crdsaIdleBlocksLeft (),
  m_crdsaNumOfConsecutiveBlocksUsed (),
  m_crdsaMaxPacketSize ()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Constructor not in use");
}

SatRandomAccess::SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, RandomAccessModel_t randomAccessModel) :
  m_uniformVariable (),
  m_randomAccessModel (randomAccessModel),
  m_randomAccessConf (randomAccessConf),

  /// Slotted ALOHA variables
  m_slottedAlohaMinRandomizationValue (randomAccessConf->GetSlottedAlohaDefaultMinRandomizationValue ()),
  m_slottedAlohaMaxRandomizationValue (randomAccessConf->GetSlottedAlohaDefaultMaxRandomizationValue ()),

  /// CRDSA variables
  m_crdsaBackoffProbability (randomAccessConf->GetCrdsaDefaultBackoffProbability ()),
  m_crdsaMaximumBackoffProbability (randomAccessConf->GetMaximumCrdsaBackoffProbability ()),
  m_crdsaMinRandomizationValue (randomAccessConf->GetCrdsaDefaultMinRandomizationValue ()),
  m_crdsaMaxRandomizationValue (randomAccessConf->GetCrdsaDefaultMaxRandomizationValue ()),
  m_crdsaNumOfInstances (randomAccessConf->GetCrdsaDefaultNumOfInstances ()),
  m_crdsaNewData (true),
  m_crdsaBackoffReleaseTime (Now ().GetSeconds ()),
  m_crdsaBackoffTime (randomAccessConf->GetCrdsaDefaultBackoffTime () / 1000),
  m_crdsaMaxUniquePayloadPerBlock (randomAccessConf->GetCrdsaDefaultMaxUniquePayloadPerBlock ()),
  m_crdsaMaxConsecutiveBlocksAccessed (randomAccessConf->GetCrdsaDefaultMaxConsecutiveBlocksAccessed()),
  m_crdsaMinIdleBlocks (randomAccessConf->GetCrdsaDefaultMinIdleBlocks ()),
  m_crdsaIdleBlocksLeft (0),
  m_crdsaNumOfConsecutiveBlocksUsed (0),
  m_crdsaMaxPacketSize (randomAccessConf->GetCrdsaDefaultMaxUniquePayloadPerBlock () * randomAccessConf->GetCrdsaDefaultPayloadBytes ())
{
  NS_LOG_FUNCTION (this);

  m_uniformVariable = CreateObject<UniformRandomVariable> ();

  if (m_randomAccessConf == NULL)
    {
      NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Configuration object is NULL");
    }
  SetRandomAccessModel (randomAccessModel);
}

SatRandomAccess::~SatRandomAccess ()
{
  NS_LOG_FUNCTION (this);
}

///---------------------------------------
/// General random access related methods
///---------------------------------------

/// TODO: implement this
bool
SatRandomAccess::IsFrameStart ()
{
  NS_LOG_FUNCTION (this);

  bool isFrameStart = m_uniformVariable->GetInteger (0,1);

  NS_LOG_INFO ("SatRandomAccess::IsFrameStart: " << isFrameStart);

  return isFrameStart;
}

/// TODO: implement this
bool
SatRandomAccess::IsDamaAvailable ()
{
  NS_LOG_FUNCTION (this);

  bool isDamaAvailable = m_uniformVariable->GetInteger (0,1);

  NS_LOG_INFO ("SatRandomAccess::IsDamaAvailable: " << isDamaAvailable);

  return isDamaAvailable;
}

/// TODO: implement this
bool
SatRandomAccess::AreBuffersEmpty ()
{
  NS_LOG_FUNCTION (this);

  bool areBuffersEmpty = m_uniformVariable->GetInteger (0,1);;

  NS_LOG_INFO ("SatRandomAccess::AreBuffersEmpty: " << areBuffersEmpty);

  return areBuffersEmpty;
}

bool
SatRandomAccess::IsCrdsaFree ()
{
  NS_LOG_FUNCTION (this);

  bool isCrdsaFree = false;

  if ((Now ().GetSeconds () >= m_crdsaBackoffReleaseTime) && (m_crdsaIdleBlocksLeft < 1))
    {
      isCrdsaFree = true;
    }

  NS_LOG_INFO ("SatRandomAccess::IsCrdsaFree: " << isCrdsaFree);

  return isCrdsaFree;
}

void
SatRandomAccess::SetRandomAccessModel (RandomAccessModel_t randomAccessModel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Setting Random Access model to: " << randomAccessModel);

  if (randomAccessModel == RA_CRDSA || randomAccessModel == RA_ANY_AVAILABLE)
    {
      NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - CRDSA model in use");

      CrdsaDoVariableSanityCheck ();
    }

  if (randomAccessModel == RA_SLOTTED_ALOHA || randomAccessModel == RA_ANY_AVAILABLE)
    {
      NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Slotted ALOHA model in use");

      SlottedAlohaDoVariableSanityCheck ();
    }

  m_randomAccessModel = randomAccessModel;

  NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Random Access model updated");
}

/// TODO implement return values!
SatRandomAccess::RandomAccessResults_s
SatRandomAccess::DoRandomAccess ()
{
  NS_LOG_FUNCTION (this);

  /// return variable initialization
  std::set<uint32_t> txOpportunities;
  double time = 0.0;
  RandomAccessResults_s results;

  NS_LOG_INFO ("------------------------------------");
  NS_LOG_INFO ("------ Starting Random Access ------");
  NS_LOG_INFO ("------------------------------------");

  /// Do CRDSA
  if (m_randomAccessModel == RA_CRDSA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Only CRDSA enabled, check frame start");

      if (IsFrameStart ())
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - At the frame start, evaluating CRDSA");

          txOpportunities = DoCrdsa ();

          results.resultType = RA_CRDSA_RESULT;
          results.crdsaResult = txOpportunities;
        }
    }
  /// Do Slotted ALOHA
  else if (m_randomAccessModel == RA_SLOTTED_ALOHA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Only SA enabled, evaluating Slotted ALOHA");

      time = DoSlottedAloha ();

      results.resultType = RA_SLOTTED_ALOHA_RESULT;
      results.slottedAlohaResult = time;
    }
  /// Frame start is a known trigger for CRDSA, which has higher priority than SA.
  /// As such SA will not be used at frame start unless CRDSA backoff probability is higher than the parameterized value
  else if (m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - All RA models enabled");

      if (!IsFrameStart ())
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Not at frame start, evaluating Slotted ALOHA");
          time = DoSlottedAloha ();

          results.resultType = RA_SLOTTED_ALOHA_RESULT;
          results.slottedAlohaResult = time;
        }
      else
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - At frame start, checking CRDSA backoff & backoff probability");

          if ((m_crdsaBackoffProbability < m_crdsaMaximumBackoffProbability) && IsCrdsaFree ())
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Low CRDSA backoff value AND CRDSA is free, evaluating CRDSA");
              txOpportunities = DoCrdsa ();

              results.resultType = RA_CRDSA_RESULT;
              results.crdsaResult = txOpportunities;
            }
          else
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - High CRDSA backoff value OR CRDSA is not free, evaluating Slotted ALOHA");
              time = DoSlottedAloha ();

              results.resultType = RA_SLOTTED_ALOHA_RESULT;
              results.slottedAlohaResult = time;

              CrdsaUpdateIdleBlocks ();
            }
        }
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid random access model");
    }

  /// For debugging purposes
  /// TODO: comment out this code at later stage
  std::set<uint32_t>::iterator iter;
  for (iter = txOpportunities.begin (); iter != txOpportunities.end (); iter++ )
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA transmission opportunity at slot: " << (*iter));
    }
  NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - SA minimum time to wait: " << time << " seconds");

  NS_LOG_INFO ("------------------------------------");
  NS_LOG_INFO ("------ Random Access FINISHED ------");
  NS_LOG_INFO ("------------------------------------");

  return results;
}

void
SatRandomAccess::PrintVariables ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Simulation time: " << Now ().GetSeconds () << " seconds");
  NS_LOG_INFO ("Backoff release time: " << m_crdsaBackoffReleaseTime << " seconds");
  NS_LOG_INFO ("Backoff time: " << m_crdsaBackoffTime << " seconds");
  NS_LOG_INFO ("Backoff probability: " << m_crdsaBackoffProbability * 100 << " %");
  NS_LOG_INFO ("New data status: " << m_crdsaNewData);
  NS_LOG_INFO ("Slot randomization: " << m_crdsaNumOfInstances * m_crdsaMaxUniquePayloadPerBlock << " Tx opportunities with range from "<< m_crdsaMinRandomizationValue << " to " << m_crdsaMaxRandomizationValue);
  NS_LOG_INFO ("Number of instances: " << m_crdsaNumOfInstances);
  NS_LOG_INFO ("Number of unique payloads per block: " << m_crdsaMaxUniquePayloadPerBlock);
  NS_LOG_INFO ("Number of consecutive blocks accessed: " << m_crdsaNumOfConsecutiveBlocksUsed << "/" << m_crdsaMaxConsecutiveBlocksAccessed);
  NS_LOG_INFO ("Number of idle blocks left: " << m_crdsaIdleBlocksLeft << "/" << m_crdsaMinIdleBlocks);
}

///-------------------------------
/// Slotted ALOHA related methods
///-------------------------------

void
SatRandomAccess::SlottedAlohaDoVariableSanityCheck ()
{
  NS_LOG_FUNCTION (this);

  if (m_slottedAlohaMinRandomizationValue < 0 || m_slottedAlohaMaxRandomizationValue < 0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::SlottedAlohaDoVariableSanityCheck - min < 0 || max < 0");
    }

  if (m_slottedAlohaMinRandomizationValue > m_slottedAlohaMaxRandomizationValue)
    {
      NS_FATAL_ERROR ("SatRandomAccess::SlottedAlohaDoVariableSanityCheck - min > max");
    }

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaDoVariableSanityCheck - Variable sanity check done");
}

void
SatRandomAccess::SlottedAlohaUpdateVariables (double min, double max)
{
  NS_LOG_FUNCTION (this << " new min: " << min << " new max: " << max);

  m_slottedAlohaMinRandomizationValue = min;
  m_slottedAlohaMaxRandomizationValue = max;

  SlottedAlohaDoVariableSanityCheck ();

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaUpdateVariables - new min: " << min << " new max: " << max);
}

double
SatRandomAccess::DoSlottedAloha ()
{
  NS_LOG_FUNCTION (this);

  /// TODO: what to return in the case SA is not used, e.g., DAMA is available?
  double time = 0;

  NS_LOG_INFO ("---------------------------------------------");
  NS_LOG_INFO ("------ Running Slotted ALOHA algorithm ------");
  NS_LOG_INFO ("---------------------------------------------");

  NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - Checking if we have DAMA allocations");

  /// Check if we have known DAMA allocations
  if (!IsDamaAvailable ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - No DAMA -> Running Slotted ALOHA");

      /// Randomize Tx opportunity release time
      time = SlottedAlohaRandomizeReleaseTime ();
    }

  NS_LOG_INFO ("----------------------------------------------");
  NS_LOG_INFO ("------ Slotted ALOHA algorithm FINISHED ------");
  NS_LOG_INFO ("----------------------------------------------");

  return time;
}

double
SatRandomAccess::SlottedAlohaRandomizeReleaseTime ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaRandomizeReleaseTime - Randomizing the release time...");

  double releaseTime = Now ().GetSeconds () + m_uniformVariable->GetValue (m_slottedAlohaMinRandomizationValue, m_slottedAlohaMaxRandomizationValue);

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaRandomizeReleaseTime - TX opportunity in the next slot after the release time at: " << releaseTime << " seconds");

  return releaseTime;
}

///-----------------------
/// CRDSA related methods
///-----------------------

void
SatRandomAccess::CrdsaDoVariableSanityCheck ()
{
  NS_LOG_FUNCTION (this);

  if (m_crdsaMinRandomizationValue < 0 || m_crdsaMaxRandomizationValue < 0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - min < 0 || max < 0");
    }

  if (m_crdsaMinRandomizationValue > m_crdsaMaxRandomizationValue)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - min > max");
    }

  /// TODO check this
  if (m_crdsaNumOfInstances < 1 || m_crdsaNumOfInstances > 3)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - instances < 1 || instances > 3");
    }

  if ( (m_crdsaMaxRandomizationValue - m_crdsaMinRandomizationValue) < m_crdsaNumOfInstances)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - (max - min) < instances");
    }

  if (m_crdsaBackoffTime < 0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaBackoffTime < 0");
    }

  if (m_crdsaBackoffProbability < 0 || m_crdsaBackoffProbability > 1.0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaBackoffProbability < 0.0 || m_crdsaBackoffProbability > 1.0");
    }

  if (m_crdsaMaxUniquePayloadPerBlock < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaMaxUniquePayloadPerBlock < 1");
    }

  if (m_crdsaMaxConsecutiveBlocksAccessed < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaMaxConsecutiveBlocksAccessed < 1");
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaDoVariableSanityCheck - Variable sanity check done");
}

void
SatRandomAccess::CrdsaUpdateRandomizationVariables (uint32_t min, uint32_t max, uint32_t numOfInstances, uint32_t maxUniquePayloadPerBlock)
{
  NS_LOG_FUNCTION (this << " min: " << min << " max: " << max << " numOfInstances: " << numOfInstances << " maxUniquePayloadPerBlock: " << maxUniquePayloadPerBlock);

  m_crdsaMinRandomizationValue = min;
  m_crdsaMaxRandomizationValue = max;
  m_crdsaNumOfInstances = numOfInstances;
  m_crdsaMaxUniquePayloadPerBlock = maxUniquePayloadPerBlock;

  CrdsaDoVariableSanityCheck ();

  NS_LOG_INFO ("SatRandomAccess::CrdsaUpdateRandomizationVariables - min: " << min << " max: " << max << " numOfInstances: " << numOfInstances << " maxUniquePayloadPerBlock: " << maxUniquePayloadPerBlock);
}

void
SatRandomAccess::CrdsaSetBackoffProbability (double crdsaBackoffProbability)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == RA_CRDSA || m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      m_crdsaBackoffProbability = crdsaBackoffProbability;
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaSetBackoffProbability - Wrong random access model in use");
    }
}

void
SatRandomAccess::CrdsaSetBackoffTime (double backoffTime)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == RA_CRDSA || m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      m_crdsaBackoffTime = backoffTime;
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaSetBackoffTime - Wrong random access model in use");
    }
}

bool
SatRandomAccess::CrdsaHasBackoffTimePassed ()
{
  NS_LOG_FUNCTION (this);

  bool hasCrdsaBackoffTimePassed = false;

  if (IsCrdsaFree ())
    {
      hasCrdsaBackoffTimePassed = true;
    }
  else
    {
      CrdsaUpdateIdleBlocks ();
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaHasBackoffTimePassed: " << hasCrdsaBackoffTimePassed);

  return hasCrdsaBackoffTimePassed;
}

void
SatRandomAccess::CrdsaUpdateIdleBlocks ()
{
  NS_LOG_FUNCTION (this);

  if (m_crdsaIdleBlocksLeft > 0)
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaUpdateIdleBlocks - Reducing idle blocks by one");
      m_crdsaIdleBlocksLeft--;
    }
}

bool
SatRandomAccess::CrdsaDoBackoff ()
{
  NS_LOG_FUNCTION (this);

  bool doCrdsaBackoff = false;

  if (m_uniformVariable->GetValue (0.0,1.0) < m_crdsaBackoffProbability)
    {
      doCrdsaBackoff = true;
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaDoBackoff: " << doCrdsaBackoff);

  return doCrdsaBackoff;
}


void
SatRandomAccess::CrdsaSetInitialBackoffTimer ()
{
  NS_LOG_FUNCTION (this);

  m_crdsaBackoffReleaseTime = Now ().GetSeconds () + m_crdsaBackoffTime;

  CrdsaUpdateIdleBlocks ();

  m_crdsaNumOfConsecutiveBlocksUsed = 0;

  NS_LOG_INFO ("SatRandomAccess::CrdsaSetInitialBackoffTimer - Setting backoff timer");
}

std::set<uint32_t>
SatRandomAccess::CrdsaPrepareToTransmit ()
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t> txOpportunities;

  NS_LOG_INFO ("SatRandomAccess::CrdsaPrepareToTransmit - Preparing for transmission...");

  txOpportunities = CrdsaRandomizeTxOpportunities ();

  if (AreBuffersEmpty ())
    {
      m_crdsaNewData = true;
    }

  CrdsaIncreaseConsecutiveBlocksUsed ();

  return txOpportunities;
}

void
SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed ()
{
  NS_LOG_FUNCTION (this);

  m_crdsaNumOfConsecutiveBlocksUsed++;

  NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed - Increasing the number of used consecutive blocks");

  if (m_crdsaNumOfConsecutiveBlocksUsed >= m_crdsaMaxConsecutiveBlocksAccessed)
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed - Maximum number of consecutive blocks reached, forcing idle blocks");
      m_crdsaIdleBlocksLeft = m_crdsaMinIdleBlocks;
      m_crdsaNumOfConsecutiveBlocksUsed = 0;
    }
}

std::set<uint32_t>
SatRandomAccess::DoCrdsa ()
{
  NS_LOG_FUNCTION (this);

  /// TODO: what to return in the case CRDSA is not used?
  std::set<uint32_t> txOpportunities;

  NS_LOG_INFO ("-------------------------------------");
  NS_LOG_INFO ("------ Running CRDSA algorithm ------");
  NS_LOG_INFO ("-------------------------------------");

  PrintVariables ();

  NS_LOG_INFO ("-------------------------------------");

  NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Checking backoff period status...");

  if (CrdsaHasBackoffTimePassed ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Backoff period over, checking DAMA status...");

      if (!IsDamaAvailable ())
        {
          NS_LOG_INFO ("SatRandomAccess::DoCrdsa - No DAMA, checking buffer status...");

          if (m_crdsaNewData)
            {
              m_crdsaNewData = false;

              NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Evaluating backoff...");

              if (CrdsaDoBackoff ())
                {
                  CrdsaSetInitialBackoffTimer ();
                }
              else
                {
                  txOpportunities = CrdsaPrepareToTransmit ();
                }
            }
          else
            {
              txOpportunities = CrdsaPrepareToTransmit ();
            }
        }
      else
        {
          CrdsaUpdateIdleBlocks ();
        }
    }

  NS_LOG_INFO ("--------------------------------------");
  NS_LOG_INFO ("------ CRDSA algorithm FINISHED ------");
  NS_LOG_INFO ("--------------------------------------");

  return txOpportunities;
}


std::set<uint32_t>
SatRandomAccess::CrdsaRandomizeTxOpportunities ()
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t> txOpportunities;
  std::pair<std::set<uint32_t>::iterator,bool> result;

  NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Randomizing TX opportunities");

  while (txOpportunities.size () < (m_crdsaNumOfInstances * m_crdsaMaxUniquePayloadPerBlock))
    {
      uint32_t slot = m_uniformVariable->GetInteger (m_crdsaMinRandomizationValue, m_crdsaMaxRandomizationValue);

      result = txOpportunities.insert (slot);

      NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Insert successful: " << result.second << " for TX opportunity slot: " << (*result.first));
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Randomizing done");

  return txOpportunities;
}

} // namespace ns3
