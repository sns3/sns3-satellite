/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#include "satellite-random-access-container.h"

#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("SatRandomAccess");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatRandomAccess);

TypeId
SatRandomAccess::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatRandomAccess").SetParent<Object>().AddConstructor<SatRandomAccess>();
    return tid;
}

SatRandomAccess::SatRandomAccess()
    : m_uniformRandomVariable(),
      m_randomAccessModel(SatEnums::RA_MODEL_OFF),
      m_randomAccessConf(),
      m_numOfAllocationChannels(),

      /// CRDSA variables
      m_crdsaNewData(true)
{
    NS_LOG_FUNCTION(this);

    NS_FATAL_ERROR("SatRandomAccess::SatRandomAccess - Constructor not in use");
}

SatRandomAccess::SatRandomAccess(Ptr<SatRandomAccessConf> randomAccessConf,
                                 SatEnums::RandomAccessModel_t randomAccessModel)
    : m_uniformRandomVariable(),
      m_randomAccessModel(randomAccessModel),
      m_randomAccessConf(randomAccessConf),
      m_numOfAllocationChannels(randomAccessConf->GetNumOfAllocationChannelsConfigurations()),

      /// CRDSA variables
      m_crdsaNewData(true)
{
    NS_LOG_FUNCTION(this);

    m_uniformRandomVariable = CreateObject<UniformRandomVariable>();

    if (!m_randomAccessConf)
    {
        NS_FATAL_ERROR("SatRandomAccess::SatRandomAccess - Configuration object is NULL");
    }

    SetRandomAccessModel(randomAccessModel);
}

SatRandomAccess::~SatRandomAccess()
{
    NS_LOG_FUNCTION(this);

    m_uniformRandomVariable = NULL;
    m_randomAccessConf = NULL;

    if (!m_isDamaAvailableCb.IsNull())
    {
        m_isDamaAvailableCb.Nullify();
    }

    if (!m_areBuffersEmptyCb.IsNull())
    {
        m_areBuffersEmptyCb.Nullify();
    }
}

void
SatRandomAccess::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_uniformRandomVariable = NULL;
    m_randomAccessConf = NULL;

    if (!m_isDamaAvailableCb.IsNull())
    {
        m_isDamaAvailableCb.Nullify();
    }

    if (!m_areBuffersEmptyCb.IsNull())
    {
        m_areBuffersEmptyCb.Nullify();
    }
}

///---------------------------------------
/// General random access related methods
///---------------------------------------

void
SatRandomAccess::SetRandomAccessModel(SatEnums::RandomAccessModel_t randomAccessModel)
{
    NS_LOG_FUNCTION(this << SatEnums::GetRandomAccessModelName(randomAccessModel));

    m_randomAccessModel = randomAccessModel;
}

bool
SatRandomAccess::IsCrdsaAllocationChannel(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this << allocationChannel);

    return m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
        ->GetCrdsaAllowed();
}

bool
SatRandomAccess::IsSlottedAlohaAllocationChannel(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this << allocationChannel);

    return m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
        ->GetSlottedAlohaAllowed();
}

