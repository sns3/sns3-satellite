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

#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include <cmath>
#include "satellite-position-allocator.h"

NS_LOG_COMPONENT_DEFINE ("SatPositionAllocator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPositionAllocator);

TypeId 
SatPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPositionAllocator")
    .SetParent<PositionAllocator> ()
    .AddAttribute ("AsGeoCoordinates",
                   "GetNext method returns Geodetic coordinates in returned Vector, x=longitude, y=latitude, z=altitude",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatPositionAllocator::m_GetAsGeoCoordinates),
                   MakeBooleanChecker ());
  return tid;
}

SatPositionAllocator::SatPositionAllocator ()
 :m_GetAsGeoCoordinates(true)
{
}

SatPositionAllocator::~SatPositionAllocator ()
{
}

Vector
SatPositionAllocator::GetNext(void) const
{
  GeoCoordinate pos = GetNextGeo();

  if ( m_GetAsGeoCoordinates )
    {
       return Vector(pos.GetLongitude(), pos.GetLatitude(), pos.GetAltitude());
    }
  else
    {
      return pos.ToVector();
    }
}

int64_t
SatPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}

NS_OBJECT_ENSURE_REGISTERED (SatListPositionAllocator);

TypeId
SatListPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatListPositionAllocator")
    .SetParent<SatPositionAllocator> ()
    .AddConstructor<SatListPositionAllocator> ()
  ;
  return tid;
}
SatListPositionAllocator::SatListPositionAllocator ()
{
}
void
SatListPositionAllocator::Add (GeoCoordinate coordinate)
{
  m_positions.push_back (coordinate);
  m_current = m_positions.begin ();
}
GeoCoordinate
SatListPositionAllocator::GetNextGeo (void) const
{
  GeoCoordinate coordinate = *m_current;
  m_current++;

  if (m_current == m_positions.end ())
    {
      m_current = m_positions.begin ();
    }
  return coordinate;
}

NS_OBJECT_ENSURE_REGISTERED (SatRandomBoxPositionAllocator);

TypeId
SatRandomBoxPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomBoxPositionAllocator")
    .SetParent<SatPositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<SatRandomBoxPositionAllocator> ()
    .AddAttribute ("Longitude",
                   "A random variable which represents the longitude coordinate of a position in a random rectangle.",
                   StringValue ("ns3::UniformRandomVariable[Min=-180.0|Max=180.0]"),
                   MakePointerAccessor (&SatRandomBoxPositionAllocator::m_longitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Latitude",
                   "A random variable which represents the latitude coordinate of a position in a random rectangle.",
                   StringValue ("ns3::UniformRandomVariable[Min=-90.0|Max=90.0]"),
                   MakePointerAccessor (&SatRandomBoxPositionAllocator::m_latitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Altitude",
                   "A random variable which represents the altitude coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0]"),
                   MakePointerAccessor (&SatRandomBoxPositionAllocator::m_altitude),
                   MakePointerChecker<RandomVariableStream> ());
  return tid;
}

SatRandomBoxPositionAllocator::SatRandomBoxPositionAllocator ()
{
}
SatRandomBoxPositionAllocator::~SatRandomBoxPositionAllocator ()
{
}

void
SatRandomBoxPositionAllocator::SetLongitude (Ptr<RandomVariableStream> longitude)
{
  m_longitude = longitude;
}
void
SatRandomBoxPositionAllocator::SetLatitude (Ptr<RandomVariableStream> latitude)
{
  m_latitude = latitude;
}

void
SatRandomBoxPositionAllocator::SetAltitude (Ptr<RandomVariableStream> altitude)
{
  m_altitude = altitude;
}

GeoCoordinate
SatRandomBoxPositionAllocator::GetNextGeo (void) const
{
  double longitude = m_longitude->GetValue ();
  double latitude = m_latitude->GetValue ();
  double altitude = m_altitude->GetValue ();

  return GeoCoordinate (longitude, latitude, altitude);
}

int64_t
SatRandomBoxPositionAllocator::AssignStreams (int64_t stream)
{
  m_longitude->SetStream (stream);
  m_latitude->SetStream (stream + 1);
  m_altitude->SetStream (stream + 2);
  return 3;
}


} // namespace ns3 
