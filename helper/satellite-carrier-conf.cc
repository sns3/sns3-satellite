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

#include "ns3/log.h"

#include "satellite-carrier-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatCarrierConf");

namespace ns3 {

SatCarrierConf::SatCarrierConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatCarrierConf::SatCarrierConf ( double frequency_hz, double bandwidth_hz )
  : m_frequency_hz (frequency_hz),
    m_bandwidth_hz (bandwidth_hz)
{
  NS_LOG_FUNCTION (this);
}

SatCarrierConf::~SatCarrierConf ()
{
  NS_LOG_FUNCTION (this);
}

}; // namespace ns3
