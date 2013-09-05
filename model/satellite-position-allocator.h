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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_POSITION_ALLOCATOR_H
#define SATELLITE_POSITION_ALLOCATOR_H

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/position-allocator.h"
#include "geo-coordinate.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Allocate a set of satellite positions. The allocation strategy is implemented in subclasses.
 *
 * This is a pure abstract base class.
 */
class SatPositionAllocator : public PositionAllocator
{
public:
  static TypeId GetTypeId (void);
  SatPositionAllocator ();
  virtual ~SatPositionAllocator ();
  /**
   * \return the next chosen position.
   *
   * This method _must_ be implement in subclasses.
   */
  virtual GeoCoordinate GetNextGeo (void) const = 0;

  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);

private:
  // this is the flag for indicating that when calling method GetNext (defined by class PositionAlocator)
  // is returned Vector filled by longitude (in x), latitude (in y) and altitude (in z)
  // this enables using ns-3 mobility helper without to convert geo coordinates first to Cartesian
  // but if for some reason used model needs Cartesian coordinates then this flag can be turned off
  // through attribute 'AsGeoCoordinates'.
  bool m_GetAsGeoCoordinates;
};

/**
 * \ingroup satellite
 * \brief Allocate positions from a deterministic list specified by the user.
 *
 * The first call to SatListPositionAllocator::GetNext  will return the
 * first element of the list, the second call, the second element, and so on.
 */
class SatListPositionAllocator : public SatPositionAllocator
{
public:
  static TypeId GetTypeId (void);
  SatListPositionAllocator ();

  /**
   * \param coordinate the position to append at the end of the list of positions to return from GetNext.
   */
  void Add (GeoCoordinate coordinate);
  virtual GeoCoordinate GetNextGeo (void) const;
private:
  std::vector<GeoCoordinate> m_positions;
  mutable std::vector<GeoCoordinate>::const_iterator m_current;
};

/**
 * \ingroup satellite
 * \brief Allocate random positions within a 3D box according to a set of three random variables (longitude, latitude, altitude).
 */
class SatRandomBoxPositionAllocator : public SatPositionAllocator
{
public:
  static TypeId GetTypeId (void);
  SatRandomBoxPositionAllocator ();
  virtual ~SatRandomBoxPositionAllocator ();

  void SetLatitude (Ptr<RandomVariableStream> latitude);
  void SetLongitude (Ptr<RandomVariableStream> longitude);
  void SetAltitude (Ptr<RandomVariableStream> altitude);

  virtual GeoCoordinate GetNextGeo (void) const;
  virtual int64_t AssignStreams (int64_t stream);
private:
  Ptr<RandomVariableStream> m_latitude;
  Ptr<RandomVariableStream> m_longitude;
  Ptr<RandomVariableStream> m_altitude;
};

} // namespace ns3

#endif /* POSITION_ALLOCATOR_H */
