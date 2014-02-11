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
  m_uniformRandomVariable (),
  m_randomAccessModel (RA_OFF),
  m_randomAccessConf (),
  m_numOfRequestClasses (),

  /// Slotted ALOHA variables
  m_slottedAlohaMinRandomizationValue (),
  m_slottedAlohaMaxRandomizationValue (),

  /// CRDSA variables
  m_crdsaBackoffProbability (),
  m_crdsaMaximumBackoffProbability (),
  m_crdsaNewData (true),
  m_crdsaBackoffReleaseTime (),
  m_crdsaBackoffTime ()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Constructor not in use");
}

SatRandomAccess::SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, RandomAccessModel_t randomAccessModel, uint32_t numOfRequestClasses) :
  m_uniformRandomVariable (),
  m_randomAccessModel (randomAccessModel),
  m_randomAccessConf (randomAccessConf),
  m_numOfRequestClasses (numOfRequestClasses),

  /// Slotted ALOHA variables
  m_slottedAlohaMinRandomizationValue (),
  m_slottedAlohaMaxRandomizationValue (),

  /// CRDSA variables
  m_crdsaBackoffProbability (),
  m_crdsaMaximumBackoffProbability (),
  m_crdsaNewData (true),
  m_crdsaBackoffReleaseTime (Now ().GetSeconds ()),
  m_crdsaBackoffTime ()
{
  NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();

  if (m_randomAccessConf == NULL)
    {
      NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Configuration object is NULL");
    }

  SetRandomAccessModel (randomAccessModel);

  CrdsaSetLoadControlParameters (randomAccessConf->GetCrdsaDefaultBackoffProbability (),
                                 randomAccessConf->GetCrdsaDefaultBackoffTime () / 1000,
                                 randomAccessConf->GetMaximumCrdsaBackoffProbability ());

  SlottedAlohaSetLoadControlVariables (randomAccessConf->GetSlottedAlohaDefaultMinRandomizationValue (),
                                       randomAccessConf->GetSlottedAlohaDefaultMaxRandomizationValue ());
}

SatRandomAccess::~SatRandomAccess ()
{
  NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = NULL;
  m_randomAccessConf = NULL;
}

///---------------------------------------
/// General random access related methods
///---------------------------------------

/// TODO: implement this
bool
SatRandomAccess::IsFrameStart ()
{
  NS_LOG_FUNCTION (this);

  bool isFrameStart = true;

  NS_LOG_INFO ("SatRandomAccess::IsFrameStart: " << isFrameStart);

  return isFrameStart;
}

/*
The known DAMA capacity condition is different for control and data.
For control the known DAMA is limited to the SF about to start, i.e.,
the look ahead is one SF. For data the known DAMA allocation can be
one or more SF in the future, i.e., the look ahead contains all known
future DAMA allocations. With CRDSA the control packets have priority
over data packets.
*/
/// TODO: implement this
bool
SatRandomAccess::IsDamaAvailable ()
{
  NS_LOG_FUNCTION (this);

  bool isDamaAvailable = false;

  NS_LOG_INFO ("SatRandomAccess::IsDamaAvailable: " << isDamaAvailable);

  return isDamaAvailable;
}

/// TODO: implement this
bool
SatRandomAccess::AreBuffersEmpty ()
{
  NS_LOG_FUNCTION (this);

  bool areBuffersEmpty = m_uniformRandomVariable->GetInteger (0,1);

  NS_LOG_INFO ("SatRandomAccess::AreBuffersEmpty: " << areBuffersEmpty);

  return areBuffersEmpty;
}

