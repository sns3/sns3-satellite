/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "satellite-utils.h"
#include "satellite-mobility-observer.h"

NS_LOG_COMPONENT_DEFINE ("SatMobilityObserver");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMobilityObserver);

TypeId
SatMobilityObserver::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMobilityObserver")
    .SetParent<Object> ()
    .AddAttribute ("OwnMobility", "Own mobility.",
                   PointerValue (),
                   MakePointerAccessor (&SatMobilityObserver::m_ownMobility),
                   MakePointerChecker<SatMobilityModel> ())
    .AddAttribute ("SatelliteMobility", "The mobility of the satellite.",
                   PointerValue (),
                   MakePointerAccessor (&SatMobilityObserver::m_geoSatMobility),
                   MakePointerChecker<SatMobilityModel> ())
    .AddAttribute ("AnotherMobility", "The mobility of the another end node.",
                   PointerValue (),
                   MakePointerAccessor (&SatMobilityObserver::m_anotherMobility),
                   MakePointerChecker<SatMobilityModel> ())
    .AddAttribute ("OwnPropagation", "Own propagation delay model.",
                   PointerValue (),
                   MakePointerAccessor (&SatMobilityObserver::m_ownProgDelayModel),
                   MakePointerChecker<SatMobilityModel> ())
    .AddAttribute ("AnotherPropagation", "The propagation delay model of the another end node.",
                   PointerValue (),
                   MakePointerAccessor (&SatMobilityObserver::m_anotherProgDelayModel),
                   MakePointerChecker<SatMobilityModel> ())
    .AddAttribute ("MinAltitude", "The minimum altitude accepted for own position.",
                   DoubleValue (0),
                   MakeDoubleAccessor (&SatMobilityObserver::m_minAltitude),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxAltitude", "The maximum altitude accepted for own position.",
                   DoubleValue (500.00),
                   MakeDoubleAccessor (&SatMobilityObserver::m_maxAltitude),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("PropertyChanged",
                     "The value of the some property has changed",
                     MakeTraceSourceAccessor (&SatMobilityObserver::m_propertyChangeTrace),
                     "ns3::SatMobilityObserver::PropertyChangedCallback")
  ;
  return tid;
}

TypeId
SatMobilityObserver::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatMobilityObserver::SatMobilityObserver ()
{
  NS_LOG_FUNCTION (this);

  // this constructor version should not be used
  NS_ASSERT (false);
}

SatMobilityObserver::SatMobilityObserver (Ptr<SatMobilityModel> ownMobility, Ptr<SatMobilityModel> geoSatMobility)
  : m_ownMobility (ownMobility),
    m_anotherMobility (NULL),
    m_geoSatMobility (geoSatMobility),
    m_ownProgDelayModel (NULL),
    m_anotherProgDelayModel (NULL),
    m_initialized (false)
{
  NS_LOG_FUNCTION (this << ownMobility << geoSatMobility);

  GeoCoordinate satellitePosition = m_geoSatMobility->GetGeoPosition ();
  GeoCoordinate ownPosition = m_ownMobility->GetGeoPosition ();

  // same reference ellipsoide must be used by mobilities
  NS_ASSERT (satellitePosition.GetRefEllipsoid () == ownPosition.GetRefEllipsoid () );

  double satelliteAltitude = satellitePosition.GetAltitude ();

  // satellite is expected to be in the sky
  NS_ASSERT ( satelliteAltitude > 0.0 );

  // calculate radius of the earth using satellite information
  m_earthRadius = CalculateDistance (satellitePosition.ToVector (), Vector (0, 0, 0)) - satelliteAltitude;

  SatelliteStatusChanged ();
  m_updateElevationAngle = true;
  m_updateTimingAdvance = true;
  m_timingAdvance_s = Seconds (0);

  m_geoSatMobility->TraceConnect ("SatCourseChange", "Satellite", MakeCallback ( &SatMobilityObserver::PositionChanged, this));
  m_ownMobility->TraceConnect ("SatCourseChange", "Own", MakeCallback (&SatMobilityObserver::PositionChanged, this));

  m_velocity = 0.0;

  m_initialized = true;
}

