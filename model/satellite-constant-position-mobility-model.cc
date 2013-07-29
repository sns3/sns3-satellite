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

#include "satellite-constant-position-mobility-model.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatConstantPositionMobilityModel);

TypeId
SatConstantPositionMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatConstantPositionMobilityModel")
    .SetParent<SatMobilityModel> ()
    .AddConstructor<SatConstantPositionMobilityModel> ()
  ;
  return tid;
}

SatConstantPositionMobilityModel::SatConstantPositionMobilityModel ()
{
}
SatConstantPositionMobilityModel::~SatConstantPositionMobilityModel ()
{
}

GeoCoordinate
SatConstantPositionMobilityModel::DoGetGeoPosition (void) const
{
  return m_geoPosition;
}
void
SatConstantPositionMobilityModel::DoSetGeoPosition (const GeoCoordinate &position)
{
  m_geoPosition = position;
  NotifyGeoCourseChange ();
}

GeoCoordinate
SatConstantPositionMobilityModel::DoGetGeoVelocity (void) const
{
  return GeoCoordinate (0.0, 0.0, 0.0);
}

} // namespace ns3
