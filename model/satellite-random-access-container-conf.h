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
  double GetSlottedAlohaDefaultMin () { return m_slottedAlohaMin; }

  /**
   *
   * \return
   */
  double GetSlottedAlohaDefaultMax () { return m_slottedAlohaMax; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaDefaultMin () { return m_crdsaMin; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaDefaultMax () { return m_crdsaMax; }

  /**
   *
   * \return
   */
  uint32_t GetCrdsaDefaultSetSize () { return m_crdsaSetSize; }

private:

  /**
   *
   */
  double m_slottedAlohaMin;

  /**
   *
   */
  double m_slottedAlohaMax;

  /**
   *
   */
  uint32_t m_crdsaMin;

  /**
   *
   */
  uint32_t m_crdsaMax;

  /**
   *
   */
  uint32_t m_crdsaSetSize;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_CONF_H */
