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

#include <cmath>

#include "satellite-mobility-model.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMobilityModel);

TypeId 
SatMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMobilityModel")
    .SetParent<MobilityModel> ()
    .AddAttribute ("SatPosition", "The current position of the mobility model.",
                   TypeId::ATTR_SET | TypeId::ATTR_GET,
                   GeoCoordinateValue (GeoCoordinate (0.0, 0.0, 0.0)),
                   MakeGeoCoordinateAccessor (&SatMobilityModel::SetGeoPosition,
                                              &SatMobilityModel::GetGeoPosition),
                   MakeGeoCoordinateChecker ())
    .AddAttribute ("SatVelocity", "The current velocity of the mobility model.",
                   TypeId::ATTR_GET,
                   GeoCoordinateValue (GeoCoordinate (0.0, 0.0, 0.0)), // ignored initial value.
                   MakeGeoCoordinateAccessor (&SatMobilityModel::GetGeoVelocity),
                   MakeGeoCoordinateChecker ())
    .AddTraceSource ("SatCourseChange",
                     "The value of the position and/or velocity coordinate changed",
                     MakeTraceSourceAccessor (&SatMobilityModel::m_satCourseChangeTrace))
  ;
  return tid;
}

SatMobilityModel::SatMobilityModel ()
{
}

SatMobilityModel::~SatMobilityModel ()
{
}

GeoCoordinate
SatMobilityModel::GetGeoPosition (void) const
{
  return DoGetGeoPosition ();
}
GeoCoordinate
SatMobilityModel::GetGeoVelocity (void) const
{
  return DoGetGeoVelocity ();
}

void
SatMobilityModel::SetGeoPosition (const GeoCoordinate &position)
{
  m_cartesianPositionOutdated = true;
  DoSetGeoPosition (position);
}

void
SatMobilityModel::NotifyGeoCourseChange (void) const
{
  m_satCourseChangeTrace (this);
  NotifyCourseChange();
}

Vector
SatMobilityModel::DoGetPosition (void) const
{
  if ( m_cartesianPositionOutdated )
    {
      Vector position = DoGetGeoPosition().ToVector();
      DoSetCartesianPosition(position);
    }

  return m_cartesianPosition;
}

void
SatMobilityModel::DoSetPosition (const Vector &position)
{
  m_cartesianPositionOutdated = false;
  m_cartesianPosition = position;
  DoSetGeoPosition( GeoCoordinate(position) );
}

Vector
SatMobilityModel::DoGetVelocity (void) const
{
  return Vector (0.0, 0.0, 0.0);
}

void
SatMobilityModel::DoSetCartesianPosition (const Vector &position) const
{
  m_cartesianPositionOutdated = false;
  m_cartesianPosition = position;
}

} // namespace ns3
