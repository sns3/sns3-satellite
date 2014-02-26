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
 * \brief Class for random access allocation channel configuration
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
   *
   * \return
   */
  uint32_t GetCrdsaBackoffTime () { return m_crdsaBackoffTime; }

  /**
   *
   * \param crdsaBackoffTime
   */
  void SetCrdsaBackoffTime (uint32_t crdsaBackoffTime) { m_crdsaBackoffTime = crdsaBackoffTime; }

  /**
   *
   * \return
   */
  double GetCrdsaBackoffReleaseTime () { return m_crdsaBackoffReleaseTime; }

  /**
   *
   * \param crdsaBackoffReleaseTime
   */
  void SetCrdsaBackoffReleaseTime (double crdsaBackoffReleaseTime) { m_crdsaBackoffReleaseTime = crdsaBackoffReleaseTime; }

  /**
   *
   * \return
   */
  double GetCrdsaBackoffProbability () { return m_crdsaBackoffProbability; }

  /**
   *
   * \param crdsaBackoffProbability
   */
  void SetCrdsaBackoffProbability (double crdsaBackoffProbability) { m_crdsaBackoffProbability = crdsaBackoffProbability; }

  /**
   *
   * \return
   */
  double GetCrdsaMaximumBackoffProbability () { return m_crdsaMaximumBackoffProbability; }

  /**
   *
   * \return
   */
  void SetCrdsaMaximumBackoffProbability (double crdsaMaximumBackoffProbability) { m_crdsaMaximumBackoffProbability = crdsaMaximumBackoffProbability; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaMinRandomizationValue () { return m_crdsaMinRandomizationValue; }

  /**
   *
   * \return
   */
  void SetCrdsaMinRandomizationValue (uint32_t minRandomizationValue) { m_crdsaMinRandomizationValue = minRandomizationValue; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaMaxRandomizationValue () { return m_crdsaMaxRandomizationValue; }

  /**
   *
   * \return
   */
  void SetCrdsaMaxRandomizationValue (uint32_t maxRandomizationValue) { m_crdsaMaxRandomizationValue = maxRandomizationValue; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaNumOfInstances () { return m_crdsaNumOfInstances; }

  /**
   *
   * \return
   */
  void SetCrdsaNumOfInstances (uint32_t numOfInstances) { m_crdsaNumOfInstances = numOfInstances; }

  /**
   *
   */
  uint32_t GetCrdsaMinIdleBlocks () { return m_crdsaMinIdleBlocks; }

  /**
   *
   */
  void SetCrdsaMinIdleBlocks (uint32_t minIdleBlocks) { m_crdsaMinIdleBlocks = minIdleBlocks; }

  /**
   *
   */
  uint32_t GetCrdsaIdleBlocksLeft () { return m_crdsaIdleBlocksLeft; }

  /**
   *
   */
  void SetCrdsaIdleBlocksLeft (uint32_t idleBlocksLeft) { m_crdsaIdleBlocksLeft = idleBlocksLeft; }

  /**
   *
   */
  uint32_t GetCrdsaMaxUniquePayloadPerBlock () { return m_crdsaMaxUniquePayloadPerBlock; }

  /**
   *
   */
  void SetCrdsaMaxUniquePayloadPerBlock (uint32_t maxUniquePayloadPerBlock) { m_crdsaMaxUniquePayloadPerBlock = maxUniquePayloadPerBlock; }

  /**
   *
   */
  uint32_t GetCrdsaMaxConsecutiveBlocksAccessed () { return m_crdsaMaxConsecutiveBlocksAccessed; }

  /**
   *
   */
  void SetCrdsaMaxConsecutiveBlocksAccessed (uint32_t maxConsecutiveBlocksAccessed) { m_crdsaMaxConsecutiveBlocksAccessed = maxConsecutiveBlocksAccessed; }

  /**
   *
   */
  uint32_t GetCrdsaNumOfConsecutiveBlocksUsed () { return m_crdsaNumOfConsecutiveBlocksUsed; }

  /**
   *
   */
  void SetCrdsaNumOfConsecutiveBlocksUsed (uint32_t numOfConsecutiveBlocksUsed) { m_crdsaNumOfConsecutiveBlocksUsed = numOfConsecutiveBlocksUsed; }

  /**
   *
   */
  uint32_t GetCrdsaPayloadBytes () { return m_crdsaPayloadBytes; }

  /**
   *
   */
  void SetCrdsaPayloadBytes (uint32_t payloadBytes) { m_crdsaPayloadBytes = payloadBytes; }

  /**
   *
   */
  void DoCrdsaVariableSanityCheck ();

private:

  /**
   *
   */
  uint32_t m_crdsaMinRandomizationValue;

  /**
   *
   */
  uint32_t m_crdsaMaxRandomizationValue;

  /**
   *
   */
  uint32_t m_crdsaNumOfInstances;

  /**
   *
   */
  uint32_t m_crdsaMinIdleBlocks;

  /**
   *
   */
  uint32_t m_crdsaIdleBlocksLeft;

  /**
   *
   */
  uint32_t m_crdsaBackoffTime;

  /**
   *
   */
  double m_crdsaBackoffProbability;

  /**
   *
   */
  double m_crdsaMaximumBackoffProbability;

  /**
   *
   */
  uint32_t m_crdsaMaxUniquePayloadPerBlock;

  /**
   *
   */
  uint32_t m_crdsaMaxConsecutiveBlocksAccessed;

  /**
   *
   */
  uint32_t m_crdsaNumOfConsecutiveBlocksUsed;

  /**
   *
   */
  double m_crdsaBackoffReleaseTime;

  /**
   *
   */
  uint32_t m_crdsaPayloadBytes;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_ALLOCATION_CHANNEL_H */
