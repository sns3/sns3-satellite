/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 CNES
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
 * Author: Joaquin Muguerza <jmuguerza@viveris.fr>
 */

#include "satellite-superframe-allocator.h"

#include <ns3/log.h>

#include <map>

NS_LOG_COMPONENT_DEFINE("SatSuperframeAllocator");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatSuperframeAllocator);

TypeId
SatSuperframeAllocator::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatSuperframeAllocator").SetParent<Object>();
    return tid;
}

TypeId
SatSuperframeAllocator::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatSuperframeAllocator::SatSuperframeAllocator(Ptr<SatSuperframeConf> superFrameConf)
    : m_superframeConf(superFrameConf)
{
    NS_LOG_FUNCTION(this);
}

SatSuperframeAllocator::~SatSuperframeAllocator()
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3