bool
SatRandomAccess::IsEssaAllocationChannel(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this << allocationChannel);

    return m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
        ->GetEssaAllowed();
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoRandomAccess(uint32_t allocationChannelId,
                                SatEnums::RandomAccessTriggerType_t triggerType)
{
    NS_LOG_FUNCTION(this << allocationChannelId
                         << SatEnums::GetRandomAccessTriggerTypeName(triggerType));

    /// return variable initialization
    RandomAccessTxOpportunities_s txOpportunities;

    txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;
    txOpportunities.allocationChannel = allocationChannelId;
    uint32_t allocationChannel = GetConfigurationIdForAllocationChannel(allocationChannelId);

    NS_LOG_INFO("------------------------------------");
    NS_LOG_INFO("------ Starting Random Access ------");
    NS_LOG_INFO("------------------------------------");

    /// Do CRDSA (MARSALA being a special form of CRDSA)
    if (triggerType == SatEnums::RA_TRIGGER_TYPE_CRDSA &&
        (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA ||
         m_randomAccessModel == SatEnums::RA_MODEL_MARSALA))
    {
        NS_LOG_INFO("Only CRDSA enabled && CRDSA trigger, checking allocation channel");

        if (IsCrdsaAllocationChannel(allocationChannel))
        {
            NS_LOG_INFO("Valid CRDSA allocation channel, checking backoff status");

            if (CrdsaHasBackoffTimePassed(allocationChannel))
            {
                NS_LOG_INFO("CRDSA backoff period over, evaluating CRDSA");
                txOpportunities = DoCrdsa(allocationChannel);
            }
            else
            {
                NS_LOG_INFO("CRDSA backoff period in effect, aborting");
                CrdsaReduceIdleBlocksForAllAllocationChannels();
                CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels();
            }
        }
        else
        {
            NS_FATAL_ERROR(
                "SatRandomAccess::DoRandomAccess - Invalid allocation channel for CRDSA");
        }
    }
    /// Do ESSA
    else if (m_randomAccessModel == SatEnums::RA_MODEL_ESSA &&
             triggerType == SatEnums::RA_TRIGGER_TYPE_ESSA)
    {
        NS_LOG_INFO(
            "SatRandomAccess::DoRandomAccess - Only ESSA enabled, checking allocation channel");

        if (IsEssaAllocationChannel(allocationChannel))
        {
            NS_LOG_INFO(
                "SatRandomAccess::DoRandomAccess - Valid allocation channel, evaluating ESSA");

            txOpportunities = DoEssa(allocationChannel);
        }
        else
        {
            NS_FATAL_ERROR("SatRandomAccess::DoRandomAccess - Invalid allocation channel for ESSA");
        }
    }
    /// Do Slotted ALOHA
    else if (triggerType == SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA &&
             m_randomAccessModel == SatEnums::RA_MODEL_SLOTTED_ALOHA)
    {
        NS_LOG_INFO("Only SA enabled, checking allocation channel");

        if (IsSlottedAlohaAllocationChannel(allocationChannel))
        {
            NS_LOG_INFO("Valid allocation channel, evaluating Slotted ALOHA");

            txOpportunities = DoSlottedAloha();
        }
        else
        {
            NS_FATAL_ERROR(
                "SatRandomAccess::DoRandomAccess - Invalid allocation channel for Slotted ALOHA");
        }
    }
    /// If all RA based on RCS2 specification is enabled, CRDSA is used if the RA parameter number
    /// of instances is set >= 2. Otherwise SA is used.
    else if (m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
        NS_LOG_INFO("RA based on RCS2 specification enabled");

        if (m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                ->GetCrdsaNumOfInstances() < 2)
        {
            if (triggerType == SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA)
            {
                if (IsSlottedAlohaAllocationChannel(allocationChannel))
                {
                    NS_LOG_INFO("Valid Slotted ALOHA allocation channel, evaluating Slotted ALOHA");
                    txOpportunities = DoSlottedAloha();
                }
                else
                {
                    NS_FATAL_ERROR("SatRandomAccess::DoRandomAccess - Invalid allocation channel "
                                   "for Slotted ALOHA");
                }
            }
            else
            {
                NS_LOG_INFO("Number of instances is < 2, only Slotted ALOHA is in use");
            }
        }
        else
        {
            if (triggerType == SatEnums::RA_TRIGGER_TYPE_CRDSA)
            {
                if (IsCrdsaAllocationChannel(allocationChannel))
                {
                    NS_LOG_INFO("Valid CRDSA allocation channel, checking backoff status");

                    if (CrdsaHasBackoffTimePassed(allocationChannel))
                    {
                        NS_LOG_INFO("CRDSA backoff period over, evaluating CRDSA");
                        txOpportunities = DoCrdsa(allocationChannel);
                    }
                    else
                    {
                        NS_LOG_INFO("CRDSA backoff period in effect, aborting");
                        CrdsaReduceIdleBlocksForAllAllocationChannels();
                        CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels();
                    }
                }
                else
                {
                    NS_FATAL_ERROR(
                        "SatRandomAccess::DoRandomAccess - Invalid allocation channel for CRDSA");
                }
            }
            else
            {
                NS_LOG_INFO("Number of instances is > 1, only CRDSA is in use");
            }
        }
    }
    else if (triggerType == SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA &&
             m_randomAccessModel != SatEnums::RA_MODEL_SLOTTED_ALOHA)
    {
        NS_LOG_INFO("Slotted ALOHA is disabled");
    }
    else if (triggerType == SatEnums::RA_TRIGGER_TYPE_CRDSA &&
             m_randomAccessModel != SatEnums::RA_MODEL_CRDSA &&
             m_randomAccessModel != SatEnums::RA_MODEL_MARSALA)
    {
        NS_LOG_INFO("CRDSA is disabled");
    }

    /// For logging/debugging purposes
    /// TODO: This if statement is utilized only for logging, and thus could be optimized
    /// to be enabled only with debug build.
    if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_CRDSA)
    {
        std::map<uint32_t, std::set<uint32_t>>::iterator iter;
        uint32_t uniquePacketId = 1;

        for (iter = txOpportunities.crdsaTxOpportunities.begin();
             iter != txOpportunities.crdsaTxOpportunities.end();
             iter++)
        {
            std::set<uint32_t>::iterator iterSet;
            for (iterSet = iter->second.begin(); iterSet != iter->second.end(); iterSet++)
            {
                NS_LOG_INFO("CRDSA transmission opportunity for unique packet: "
                            << uniquePacketId << " at slot: " << (*iterSet));
            }
            uniquePacketId++;
        }
    }
    else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_SLOTTED_ALOHA)
    {
        NS_LOG_INFO("SA minimum time to wait: " << txOpportunities.slottedAlohaTxOpportunity
                                                << " milliseconds");
    }
    else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_ESSA)
    {
        NS_LOG_INFO("SatRandomAccess::DoRandomAccess - ESSA minimum time to wait: "
                    << txOpportunities.slottedAlohaTxOpportunity << " milliseconds");
    }
    else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING)
    {
        NS_LOG_INFO("No Tx opportunity");
    }
    else
    {
        NS_FATAL_ERROR("SatRandomAccess::DoRandomAccess - Invalid result type");
    }

    NS_LOG_INFO("------------------------------------");
    NS_LOG_INFO("------ Random Access FINISHED ------");
    NS_LOG_INFO("------------------------------------");

    return txOpportunities;
}

