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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "satellite-interference-elimination.h"

NS_LOG_COMPONENT_DEFINE ("SatInterferenceElimination");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatInterferenceElimination);

TypeId
SatInterferenceElimination::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatInterferenceElimination")
    .SetParent<Object> ();

  return tid;
}

TypeId
SatInterferenceElimination::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatInterferenceElimination::SatInterferenceElimination ()
{

}

SatInterferenceElimination::~SatInterferenceElimination ()
{

}

}  // namespace ns3
