/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
 * Copyright (c) 2020 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/type-id.h>
#include <ns3/log.h>

#include "satellite-traffic-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatelliteTrafficHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTrafficHelper);

TypeId
SatTrafficHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTrafficHelper")
    .SetParent<Object> ()
    .AddConstructor<SatTrafficHelper> ();
  return tid;
}

TypeId
SatTrafficHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatTrafficHelper::SatTrafficHelper ()
{
	//TODO
}

} // namespace ns3