void
SatRandomAccess::PrintVariables()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Simulation time: " << Now().GetSeconds() << " seconds");
    NS_LOG_INFO("Num of allocation channels: " << m_numOfAllocationChannels);
    NS_LOG_INFO("New data status: " << m_crdsaNewData);

    NS_LOG_INFO("---------------");

    for (uint32_t index = 0; index < m_numOfAllocationChannels; index++)
    {
        Ptr<SatRandomAccessAllocationChannel> allocationChannel =
            m_randomAccessConf->GetAllocationChannelConfiguration(index);
        NS_LOG_INFO("ALLOCATION CHANNEL: " << index);
        NS_LOG_INFO("Backoff release at: "
                    << allocationChannel->GetCrdsaBackoffReleaseTime().GetSeconds() << " seconds");
        NS_LOG_INFO("Backoff time: " << allocationChannel->GetCrdsaBackoffTimeInMilliSeconds()
                                     << " milliseconds");
        NS_LOG_INFO("Backoff probability: " << allocationChannel->GetCrdsaBackoffProbability() * 100
                                            << " %");
        NS_LOG_INFO("Slot randomization: "
                    << allocationChannel->GetCrdsaNumOfInstances() *
                           allocationChannel->GetCrdsaMaxUniquePayloadPerBlock()
                    << " Tx opportunities with range from "
                    << allocationChannel->GetCrdsaMinRandomizationValue() << " to "
                    << allocationChannel->GetCrdsaMaxRandomizationValue());
        NS_LOG_INFO("Number of unique payloads per block: "
                    << allocationChannel->GetCrdsaMaxUniquePayloadPerBlock());
        NS_LOG_INFO("Number of instances: " << allocationChannel->GetCrdsaNumOfInstances());
        NS_LOG_INFO("Number of consecutive blocks accessed: "
                    << allocationChannel->GetCrdsaNumOfConsecutiveBlocksUsed() << "/"
                    << allocationChannel->GetCrdsaMaxConsecutiveBlocksAccessed());
        NS_LOG_INFO("Number of idle blocks left: " << allocationChannel->GetCrdsaIdleBlocksLeft()
                                                   << "/"
                                                   << allocationChannel->GetCrdsaMinIdleBlocks());
    }
}

