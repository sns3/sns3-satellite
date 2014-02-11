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
#ifndef SATELLITE_RANDOM_ACCESS_REQUEST_CLASS_H
#define SATELLITE_RANDOM_ACCESS_REQUEST_CLASS_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access request class configuration
 */
class SatRandomAccessRequestClass : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatRandomAccessRequestClass ();

  /**
   *
   * \param minRandomizationValue
   * \param maxRandomizationValue
   * \param numOfInstances
   * \param maxUniquePayloadPerBlock
   * \param maxConsecutiveBlocksAccessed
   * \param minIdleBlocks
   * \param payloadBytes
   */
  SatRandomAccessRequestClass (uint32_t minRandomizationValue,
                               uint32_t maxRandomizationValue,
                               uint32_t numOfInstances,
                               uint32_t minIdleBlocks,
                               uint32_t payloadBytes);

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccessRequestClass ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *
   * \return
   */
  uint32_t GetMinRandomizationValue () { return m_minRandomizationValue; }

  /**
   *
   * \return
   */
  void SetMinRandomizationValue (uint32_t minRandomizationValue) { m_minRandomizationValue = minRandomizationValue; }

  /**
   *
   * \return
   */
  uint32_t GetMaxRandomizationValue () { return m_maxRandomizationValue; }

  /**
   *
   * \return
   */
  void SetMaxRandomizationValue (uint32_t maxRandomizationValue) { m_maxRandomizationValue = maxRandomizationValue; }

  /**
   *
   * \return
   */
  uint32_t GetNumOfInstances () { return m_numOfInstances; }

  /**
   *
   * \return
   */
  void SetNumOfInstances (uint32_t numOfInstances) { m_numOfInstances = numOfInstances; }

  /**
   *
   */
  uint32_t GetMinIdleBlocks () { return m_minIdleBlocks; }

  /**
   *
   */
  void SetMinIdleBlocks (uint32_t minIdleBlocks) { m_minIdleBlocks = minIdleBlocks; }

  /**
   *
   */
  uint32_t GetPayloadBytes () { return m_payloadBytes; }

  /**
   *
   */
  void SetPayloadBytes (uint32_t  payloadBytes) { m_payloadBytes = payloadBytes; }

  /**
   *
   */
  uint32_t GetIdleBlocksLeft () { return m_idleBlocksLeft; }

  /**
   *
   */
  void SetIdleBlocksLeft (uint32_t idleBlocksLeft) { m_idleBlocksLeft = idleBlocksLeft; }

private:

  /**
   *
   */
  void DoVariableSanityCheck ();

  /**
   *
   */
  uint32_t m_minRandomizationValue;

  /**
   *
   */
  uint32_t m_maxRandomizationValue;

  /**
   *
   */
  uint32_t m_numOfInstances;

  /**
   *
   */
  uint32_t m_minIdleBlocks;

  /**
   *
   */
  uint32_t m_payloadBytes;

  /**
   *
   */
  uint32_t m_idleBlocksLeft;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_REQUEST_CLASS_H */
