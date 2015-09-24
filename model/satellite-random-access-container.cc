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
    .SetParent<Object> ()
    .AddConstructor<SatRandomAccess> ();
  return tid;
}

SatRandomAccess::SatRandomAccess ()
  : m_uniformRandomVariable (),
    m_randomAccessModel (SatEnums::RA_MODEL_OFF),
    m_randomAccessConf (),
    m_numOfAllocationChannels (),

    /// CRDSA variables
    m_crdsaNewData (true)
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Constructor not in use");
}

SatRandomAccess::SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, SatEnums::RandomAccessModel_t randomAccessModel)
  : m_uniformRandomVariable (),
    m_randomAccessModel (randomAccessModel),
    m_randomAccessConf (randomAccessConf),
    m_numOfAllocationChannels (randomAccessConf->GetNumOfAllocationChannels ()),

    /// CRDSA variables
    m_crdsaNewData (true)
{
  NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();

  if (m_randomAccessConf == NULL)
    {
      NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Configuration object is NULL");
    }

  SetRandomAccessModel (randomAccessModel);
}

SatRandomAccess::~SatRandomAccess ()
{
  NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = NULL;
  m_randomAccessConf = NULL;

  if (!m_isDamaAvailableCb.IsNull ())
    {
      m_isDamaAvailableCb.Nullify ();
    }

  if (!m_areBuffersEmptyCb.IsNull ())
    {
      m_areBuffersEmptyCb.Nullify ();
    }

  m_crdsaAllocationChannels.clear ();
  m_slottedAlohaAllocationChannels.clear ();
}

void
SatRandomAccess::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = NULL;
  m_randomAccessConf = NULL;

  if (!m_isDamaAvailableCb.IsNull ())
    {
      m_isDamaAvailableCb.Nullify ();
    }

  if (!m_areBuffersEmptyCb.IsNull ())
    {
      m_areBuffersEmptyCb.Nullify ();
    }

  m_crdsaAllocationChannels.clear ();
  m_slottedAlohaAllocationChannels.clear ();
}

///---------------------------------------
/// General random access related methods
///---------------------------------------

void
SatRandomAccess::SetRandomAccessModel (SatEnums::RandomAccessModel_t randomAccessModel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Setting Random Access model to: " << SatEnums::GetRandomAccessModelName (randomAccessModel));

  m_randomAccessModel = randomAccessModel;

  NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Random Access model updated");
}

void
SatRandomAccess::AddCrdsaAllocationChannel (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  std::pair<std::set<uint32_t>::iterator,bool> result = m_crdsaAllocationChannels.insert (allocationChannel);

  if (!result.second)
    {
      NS_FATAL_ERROR ("SatRandomAccess::AddCrdsaAllocationChannel - insert failed, this allocation channel is already set");
    }
}

void
SatRandomAccess::AddSlottedAlohaAllocationChannel (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  std::pair<std::set<uint32_t>::iterator,bool> result = m_slottedAlohaAllocationChannels.insert (allocationChannel);

  if (!result.second)
    {
      NS_FATAL_ERROR ("SatRandomAccess::AddSlottedAlohaAllocationChannel - insert failed, this allocation channel is already set");
    }
}

bool
SatRandomAccess::IsCrdsaAllocationChannel (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t>::iterator iter;
  iter = m_crdsaAllocationChannels.find (allocationChannel);

  if (iter == m_crdsaAllocationChannels.end ())
    {
      return false;
    }
  return true;
}