uint32_t
SatRandomAccess::GetConfigurationIdForAllocationChannel(uint32_t allocationChannelId)
{
    return m_randomAccessConf->GetAllocationChannelConfigurationId(allocationChannelId);
}

void
SatRandomAccess::SetBackoffTime(uint32_t allocationChannel, uint32_t backoffTime)
{
    NS_LOG_FUNCTION(this);

    switch (m_randomAccessModel)
    {
    case SatEnums::RA_MODEL_ESSA: {
        SetFSimBackoffTimeInFrames(allocationChannel, backoffTime);
        break;
    }
    case SatEnums::RA_MODEL_CRDSA:
    case SatEnums::RA_MODEL_RCS2_SPECIFICATION: {
        SetCrdsaBackoffTimeInMilliSeconds(allocationChannel, backoffTime);
        break;
    }
    default: {
        NS_FATAL_ERROR("SatRandomAccess::SetBackoffTime - Wrong random access model in use");
        break;
    }
    }
}

void
SatRandomAccess::SetBackoffProbability(uint32_t allocationChannel, uint16_t backoffProbability)
{
    NS_LOG_FUNCTION(this);

    switch (m_randomAccessModel)
    {
    case SatEnums::RA_MODEL_ESSA: {
        SetFSimBackoffProbability(allocationChannel, backoffProbability);
        break;
    }
    case SatEnums::RA_MODEL_CRDSA:
    case SatEnums::RA_MODEL_RCS2_SPECIFICATION: {
        SetCrdsaBackoffProbability(allocationChannel, backoffProbability);
        break;
    }
    default: {
        NS_FATAL_ERROR("SatRandomAccess::SetBackoffProbability - Wrong random access model in use");
        break;
    }
    }
}

///-------------------------------
/// Slotted ALOHA related methods
///-------------------------------

uint32_t
SatRandomAccess::GetSlottedAlohaSignalingOverheadInBytes()
{
    return m_randomAccessConf->GetSlottedAlohaSignalingOverheadInBytes();
}

void
SatRandomAccess::SetSlottedAlohaControlRandomizationIntervalInMilliSeconds(
    uint32_t controlRandomizationIntervalInMilliSeconds)
{
    NS_LOG_FUNCTION(this << controlRandomizationIntervalInMilliSeconds);

    if (m_randomAccessModel == SatEnums::RA_MODEL_SLOTTED_ALOHA ||
        m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
        m_randomAccessConf->SetSlottedAlohaControlRandomizationIntervalInMilliSeconds(
            controlRandomizationIntervalInMilliSeconds);

        m_randomAccessConf->DoSlottedAlohaVariableSanityCheck();
    }
    else
    {
        NS_FATAL_ERROR("SatRandomAccess::SetSlottedAlohaControlRandomizationIntervalInMilliSeconds "
                       "- Wrong random access model in use");
    }

    NS_LOG_INFO(
        "New control randomization interval : " << controlRandomizationIntervalInMilliSeconds);
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoSlottedAloha()
{
    NS_LOG_FUNCTION(this);

    RandomAccessTxOpportunities_s txOpportunity;
    txOpportunity.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;

    NS_LOG_INFO("---------------------------------------------");
    NS_LOG_INFO("------ Running Slotted ALOHA algorithm ------");
    NS_LOG_INFO("---------------------------------------------");
    NS_LOG_INFO("Slotted ALOHA control randomization interval: "
                << m_randomAccessConf->GetSlottedAlohaControlRandomizationIntervalInMilliSeconds()
                << " milliseconds");
    NS_LOG_INFO("---------------------------------------------");

    NS_LOG_INFO("Checking if we have DAMA allocations");

    /// Check if we have known DAMA allocations
    if (!m_isDamaAvailableCb())
    {
        NS_LOG_INFO("No DAMA -> Running Slotted ALOHA");

        /// Randomize Tx opportunity release time
        txOpportunity.slottedAlohaTxOpportunity = SlottedAlohaRandomizeReleaseTime();
        txOpportunity.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_SLOTTED_ALOHA;
    }

    NS_LOG_INFO("----------------------------------------------");
    NS_LOG_INFO("------ Slotted ALOHA algorithm FINISHED ------");
    NS_LOG_INFO("----------------------------------------------");

    return txOpportunity;
}

uint32_t
SatRandomAccess::SlottedAlohaRandomizeReleaseTime()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Randomizing the release time...");

    uint32_t releaseTime = m_uniformRandomVariable->GetInteger(
        0,
        m_randomAccessConf->GetSlottedAlohaControlRandomizationIntervalInMilliSeconds());

    NS_LOG_INFO("TX opportunity in the next slot after " << releaseTime << " milliseconds");

    return releaseTime;
}

