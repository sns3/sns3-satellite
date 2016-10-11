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
#include "satellite-fading-input-trace.h"

NS_LOG_COMPONENT_DEFINE ("SatFadingInputTrace");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFadingInputTrace);

TypeId
SatFadingInputTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingInputTrace")
    .SetParent<SatBaseFading> ()
    .AddConstructor<SatFadingInputTrace> ();
  return tid;
}

SatFadingInputTrace::SatFadingInputTrace (Ptr<SatFadingInputTraceContainer> satFadingInputTraceContainer)
  : m_satFadingInputTraceContainer (satFadingInputTraceContainer)
{
  NS_LOG_FUNCTION (this);
}

SatFadingInputTrace::SatFadingInputTrace ()
  : m_satFadingInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatFadingInputTrace::SatFadingInputTrace - Constructor not in use.");
}

SatFadingInputTrace::~SatFadingInputTrace ()
{
  NS_LOG_FUNCTION (this);
}

double
SatFadingInputTrace::DoGetFading (Address macAddress, SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this);

  return m_satFadingInputTraceContainer->GetFadingValue (std::make_pair (macAddress,channelType));
}

} // namespace ns3
