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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#include "satellite-base-trace-container.h"

NS_LOG_COMPONENT_DEFINE ("SatBaseTraceContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBaseTraceContainer);

const uint32_t SatBaseTraceContainer::RX_POWER_TRACE_DEFAULT_RX_POWER_DENSITY_INDEX;
const uint32_t SatBaseTraceContainer::RX_POWER_TRACE_DEFAULT_NUMBER_OF_COLUMNS;
const uint32_t SatBaseTraceContainer::RX_CNO_TRACE_DEFAULT_RX_POWER_DENSITY_INDEX;
const uint32_t SatBaseTraceContainer::RX_CNO_TRACE_DEFAULT_NUMBER_OF_COLUMNS;
const uint32_t SatBaseTraceContainer::INTF_TRACE_DEFAULT_INTF_DENSITY_INDEX;
const uint32_t SatBaseTraceContainer::INTF_TRACE_DEFAULT_NUMBER_OF_COLUMNS;
const uint32_t SatBaseTraceContainer::FADING_TRACE_DEFAULT_FADING_VALUE_INDEX;
const uint32_t SatBaseTraceContainer::FADING_TRACE_DEFAULT_NUMBER_OF_COLUMNS;
const uint32_t SatBaseTraceContainer::CSINR_TRACE_DEFAULT_FADING_VALUE_INDEX;
const uint32_t SatBaseTraceContainer::CSINR_TRACE_DEFAULT_NUMBER_OF_COLUMNS;
const uint32_t SatBaseTraceContainer::POSITION_TRACE_DEFAULT_LATITUDE_INDEX;
const uint32_t SatBaseTraceContainer::POSITION_TRACE_DEFAULT_LONGITUDE_INDEX;
const uint32_t SatBaseTraceContainer::POSITION_TRACE_DEFAULT_ALTITUDE_INDEX;
const uint32_t SatBaseTraceContainer::POSITION_TRACE_DEFAULT_NUMBER_OF_COLUMNS;

TypeId
SatBaseTraceContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBaseTraceContainer")
    .SetParent<Object> ();
  return tid;
}

TypeId
SatBaseTraceContainer::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatBaseTraceContainer::SatBaseTraceContainer ()
{
  NS_LOG_FUNCTION (this);
}

SatBaseTraceContainer::~SatBaseTraceContainer ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
