/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#ifndef SAT_LOO_CONF_H
#define SAT_LOO_CONF_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \brief A configuration class for Loo model
 *
 */
class SatLooConf : public Object
{
public:

  /**
   *
   */
  static const uint32_t DEFAULT_ELEVATION_COUNT = 4;
  static const uint32_t DEFAULT_STATE_COUNT = 3;
  static const uint32_t DEFAULT_LOO_PARAMETER_COUNT = 7;

  /**
   *
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   *
   */
  SatLooConf ();

  /**
   *
   */
  virtual ~SatLooConf () {}

  /**
   *
   * \param setId
   * \return
   */
  std::vector<std::vector<double> > GetLooParameters (uint32_t set);

private:

  /**
   *
   */
  uint32_t m_elevationCount;
  uint32_t m_stateCount;
  std::vector<std::vector<std::vector<double> > > m_looParameters;
};

} // namespace ns3

#endif /* SAT_LOO_CONF_H */
