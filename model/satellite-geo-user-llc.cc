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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-geo-user-llc.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoUserLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoUserLlc);

TypeId
SatGeoUserLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoUserLlc")
    .SetParent<SatGeoLlc> ()
  ;
  return tid;
}

SatGeoUserLlc::SatGeoUserLlc ()
 : SatGeoLlc ()
{
  NS_LOG_FUNCTION (this);
}

SatGeoUserLlc::~SatGeoUserLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoUserLlc::DoDispose ()
{
  Object::DoDispose ();
}

} // namespace ns3