bool
SatRandomAccess::IsSlottedAlohaAllocationChannel (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t>::iterator iter;
  iter = m_slottedAlohaAllocationChannels.find (allocationChannel);

  if (iter == m_slottedAlohaAllocationChannels.end ())
    {
      return false;
    }
  return true;
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoRandomAccess (uint32_t allocationChannel, SatEnums::RandomAccessTriggerType_t triggerType)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::DoRandomAccess, AC: " << allocationChannel << " trigger type: " << SatEnums::GetRandomAccessTriggerTypeName (triggerType));

  /// return variable initialization
  RandomAccessTxOpportunities_s txOpportunities;

  txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;
  txOpportunities.allocationChannel = allocationChannel;

  NS_LOG_INFO ("------------------------------------");
  NS_LOG_INFO ("------ Starting Random Access ------");
  NS_LOG_INFO ("------------------------------------");

  /// Do CRDSA
  if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA && triggerType == SatEnums::RA_TRIGGER_TYPE_CRDSA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Only CRDSA enabled && CRDSA trigger, checking allocation channel");

      if (IsCrdsaAllocationChannel (allocationChannel))
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Valid CRDSA allocation channel, checking backoff status");

          if (CrdsaHasBackoffTimePassed (allocationChannel))
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA backoff period over, evaluating CRDSA");
              txOpportunities = DoCrdsa (allocationChannel);
            }
          else
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA backoff period in effect, aborting");
              CrdsaReduceIdleBlocksForAllAllocationChannels ();
              CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();
            }
        }
      else
        {
          NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid allocation channel for CRDSA");
        }
    }
  /// Do Slotted ALOHA
  else if (m_randomAccessModel == SatEnums::RA_MODEL_SLOTTED_ALOHA && triggerType == SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Only SA enabled, checking allocation channel");

      if (IsSlottedAlohaAllocationChannel (allocationChannel))
        {
          NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Valid allocation channel, evaluating Slotted ALOHA");

          txOpportunities = DoSlottedAloha ();
        }
      else
        {
          NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid allocation channel for Slotted ALOHA");
        }
    }
  /// If all RA based on RCS2 specification is enabled, CRDSA is used if the RA parameter number of instances is set >= 2. Otherwise SA is used.
  else if (m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - RA based on RCS2 specification enabled");

      if (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaNumOfInstances () < 2)
        {
          if (triggerType == SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA)
            {
              if (IsSlottedAlohaAllocationChannel (allocationChannel))
                {
                  NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Valid Slotted ALOHA allocation channel, evaluating Slotted ALOHA");
                  txOpportunities = DoSlottedAloha ();
                }
              else
                {
                  NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid allocation channel for Slotted ALOHA");
                }
            }
          else
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Number of instances is < 2, only Slotted ALOHA is in use");
            }
        }
      else
        {
          if (triggerType == SatEnums::RA_TRIGGER_TYPE_CRDSA)
            {
              if (IsCrdsaAllocationChannel (allocationChannel))
                {
                  NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Valid CRDSA allocation channel, checking backoff status");

                  if (CrdsaHasBackoffTimePassed (allocationChannel))
                    {
                      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA backoff period over, evaluating CRDSA");
                      txOpportunities = DoCrdsa (allocationChannel);
                    }
                  else
                    {
                      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA backoff period in effect, aborting");
                      CrdsaReduceIdleBlocksForAllAllocationChannels ();
                      CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();
                    }
                }
              else
                {
                  NS_FATAL_ERROR ("SatRandomAccess::DoRandomAccess - Invalid allocation channel for CRDSA");
                }
            }
          else
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Number of instances is > 1, only CRDSA is in use");
            }
        }
    }
  else if (m_randomAccessModel != SatEnums::RA_MODEL_SLOTTED_ALOHA && triggerType == SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - Slotted ALOHA is disabled");
    }
  else if (m_randomAccessModel != SatEnums::RA_MODEL_CRDSA && triggerType == SatEnums::RA_TRIGGER_TYPE_CRDSA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA is disabled");
    }

  /// For logging/debugging purposes
  /// TODO: This if statement is utilized only for logging, and thus could be optimized
  /// to be enabled only with debug build.
  if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_CRDSA)
    {
      std::map<uint32_t, std::set<uint32_t> >::iterator iter;
      uint32_t uniquePacketId = 1;

      for (iter = txOpportunities.crdsaTxOpportunities.begin (); iter != txOpportunities.crdsaTxOpportunities.end (); iter++)
        {
          std::set<uint32_t>::iterator iterSet;
          for (iterSet = iter->second.begin (); iterSet != iter->second.end (); iterSet++)
            {
              NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - CRDSA transmission opportunity for unique packet: " << uniquePacketId << " at slot: " << (*iterSet));
            }
          uniquePacketId++;
        }
    }
  else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_SLOTTED_ALOHA)
    {
      NS_LOG_INFO ("SatRandomAccess::DoRandomAccess - SA minimum time to wait: " << txOpportunities.slottedAlohaTxOpportunity << " milliseconds");
    }
  else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING)
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

  return txOpportunities;
}

