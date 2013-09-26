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

#include "satellite-wave-form-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatWaveFormConf");

namespace ns3 {

// BTU conf

SatWaveFormConf::SatWaveFormConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatWaveFormConf::SatWaveFormConf (double codingRate, uint32_t payloadLength_byte, uint32_t burstLength_sym)
  : m_codingRate (codingRate),
    m_payloadLength_byte (payloadLength_byte),
    m_burstLength_sym (burstLength_sym)
{
  NS_LOG_FUNCTION (this);
}

SatWaveFormConf::~SatWaveFormConf ()
{
  NS_LOG_FUNCTION (this);
}


}; // namespace ns3
