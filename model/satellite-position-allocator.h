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
#include "satellite-antenna-gain-pattern-container.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Allocate a set of satellite positions. The allocation strategy is implemented in
 * inherited classes.
 *
 * This is a pure abstract base class.
 */
class SatPositionAllocator : public PositionAllocator
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatPositionAllocator ();

  /**
   * Destructor for SatPositionAllocator
   */
  virtual ~SatPositionAllocator ();

  /**
   * \brief Get next position
   * \return The next chosen position.
   */
  virtual GeoCoordinate GetNextGeoPosition (void) const = 0;

  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);

private:
  /*
   * This is the flag for indicating that when calling method GetNext (defined by class PositionAlocator)
   * is returned Vector filled by longitude (in x), latitude (in y) and altitude (in z)
   * this enables using ns-3 mobility helper without to convert geo coordinates first to Cartesian
   * but if for some reason used model needs Cartesian coordinates then this flag can be turned off
   * through attribute 'AsGeoCoordinates'.
   */
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

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatListPositionAllocator ();

  /**
   * \param coordinate the position to append at the end of the list of positions to return from GetNext.
   */
  void Add (GeoCoordinate coordinate);

  /**
   * \brief Get next position
   * \return The next chosen position.
   */
  virtual GeoCoordinate GetNextGeoPosition (void) const;

  inline virtual void SetToBegin ()
  {
    m_current = m_positions.begin ();
  }
  inline virtual uint32_t GetCount ()
  {
    return m_positions.size ();
  }

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

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatRandomBoxPositionAllocator ();

  /**
   * Destructor for SatRandomBoxPositionAllocator
   */
  virtual ~SatRandomBoxPositionAllocator ();

  void SetLatitude (Ptr<RandomVariableStream> latitude);
  void SetLongitude (Ptr<RandomVariableStream> longitude);
  void SetAltitude (Ptr<RandomVariableStream> altitude);

  /**
   * \brief Get next position
   * \return The next chosen position.
   */
  virtual GeoCoordinate GetNextGeoPosition (void) const;
  virtual int64_t AssignStreams (int64_t stream);

private:
  Ptr<RandomVariableStream> m_latitude;
  Ptr<RandomVariableStream> m_longitude;
  Ptr<RandomVariableStream> m_altitude;
};

/**
 * \ingroup satellite
 * \brief Allocate random positions within the area of a certain spot-beam.
 */
class SatSpotBeamPositionAllocator : public SatPositionAllocator
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatSpotBeamPositionAllocator ();

  /**
   * SatSpotBeamPositionAllocator constructor
   * \param beamId Beam id to which we are placing an UT
   * \param patternContainer Container holding the antenna patterns
   * \param geoPos GEO coordinate of the satellite for elevation angle calculations
   */
  SatSpotBeamPositionAllocator (uint32_t beamId, Ptr<SatAntennaGainPatternContainer> patternContainer, GeoCoordinate geoPos);

  /**
   * Destructor for SatSpotBeamPositionAllocator
   */
  virtual ~SatSpotBeamPositionAllocator ();

  void SetAltitude (Ptr<RandomVariableStream> altitude);

  /**
   * \brief Get next position
   * \return The next chosen position.
   */
  virtual GeoCoordinate GetNextGeoPosition (void) const;
  virtual int64_t AssignStreams (int64_t stream);
private:
  /**
   * Max number of tries to pick a random position for a UT.
   */
  static constexpr uint32_t MAX_TRIES = 100;

  /**
   * Minimum accepted antenna gain for a UT.
   */
  static constexpr double MIN_ANTENNA_GAIN = 40.0;

  /**
   * Target beam id to which the UT is tried to be placed.
   */
  uint32_t m_targetBeamId;

  /**
   * Minimum accepted elevation angle in degrees for UTs. This
   * is placed to guarantee that UTs are not positioned into too
   * low elevation angles (high latitudes, longitudes far from
   * GEO satellite latitude).
   */
  double m_minElevationAngleInDeg;

  /**
   * Antenna pattern used to check that the give position is valid based on
   * antenna gains. I.e. UT should be placed into a position where the m_targetBeamId
   * has the best antenna gain.
   */
  Ptr<SatAntennaGainPatternContainer> m_antennaGainPatterns;

  /**
   * Position of the GEO satellite
   */
  GeoCoordinate m_geoPos;

  /**
   * A random variable stream for altitude.
   */
  Ptr<RandomVariableStream> m_altitude;
};



} // namespace ns3

#endif /* POSITION_ALLOCATOR_H */
