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
#ifndef SATELLITE_RANDOM_ACCESS_ALLOCATION_CHANNEL_H
#define SATELLITE_RANDOM_ACCESS_ALLOCATION_CHANNEL_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access allocation channel configuration.
 * This class holds the allocation channel specific variables and
 * the configuration parameters used by the random access module
 */
class SatRandomAccessAllocationChannel : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatRandomAccessAllocationChannel ();

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccessAllocationChannel ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for getting the CRDSA backoff time
   * \return CRDSA  backoff time
   */
  uint32_t GetCrdsaBackoffTimeInMilliSeconds ()
  {
    return m_crdsaBackoffTimeInMilliSeconds;
  }

  /**
   * \brief Function for setting the CRDSA backoff time
   * \param crdsaBackoffTimeInMilliSeconds CRDSA backoff time in milliseconds
   */
  void SetCrdsaBackoffTimeInMilliSeconds (uint32_t crdsaBackoffTimeInMilliSeconds)
  {
    m_crdsaBackoffTimeInMilliSeconds = crdsaBackoffTimeInMilliSeconds;
  }

  /**
   * \brief Function for getting the CRDSA backoff release time
   * \return CRDSA backoff release time
   */
  Time GetCrdsaBackoffReleaseTime ()
  {
    return m_crdsaBackoffReleaseTime;
  }

  /**
   * \brief Function for setting the CRDSA backoff release time
   * \param crdsaBackoffReleaseTime CRDSA  backoff time
   */
  void SetCrdsaBackoffReleaseTime (Time crdsaBackoffReleaseTime)
  {
    m_crdsaBackoffReleaseTime = crdsaBackoffReleaseTime;
  }

  /**
   * \brief Function for getting the CRDSA backoff probability
   * \return CRDSA backoff probability
   */
  double GetCrdsaBackoffProbability ()
  {
    return m_crdsaBackoffProbability;
  }

  /**
   * \brief Function for setting the CRDSA backoff probability
   * \param crdsaBackoffProbability CRDSA backoff probability
   */
  void SetCrdsaBackoffProbability (uint16_t crdsaBackoffProbability)
  {
    m_crdsaBackoffProbability = (crdsaBackoffProbability + 1) * (1 / (std::pow (2,16)));
  }

  /**
   * \brief Function for getting the CRDSA minimum randomization value
   * \return CRDSA minimum randomization value
   */
  uint32_t GetCrdsaMinRandomizationValue ()
  {
    return m_crdsaMinRandomizationValue;
  }

  /**
   * \brief Function for setting the CRDSA minimum randomization value
   * \param minRandomizationValue CRDSA minimum randomization value
   */
  void SetCrdsaMinRandomizationValue (uint32_t minRandomizationValue)
  {
    m_crdsaMinRandomizationValue = minRandomizationValue;
  }

  /**
   * \brief Function for getting the CRDSA maximum randomization value
   * \return CRDSA maximum randomization value
   */
  uint32_t GetCrdsaMaxRandomizationValue ()
  {
    return m_crdsaMaxRandomizationValue;
  }

  /**
   * \brief Function for setting the CRDSA maximum randomization value
   * \param maxRandomizationValue CRDSA maximum randomization value
   */
  void SetCrdsaMaxRandomizationValue (uint32_t maxRandomizationValue)
  {
    m_crdsaMaxRandomizationValue = maxRandomizationValue;
  }

  /**
   * \brief Function for getting the CRDSA number of packet instances (replicas)
   * \return CRDSA number of instances
   */
  uint32_t GetCrdsaNumOfInstances ()
  {
    return m_crdsaNumOfInstances;
  }

  /**
   * \brief Function for setting the CRDSA number of packet instances (replicas)
   * \param numOfInstances CRDSA number of instances
   */
  void SetCrdsaNumOfInstances (uint32_t numOfInstances)
  {
    m_crdsaNumOfInstances = numOfInstances;
  }

  /**
   * \brief Function for getting the CRDSA minimum number of idle blocks
   * \return CRDSA minimum idle blocks
   */
  uint32_t GetCrdsaMinIdleBlocks ()
  {
    return m_crdsaMinIdleBlocks;
  }

  /**
   * \brief Function for setting the CRDSA minimum number of idle blocks
   * \param minIdleBlocks CRDSA minimum idle blocks
   */
  void SetCrdsaMinIdleBlocks (uint32_t minIdleBlocks)
  {
    m_crdsaMinIdleBlocks = minIdleBlocks;
  }

  /**
   * \brief Function for getting the CRDSA number of idle blocks left
   * \return CRDSA number of idle blocks left
   */
  uint32_t GetCrdsaIdleBlocksLeft ()
  {
    return m_crdsaIdleBlocksLeft;
  }

  /**
   * \brief Function for setting the CRDSA number of idle blocks left
   * \param idleBlocksLeft CRDSA number of idle blocks left
   */
  void SetCrdsaIdleBlocksLeft (uint32_t idleBlocksLeft)
  {
    m_crdsaIdleBlocksLeft = idleBlocksLeft;
  }

  /**
   * \brief Function for getting the CRDSA maximum number of unique payloads per block
   * \return CRDSA maximum number of unique payloads per block
   */
  uint32_t GetCrdsaMaxUniquePayloadPerBlock ()
  {
    return m_crdsaMaxUniquePayloadPerBlock;
  }

  /**
   * \brief Function for setting the CRDSA maximum number of unique payloads per block
   * \param maxUniquePayloadPerBlock CRDSA maximum number of unique payloads per block
   */
  void SetCrdsaMaxUniquePayloadPerBlock (uint32_t maxUniquePayloadPerBlock)
  {
    m_crdsaMaxUniquePayloadPerBlock = maxUniquePayloadPerBlock;
  }

  /**
   * \brief Function for getting the CRDSA number of consecutive blocks accessed
   * \return CRDSA number of consecutive blocks accessed
   */
  uint32_t GetCrdsaMaxConsecutiveBlocksAccessed ()
  {
    return m_crdsaMaxConsecutiveBlocksAccessed;
  }

  /**
   * \brief Function for setting the CRDSA maximum number of consecutive blocks accessed
   * \param maxConsecutiveBlocksAccessed CRDSA maximum number of consecutive blocks accessed
   */
  void SetCrdsaMaxConsecutiveBlocksAccessed (uint32_t maxConsecutiveBlocksAccessed)
  {
    m_crdsaMaxConsecutiveBlocksAccessed = maxConsecutiveBlocksAccessed;
  }

  /**
   * \brief Function for getting the CRDSA number of consecutive blocks used
   * \return CRDSA number of consecutive blocks used
   */
  uint32_t GetCrdsaNumOfConsecutiveBlocksUsed ()
  {
    return m_crdsaNumOfConsecutiveBlocksUsed;
  }

  /**
   * \brief Function for setting the CRDSA number of consecutive blocks used
   * \param numOfConsecutiveBlocksUsed CRDSA number of consecutive blocks used
   */
  void SetCrdsaNumOfConsecutiveBlocksUsed (uint32_t numOfConsecutiveBlocksUsed)
  {
    m_crdsaNumOfConsecutiveBlocksUsed = numOfConsecutiveBlocksUsed;
  }

  /**
   * \brief Function for checking the CRDSA parameter sanity
   */
  void DoCrdsaVariableSanityCheck ();

