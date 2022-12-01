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

#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/singleton.h>
#include "satellite-traced-mobility-model.h"
#include "satellite-position-input-trace-container.h"


NS_LOG_COMPONENT_DEFINE ("SatTracedMobilityModel");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTracedMobilityModel);

TypeId
SatTracedMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTracedMobilityModel")
    .SetParent<SatMobilityModel> ()
    .AddConstructor<SatTracedMobilityModel> ()
    .AddAttribute ("ReferenceEllipsoid",
                   "Reference Ellipsoid model to use when reading coordinates",
                   EnumValue (GeoCoordinate::SPHERE),
                   MakeEnumAccessor (&SatTracedMobilityModel::m_refEllipsoid),
                   MakeEnumChecker (GeoCoordinate::SPHERE, "Sphere",
                                    GeoCoordinate::WGS84, "WGS84",
                                    GeoCoordinate::GRS80, "GRS80"))
    .AddAttribute ("UpdateInterval",
                   "Interval at which the position should update",
                   TimeValue (MilliSeconds (1)),
                   MakeTimeAccessor (&SatTracedMobilityModel::m_updateInterval),
                   MakeTimeChecker (FemtoSeconds (1)))
  ;

  return tid;
}

TypeId
SatTracedMobilityModel::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SatTracedMobilityModel::DoDispose ()
{
  m_antennaGainPatterns = NULL;

  Object::DoDispose ();
}

SatTracedMobilityModel::SatTracedMobilityModel ()
{
  NS_FATAL_ERROR ("SatTracedMobilityModel default constructor should not be used");
}

SatTracedMobilityModel::SatTracedMobilityModel (uint32_t satId, const std::string& filename, Ptr<SatAntennaGainPatternContainer> agp)
  : m_satId (satId),
  m_traceFilename (filename),
  m_updateInterval (MilliSeconds (1)),
  m_refEllipsoid (GeoCoordinate::SPHERE),
  m_geoPosition (0.0, 0.0, 0.0),
  m_velocity (0.0, 0.0, 0.0),
  m_antennaGainPatterns (agp)
{
  NS_LOG_FUNCTION (this);

  UpdateGeoPositionFromFile ();
}

SatTracedMobilityModel::~SatTracedMobilityModel ()
{
  NS_LOG_FUNCTION (this);
}

GeoCoordinate
SatTracedMobilityModel::DoGetGeoPosition (void) const
{
  NS_LOG_FUNCTION (this);

  return m_geoPosition;
}

void
SatTracedMobilityModel::DoSetGeoPosition (const GeoCoordinate &position)
{
  NS_LOG_FUNCTION (this << position);

  double time = m_updateInterval.GetSeconds ();
  Vector distance = position.ToVector () - m_geoPosition.ToVector ();
  m_velocity = Vector (distance.x / time, distance.y / time, distance.z / time);

  NS_LOG_INFO ("Changing position from " << m_geoPosition << " to " << position << ". Velocity is " << m_velocity << " (" << m_velocity.GetLength () << ")");

  m_geoPosition = position;
  NotifyGeoCourseChange ();
}

Vector
SatTracedMobilityModel::DoGetVelocity (void) const
{
  NS_LOG_FUNCTION (this);

  return m_velocity;
}

void
SatTracedMobilityModel::UpdateGeoPositionFromFile (void)
{
  NS_LOG_FUNCTION (this);

  GeoCoordinate newPosition = Singleton<SatPositionInputTraceContainer>::Get ()->GetPosition (m_traceFilename, m_refEllipsoid);
  DoSetGeoPosition (newPosition);

  Simulator::Schedule (m_updateInterval, &SatTracedMobilityModel::UpdateGeoPositionFromFile, this);
}

uint32_t
SatTracedMobilityModel::GetBestBeamId (void) const
{
  return m_antennaGainPatterns->GetBestBeamId (m_satId, m_geoPosition, false);
}

}
