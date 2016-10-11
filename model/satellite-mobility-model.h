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

#ifndef SATELLITE_MOBILITY_MODEL_H
#define SATELLITE_MOBILITY_MODEL_H

#include "ns3/mobility-model.h"
#include "geo-coordinate.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Keep track of the current position and velocity of an object in satellite network.
 *
 * All space coordinates in this class and its subclasses are
 * treated as geodetic coordinates. Support for setting and getting information
 * in Cartesian coordinates also provided.
 *
 * This is a base class for all satellite mobility models.
 */
class SatMobilityModel : public MobilityModel
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatMobilityModel ();

  /**
   * Destructor for SatMobilityModel
   */
  virtual ~SatMobilityModel () = 0;

  /**
   * \return the current satellite position
   */
  GeoCoordinate GetGeoPosition (void) const;
  /**
   * \param position the satellite position to set.
   */
  void SetGeoPosition (const GeoCoordinate &position);

  void NotifyGeoCourseChange (void) const;

  /**
   * Callback signature for `SatCourseChange` trace source.
   *
   * \param model the SatMobilityModel which is changing course
   */
  typedef void (*CourseChangeCallback)
    (const Ptr<const SatMobilityModel> model);

private:
  /**
   * \return the current position.
   *
   * Concrete subclasses of this base class must
   * implement this method.
   */
  virtual GeoCoordinate DoGetGeoPosition (void) const = 0;
  /**
   * \param position the position to set.
   *
   * Concrete subclasses of this base class must
   * implement this method.
   */
  virtual void DoSetGeoPosition (const GeoCoordinate &position) = 0;
  /**
   * This method is used to force update of cartesian position.
   * Cartesian position is updated when position is set by method DoSetPosition.
   * In case that position is updated by method DoSetGeoPosition cartesian position is
   * updated only if it is requested by method DoGetPosition.
   *
   * \param position position in cartesian format to set
   *
   */
  void DoSetCartesianPosition (const Vector &position) const;

  /**
   * \return cartesian format position as vector
   *
   * Implementation for method defined by MobilityModel
   */
  virtual Vector DoGetPosition (void) const;

  /**
   * \param position position in Cartesian format to set
   *
   * Implementation for method defined by MobilityModel
   */
  virtual void DoSetPosition (const Vector &position);

  /**
   * Used to alert subscribers that a change in direction, velocity,
   * or position has occurred.
   */
  ns3::TracedCallback<Ptr<const SatMobilityModel> > m_satCourseChangeTrace;

  // These are defined as mutable in order to support 'lazy' update.

  // position info in Cartesian format
  mutable Vector m_cartesianPosition;

  // flag to indicated if position in Cartesian format is out of date.
  mutable bool m_cartesianPositionOutdated;

  // this is the flag for indicating that when calling method DoSetPosition (defined by class Mobility Model)
  // is taking Vector filled by longitude (in x), latitude (in y) and altitude (in z)
  // this enables using ns-3 mobility helper without to convert geo coordinates first to Cartesian
  // but if for some reason used model needs Cartesian coordinates then this flag can be turned off
  // through attribute 'AsGeoCoordinates'.
  bool m_GetAsGeoCoordinates;
};

} // namespace ns3

#endif /* SATELLITE_MOBILITY_MODEL_H */