private:
  /**
   * \brief CRDSA minimum randomization value
   */
  uint32_t m_crdsaMinRandomizationValue;

  /**
   * \brief CRDSA maximum randomization value
   */
  uint32_t m_crdsaMaxRandomizationValue;

  /**
   * \brief CRDSA number of packet instances (replicas)
   */
  uint32_t m_crdsaNumOfInstances;

  /**
   * \brief CRDSA minimum number of idle blocks
   */
  uint32_t m_crdsaMinIdleBlocks;

  /**
   * \brief CRDSA number of idle blocks left
   */
  uint32_t m_crdsaIdleBlocksLeft;

  /**
   * \brief CRDSA backoff time
   */
  uint32_t m_crdsaBackoffTimeInMilliSeconds;

  /**
   * \brief CRDSA backoff probability
   */
  double m_crdsaBackoffProbability;

  /**
   * \brief CRDSA maximum number of unique payloads per block
   */
  uint32_t m_crdsaMaxUniquePayloadPerBlock;

  /**
   * \brief CRDSA maximum number of consecutive blocks accessed
   */
  uint32_t m_crdsaMaxConsecutiveBlocksAccessed;

  /**
   * \brief CRDSA number of consecutive blocks used
   */
  uint32_t m_crdsaNumOfConsecutiveBlocksUsed;

  /**
   * \brief CRDSA backoff release time
   */
  Time m_crdsaBackoffReleaseTime;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_ALLOCATION_CHANNEL_H */