///-------------------------------
/// ESSA related methods
///-------------------------------
void
SatRandomAccess::SetFSimBackoffTimeInFrames(uint32_t allocationChannel,
                                            uint32_t backoffTimeInFrames)
{
    /// NOTE: this could be done with the same functions for Crdsa (SetCrdsaBackOffTime)
    /// but changing the name of the function. The NCC should then send the backofftime
    /// in ms, and not in frames.
    NS_LOG_FUNCTION(this);

    if (m_randomAccessModel == SatEnums::RA_MODEL_ESSA)
    {
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetFSimBackoffTimeInFrames(backoffTimeInFrames);
    }
    else
    {
        NS_FATAL_ERROR(
            "SatRandomAccess::SetFSimBackoffTimeInFrames - Wrong random access model in use");
    }
}

void
SatRandomAccess::SetFSimBackoffProbability(uint32_t allocationChannel, uint16_t backoffPersistence)
{
    /// NOTE: this could be done with the same functions for Crdsa (SetCrdsaBackOffProbabaility)
    /// but changing the name of the function. The NCC should then send the backoff probability
    /// and not the persistence.
    NS_LOG_FUNCTION(this);

    if (m_randomAccessModel == SatEnums::RA_MODEL_ESSA)
    {
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetFSimBackoffProbability(backoffPersistence);
    }
    else
    {
        NS_FATAL_ERROR(
            "SatRandomAccess::SetFSimBackoffProbability - Wrong random access model in use");
    }
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoEssa(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this);

    RandomAccessTxOpportunities_s txOpportunity;

    NS_LOG_INFO("------------------------------------");
    NS_LOG_INFO("------ Running ESSA algorithm ------");
    NS_LOG_INFO("------------------------------------");

    /// TODO: take into account inter-packet time ?

    /// Calculate how many backoff slots must wait
    /// TODO: this should be done in a semi-periodic way:
    /// each time a backoff time has passed, check if we
    /// can send or not (the back-off time and probability
    /// may have changed in between).
    uint32_t numberOfBackoff = 0;
    while (m_uniformRandomVariable->GetValue(0.0, 1.0) >
           m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
               ->GetFSimBackoffProbability())
    {
        numberOfBackoff++;
    }
    txOpportunity.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_ESSA;
    txOpportunity.slottedAlohaTxOpportunity =
        numberOfBackoff * m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                              ->GetFSimBackoffTimeInMilliSeconds(); // NOTE: could rename variable

    NS_LOG_INFO("-------------------------------------");
    NS_LOG_INFO("------ ESSA algorithm FINISHED ------");
    NS_LOG_INFO("-------------------------------------");

    return txOpportunity;
}

///-----------------------
/// CRDSA related methods
///-----------------------

void
SatRandomAccess::SetCrdsaBackoffTimeInMilliSeconds(uint32_t allocationChannel,
                                                   uint32_t backoffTimeInMilliSeconds)
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA ||
        m_randomAccessModel == SatEnums::RA_MODEL_MARSALA ||
        m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaBackoffTimeInMilliSeconds(backoffTimeInMilliSeconds);

        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->DoCrdsaVariableSanityCheck();
    }
    else
    {
        NS_FATAL_ERROR("SatRandomAccess::SetCrdsaBackoffTimeInMilliSeconds - Wrong random access "
                       "model in use");
    }
}

void
SatRandomAccess::SetCrdsaBackoffProbability(uint32_t allocationChannel, uint16_t backoffProbability)
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA ||
        m_randomAccessModel == SatEnums::RA_MODEL_MARSALA ||
        m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaBackoffProbability(backoffProbability);

        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->DoCrdsaVariableSanityCheck();
    }
    else
    {
        NS_FATAL_ERROR(
            "SatRandomAccess::SetCrdsaBackoffProbability - Wrong random access model in use");
    }
}

