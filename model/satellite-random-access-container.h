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
#ifndef SATELLITE_RANDOM_ACCESS_H
#define SATELLITE_RANDOM_ACCESS_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-random-access-container-conf.h"
#include "ns3/random-variable-stream.h"
#include <set>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access
 */
class SatRandomAccess : public Object
{
public:

  /**
   * \enum RandomAccessModel_t
   * \brief Random access models
   */
  typedef enum
  {
    RA_OFF = 0,
    RA_SLOTTED_ALOHA = 1,
    RA_CRDSA = 2,
    RA_ANY_AVAILABLE = 3
  } RandomAccessModel_t;

  /**
   * \enum RandomAccessResultType_t
   * \brief Random access result types
   */
  typedef enum
  {
    RA_DO_NOTHING = 0,
    RA_SLOTTED_ALOHA_TX_OPPORTUNITY = 1,
    RA_CRDSA_TX_OPPORTUNITY = 2,
  } RandomAccessResultType_t;

  /**
   * \struct RandomAccessResults_s
   * \brief Random access results
   */
  typedef struct
  {
    RandomAccessResultType_t resultType;
    uint32_t slottedAlohaResult;
    std::map<uint32_t,std::set<uint32_t> > crdsaResult;
  } RandomAccessResults_s;

  /**
   * \brief Constructor
   */
  SatRandomAccess ();

  /**
   * \brief Constructor
   */
  SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, RandomAccessModel_t randomAccessModel);

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccess ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *
   * \param randomAccessModel
   */
  void SetRandomAccessModel (RandomAccessModel_t randomAccessModel);

  /**
   *
   * \param allocationChannel
   * \param crdsaBackoffProbability
   * \param backoffTime
   */
  void CrdsaSetLoadControlParameters (uint32_t allocationChannel,
                                      double backoffProbability,
                                      uint32_t backoffTime);

  /**
   *
   * \param allocationChannel
   * \param maximumBackoffProbability
   */
  void CrdsaSetMaximumBackoffProbability (uint32_t allocationChannel,
                                          double maximumBackoffProbability);

  /**
   *
   * \param allocationChannel
   * \param minRandomizationValue
   * \param maxRandomizationValue
   * \param numOfInstances
   */
  void CrdsaSetRandomizationParameters (uint32_t allocationChannel,
                                        uint32_t minRandomizationValue,
                                        uint32_t maxRandomizationValue,
                                        uint32_t numOfInstances);

  /**
   *
   * \param allocationChannel
   * \param maxUniquePayloadPerBlock
   * \param maxConsecutiveBlocksAccessed
   * \param minIdleBlocks
   */
  void CrdsaSetMaximumDataRateLimitationParameters (uint32_t allocationChannel,
                                                    uint32_t maxUniquePayloadPerBlock,
                                                    uint32_t maxConsecutiveBlocksAccessed,
                                                    uint32_t minIdleBlocks);

  /**
   *
   * \param controlRandomizationInterval
   */
  void SlottedAlohaSetControlRandomizationInterval (double controlRandomizationInterval);

  /**
   *
   * \return
   */
  SatRandomAccess::RandomAccessResults_s DoRandomAccess (uint32_t allocationChannel);

private:

  /**
   *
   * \return
   */
  bool IsFrameStart ();

  /**
   *
   * \return
   */
  bool IsDamaAvailable ();

  /**
   *
   * \return
   */
  bool AreBuffersEmpty ();

  /**
   *
   * \param allocationChannel
   * \return
   */
  bool IsCrdsaAllocationChannelFree (uint32_t allocationChannel);

  /**
   *
   * \param allocationChannel
   * \return
   */
  bool IsCrdsaBackoffProbabilityTooHigh (uint32_t allocationChannel);

  /**
   *
   */
  void PrintVariables ();

  /**
   *
   * \return
   */
  SatRandomAccess::RandomAccessResults_s DoSlottedAloha ();

  /**
    *
    * \return
    */
  uint32_t SlottedAlohaRandomizeReleaseTime ();

  /**
   *
   */
  void SlottedAlohaDoVariableSanityCheck ();

  /**
   *
   * \return
   */
  SatRandomAccess::RandomAccessResults_s DoCrdsa (uint32_t allocationChannel);

  /**
   *
   * \return
   */
  bool CrdsaHasBackoffTimePassed (uint32_t allocationChannel);

  /**
   *
   * \return
   */
  bool CrdsaDoBackoff (uint32_t allocationChannel);

  /**
   *
   * \param requestClass
   * \param txOpportunities
   * \return
   */
  std::set<uint32_t> CrdsaRandomizeTxOpportunities (uint32_t allocationChannel, std::set<uint32_t> txOpportunities);

  /**
   *
   * \return
   */
  SatRandomAccess::RandomAccessResults_s CrdsaPrepareToTransmit (uint32_t allocationChannel);

  /**
   *
   */
  void CrdsaSetBackoffTimer (uint32_t allocationChannel);

  /**
   *
   */
  void CrdsaIncreaseConsecutiveBlocksUsed (uint32_t allocationChannel);

  /**
   *
   */
  void CrdsaReduceIdleBlocks (uint32_t allocationChannel);

  /**
   *
   */
  void CrdsaReduceIdleBlocksForAllAllocationChannels ();

  /**
   *
   * \return
   */
  bool CrdsaIsAllocationChannelFree (uint32_t allocationChannel);

  /**
   *
   */
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /**
   *
   */
  RandomAccessModel_t m_randomAccessModel;

  /**
   *
   */
  Ptr<SatRandomAccessConf> m_randomAccessConf;

  /**
   *
   */
  uint32_t m_numOfAllocationChannels;

  /**
   *
   */
  bool m_crdsaNewData;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_H */
