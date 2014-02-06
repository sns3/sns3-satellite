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
   * \param min
   * \param max
   * \param setSize
   */
  void CrdsaUpdateRandomizationVariables (uint32_t min, uint32_t max, uint32_t numOfInstances, uint32_t maxUniquePayloadPerBlock);

  /**
   *
   * \param backoffProbability
   */
  void CrdsaSetBackoffProbability (double backoffProbability);

  /**
   *
   * \param backoffTime
   */
  void CrdsaSetBackoffTime (double backoffTime);

  /**
   *
   * \return
   */
  uint32_t CrdsaGetNumOfInstances () { return m_crdsaNumOfInstances; }

  /**
   *
   * \return
   */
  uint32_t CrdsaGetMaxUniquePayloadPerBlock () { return m_crdsaMaxUniquePayloadPerBlock; }

  /**
   *
   * \return
   */
  uint32_t CrdsaGetMaxPacketSize () { return m_crdsaMaxPacketSize; }

  /**
   *
   * \param min
   * \param max
   */
  void SlottedAlohaUpdateVariables (double min, double max);

  /**
   *
   */
  void DoRandomAccess ();

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
   * \return
   */
  bool IsCrdsaFree ();

  /**
   *
   */
  void PrintVariables ();

  /**
   *
   * \return
   */
  double DoSlottedAloha ();

  /**
    *
    * \return
    */
  double SlottedAlohaRandomizeReleaseTime ();

  /**
   *
   */
  void SlottedAlohaDoVariableSanityCheck ();

  /**
   *
   * \return
   */
  std::set<uint32_t> DoCrdsa ();

  /**
   *
   * \return
   */
  bool CrdsaHasBackoffTimePassed ();

  /**
   *
   * \return
   */
  bool CrdsaDoBackoff ();

  /**
   *
   * \return
   */
  std::set<uint32_t> CrdsaRandomizeTxOpportunities ();

  /**
   *
   * \return
   */
  std::set<uint32_t> CrdsaPrepareToTransmit ();

  /**
   *
   */
  void CrdsaSetInitialBackoffTimer ();

  /**
   *
   */
  void CrdsaDoVariableSanityCheck ();

  /**
   *
   */
  void CrdsaIncreaseConsecutiveBlocksUsed ();

  /**
   *
   */
  void CrdsaUpdateIdleBlocks ();

  /**
   *
   */
  Ptr<UniformRandomVariable> m_uniformVariable;

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
  double m_slottedAlohaMinRandomizationValue;

  /**
   *
   */
  double m_slottedAlohaMaxRandomizationValue;

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
  bool m_crdsaNewData;

  /**
   *
   */
  double m_crdsaBackoffReleaseTime;

  /**
   *
   */
  double m_crdsaBackoffTime;

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
  uint32_t m_crdsaIdleBlocksLeft;

  /**
   *
   */
  uint32_t m_crdsaNumOfConsecutiveBlocksUsed;

  /**
   *
   */
  uint32_t m_crdsaMaxPacketSize;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_H */
