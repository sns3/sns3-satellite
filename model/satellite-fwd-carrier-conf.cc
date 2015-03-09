/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
#include "ns3/fatal-error.h"
#include "satellite-fwd-carrier-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatFwdCarrierConf");

namespace ns3 {

// Forward Link Carrier conf

SatFwdCarrierConf::SatFwdCarrierConf ()
  : m_allocatedBandwidthInHz (0.0),
    m_occupiedBandwidthInHz (0.0),
    m_effectiveBandwidthInHz (0.0)
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatFwdCarrierConf::SatFwdCarrierConf (double bandwidthInHz, double rollOff, double spacing)
  : m_allocatedBandwidthInHz (bandwidthInHz)
{
  NS_LOG_FUNCTION (this << bandwidthInHz << rollOff << spacing);

  if ((spacing < 0.00 ) || ( spacing > 1.00 )
      || (rollOff < 0.00 ) || ( rollOff > 1.00 ))
    {
      NS_FATAL_ERROR ("Unvalid carrier spacing and/or roll-off parameters!");
    }

  m_occupiedBandwidthInHz = m_allocatedBandwidthInHz / (rollOff + 1.00);
  m_effectiveBandwidthInHz = m_allocatedBandwidthInHz / ((spacing + 1.00) * (rollOff + 1.00));
}

SatFwdCarrierConf::~SatFwdCarrierConf ()
{
  NS_LOG_FUNCTION (this);
}

}; // namespace ns3
