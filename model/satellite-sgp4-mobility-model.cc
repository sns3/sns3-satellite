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

#include "ns3/simulator.h"
#include "ns3/string.h"
#include "vector-extensions.h"

NS_LOG_COMPONENT_DEFINE ("sat-sgp4-mobility-model");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSGP4MobilityModel);

const gravconsttype SatSGP4MobilityModel::WGeoSys = wgs72;   // recommended for SGP4/SDP4
const uint32_t SatSGP4MobilityModel::TleSatInfoWidth = 69;

TypeId
SatSGP4MobilityModel::GetTypeId (void) {
  static TypeId tid = TypeId ("ns3::SatSGP4MobilityModel")
    .SetParent<SatMobilityModel> ()
    .AddConstructor<SatSGP4MobilityModel> ()
    .AddAttribute ("StartDateStr",
                   "Absolute start time of simulation (UTC)",
                   StringValue ("1992-01-01 00:00:00"),
                   MakeStringAccessor (&SatSGP4MobilityModel::m_startStr),
                   MakeStringChecker ())
  ;
  return tid;
}

TypeId
SatSGP4MobilityModel::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatSGP4MobilityModel::SatSGP4MobilityModel ()
  : m_startStr ("1992-01-01 00:00:00")
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());
  SetStartTime (JulianDate (m_startStr));
}

SatSGP4MobilityModel::~SatSGP4MobilityModel () { }

bool
SatSGP4MobilityModel::IsInitialized () const
{
  return ((m_sgp4_record.jdsatepoch > 0) && (m_tle1 != "") && (m_tle2 != ""));
}

JulianDate
SatSGP4MobilityModel::GetStartTime () const
{
  return m_start;
}

void
SatSGP4MobilityModel::SetStartTime (const JulianDate &t)
{
  m_start = t;
}

Vector3D
SatSGP4MobilityModel::DoGetVelocity () const
{
  JulianDate cur = m_start + Simulator::Now ();

  double r[3], v[3];
  double delta = (cur - GetTleEpoch ()).GetMinutes();

  if (!IsInitialized ())
    return Vector3D ();

  sgp4 (WGeoSys, m_sgp4_record, delta, r, v);

  if (m_sgp4_record.error != 0)
    return Vector3D ();

  // velocity vector is in km/s so it needs to be converted to m/s
  return 1000*rvTemeTovItrf (Vector3D (r[0], r[1], r[2]), Vector3D (v[0], v[1], v[2]), cur);
}

Vector
SatSGP4MobilityModel::DoGetPosition () const
{
  NS_LOG_FUNCTION (this);

  return DoGetGeoPosition().ToVector ();
}

void
SatSGP4MobilityModel::DoSetPosition (const Vector &position)
{
  NS_LOG_FUNCTION (this << position);

  DoSetGeoPosition(GeoCoordinate (position));
}

GeoCoordinate
SatSGP4MobilityModel::DoGetGeoPosition () const
{
  NS_LOG_FUNCTION (this);

  JulianDate cur = m_start + Simulator::Now ();

  double r[3], v[3];
  double delta = (cur - GetTleEpoch ()).GetMinutes();

  if (!IsInitialized ())
    return Vector3D ();

  sgp4 (WGeoSys, m_sgp4_record, delta, r, v);

  if (m_sgp4_record.error != 0)
    return Vector3D ();

  // vector r is in km so it needs to be converted to meters
  m_lastPosition = rTemeTorItrf (Vector3D (r[0], r[1], r[2]), cur)*1000;

  return m_lastPosition;
}

void
SatSGP4MobilityModel::DoSetGeoPosition (const GeoCoordinate &position)
{
  NS_LOG_FUNCTION (this << position);

  m_lastPosition = position;
  NotifyGeoCourseChange ();
}

JulianDate
SatSGP4MobilityModel::GetTleEpoch (void) const {
  if (IsInitialized ())
    return JulianDate (m_sgp4_record.jdsatepoch);

  return JulianDate ();
}

