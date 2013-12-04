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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

#include "ns3/log.h"
#include "satellite-rx-error-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatRxErrorTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRxErrorTag);


SatRxErrorTag::SatRxErrorTag ():
  m_error (false)
{
  NS_LOG_FUNCTION (this);
}

SatRxErrorTag::~SatRxErrorTag ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatRxErrorTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRxErrorTag")
    .SetParent<Tag> ()
    .AddConstructor<SatRxErrorTag> ()
  ;
  return tid;
}
TypeId
SatRxErrorTag::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

void
SatRxErrorTag::SetError (bool error)
{
  NS_LOG_FUNCTION (this << error);
  m_error = error;
}

bool
SatRxErrorTag::GetError (void) const
{
  NS_LOG_FUNCTION (this);
  return m_error;
}

uint32_t
SatRxErrorTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);

  return ( sizeof (m_error) );
}
void
SatRxErrorTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);

  i.Write ((uint8_t*)&m_error, sizeof (m_error));
}

void
SatRxErrorTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);

  i.Read ((uint8_t*)&m_error, sizeof (m_error));
}

void
SatRxErrorTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "error=" << m_error;
}


} // namespace ns3