void SatMobilityObserver::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_ownMobility = NULL;
  m_anotherMobility = NULL;
  m_geoSatMobility = NULL;
  m_ownProgDelayModel = NULL;
  m_anotherProgDelayModel = NULL;
}

SatMobilityObserver::~SatMobilityObserver ()
{
  NS_LOG_FUNCTION (this);
}

void
SatMobilityObserver::ObserveTimingAdvance (Ptr<PropagationDelayModel> ownDelayModel,
                                           Ptr<PropagationDelayModel> anotherDelayModel,
                                           Ptr<SatMobilityModel> anotherMobility)
{
  NS_LOG_FUNCTION ( this << ownDelayModel << anotherDelayModel << anotherMobility);

  NS_ASSERT ( ownDelayModel != NULL );
  NS_ASSERT ( anotherDelayModel != NULL );
  NS_ASSERT ( anotherMobility != NULL );

  m_ownProgDelayModel = ownDelayModel;
  m_anotherProgDelayModel = anotherDelayModel;
  m_anotherMobility = anotherMobility;

  // same reference ellipsoide must be used by mobilities
  NS_ASSERT (m_anotherMobility->GetGeoPosition ().GetRefEllipsoid () == m_ownMobility->GetGeoPosition ().GetRefEllipsoid () );

  m_anotherMobility->TraceConnect ("SatCourseChange", "Another", MakeCallback (&SatMobilityObserver::PositionChanged, this));
}

double
SatMobilityObserver::GetElevationAngle (void)
{
  NS_LOG_FUNCTION (this);

  if ( m_updateElevationAngle == true )
    {
      // same reference ellipsoide must be used by mobilities
      NS_ASSERT (m_geoSatMobility->GetGeoPosition ().GetRefEllipsoid () == m_ownMobility->GetGeoPosition ().GetRefEllipsoid () );

      UpdateElevationAngle ();
      m_updateElevationAngle = false;
    }

  return m_elevationAngle;
}

double
SatMobilityObserver::GetVelocity (void)
{
  NS_LOG_FUNCTION (this);

  Vector velocity = m_ownMobility->GetVelocity ();
  m_velocity = std::sqrt ( ( velocity.x * velocity.x ) + ( velocity.y * velocity.y ) + ( velocity.z * velocity.z ) );

  return m_velocity;
}

Time
SatMobilityObserver::GetTimingAdvance (void)
{
  NS_LOG_FUNCTION (this);

  // update timing advance, if another end is given and update needed
  if ( (m_anotherMobility != NULL) &&  ( m_updateTimingAdvance == true ) )
    {
      // another propagation delay is expected to be given
      NS_ASSERT ( m_anotherProgDelayModel != NULL );

      // same reference ellipsoide must be used by mobilities
      NS_ASSERT (m_geoSatMobility->GetGeoPosition ().GetRefEllipsoid () == m_ownMobility->GetGeoPosition ().GetRefEllipsoid () );

      // same reference ellipsoide must be used by mobilities
      NS_ASSERT (m_anotherMobility->GetGeoPosition ().GetRefEllipsoid () == m_ownMobility->GetGeoPosition ().GetRefEllipsoid () );

      UpdateTimingAdvance ();
      m_updateTimingAdvance = false;
    }

  return m_timingAdvance_s;
}

void
SatMobilityObserver::NotifyPropertyChange (void) const
{
  NS_LOG_FUNCTION (this);

  if ( m_initialized )
    {
      m_propertyChangeTrace (this);
    }
}