void
SatRandomAccess::PrintVariables ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Simulation time: " << Now ().GetSeconds () << " seconds");
  NS_LOG_INFO ("Num of allocation channels: " << m_numOfAllocationChannels);
  NS_LOG_INFO ("New data status: " << m_crdsaNewData);

  NS_LOG_INFO ("---------------");

  for (uint32_t index = 0; index < m_numOfAllocationChannels; index++)
    {
      NS_LOG_INFO ("ALLOCATION CHANNEL: " << index);
      NS_LOG_INFO ("Backoff release at: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaBackoffReleaseTime ().GetSeconds () << " seconds");
      NS_LOG_INFO ("Backoff time: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaBackoffTimeInMilliSeconds () << " milliseconds");
      NS_LOG_INFO ("Backoff probability: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaBackoffProbability () * 100 << " %");
      NS_LOG_INFO ("Slot randomization: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaNumOfInstances () * m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaMaxUniquePayloadPerBlock () <<
                   " Tx opportunities with range from " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaMinRandomizationValue () <<
                   " to " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaMaxRandomizationValue ());
      NS_LOG_INFO ("Number of unique payloads per block: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaMaxUniquePayloadPerBlock ());
      NS_LOG_INFO ("Number of instances: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaNumOfInstances ());
      NS_LOG_INFO ("Number of consecutive blocks accessed: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaNumOfConsecutiveBlocksUsed () << "/" << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaMaxConsecutiveBlocksAccessed ());
      NS_LOG_INFO ("Number of idle blocks left: " << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaIdleBlocksLeft () << "/" << m_randomAccessConf->GetAllocationChannelConfiguration (index)->GetCrdsaMinIdleBlocks ());
    }
}

///-------------------------------
/// Slotted ALOHA related methods
///-------------------------------

uint32_t
SatRandomAccess::GetSlottedAlohaSignalingOverheadInBytes ()
{
  return m_randomAccessConf->GetSlottedAlohaSignalingOverheadInBytes ();
}

void
SatRandomAccess::SetSlottedAlohaControlRandomizationIntervalInMilliSeconds (uint32_t controlRandomizationIntervalInMilliSeconds)
{
  NS_LOG_FUNCTION (this << controlRandomizationIntervalInMilliSeconds);

  if (m_randomAccessModel == SatEnums::RA_MODEL_SLOTTED_ALOHA || m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
      m_randomAccessConf->SetSlottedAlohaControlRandomizationIntervalInMilliSeconds (controlRandomizationIntervalInMilliSeconds);

      m_randomAccessConf->DoSlottedAlohaVariableSanityCheck ();
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::SetSlottedAlohaControlRandomizationIntervalInMilliSeconds - Wrong random access model in use");
    }

  NS_LOG_INFO ("SatRandomAccess::SetSlottedAlohaControlRandomizationIntervalInMilliSeconds - new control randomization interval : " << controlRandomizationIntervalInMilliSeconds);
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoSlottedAloha ()
{
  NS_LOG_FUNCTION (this);

  RandomAccessTxOpportunities_s txOpportunity;
  txOpportunity.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;

  NS_LOG_INFO ("---------------------------------------------");
  NS_LOG_INFO ("------ Running Slotted ALOHA algorithm ------");
  NS_LOG_INFO ("---------------------------------------------");
  NS_LOG_INFO ("Slotted ALOHA control randomization interval: " << m_randomAccessConf->GetSlottedAlohaControlRandomizationIntervalInMilliSeconds () << " milliseconds");
  NS_LOG_INFO ("---------------------------------------------");

  NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - Checking if we have DAMA allocations");

  /// Check if we have known DAMA allocations
  if (!m_isDamaAvailableCb ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - No DAMA -> Running Slotted ALOHA");

      /// Randomize Tx opportunity release time
      txOpportunity.slottedAlohaTxOpportunity = SlottedAlohaRandomizeReleaseTime ();
      txOpportunity.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_SLOTTED_ALOHA;
    }

  NS_LOG_INFO ("----------------------------------------------");
  NS_LOG_INFO ("------ Slotted ALOHA algorithm FINISHED ------");
  NS_LOG_INFO ("----------------------------------------------");

  return txOpportunity;
}

uint32_t
SatRandomAccess::SlottedAlohaRandomizeReleaseTime ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaRandomizeReleaseTime - Randomizing the release time...");

  uint32_t releaseTime = m_uniformRandomVariable->GetInteger (0, m_randomAccessConf->GetSlottedAlohaControlRandomizationIntervalInMilliSeconds ());

  NS_LOG_INFO ("SatRandomAccess::SlottedAlohaRandomizeReleaseTime - TX opportunity in the next slot after " << releaseTime << " milliseconds");

  return releaseTime;
}

///-----------------------
/// CRDSA related methods
///-----------------------

void
SatRandomAccess::SetCrdsaBackoffTimeInMilliSeconds (uint32_t allocationChannel,
                                                    uint32_t backoffTimeInMilliSeconds)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA || m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaBackoffTimeInMilliSeconds (backoffTimeInMilliSeconds);

      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->DoCrdsaVariableSanityCheck ();
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::SetCrdsaBackoffTimeInMilliSeconds - Wrong random access model in use");
    }
}

void
SatRandomAccess::SetCrdsaBackoffProbability (uint32_t allocationChannel,
                                             uint16_t backoffProbability)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA || m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaBackoffProbability (backoffProbability);

      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->DoCrdsaVariableSanityCheck ();
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::SetCrdsaBackoffProbability - Wrong random access model in use");
    }
}

void
SatRandomAccess::SetCrdsaRandomizationParameters (uint32_t allocationChannel,
                                                  uint32_t minRandomizationValue,
                                                  uint32_t maxRandomizationValue,
                                                  uint32_t numOfInstances)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA || m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaMinRandomizationValue (minRandomizationValue);
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaMaxRandomizationValue (maxRandomizationValue);
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaNumOfInstances (numOfInstances);

      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->DoCrdsaVariableSanityCheck ();
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::SetCrdsaRandomizationParameters - Wrong random access model in use");
    }
}

