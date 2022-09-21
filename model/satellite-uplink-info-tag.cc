/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-uplink-info-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUplinkInfoTag);

SatUplinkInfoTag::SatUplinkInfoTag ()
  : m_satelliteReceptionTime (Seconds (0)),
  m_sinr (0.0),
  m_sinrCalculate (),
  m_sinrComputed (false)
{
  // Nothing to do here
}

SatUplinkInfoTag::SatUplinkInfoTag (Time satelliteReceptionTime, double sinr, Callback<double, double> sinrCalculate)
  : m_satelliteReceptionTime (satelliteReceptionTime),
  m_sinr (sinr),
  m_sinrCalculate (sinrCalculate),
  m_sinrComputed (true)
{
  // Nothing to do here
}

TypeId
SatUplinkInfoTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUplinkInfoTag")
    .SetParent<Tag> ()
    .AddConstructor<SatUplinkInfoTag> ();
  return tid;
}

TypeId
SatUplinkInfoTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatUplinkInfoTag::GetSerializedSize (void) const
{
  return sizeof(Time) + sizeof(double) + sizeof(Callback<double, double>) + sizeof(bool);
}

void
SatUplinkInfoTag::Serialize (TagBuffer i) const
{
  int64_t satelliteReceptionTime = m_satelliteReceptionTime.GetNanoSeconds ();
  i.Write ((const uint8_t *)&satelliteReceptionTime, sizeof(int64_t));

  i.WriteDouble (m_sinr);

  i.Write ((const uint8_t *)&m_sinrCalculate, sizeof(Callback<double, double>));

  i.WriteU8 (m_sinrComputed);
}

void
SatUplinkInfoTag::Deserialize (TagBuffer i)
{
  int64_t satelliteReceptionTime;
  i.Read ((uint8_t *)&satelliteReceptionTime, 8);
  m_satelliteReceptionTime = NanoSeconds (satelliteReceptionTime);

  m_sinr = i.ReadDouble ();

  i.Read ((uint8_t *)&m_sinrCalculate, 8);

  m_sinrComputed = i.ReadU8 ();
}

void
SatUplinkInfoTag::Print (std::ostream &os) const
{
  os << m_satelliteReceptionTime << " " << m_sinr;
}

Time
SatUplinkInfoTag::GetSatelliteReceptionTime (void) const
{
  return m_satelliteReceptionTime;
}

void
SatUplinkInfoTag::SetSatelliteReceptionTime (Time satelliteReceptionTime)
{
  m_satelliteReceptionTime = satelliteReceptionTime;
}

double
SatUplinkInfoTag::GetSinr (void) const
{
  return m_sinr;
}

void
SatUplinkInfoTag::SetSinr (double sinr, Callback<double, double> sinrCalculate)
{
  m_sinr = sinr;
  m_sinrCalculate = sinrCalculate;
}

Callback<double, double>
SatUplinkInfoTag::GetSinrCalculate (void) const
{
  return m_sinrCalculate;
}

bool
SatUplinkInfoTag::IsSinrComputed (void) const
{
  return m_sinrComputed;
}


} // namespace ns3