void
SatRandomAccess::SetCrdsaRandomizationParameters(uint32_t allocationChannel,
                                                 uint32_t minRandomizationValue,
                                                 uint32_t maxRandomizationValue,
                                                 uint32_t numOfInstances)
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA ||
        m_randomAccessModel == SatEnums::RA_MODEL_MARSALA ||
        m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaMinRandomizationValue(minRandomizationValue);
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaMaxRandomizationValue(maxRandomizationValue);
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaNumOfInstances(numOfInstances);

        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->DoCrdsaVariableSanityCheck();
    }
    else
    {
        NS_FATAL_ERROR(
            "SatRandomAccess::SetCrdsaRandomizationParameters - Wrong random access model in use");
    }
}

void
SatRandomAccess::SetCrdsaMaximumDataRateLimitationParameters(uint32_t allocationChannel,
                                                             uint32_t maxUniquePayloadPerBlock,
                                                             uint32_t maxConsecutiveBlocksAccessed,
                                                             uint32_t minIdleBlocks)
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessModel == SatEnums::RA_MODEL_CRDSA ||
        m_randomAccessModel == SatEnums::RA_MODEL_MARSALA ||
        m_randomAccessModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
    {
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaMaxUniquePayloadPerBlock(maxUniquePayloadPerBlock);
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaMaxConsecutiveBlocksAccessed(maxConsecutiveBlocksAccessed);
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaMinIdleBlocks(minIdleBlocks);

        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->DoCrdsaVariableSanityCheck();
    }
    else
    {
        NS_FATAL_ERROR("SatRandomAccess::SetCrdsaMaximumDataRateLimitationParameters - Wrong "
                       "random access model in use");
    }
}

uint32_t
SatRandomAccess::GetCrdsaSignalingOverheadInBytes()
{
    NS_LOG_FUNCTION(this);

    return m_randomAccessConf->GetCrdsaSignalingOverheadInBytes();
}

bool
SatRandomAccess::CrdsaHasBackoffTimePassed(uint32_t allocationChannel) const
{
    NS_LOG_FUNCTION(this);

    bool hasCrdsaBackoffTimePassed = false;

    if ((Now() >= m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                      ->GetCrdsaBackoffReleaseTime()))
    {
        hasCrdsaBackoffTimePassed = true;
    }

    NS_LOG_INFO("For allocation channel " << allocationChannel << ": "
                                          << hasCrdsaBackoffTimePassed);

    return hasCrdsaBackoffTimePassed;
}

void
SatRandomAccess::CrdsaReduceIdleBlocks(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this);

    uint32_t idleBlocksLeft =
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->GetCrdsaIdleBlocksLeft();

    if (idleBlocksLeft > 0)
    {
        NS_LOG_INFO("Reducing allocation channel: " << allocationChannel << " idle blocks by one");
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaIdleBlocksLeft(idleBlocksLeft - 1);
    }
}

void
SatRandomAccess::CrdsaReduceIdleBlocksForAllAllocationChannels()
{
    NS_LOG_FUNCTION(this);

    for (uint32_t i = 0; i < m_numOfAllocationChannels; i++)
    {
        CrdsaReduceIdleBlocks(i);
    }
}

void
SatRandomAccess::CrdsaResetConsecutiveBlocksUsed(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this << allocationChannel);

    m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
        ->SetCrdsaNumOfConsecutiveBlocksUsed(0);
}

void
SatRandomAccess::CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels()
{
    NS_LOG_FUNCTION(this);

    for (uint32_t i = 0; i < m_numOfAllocationChannels; i++)
    {
        CrdsaResetConsecutiveBlocksUsed(i);
    }
}

bool
SatRandomAccess::CrdsaIsAllocationChannelFree(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->GetCrdsaIdleBlocksLeft() > 0)
    {
        NS_LOG_INFO("Allocation channel: " << allocationChannel << " idle in effect");
        return false;
    }
    NS_LOG_INFO("Allocation channel: " << allocationChannel << " free");
    return true;
}