void
SatRandomAccess::SetCrdsaMaximumDataRateLimitationParameters (uint32_t allocationChannel,
                                                              uint32_t maxUniquePayloadPerBlock,
                                                              uint32_t maxConsecutiveBlocksAccessed,
                                                              uint32_t minIdleBlocks)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA || m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaMaxUniquePayloadPerBlock (maxUniquePayloadPerBlock);
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaMaxConsecutiveBlocksAccessed (maxConsecutiveBlocksAccessed);
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaMinIdleBlocks (minIdleBlocks);

      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->DoCrdsaVariableSanityCheck ();
    }
  else
    {
      NS_FATAL_ERROR ("SatRandomAccess::SetCrdsaMaximumDataRateLimitationParameters - Wrong random access model in use");
    }
}

uint32_t
SatRandomAccess::GetCrdsaSignalingOverheadInBytes ()
{
  NS_LOG_INFO ("SatRandomAccess::GetCrdsaSignalingOverheadInBytes");

  return m_randomAccessConf->GetCrdsaSignalingOverheadInBytes ();
}

bool
SatRandomAccess::CrdsaHasBackoffTimePassed (uint32_t allocationChannel) const
{
  NS_LOG_FUNCTION (this);

  bool hasCrdsaBackoffTimePassed = false;

  if ((Now () >= m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaBackoffReleaseTime ()))
    {
      hasCrdsaBackoffTimePassed = true;
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaHasBackoffTimePassed for allocation channel " << allocationChannel << ": " << hasCrdsaBackoffTimePassed);

  return hasCrdsaBackoffTimePassed;
}

void
SatRandomAccess::CrdsaReduceIdleBlocks (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  uint32_t idleBlocksLeft = m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaIdleBlocksLeft ();

  if (idleBlocksLeft > 0)
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaReduceIdleBlocks - Reducing allocation channel: " << allocationChannel << " idle blocks by one");
      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaIdleBlocksLeft (idleBlocksLeft - 1);
    }
}

void
SatRandomAccess::CrdsaReduceIdleBlocksForAllAllocationChannels ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::CrdsaReduceIdleBlocksForAllAllocationChannels");

  for (uint32_t i = 0; i < m_numOfAllocationChannels; i++)
    {
      CrdsaReduceIdleBlocks (i);
    }
}

void
SatRandomAccess::CrdsaResetConsecutiveBlocksUsed (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::CrdsaResetConsecutiveBlocksUsed for AC: " << allocationChannel);

  m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaNumOfConsecutiveBlocksUsed (0);
}