bool
SatSGP4MobilityModel::SetTleInfo (const std::string &tle)
{
  uint32_t delimPos = tle.find("\n");
  const std::string line1 = tle.substr(0, delimPos);
  const std::string line2 = tle.substr(delimPos + 1, tle.size() - delimPos);

  double start, stop, delta;
  char l1[TleSatInfoWidth+1], l2[TleSatInfoWidth+1];
  double r[3], v[3];

  NS_ASSERT_MSG (
    line1.size () == TleSatInfoWidth && line2.size () == TleSatInfoWidth,
    "Two-Line Element info lines must be of length" << TleSatInfoWidth << "!"
  );

  m_tle1 = std::string (line1.c_str ());
  m_tle2 = std::string (line2.c_str ());

  memcpy (l1, line1.c_str (), sizeof (l1));
  memcpy (l2, line2.c_str (), sizeof (l2));

  // 'c' => catalog mode run
  // 'e' => epoch time (relative to TLE lines)
  // 'i' => improved mode of operation
  twoline2rv (
    l1, l2, 'c', 'e', 'i', WGeoSys, start, stop, delta, m_sgp4_record
  );

  // call propagator to check if it has been properly initialized
  sgp4 (WGeoSys, m_sgp4_record, 0, r, v);

  if (m_sgp4_record.error == 0)
  {
    DoGetGeoPosition();
    return true;
  }

  return false;
}

Vector3D
SatSGP4MobilityModel::rTemeTorItrf (const Vector3D &rteme, const JulianDate &t)
{
  Matrix pmt = PefToItrf (t);                   // PEF->ITRF matrix transposed
  Matrix tmt = TemeToPef (t);                   // TEME->PEF matrix

  return pmt*(tmt*rteme);
}

Vector3D
SatSGP4MobilityModel::rvTemeTovItrf(
  const Vector3D &rteme, const Vector3D &vteme, const JulianDate &t
)
{
  Matrix pmt = PefToItrf (t);                   // PEF->ITRF matrix transposed
  Matrix tmt = TemeToPef (t);                   // TEME->PEF matrix
  Vector3D w (0.0, 0.0, t.GetOmegaEarth ());

  return pmt*((tmt*vteme) - CrossProduct (w, tmt*rteme));
}

SatSGP4MobilityModel::Matrix
SatSGP4MobilityModel::PefToItrf (const JulianDate &t)
{
  std::pair<double, double> eop = t.GetPolarMotion ();

  const double &xp = eop.first, &yp = eop.second;
  const double cosxp = cos (xp), cosyp = cos (yp);
  const double sinxp = sin (xp), sinyp = sin (yp);

  // [from AIAA-2006-6753 Report, Page 32, Appendix C - TEME Coordinate System]
  //
  // Matrix(ITRF<->PEF) = ROT1(yp)*ROT2(xp) [using c for cos, and s for sin]
  //
  // | 1    0     0   |*| c(xp) 0 -s(xp) |=|    c(xp)       0      -s(xp)   |
  // | 0  c(yp) s(yp) | |   0   1    0   | | s(yp)*s(xp)  c(yp) s(yp)*c(xp) |
  // | 0 -s(yp) c(yp) | | s(xp) 0  c(xp) | | c(yp)*s(xp) -s(yp) c(yp)*c(xp) |
  //

  // we return the transpose because it is what's needed
  return Matrix(
     cosxp, sinyp*sinxp, cosyp*sinxp,
       0,      cosyp,      -sinyp,
    -sinxp, sinyp*cosxp, cosyp*cosxp
  );
}

SatSGP4MobilityModel::Matrix
SatSGP4MobilityModel::TemeToPef (const JulianDate &t)
{
  const double gmst = t.GetGmst ();
  const double cosg = cos (gmst), sing = sin (gmst);

  // [from AIAA-2006-6753 Report, Page 32, Appendix C - TEME Coordinate System]
  //
  // rPEF = ROT3(gmst)*rTEME
  //
  // |  cos(gmst) sin(gmst) 0 |
  // | -sin(gmst) cos(gmst) 0 |
  // |      0         0     1 |
  //

  return Matrix(
     cosg, sing, 0,
    -sing, cosg, 0,
       0,    0,  1
  );
}

SatSGP4MobilityModel::Matrix::Matrix (
  double c00, double c01, double c02,
  double c10, double c11, double c12,
  double c20, double c21, double c22
)
{
  m[0][0] = c00;
  m[0][1] = c01;
  m[0][2] = c02;
  m[1][0] = c10;
  m[1][1] = c11;
  m[1][2] = c12;
  m[2][0] = c20;
  m[2][1] = c21;
  m[2][2] = c22;
}

Vector3D
SatSGP4MobilityModel::Matrix::operator* (const Vector3D& v) const
{
  return Vector3D (
    m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
    m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
    m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
  );
}

} // namespace ns3
