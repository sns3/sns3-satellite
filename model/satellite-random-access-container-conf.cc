/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
#include "satellite-random-access-container-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatRandomAccessConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRandomAccessConf);

TypeId 
SatRandomAccessConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomAccessConf")
    .SetParent<Object> ();
  return tid;
}

SatRandomAccessConf::SatRandomAccessConf () :
  m_slottedAlohaMin (0.5),
  m_slottedAlohaMax (2.0),
  m_crdsaMin (1),
  m_crdsaMax (160),
  m_crdsaSetSize (3)
{
  NS_LOG_FUNCTION (this);
}

SatRandomAccessConf::~SatRandomAccessConf ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