void
SatRandomAccess::CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels");

  for (uint32_t i = 0; i < m_numOfAllocationChannels; i++)
    {
      CrdsaResetConsecutiveBlocksUsed (i);
    }
}

bool
SatRandomAccess::CrdsaIsAllocationChannelFree (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaIdleBlocksLeft () > 0)
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaIsAllocationChannelFree - Allocation channel: " << allocationChannel << " idle in effect");
      return false;
    }
  NS_LOG_INFO ("SatRandomAccess::CrdsaIsAllocationChannelFree - Allocation channel: " << allocationChannel << " free");
  return true;
}

bool
SatRandomAccess::CrdsaDoBackoff (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  bool doCrdsaBackoff = false;

  if (m_uniformRandomVariable->GetValue (0.0,1.0) < m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaBackoffProbability ())
    {
      doCrdsaBackoff = true;
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaDoBackoff for allocation channel " << allocationChannel << ": " << doCrdsaBackoff);

  return doCrdsaBackoff;
}

void
SatRandomAccess::CrdsaSetBackoffTimer (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaBackoffReleaseTime (Now ()
                                                                                                         + MilliSeconds (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaBackoffTimeInMilliSeconds ()));

  NS_LOG_INFO ("SatRandomAccess::CrdsaSetBackoffTimer - Setting backoff timer for allocation channel: " << allocationChannel);
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::CrdsaPrepareToTransmit (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  RandomAccessTxOpportunities_s txOpportunities;
  txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;

  uint32_t maxUniquePackets = m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaMaxUniquePayloadPerBlock ();

  /// TODO when multiple overlapping allocation channels for a single UT needs to be supported
  /// slots.first can be updated to take into account the reserved RA slots from MAC.
  /// For this the logic for determining the reserved slots needs to be implemented in UT MAC
  /// and the slots should be passed as parameter to RA logic and used instead of slots.first

  /// This should be done by including the list of used slots in this SF as a parameter for the
  /// random access algorithm call. This functionality is needed with, e.g., multiple allocation channels
  std::pair <std::set<uint32_t>, std::set<uint32_t> > slots;

  for (uint32_t i = 0; i < maxUniquePackets; i++)
    {
      if (CrdsaDoBackoff (allocationChannel))
        {
          CrdsaSetBackoffTimer (allocationChannel);
          break;
        }
      else
        {
          NS_LOG_INFO ("SatRandomAccess::CrdsaPrepareToTransmit - New Tx candidate for allocation channel: " << allocationChannel);

          if (CrdsaIsAllocationChannelFree (allocationChannel))
            {
              NS_LOG_INFO ("SatRandomAccess::CrdsaPrepareToTransmit - Preparing for transmission with allocation channel: " << allocationChannel);

              /// randomize instance slots for this unique packet
              slots = CrdsaRandomizeTxOpportunities (allocationChannel,slots);

              /// save the packet specific Tx opportunities into a vector
              txOpportunities.crdsaTxOpportunities.insert (std::make_pair (*slots.second.begin (), slots.second));

              if (m_areBuffersEmptyCb ())
                {
                  m_crdsaNewData = true;
                }

              /// save the rest of the CRDSA Tx opportunity results
              txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_CRDSA;
            }
        }
    }

  return txOpportunities;
}

void
SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaNumOfConsecutiveBlocksUsed (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaNumOfConsecutiveBlocksUsed () + 1);

  NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed - Increasing the number of used consecutive blocks for allocation channel: " << allocationChannel);

  if (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaNumOfConsecutiveBlocksUsed () >= m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaMaxConsecutiveBlocksAccessed ())
    {
      NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed - Maximum number of consecutive blocks reached, forcing idle blocks for allocation channel: " << allocationChannel);

      m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->SetCrdsaIdleBlocksLeft (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaMinIdleBlocks ());

      CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();
    }
}

void
SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsedForAllAllocationChannels ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsedForAllAllocationChannels");

  for (uint32_t i = 0; i < m_numOfAllocationChannels; i++)
    {
      CrdsaIncreaseConsecutiveBlocksUsed (i);
    }
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoCrdsa (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  RandomAccessTxOpportunities_s txOpportunities;
  txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;

  NS_LOG_INFO ("-------------------------------------");
  NS_LOG_INFO ("------ Running CRDSA algorithm ------");
  NS_LOG_INFO ("-------------------------------------");

  PrintVariables ();

  NS_LOG_INFO ("-------------------------------------");

  NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Backoff period over, checking DAMA status...");

  if (!m_isDamaAvailableCb ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoCrdsa - No DAMA, checking buffer status...");

      if (!m_areBuffersEmptyCb ())
        {
          NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Data in buffer, continuing CRDSA");

          if (m_crdsaNewData)
            {
              m_crdsaNewData = false;

              NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Evaluating back off...");

              if (CrdsaDoBackoff (allocationChannel))
                {
                  NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Initial new data backoff triggered");
                  CrdsaSetBackoffTimer (allocationChannel);
                }
              else
                {
                  txOpportunities = CrdsaPrepareToTransmit (allocationChannel);
                }
            }
          else
            {
              txOpportunities = CrdsaPrepareToTransmit (allocationChannel);
            }

          if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_CRDSA)
            {
              NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Tx opportunity, increasing consecutive blocks used");

              CrdsaIncreaseConsecutiveBlocksUsedForAllAllocationChannels ();
            }
          else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING)
            {
              NS_LOG_INFO ("SatRandomAccess::DoCrdsa - No Tx opportunity, reducing idle blocks & resetting consecutive blocks");

              CrdsaReduceIdleBlocksForAllAllocationChannels ();
              CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();
            }
        }
      else
        {
          NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Empty buffer, reducing idle blocks & resetting consecutive blocks, aborting CRDSA...");

          CrdsaReduceIdleBlocksForAllAllocationChannels ();
          CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();
        }
    }
  else
    {
      NS_LOG_INFO ("SatRandomAccess::DoCrdsa - DAMA allocation found, aborting CRDSA...");

      CrdsaReduceIdleBlocksForAllAllocationChannels ();
      CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();
    }

  NS_LOG_INFO ("--------------------------------------");
  NS_LOG_INFO ("------ CRDSA algorithm FINISHED ------");
  NS_LOG_INFO ("------ Result: " << SatEnums::GetRandomAccessOpportunityTypeName (txOpportunities.txOpportunityType) << " ---------------------");
  NS_LOG_INFO ("--------------------------------------");

  return txOpportunities;
}

