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
    double slottedAlohaResult;
    std::map<uint32_t,std::set<uint32_t> > crdsaResult;
  } RandomAccessResults_s;

  /**
   * \brief Constructor
   */
  SatRandomAccess ();

  /**
   * \brief Constructor
   */
  SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, RandomAccessModel_t randomAccessModel, uint32_t numOfRequestClasses);

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
   * \param min
   * \param max
   */
  void SlottedAlohaUpdateVariables (double min, double max);

  /**
   *
   * \return
   */
  SatRandomAccess::RandomAccessResults_s DoRandomAccess ();

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
  SatRandomAccess::RandomAccessResults_s DoSlottedAloha ();

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
  SatRandomAccess::RandomAccessResults_s DoCrdsa ();

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
  std::set<uint32_t> CrdsaRandomizeTxOpportunities (uint32_t requestClass, std::set<uint32_t> txOpportunities);

  /**
   *
   * \return
   */
  SatRandomAccess::RandomAccessResults_s CrdsaPrepareToTransmit ();

  /**
   *
   */
  void CrdsaSetBackoffTimer ();

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
   * \param requestClass
   */
  void CrdsaReduceIdleBlocks (uint32_t requestClass);

  /**
   *
   */
  void CrdsaReduceIdleBlocksFromAllRequestClasses ();

  /**
   *
   * \param requestClass
   * \return
   */
  bool CrdsaIsRequestClassFree (uint32_t requestClass);

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
  uint32_t m_numOfRequestClasses;

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
  bool m_crdsaNewData;

  /**
   *
   */
  double m_crdsaBackoffReleaseTime;

  /**
   *
   */
  double m_crdsaBackoffTime;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_H */
