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

#include "ns3/boolean.h"
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
    .AddAttribute ("AsGeoCoordinates",
                   "SetPosition method takes Geodetic coordinates in given Vector, x=longitude, y=latitude, z=altitude",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatMobilityModel::m_GetAsGeoCoordinates),
                   MakeBooleanChecker ())
    .AddTraceSource ("SatCourseChange",
                     "The value of the position and/or velocity coordinate changed",
                     MakeTraceSourceAccessor (&SatMobilityModel::m_satCourseChangeTrace),
                     "ns3::SatMobilityModel::CourseChangeCallback")
  ;
  return tid;
}

TypeId
SatMobilityModel::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatMobilityModel::SatMobilityModel ()
  : m_GetAsGeoCoordinates (true)
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
  NotifyCourseChange ();
}

Vector
SatMobilityModel::DoGetPosition (void) const
{
  if ( m_cartesianPositionOutdated )
    {
      Vector position = DoGetGeoPosition ().ToVector ();
      DoSetCartesianPosition (position);
    }

  return m_cartesianPosition;
}

void
SatMobilityModel::DoSetPosition (const Vector &position)
{
  if ( m_GetAsGeoCoordinates )
    {
      m_cartesianPositionOutdated = true;
      DoSetGeoPosition ( GeoCoordinate (position.x, position.y, position.z) );
    }
  else
    {
      m_cartesianPositionOutdated = false;
      m_cartesianPosition = position;
      DoSetGeoPosition ( GeoCoordinate (position) );
    }

}

void
SatMobilityModel::DoSetCartesianPosition (const Vector &position) const
{
  m_cartesianPositionOutdated = false;
  m_cartesianPosition = position;
}

} // namespace ns3
