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
#ifndef SATELLITE_RANDOM_ACCESS_CONF_H
#define SATELLITE_RANDOM_ACCESS_CONF_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-random-access-request-class.h"
#include <map>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access configuration
 */
class SatRandomAccessConf : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatRandomAccessConf ();

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccessConf ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *
   * \return
   */
  double GetSlottedAlohaDefaultMinRandomizationValue () { return m_slottedAlohaMinRandomizationValue; }

  /**
   *
   * \return
   */
  double GetSlottedAlohaDefaultMaxRandomizationValue () { return m_slottedAlohaMaxRandomizationValue; }

  /**
   *
   * \return
   */
  double GetCrdsaDefaultBackoffTime () { return m_crdsaBackoffTime; }

  /**
   *
   * \return
   */
  double GetCrdsaDefaultBackoffProbability () { return m_crdsaBackoffProbability; }

  /**
   *
   * \return
   */
  double GetMaximumCrdsaBackoffProbability () { return m_raLogicMaximumCrdsaBackoffProbability; }

  /**
   *
   * \return
   */
  Ptr<SatRandomAccessRequestClass> GetRequestClassConfiguration (uint32_t requestClass);

  /**
   *
   */
  uint32_t GetMaxUniquePayloadPerBlock () { return m_crdsaMaxUniquePayloadPerBlock; }

  /**
   *
   */
  void SetMaxUniquePayloadPerBlock (uint32_t maxUniquePayloadPerBlock) { m_crdsaMaxUniquePayloadPerBlock = maxUniquePayloadPerBlock; }

  /**
   *
   */
  uint32_t GetMaxConsecutiveBlocksAccessed () { return m_crdsaMaxConsecutiveBlocksAccessed; }

  /**
   *
   */
  void SetMaxConsecutiveBlocksAccessed (uint32_t maxConsecutiveBlocksAccessed) { m_crdsaMaxConsecutiveBlocksAccessed = maxConsecutiveBlocksAccessed; }

  /**
   *
   */
  uint32_t GetNumOfConsecutiveBlocksUsed () { return m_crdsaNumOfConsecutiveBlocksUsed; }

  /**
   *
   */
  void SetNumOfConsecutiveBlocksUsed (uint32_t numOfConsecutiveBlocksUsed) { m_crdsaNumOfConsecutiveBlocksUsed = numOfConsecutiveBlocksUsed; }

private:

  /**
   *
   */
  double m_slottedAlohaMinRandomizationValue;

  /**
   *
   */
  double m_slottedAlohaMaxRandomizationValue;

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
  double m_raLogicMaximumCrdsaBackoffProbability;

  /**
   *
   */
  std::map<uint32_t,Ptr<SatRandomAccessRequestClass> > m_requestClassConf;

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
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_CONF_H */
