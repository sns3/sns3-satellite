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
  uint32_t GetCrdsaDefaultMinRandomizationValue () { return m_crdsaMinRandomizationValue; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaDefaultMaxRandomizationValue () { return m_crdsaMaxRandomizationValue; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaDefaultNumOfInstances () { return m_crdsaNumOfInstances; }

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
   */
  uint32_t GetCrdsaDefaultMaxUniquePayloadPerBlock () { return m_crdsaMaxUniquePayloadPerBlock; }

  /**
   *
   */
  uint32_t GetCrdsaDefaultMaxConsecutiveBlocksAccessed () { return m_crdsaMaxConsecutiveBlocksAccessed; }

  /**
   *
   */
  uint32_t GetCrdsaDefaultMinIdleBlocks () { return m_crdsaMinIdleBlocks; }

  /**
   *
   */
  uint32_t GetCrdsaDefaultPayloadBytes () { return m_crdsaPayloadBytes; }

  /**
   *
   * \return
   */
  double GetMaximumCrdsaBackoffProbability () { return m_raLogicMaximumCrdsaBackoffProbability; }

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
  uint32_t m_crdsaBackoffTime;

  /**
   *
   */
  double m_crdsaBackoffProbability;

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
  uint32_t m_crdsaMinIdleBlocks;

  /**
   *
   */
  uint32_t m_crdsaPayloadBytes;

  /**
   *
   */
  double m_raLogicMaximumCrdsaBackoffProbability;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_CONF_H */