bool
SatRandomAccess::IsCrdsaFree ()
{
  NS_LOG_FUNCTION (this);

  bool isCrdsaFree = false;

  if ((Now ().GetSeconds () >= m_crdsaBackoffReleaseTime))
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

SatRandomAccess::RandomAccessResults_s
SatRandomAccess::DoRandomAccess ()
{
  NS_LOG_FUNCTION (this);

  /// return variable initialization
  RandomAccessResults_s results;

  results.resultType = SatRandomAccess::RA_DO_NOTHING;

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

          results = DoCrdsa ();
        }
    }
  /// Do Slotted ALOHA
  else if (m_randomAccessModel == RA_SLOTTED_ALOHA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Only SA enabled, evaluating Slotted ALOHA");

      results = DoSlottedAloha ();
    }
  /// Frame start is a known trigger for CRDSA, which has higher priority than SA.
  /// As such SA will not be used at frame start unless:
  /// 1) CRDSA back off probability is higher than the parameterized value
  /// 2) CRDSA back off is in effect
  else if (m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - All RA models enabled");

      if (!IsFrameStart ())
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Not at frame start, evaluating Slotted ALOHA");
          results = DoSlottedAloha ();
        }
      else
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - At frame start, checking CRDSA backoff & backoff probability");

          if ( IsCrdsaFree () && (m_crdsaBackoffProbability < m_crdsaMaximumBackoffProbability))
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Low CRDSA backoff value AND CRDSA is free, evaluating CRDSA");
              results = DoCrdsa ();
            }
          else
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - High CRDSA backoff value OR CRDSA is not free, evaluating Slotted ALOHA");
              results = DoSlottedAloha ();

              CrdsaReduceIdleBlocksFromAllRequestClasses ();
            }
        }
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid random access model");
    }

  /// For debugging purposes
  /// TODO: comment out this code at later stage
  if (results.resultType == SatRandomAccess::RA_CRDSA_TX_OPPORTUNITY)
    {
      std::map<uint32_t, std::set<uint32_t> >::iterator iterMap;
      for (iterMap = results.crdsaResult.begin (); iterMap != results.crdsaResult.end (); iterMap++ )
        {
          std::set<uint32_t>::iterator iterSet;
          for (iterSet = iterMap->second.begin(); iterSet != iterMap->second.end(); iterSet++)
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA transmission opportunity for request class: " << iterMap->first << " at slot: " << (*iterSet));
            }
        }
    }
  else if (results.resultType == SatRandomAccess::RA_SLOTTED_ALOHA_TX_OPPORTUNITY)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - SA minimum time to wait: " << results.slottedAlohaResult << " seconds");
    }
  else if (results.resultType == SatRandomAccess::RA_DO_NOTHING)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - No Tx opportunity");
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid result type");
    }

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
  NS_LOG_INFO ("Number of unique payloads per block: " << m_randomAccessConf->GetMaxUniquePayloadPerBlock ());
  NS_LOG_INFO ("Number of consecutive blocks accessed: " << m_randomAccessConf->GetNumOfConsecutiveBlocksUsed () << "/" << m_randomAccessConf->GetMaxConsecutiveBlocksAccessed ());

  for (uint32_t index = 0; index < m_numOfRequestClasses; index++)
    {
      NS_LOG_INFO ("---------------");
      NS_LOG_INFO ("REQUEST CLASS: " << index);
      NS_LOG_INFO ("Slot randomization: " << m_randomAccessConf->GetRequestClassConfiguration (index)->GetNumOfInstances () * m_randomAccessConf->GetMaxUniquePayloadPerBlock () <<
                   " Tx opportunities with range from " << m_randomAccessConf->GetRequestClassConfiguration (index)->GetMinRandomizationValue () <<
                   " to " << m_randomAccessConf->GetRequestClassConfiguration (index)->GetMaxRandomizationValue ());
      NS_LOG_INFO ("Number of instances: " << m_randomAccessConf->GetRequestClassConfiguration (index)->GetNumOfInstances ());
      NS_LOG_INFO ("Number of idle blocks left: " << m_randomAccessConf->GetRequestClassConfiguration (index)->GetIdleBlocksLeft () << "/" << m_randomAccessConf->GetRequestClassConfiguration (index)->GetMinIdleBlocks ());
    }
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
SatRandomAccess::SlottedAlohaSetLoadControlVariables (double min, double max)
{
  NS_LOG_FUNCTION (this << " new min: " << min << " new max: " << max);

  if (m_randomAccessModel == RA_SLOTTED_ALOHA || m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      m_slottedAlohaMinRandomizationValue = min;
      m_slottedAlohaMaxRandomizationValue = max;
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::SlottedAlohaSetLoadControlVariables - Wrong random access model in use");
    }

  SlottedAlohaDoVariableSanityCheck ();

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaSetLoadControlVariables - new min: " << min << " new max: " << max);
}