bool
SatRandomAccess::CrdsaDoBackoff(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this);

    bool doCrdsaBackoff = false;

    if (m_uniformRandomVariable->GetValue(0.0, 1.0) <
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->GetCrdsaBackoffProbability())
    {
        doCrdsaBackoff = true;
    }

    NS_LOG_INFO("For allocation channel " << allocationChannel << ": " << doCrdsaBackoff);

    return doCrdsaBackoff;
}

void
SatRandomAccess::CrdsaSetBackoffTimer(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this << allocationChannel);

    uint32_t backoff = m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                           ->GetCrdsaBackoffTimeInMilliSeconds();
    m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
        ->SetCrdsaBackoffReleaseTime(Now() + MilliSeconds(backoff));
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::CrdsaPrepareToTransmit(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this);

    RandomAccessTxOpportunities_s txOpportunities;
    txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;

    uint32_t maxUniquePackets =
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->GetCrdsaMaxUniquePayloadPerBlock();

    /// TODO when multiple overlapping allocation channels for a single UT needs to be supported
    /// slots.first can be updated to take into account the reserved RA slots from MAC.
    /// For this the logic for determining the reserved slots needs to be implemented in UT MAC
    /// and the slots should be passed as parameter to RA logic and used instead of slots.first

    /// This should be done by including the list of used slots in this SF as a parameter for the
    /// random access algorithm call. This functionality is needed with, e.g., multiple allocation
    /// channels
    std::pair<std::set<uint32_t>, std::set<uint32_t>> slots;

    for (uint32_t i = 0; i < maxUniquePackets; i++)
    {
        if (CrdsaDoBackoff(allocationChannel))
        {
            CrdsaSetBackoffTimer(allocationChannel);
            break;
        }
        else
        {
            NS_LOG_INFO("New Tx candidate for allocation channel: " << allocationChannel);

            if (CrdsaIsAllocationChannelFree(allocationChannel))
            {
                NS_LOG_INFO(
                    "Preparing for transmission with allocation channel: " << allocationChannel);

                /// randomize instance slots for this unique packet
                slots = CrdsaRandomizeTxOpportunities(allocationChannel, slots);

                /// save the packet specific Tx opportunities into a vector
                txOpportunities.crdsaTxOpportunities.insert(
                    std::make_pair(*slots.second.begin(), slots.second));

                if (m_areBuffersEmptyCb())
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
SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsed(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this << allocationChannel);

    m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
        ->SetCrdsaNumOfConsecutiveBlocksUsed(
            m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                ->GetCrdsaNumOfConsecutiveBlocksUsed() +
            1);

    if (m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->GetCrdsaNumOfConsecutiveBlocksUsed() >=
        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->GetCrdsaMaxConsecutiveBlocksAccessed())
    {
        NS_LOG_INFO("Maximum number of consecutive blocks reached, forcing idle blocks for "
                    "allocation channel: "
                    << allocationChannel);

        m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
            ->SetCrdsaIdleBlocksLeft(
                m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                    ->GetCrdsaMinIdleBlocks());

        CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels();
    }
}

void
SatRandomAccess::CrdsaIncreaseConsecutiveBlocksUsedForAllAllocationChannels()
{
    NS_LOG_FUNCTION(this);

    for (uint32_t i = 0; i < m_numOfAllocationChannels; i++)
    {
        CrdsaIncreaseConsecutiveBlocksUsed(i);
    }
}

SatRandomAccess::RandomAccessTxOpportunities_s
SatRandomAccess::DoCrdsa(uint32_t allocationChannel)
{
    NS_LOG_FUNCTION(this);

    RandomAccessTxOpportunities_s txOpportunities;
    txOpportunities.txOpportunityType = SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING;

    NS_LOG_INFO("-------------------------------------");
    NS_LOG_INFO("------ Running CRDSA algorithm ------");
    NS_LOG_INFO("-------------------------------------");

    PrintVariables();

    NS_LOG_INFO("-------------------------------------");

    NS_LOG_INFO("Backoff period over, checking DAMA status...");

    if (!m_isDamaAvailableCb())
    {
        NS_LOG_INFO("No DAMA, checking buffer status...");

        if (!m_areBuffersEmptyCb())
        {
            NS_LOG_INFO("Data in buffer, continuing CRDSA");

            if (m_crdsaNewData)
            {
                m_crdsaNewData = false;

                NS_LOG_INFO("Evaluating back off...");

                if (CrdsaDoBackoff(allocationChannel))
                {
                    NS_LOG_INFO("Initial new data backoff triggered");
                    CrdsaSetBackoffTimer(allocationChannel);
                }
                else
                {
                    txOpportunities = CrdsaPrepareToTransmit(allocationChannel);
                }
            }
            else
            {
                txOpportunities = CrdsaPrepareToTransmit(allocationChannel);
            }

            if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_CRDSA)
            {
                NS_LOG_INFO("Tx opportunity, increasing consecutive blocks used");

                CrdsaIncreaseConsecutiveBlocksUsedForAllAllocationChannels();
            }
            else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_DO_NOTHING)
            {
                NS_LOG_INFO(
                    "No Tx opportunity, reducing idle blocks & resetting consecutive blocks");

                CrdsaReduceIdleBlocksForAllAllocationChannels();
                CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels();
            }
        }
        else
        {
            NS_LOG_INFO("Empty buffer, reducing idle blocks & resetting consecutive blocks, "
                        "aborting CRDSA...");

            CrdsaReduceIdleBlocksForAllAllocationChannels();
            CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels();
        }
    }
    else
    {
        NS_LOG_INFO("DAMA allocation found, aborting CRDSA...");

        CrdsaReduceIdleBlocksForAllAllocationChannels();
        CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels();
    }

    NS_LOG_INFO("--------------------------------------");
    NS_LOG_INFO("------ CRDSA algorithm FINISHED ------");
    NS_LOG_INFO("------ Result: " << SatEnums::GetRandomAccessOpportunityTypeName(
                                         txOpportunities.txOpportunityType)
                                  << " ---------------------");
    NS_LOG_INFO("--------------------------------------");

    return txOpportunities;
}