std::pair <std::set<uint32_t>, std::set<uint32_t> >
SatRandomAccess::CrdsaRandomizeTxOpportunities (uint32_t allocationChannel, std::pair <std::set<uint32_t>, std::set<uint32_t> > slots)
{
  NS_LOG_FUNCTION (this);

  std::pair<std::set<uint32_t>::iterator,bool> resultAllSlotsInFrame;
  std::pair<std::set<uint32_t>::iterator,bool> resultThisUniquePacket;

  std::set<uint32_t> emptySet;
  slots.second = emptySet;

  NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Randomizing TX opportunities for allocation channel: " << allocationChannel);

  uint32_t successfulInserts = 0;
  uint32_t instances = m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaNumOfInstances ();
  while (successfulInserts < instances)
    {
      uint32_t slot = m_uniformRandomVariable->GetInteger (m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaMinRandomizationValue (),
                                                           m_randomAccessConf->GetAllocationChannelConfiguration (allocationChannel)->GetCrdsaMaxRandomizationValue ());

      resultAllSlotsInFrame = slots.first.insert (slot);

      if (resultAllSlotsInFrame.second)
        {
          successfulInserts++;

          resultThisUniquePacket = slots.second.insert (slot);

          if (!resultAllSlotsInFrame.second)
            {
              NS_FATAL_ERROR ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Slots out of sync, this should never happen");
            }
        }

      NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Allocation channel: " << allocationChannel << " insert successful " << resultAllSlotsInFrame.second << " for TX opportunity slot: " << (*resultAllSlotsInFrame.first));
    }

  NS_LOG_INFO ("SatRandomAccess::CrdsaRandomizeTxOpportunities - Randomizing done");

  return slots;
}

void
SatRandomAccess::SetIsDamaAvailableCallback (SatRandomAccess::IsDamaAvailableCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);

  m_isDamaAvailableCb = callback;
}

void
SatRandomAccess::SetAreBuffersEmptyCallback (SatRandomAccess::AreBuffersEmptyCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);

  m_areBuffersEmptyCb = callback;
}

} // namespace ns3
