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

#include <limits>
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include <cmath>
#include "satellite-position-allocator.h"
#include "satellite-antenna-gain-pattern-container.h"
#include "satellite-utils.h"
#include "satellite-constant-position-mobility-model.h"
#include "satellite-mobility-observer.h"

NS_LOG_COMPONENT_DEFINE ("SatPositionAllocator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPositionAllocator);

TypeId
SatPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPositionAllocator")
    .SetParent<PositionAllocator> ()
    .AddAttribute ("AsGeoCoordinates",
                   "GetNext method returns geodetic coordinates in returned Vector, x=longitude, y=latitude, z=altitude",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatPositionAllocator::m_GetAsGeoCoordinates),
                   MakeBooleanChecker ());
  return tid;
}

SatPositionAllocator::SatPositionAllocator ()
  : m_GetAsGeoCoordinates (true)
{
}


SatPositionAllocator::~SatPositionAllocator ()
{
}

Vector
SatPositionAllocator::GetNext () const
{
  NS_LOG_INFO (this);

  GeoCoordinate pos = GetNextGeoPosition ();

  if ( m_GetAsGeoCoordinates )
    {
      return Vector (pos.GetLatitude (), pos.GetLongitude (), pos.GetAltitude ());
    }
  else
    {
      return pos.ToVector ();
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
  NS_LOG_INFO (this << coordinate.GetLatitude () << coordinate.GetLongitude () << coordinate.GetAltitude ());

  m_positions.push_back (coordinate);
  m_current = m_positions.begin ();
}
GeoCoordinate
SatListPositionAllocator::GetNextGeoPosition () const
{
  NS_LOG_INFO (this);

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
  NS_LOG_INFO (this);

  m_longitude = longitude;
}
void
SatRandomBoxPositionAllocator::SetLatitude (Ptr<RandomVariableStream> latitude)
{
  NS_LOG_INFO (this);

  m_latitude = latitude;
}

void
SatRandomBoxPositionAllocator::SetAltitude (Ptr<RandomVariableStream> altitude)
{
  NS_LOG_INFO (this);

  m_altitude = altitude;
}

GeoCoordinate
SatRandomBoxPositionAllocator::GetNextGeoPosition () const
{
  NS_LOG_INFO (this);
  double longitude = m_longitude->GetValue ();
  double latitude = m_latitude->GetValue ();
  double altitude = m_altitude->GetValue ();

  return GeoCoordinate (latitude, longitude, altitude);
}

int64_t
SatRandomBoxPositionAllocator::AssignStreams (int64_t stream)
{
  m_longitude->SetStream (stream);
  m_latitude->SetStream (stream + 1);
  m_altitude->SetStream (stream + 2);
  return 3;
}


NS_OBJECT_ENSURE_REGISTERED (SatSpotBeamPositionAllocator);

TypeId
SatSpotBeamPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSpotBeamPositionAllocator")
    .SetParent<SatPositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<SatSpotBeamPositionAllocator> ()
    .AddAttribute ("Altitude",
                   "A random variable which represents the altitude coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0]"),
                   MakePointerAccessor (&SatSpotBeamPositionAllocator::m_altitude),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("MinElevationAngleInDegForUT",
                   "Minimum accepted elevation angle in degrees for UTs",
                   DoubleValue (5.00),
                   MakeDoubleAccessor (&SatSpotBeamPositionAllocator::m_minElevationAngleInDeg),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

SatSpotBeamPositionAllocator::SatSpotBeamPositionAllocator ()
  : m_targetBeamId (0),
    m_minElevationAngleInDeg (1)
{

}

SatSpotBeamPositionAllocator::SatSpotBeamPositionAllocator (uint32_t beamId, Ptr<SatAntennaGainPatternContainer> patterns, GeoCoordinate geoPos)
  : m_targetBeamId (beamId),
    m_minElevationAngleInDeg (1),
    m_antennaGainPatterns (patterns),
    m_geoPos (geoPos)
{
}


SatSpotBeamPositionAllocator::~SatSpotBeamPositionAllocator ()
{
}

void
SatSpotBeamPositionAllocator::SetAltitude (Ptr<RandomVariableStream> altitude)
{
  NS_LOG_FUNCTION (this);

  m_altitude = altitude;
}

GeoCoordinate
SatSpotBeamPositionAllocator::GetNextGeoPosition () const
{
  NS_LOG_FUNCTION (this);

  uint32_t bestBeamId (std::numeric_limits<uint32_t>::max ());
  Ptr<SatAntennaGainPattern> agp = m_antennaGainPatterns->GetAntennaGainPattern (m_targetBeamId);
  uint32_t tries (0);
  GeoCoordinate pos;

  Ptr<SatConstantPositionMobilityModel> utMob = CreateObject<SatConstantPositionMobilityModel> ();
  Ptr<SatConstantPositionMobilityModel> geoMob = CreateObject<SatConstantPositionMobilityModel> ();
  utMob->SetGeoPosition (GeoCoordinate (0.00, 0.00, 0.00));
  geoMob->SetGeoPosition (m_geoPos);
  Ptr<SatMobilityObserver> utObserver = CreateObject<SatMobilityObserver> (utMob, geoMob);

  double elevation (std::numeric_limits<double>::max ());

  // Try until
  // - we have a valid position
  // - the MAX_TRIES have been exceeded
  // - elevation is NOT NaN
  // - elevation is not higher than threshold
  while ( ( bestBeamId != m_targetBeamId || std::isnan (elevation) || elevation < m_minElevationAngleInDeg ) && tries < MAX_TRIES)
    {
      pos = agp->GetValidRandomPosition ();
      bestBeamId = m_antennaGainPatterns->GetBestBeamId (pos);

      // Set the new position to the UT mobility
      utMob->SetGeoPosition (pos);

      // Calculate the elevation angle
      elevation = utObserver->GetElevationAngle ();

      ++tries;
    }

  // If the positioning fails
  if (tries >= MAX_TRIES)
    {
      NS_FATAL_ERROR (this << " max number of tries for spot-beam allocation exceeded!");
    }

  // Set a random altitude
  pos.SetAltitude (m_altitude->GetValue ());

  if (pos.GetLatitude () < -90.0 || pos.GetLatitude () > 90.0
      || pos.GetLongitude () < -180.0 || pos.GetLongitude () > 180.0
      || elevation < m_minElevationAngleInDeg || elevation > 90.0)
    {
      NS_FATAL_ERROR ("SatSpotBeamPositionAllocator::GetNextGeoPosition - unvalid selected position!");
    }

  return pos;
}

int64_t
SatSpotBeamPositionAllocator::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this);

  m_altitude->SetStream (stream + 2);
  return 3;
}


} // namespace ns3