std::pair<std::set<uint32_t>, std::set<uint32_t>>
SatRandomAccess::CrdsaRandomizeTxOpportunities(
    uint32_t allocationChannel,
    std::pair<std::set<uint32_t>, std::set<uint32_t>> slots)
{
    NS_LOG_FUNCTION(this);

    std::pair<std::set<uint32_t>::iterator, bool> resultAllSlotsInFrame;
    std::pair<std::set<uint32_t>::iterator, bool> resultThisUniquePacket;

    std::set<uint32_t> emptySet;
    slots.second = emptySet;

    NS_LOG_INFO("Randomizing TX opportunities for allocation channel: " << allocationChannel);

    uint32_t successfulInserts = 0;
    uint32_t instances = m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                             ->GetCrdsaNumOfInstances();
    while (successfulInserts < instances)
    {
        uint32_t slot = m_uniformRandomVariable->GetInteger(
            m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                ->GetCrdsaMinRandomizationValue(),
            m_randomAccessConf->GetAllocationChannelConfiguration(allocationChannel)
                ->GetCrdsaMaxRandomizationValue());

        resultAllSlotsInFrame = slots.first.insert(slot);

        if (resultAllSlotsInFrame.second)
        {
            successfulInserts++;

            resultThisUniquePacket = slots.second.insert(slot);

            if (!resultAllSlotsInFrame.second)
            {
                NS_FATAL_ERROR("SatRandomAccess::CrdsaRandomizeTxOpportunities - Slots out of "
                               "sync, this should never happen");
            }
        }

        NS_LOG_INFO("Allocation channel: "
                    << allocationChannel << " insert successful " << resultAllSlotsInFrame.second
                    << " for TX opportunity slot: " << (*resultAllSlotsInFrame.first));
    }

    NS_LOG_INFO("Randomizing done");

    return slots;
}

void
SatRandomAccess::SetIsDamaAvailableCallback(SatRandomAccess::IsDamaAvailableCallback callback)
{
    NS_LOG_FUNCTION(this << &callback);

    m_isDamaAvailableCb = callback;
}

void
SatRandomAccess::SetAreBuffersEmptyCallback(SatRandomAccess::AreBuffersEmptyCallback callback)
{
    NS_LOG_FUNCTION(this << &callback);

    m_areBuffersEmptyCb = callback;
}

} // namespace ns3
