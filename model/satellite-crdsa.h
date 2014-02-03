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
#ifndef SATELLITE_CRDSA_H
#define SATELLITE_CRDSA_H

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
 * \brief Class for CRDSA
 */
class SatCrdsa : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatCrdsa ();

  /**
   * \brief Constructor
   */
  SatCrdsa (Ptr<SatRandomAccessConf> randomAccessConf);

  /**
   * \brief Destructor
   */
  virtual ~SatCrdsa ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *
   * \return
   */
  std::set<uint32_t> DoCrdsa ();

  /**
   *
   * \param min
   * \param max
   * \param setSize
   */
  void UpdateRandomizationVariables (uint32_t min, uint32_t max, uint32_t numOfInstances);

  /**
   *
   * \param backoffProbability
   */
  void SetBackoffProbability (double backoffProbability);

  /**
   *
   * \param backoffTime
   */
  void SetBackoffTime (double backoffTime);

private:

  /**
   *
   * \return
   */
  std::set<uint32_t> RandomizeTxOpportunities ();

  /**
   *
   * \return
   */
  std::set<uint32_t> PrepareToTransmit ();

  /**
   *
   * \return
   */
  bool IsDamaAvailable ();

  /**
   *
   * \return
   */
  bool HasBackoffTimePassed ();

  /**
   *
   * \return
   */
  bool DoBackoff ();

  /**
   *
   * \return
   */
  bool AreBuffersEmpty ();

  /**
   *
   */
  void UpdateMaximumRateLimitationParameters ();

  /**
   *
   */
  void CheckMaximumRateLimitations ();

  /**
   *
   */
  void SetBackoffTimer ();

  /**
   *
   */
  void DoVariableSanityCheck ();

  /**
   *
   */
  void InitializeVariables ();

  /**
   *
   */
  void PrintVariables ();

  /**
   *
   */
  Ptr<SatRandomAccessConf> m_randomAccessConf;

  /**
   *
   */
  Ptr<UniformRandomVariable> m_uniformVariable;

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
  bool m_newData;

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
  double m_crdsaBackoffProbability;
};

} // namespace ns3

#endif /* SATELLITE_CRDSA_H */