void
SatMobilityObserver::PositionChanged (std::string context, Ptr<const SatMobilityModel> position)
{
  NS_LOG_FUNCTION (this << context << position);

  // set flag on to idicate that elevation angle is needed to update,
  // when its status is requested with method GetElevationAngle
  m_updateElevationAngle = true;

  // set flag on to idicate that timing advance is needed to update,
  // when its status is requested with method GetTimingAdvance
  m_updateTimingAdvance = true;

  // call satellite statis updated routine to update needed variables
  if ( context == "Satellite" )
    {
      SatelliteStatusChanged ();
    }

  NotifyPropertyChange ();

}

void
SatMobilityObserver::UpdateElevationAngle ()
{
  NS_LOG_FUNCTION (this);

  m_elevationAngle = NAN;

  GeoCoordinate ownPosition = m_ownMobility->GetGeoPosition ();
  GeoCoordinate satellitePosition = m_geoSatMobility->GetGeoPosition ();

  NS_ASSERT ( ownPosition.GetAltitude () >= m_minAltitude && ownPosition.GetAltitude () <= m_maxAltitude );

  // elevation angle is always calculated at earth surface, so set altitude to zero
  ownPosition.SetAltitude (0);

  // calculate distance from Earth location to satellite
  double distanceToSatellite = CalculateDistance (ownPosition.ToVector (), satellitePosition.ToVector () );

  // calculate elevation angle only, if satellite can be seen from own position
  if ( distanceToSatellite <= m_maxDistanceToSatellite )
    {
      double earthLatitude = SatUtils::DegreesToRadians (ownPosition.GetLatitude ());
      double satLatitude = SatUtils::DegreesToRadians (satellitePosition.GetLatitude ());

      double earthLongitude = SatUtils::DegreesToRadians (ownPosition.GetLongitude ());
      double satLongitude = SatUtils::DegreesToRadians (satellitePosition.GetLongitude ());

      double longitudeDelta = satLongitude - earthLongitude;

      // Calculate cosini of the central angle
      // TODO: Currently we have assumed that the reference ellipsoide is a sphere.
      // This should be accurate enough for elevation angle calculation with also other
      // reference ellipsoides. But, if more accurate calculation is needed, then the used
      // reference ellipsoide is needed to be take into account.
      double centralAngleCos = ( std::cos (earthLatitude) * std::cos (satLatitude) * std::cos (longitudeDelta) ) +
        ( std::sin (earthLatitude) * std::sin (satLatitude ) );

      // Calculate cosini of the elavation angle
      double elCos = std::sin ( std::acos (centralAngleCos)) / std::sqrt ( 1 + std::pow (m_radiusRatio, 2) - 2 * m_radiusRatio * centralAngleCos);

      m_elevationAngle = SatUtils::RadiansToDegrees (std::acos (elCos) );
    }
}

void
SatMobilityObserver::UpdateTimingAdvance ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_ownProgDelayModel != NULL);
  NS_ASSERT (m_anotherProgDelayModel != NULL);

  m_timingAdvance_s = m_ownProgDelayModel->GetDelay ( m_ownMobility, m_geoSatMobility ) +
    m_anotherProgDelayModel->GetDelay ( m_anotherMobility, m_geoSatMobility );

}


void SatMobilityObserver::SatelliteStatusChanged ()
{
  NS_LOG_FUNCTION (this);

  double satelliteAltitude = m_geoSatMobility->GetGeoPosition ().GetAltitude ();

  // satellite is expected to be in the sky
  NS_ASSERT ( satelliteAltitude > 0.0 );

  // calculate maximum distance where UT or GW can be on the Earth.
  double satelliteRadius = satelliteAltitude + m_earthRadius;
  m_maxDistanceToSatellite = std::sqrt ( (satelliteRadius * satelliteRadius) - (m_earthRadius * m_earthRadius) );

  // calculate ratio of the earth and satellite radius
  m_radiusRatio = m_earthRadius / satelliteRadius;
}

} // namespace ns3
