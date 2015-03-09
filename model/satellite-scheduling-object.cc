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
  : m_macAddress (),
    m_bufferedBytes (0),
    m_minTxOpportunity (0),
    m_holDelay (Seconds (0.0)),
    m_flowId ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * Default constructor is not meant to be used!
   */
}


SatSchedulingObject::SatSchedulingObject (Mac48Address addr, uint32_t bytes, uint32_t minTxOpportunity, Time holDelay, uint8_t flowId)
  : m_macAddress (addr),
    m_bufferedBytes (bytes),
    m_minTxOpportunity (minTxOpportunity),
    m_holDelay (holDelay),
    m_flowId (flowId)
{
  NS_LOG_FUNCTION (this << addr << bytes << holDelay << (uint32_t) flowId);
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

uint32_t
SatSchedulingObject::GetMinTxOpportunityInBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_minTxOpportunity;
}

uint8_t
SatSchedulingObject::GetFlowId () const
{
  NS_LOG_FUNCTION (this);
  return m_flowId;
}

Time
SatSchedulingObject::GetHolDelay () const
{
  NS_LOG_FUNCTION (this);
  return m_holDelay;
}

} // namespace ns3
