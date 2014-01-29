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
#ifndef SATELLITE_SLOTTED_ALOHA_H
#define SATELLITE_SLOTTED_ALOHA_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-random-access-container-conf.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for slotted aloha
 */
class SatSlottedAloha : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatSlottedAloha ();

  /**
   * \brief Constructor
   */
  SatSlottedAloha (Ptr<SatRandomAccessConf> randomAccessConf);

  /**
   * \brief Destructor
   */
  virtual ~SatSlottedAloha ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *
   * \return
   */
  double DoSlottedAloha ();

  /**
   *
   * \param min
   * \param max
   */
  void UpdateVariables (double min, double max);

private:

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
  Ptr<SatRandomAccessConf> m_randomAccessConf;

  /**
   *
   */
  Ptr<UniformRandomVariable> m_uniformVariable;

  /**
   *
   */
  double m_min;

  /**
   *
   */
  double m_max;

};

} // namespace ns3

#endif /* SATELLITE_SLOTTED_ALOHA_H */