SatRandomAccess::RandomAccessResults_s
SatRandomAccess::DoSlottedAloha ()
{
  NS_LOG_FUNCTION (this);

  RandomAccessResults_s results;
  results.resultType = SatRandomAccess::RA_DO_NOTHING;

  NS_LOG_INFO ("---------------------------------------------");
  NS_LOG_INFO ("------ Running Slotted ALOHA algorithm ------");
  NS_LOG_INFO ("---------------------------------------------");

  NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - Checking if we have DAMA allocations");

  /// Check if we have known DAMA allocations
  if (!IsDamaAvailable ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - No DAMA -> Running Slotted ALOHA");

      /// Randomize Tx opportunity release time
      results.slottedAlohaResult = SlottedAlohaRandomizeReleaseTime ();
      results.resultType = SatRandomAccess::RA_SLOTTED_ALOHA_TX_OPPORTUNITY;
    }

  NS_LOG_INFO ("----------------------------------------------");
  NS_LOG_INFO ("------ Slotted ALOHA algorithm FINISHED ------");
  NS_LOG_INFO ("----------------------------------------------");

  return results;
}

double
SatRandomAccess::SlottedAlohaRandomizeReleaseTime ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaRandomizeReleaseTime - Randomizing the release time...");

  double releaseTime = Now ().GetSeconds () + m_uniformRandomVariable->GetValue (m_slottedAlohaMinRandomizationValue, m_slottedAlohaMaxRandomizationValue);

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

  if (m_crdsaBackoffTime < 0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaBackoffTime < 0");
    }

  if (m_crdsaBackoffProbability < 0.0 || m_crdsaBackoffProbability > 1.0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaBackoffProbability < 0.0 || m_crdsaBackoffProbability > 1.0");
    }

  if (m_crdsaMaximumBackoffProbability < 0.0 || m_crdsaMaximumBackoffProbability > 1.0)
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaDoVariableSanityCheck - m_crdsaMaximumBackoffProbability < 0.0 || m_crdsaMaximumBackoffProbability > 1.0");
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaDoVariableSanityCheck - Variable sanity check done");
}

void
SatRandomAccess::CrdsaSetLoadControlParameters (double crdsaBackoffProbability, double backoffTime, double maximumBackoffProbability)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == RA_CRDSA || m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      m_crdsaBackoffProbability = crdsaBackoffProbability;
      m_crdsaBackoffTime = backoffTime;
      m_crdsaMaximumBackoffProbability = maximumBackoffProbability;
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaSetLoadControlParameters - Wrong random access model in use");
    }

  CrdsaDoVariableSanityCheck ();
}

void
SatRandomAccess::CrdsaSetMaximumDataRateLimitationParameters (uint32_t requestClass,
                                                              uint32_t minRandomizationValue,
                                                              uint32_t maxRandomizationValue,
                                                              uint32_t minIdleBlocks,
                                                              uint32_t numOfInstances,
                                                              uint32_t payloadBytes)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == RA_CRDSA || m_randomAccessModel == RA_ANY_AVAILABLE)
    {
      m_randomAccessConf->GetRequestClassConfiguration (requestClass)->SetMinRandomizationValue (minRandomizationValue);
      m_randomAccessConf->GetRequestClassConfiguration (requestClass)->SetMaxRandomizationValue (maxRandomizationValue);
      m_randomAccessConf->GetRequestClassConfiguration (requestClass)->SetMinIdleBlocks (minIdleBlocks);
      m_randomAccessConf->GetRequestClassConfiguration (requestClass)->SetNumOfInstances (numOfInstances);
      m_randomAccessConf->GetRequestClassConfiguration (requestClass)->SetPayloadBytes (payloadBytes);

      m_randomAccessConf->GetRequestClassConfiguration (requestClass)->DoVariableSanityCheck ();
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::CrdsaSetLoadControlParameters - Wrong random access model in use");
    }

  CrdsaDoVariableSanityCheck ();
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

  NS_LOG_INFO ("SatRandomAccess::CrdsaHasBackoffTimePassed: " << hasCrdsaBackoffTimePassed);

  return hasCrdsaBackoffTimePassed;
}

