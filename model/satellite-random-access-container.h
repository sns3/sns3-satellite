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
#include "satellite-slotted-aloha.h"
#include "satellite-crdsa.h"
#include "satellite-random-access-container-conf.h"

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
   * \param isFrameStart
   */
  void DoRandomAccess (bool isFrameStart);

private:

  /**
   *
   */
  void DoSlottedAloha ();

  /**
   *
   */
  void DoCrdsa ();

  /**
   *
   * \return
   */
  bool IsDamaAvailable ();

  /**
   *
   * \return
   */
  bool IsDataAvailable ();

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
  Ptr<SatSlottedAloha> m_slottedAlohaModel;

  /**
   *
   */
  Ptr<SatCrdsa> m_crdsaModel;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_H */
