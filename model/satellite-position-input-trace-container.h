/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_POSITION_INPUT_TRACE_CONTAINER_H
#define SATELLITE_POSITION_INPUT_TRACE_CONTAINER_H

#include <ns3/satellite-input-fstream-time-double-container.h>
#include <ns3/mac48-address.h>
#include "satellite-base-trace-container.h"
#include "geo-coordinate.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for Rx power input trace container. The class contains
 * multiple Rx power input sample traces and provides an interface to them.
 */
class SatPositionInputTraceContainer : public SatBaseTraceContainer
{
public:
  /**
   * \brief typedef for map of containers
   */
  typedef std::map <Address, Ptr<SatInputFileStreamTimeDoubleContainer> > container_t;

  /**
   * \brief Constructor
   */
  SatPositionInputTraceContainer ();

  /**
   * \brief Destructor
   */
  ~SatPositionInputTraceContainer ();

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
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

  /**
   * \brief Function for getting the Rx power density
   * \param key key
   * \return Rx power density
   */
  GeoCoordinate GetPosition (Address key, GeoCoordinate::ReferenceEllipsoid_t refEllipsoid);

  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

private:
  /**
   * \brief Function for adding the node to the map
   * \param key key
   * \return pointer to the added container
   */
  Ptr<SatInputFileStreamTimeDoubleContainer> AddNode (Address key);

  /**
   * \brief Function for finding the container matching the key
   * \param key key
   * \return matching container
   */
  Ptr<SatInputFileStreamTimeDoubleContainer> FindNode (Address key);

  /**
   * \brief Map for containers
   */
  container_t m_container;
};

} // namespace ns3

#endif /* SATELLITE_POSITION_INPUT_TRACE_CONTAINER_H */