void
SatRandomAccess::CrdsaReduceIdleBlocksFromAllRequestClasses ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t requestClass = 0; requestClass < m_numOfRequestClasses; requestClass++)
    {
      uint32_t idleBlocksLeft = m_randomAccessConf->GetRequestClassConfiguration (requestClass)->GetIdleBlocksLeft ();

      if (idleBlocksLeft > 0)
        {
          NS_LOG_INFO ("SatRandomAccess::CrdsaReduceIdleBlocks - Reducing request class: " << requestClass << " idle blocks by one");
          m_randomAccessConf->GetRequestClassConfiguration (requestClass)->SetIdleBlocksLeft (idleBlocksLeft - 1);
        }
    }
}

bool
SatRandomAccess::CrdsaIsRequestClassFree (uint32_t requestClass)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessConf->GetRequestClassConfiguration (requestClass)->GetIdleBlocksLeft () > 0)
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaIsRequestClassFree - Request class: " << requestClass << " idle in effect");
      return false;
    }
  NS_LOG_INFO ("SatRandomAccess::CrdsaIsRequestClassFree - Request class: " << requestClass << " free");
  return true;
}

bool
SatRandomAccess::CrdsaDoBackoff ()
{
  NS_LOG_FUNCTION (this);

  bool doCrdsaBackoff = false;

  if (m_uniformRandomVariable->GetValue (0.0,1.0) < m_crdsaBackoffProbability)
    {
      doCrdsaBackoff = true;
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaDoBackoff: " << doCrdsaBackoff);

  return doCrdsaBackoff;
}


void
SatRandomAccess::CrdsaSetBackoffTimer ()
{
  NS_LOG_FUNCTION (this);

  m_crdsaBackoffReleaseTime = Now ().GetSeconds () + m_crdsaBackoffTime;

  CrdsaReduceIdleBlocksFromAllRequestClasses ();

  NS_LOG_INFO ("SatRandomAccess::CrdsaSetBackoffTimer - Setting backoff timer");
}

SatRandomAccess::RandomAccessResults_s
SatRandomAccess::CrdsaPrepareToTransmit ()
{
  NS_LOG_FUNCTION (this);

  RandomAccessResults_s results;
  results.resultType = SatRandomAccess::RA_DO_NOTHING;

  uint32_t limit = m_randomAccessConf->GetMaxUniquePayloadPerBlock ();

  for (uint32_t i = 0; i < limit; i++)
    {
      /// TODO this needs to be implemented
      uint32_t candidateRequestClass = m_uniformRandomVariable->GetInteger (0,(m_numOfRequestClasses-1));

      /// TODO implement BREAK if we get no suitable candidates!
      if (m_uniformRandomVariable->GetValue (0.0,1.0) < 0.2)
        {
          NS_LOG_INFO ("SatRandomAccess::CrdsaPrepareToTransmit - No suitable candidates found");
          break;
        }

      NS_LOG_INFO ("SatRandomAccess::CrdsaPrepareToTransmit - New Tx candidate from request class: " << candidateRequestClass);

      if (CrdsaIsRequestClassFree (candidateRequestClass))
        {
          NS_LOG_INFO ("SatRandomAccess::CrdsaPrepareToTransmit - Preparing for transmission with request class: " << candidateRequestClass);

          std::map<uint32_t,std::set<uint32_t> >::iterator iter = results.crdsaResult.find (candidateRequestClass);

          if (iter == results.crdsaResult.end ())
            {
              std::set<uint32_t> emptySet;
              results.crdsaResult.insert (make_pair(candidateRequestClass, CrdsaRandomizeTxOpportunities (candidateRequestClass,emptySet)));
            }
          else
            {
              results.crdsaResult.at(candidateRequestClass) = CrdsaRandomizeTxOpportunities (iter->first,iter->second);
            }

          results.resultType = SatRandomAccess::RA_CRDSA_TX_OPPORTUNITY;

          if (AreBuffersEmpty ())
            {
              m_crdsaNewData = true;
            }
        }
    }

  CrdsaReduceIdleBlocksFromAllRequestClasses ();

  return results;
}

void
SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed ()
{
  NS_LOG_FUNCTION (this);

  m_randomAccessConf->SetNumOfConsecutiveBlocksUsed (m_randomAccessConf->GetNumOfConsecutiveBlocksUsed () + 1);

  NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed - Increasing the number of used consecutive blocks");

  if (m_randomAccessConf->GetNumOfConsecutiveBlocksUsed () >= m_randomAccessConf->GetMaxConsecutiveBlocksAccessed ())
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed - Maximum number of consecutive blocks reached, forcing idle blocks");

      for (uint32_t index = 0; index < m_numOfRequestClasses; index++)
        {
          m_randomAccessConf->GetRequestClassConfiguration (index)->SetIdleBlocksLeft (m_randomAccessConf->GetRequestClassConfiguration (index)->GetMinIdleBlocks ());
        }

      m_randomAccessConf->SetNumOfConsecutiveBlocksUsed (0);
    }
}

