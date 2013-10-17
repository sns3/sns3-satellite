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
#include "satellite-fader.h"

NS_LOG_COMPONENT_DEFINE ("SatFader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFader);

TypeId 
SatFader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFader")
    .SetParent<Object> ()
    .AddAttribute ("Set Id", "The set id of this fader.",
                   TypeId::ATTR_GET,
                   UintegerValue (0),
                   MakeUintegerAccessor (&SatFader::m_setId),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("State Id", "The state id of this fader.",
                   TypeId::ATTR_GET,
                   UintegerValue (0),
                   MakeUintegerAccessor (&SatFader::m_stateId),
                   MakeUintegerChecker<uint32_t> ());
  return tid;
}

SatFader::SatFader ()
{
  NS_LOG_FUNCTION (this);
}

SatFader::~SatFader ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
