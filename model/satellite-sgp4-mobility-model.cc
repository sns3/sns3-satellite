/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 INESC TEC
 * Copyright (c) 2021 CNES
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
 * Author: Pedro Silva  <pmms@inesctec.pt>
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */


#include "satellite-sgp4-mobility-model.h"

NS_LOG_COMPONENT_DEFINE ("sat-sgp4-mobility-model");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSGP4MobilityModel);


TypeId
SatSGP4MobilityModel::GetTypeId (void) {
  static TypeId tid = TypeId ("ns3::SatSGP4MobilityModel")
    .SetParent<SatMobilityModel> ()
    .AddConstructor<SatSGP4MobilityModel> ()
  ;

  return tid;
}

SatSGP4MobilityModel::SatSGP4MobilityModel () { }
SatSGP4MobilityModel::~SatSGP4MobilityModel () { }

JulianDate
SatSGP4MobilityModel::GetStartTime () const
{
  return JulianDate ();
  // TODO
  //return m_helper.GetStartTime ();
}

void
SatSGP4MobilityModel::SetStartTime (const JulianDate &t)
{
  // TODO
  //m_helper.SetStartTime (t);
}

Vector3D
SatSGP4MobilityModel::DoGetVelocity () const
{
  return Vector3D();
  // TODO
  //return m_helper.GetVelocity ();
}

GeoCoordinate
SatSGP4MobilityModel::DoGetGeoPosition () const
{
  NS_LOG_FUNCTION (this);

  std::cout << "DoGetGeoPosition" << std::endl;

  return GeoCoordinate (0.0, 0.0, 400.0*0 + 360000000.0);

  return m_geoPosition;
}
void
SatSGP4MobilityModel::DoSetGeoPosition (const GeoCoordinate &position)
{
  NS_LOG_FUNCTION (this << position);

  std::cout << "DoSetGeoPosition" << std::endl;

  m_geoPosition = position;
  NotifyGeoCourseChange ();
}

} // namespace ns3
