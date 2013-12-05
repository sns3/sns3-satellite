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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "satellite-scheduling-object.h"

NS_LOG_COMPONENT_DEFINE ("SatSchedulingObject");

namespace ns3 {


SatSchedulingObject::SatSchedulingObject ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}


SatSchedulingObject::SatSchedulingObject (Mac48Address addr, uint32_t bytes, Time holDelay, bool isControl)
  :m_macAddress (addr),
   m_bufferedBytes (bytes),
   m_holDelay (holDelay),
   m_isControl (isControl)
{
  NS_LOG_FUNCTION (this << addr << bytes << holDelay << isControl);
}

SatSchedulingObject::~SatSchedulingObject ()
{

}

Mac48Address
SatSchedulingObject::GetMacAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_macAddress;
}

uint32_t
SatSchedulingObject::GetBufferedBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_bufferedBytes;
}

bool
SatSchedulingObject::IsControl () const
{
  NS_LOG_FUNCTION (this);
  return m_isControl;
}

} // namespace ns3
