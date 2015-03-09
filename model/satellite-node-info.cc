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

#include <ns3/log.h>
#include "satellite-node-info.h"

NS_LOG_COMPONENT_DEFINE ("SatNodeInfo");

namespace ns3 {


SatNodeInfo::SatNodeInfo ()
  : m_nodeId (0),
    m_nodeType (SatEnums::NT_UNDEFINED),
    m_macAddress ()
{

}

SatNodeInfo::SatNodeInfo (SatEnums::SatNodeType_t nodeType, uint32_t nodeId, Mac48Address macAddress)
  : m_nodeId (nodeId),
    m_nodeType (nodeType),
    m_macAddress (macAddress)
{

}

SatNodeInfo::~SatNodeInfo ()
{

}

uint32_t
SatNodeInfo::GetNodeId () const
{
  NS_LOG_FUNCTION (this);
  return m_nodeId;
}


SatEnums::SatNodeType_t
SatNodeInfo::GetNodeType () const
{
  NS_LOG_FUNCTION (this);
  return m_nodeType;
}

Mac48Address
SatNodeInfo::GetMacAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_macAddress;
}

} // namespace ns3