SatRandomAccess::RandomAccessResults_s
SatRandomAccess::DoCrdsa ()
{
  NS_LOG_FUNCTION (this);

  RandomAccessResults_s results;
  results.resultType = SatRandomAccess::RA_DO_NOTHING;

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
                  CrdsaSetBackoffTimer ();
                }
              else
                {
                  results = CrdsaPrepareToTransmit ();
                }
            }
          else
            {
              results = CrdsaPrepareToTransmit ();
            }

          if (results.resultType == SatRandomAccess::RA_CRDSA_TX_OPPORTUNITY)
            {
              CrdsaIncreaseConsecutiveBlocksUsed ();
            }
          else if (results.resultType == SatRandomAccess::RA_DO_NOTHING)
            {
              m_randomAccessConf->SetNumOfConsecutiveBlocksUsed (0);
            }
        }
      else
        {
          CrdsaReduceIdleBlocksFromAllRequestClasses ();
        }
    }
  else
    {
      CrdsaReduceIdleBlocksFromAllRequestClasses ();
    }

  NS_LOG_INFO ("--------------------------------------");
  NS_LOG_INFO ("------ CRDSA algorithm FINISHED ------");
  NS_LOG_INFO ("------ Result: " << results.resultType << " ---------------------");
  NS_LOG_INFO ("--------------------------------------");

  return results;
}


std::set<uint32_t>
SatRandomAccess::CrdsaRandomizeTxOpportunities (uint32_t requestClass, std::set<uint32_t> txOpportunities)
{
  NS_LOG_FUNCTION (this);

  std::pair<std::set<uint32_t>::iterator,bool> result;

  NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Randomizing TX opportunities for request class: " << requestClass);

  if (CrdsaDoBackoff ())
    {
      CrdsaSetBackoffTimer ();
    }
  else
    {
      uint32_t successfulInserts = 0;
      uint32_t instances = m_randomAccessConf->GetRequestClassConfiguration (requestClass)->GetNumOfInstances ();
      while (successfulInserts < instances)
        {
          uint32_t slot = m_uniformRandomVariable->GetInteger (m_randomAccessConf->GetRequestClassConfiguration (requestClass)->GetMinRandomizationValue (),
                                                               m_randomAccessConf->GetRequestClassConfiguration (requestClass)->GetMaxRandomizationValue ());

          result = txOpportunities.insert (slot);

          if (result.second)
            {
              successfulInserts++;
            }

          NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Request class: " << requestClass << " insert successful " << result.second << " for TX opportunity slot: " << (*result.first));
        }
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Randomizing done");

  return txOpportunities;
}

} // namespace ns3
