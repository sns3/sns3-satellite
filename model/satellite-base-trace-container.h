/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
#ifndef SATELLITE_BASE_TRACE_CONTAINER_H
#define SATELLITE_BASE_TRACE_CONTAINER_H

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Base class for trace containers such as interference or fading traces.
 * This class defines the functions which all the different trace models must
 * implement for the trace interface. This base class itself is abstract and will not
 * implement any real functionality.
 */

class SatBaseTraceContainer : public Object
{
public:
  /**
   * \brief Default Rx power density index for Rx power traces
   */
  static const uint32_t RX_POWER_TRACE_DEFAULT_RX_POWER_DENSITY_INDEX = 1;

  /**
   * \brief Default Rx power density index for Rx power traces
   */
  static const uint32_t RX_POWER_TRACE_DEFAULT_NUMBER_OF_COLUMNS = 2;

  /**
   * \brief Default interference density index for interference traces
   */
  static const uint32_t INTF_TRACE_DEFAULT_INTF_DENSITY_INDEX = 1;

  /**
   * \brief Default number of columns for interference traces
   */
  static const uint32_t INTF_TRACE_DEFAULT_NUMBER_OF_COLUMNS = 2;

  /**
   * \brief Default fading value index for fading traces
   */
  static const uint32_t FADING_TRACE_DEFAULT_FADING_VALUE_INDEX = 1;

  /**
   * \brief Default number of columns for fading traces
   */
  static const uint32_t FADING_TRACE_DEFAULT_NUMBER_OF_COLUMNS = 2;

  /**
   * \brief Default sinr value index for composite sinr traces
   */
  static const uint32_t CSINR_TRACE_DEFAULT_FADING_VALUE_INDEX = 1;

  /**
   * \brief Default number of columns for composite sinr traces
   */
  static const uint32_t CSINR_TRACE_DEFAULT_NUMBER_OF_COLUMNS = 2;

  /**
   * \brief Constructor
   */
  SatBaseTraceContainer ();

  /**
   * \brief Destructor
   */
  virtual ~SatBaseTraceContainer ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NS-3 instance type id function
   * \return Instance type is
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Function for resetting the trace
   */
  virtual void Reset () = 0;

private:
};

} // namespace ns3

#endif /* SATELLITE_BASE_TRACE_CONTAINER_H */
