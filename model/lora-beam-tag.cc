/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/log.h>

#include "lora-beam-tag.h"

NS_LOG_COMPONENT_DEFINE ("LoraBeamTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoraBeamTag);

TypeId
LoraBeamTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraBeamTag")
    .SetParent<Tag> ()
    .AddConstructor<LoraBeamTag> ()
  ;
  return tid;
}

TypeId
LoraBeamTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

LoraBeamTag::LoraBeamTag (uint8_t beamId) :
  m_beamId (beamId)
{
}

LoraBeamTag::~LoraBeamTag ()
{
}

uint32_t
LoraBeamTag::GetSerializedSize (void) const
{
  return 1;
}

void
LoraBeamTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_beamId);
}

void
LoraBeamTag::Deserialize (TagBuffer i)
{
  m_beamId = i.ReadU8 ();
}

void
LoraBeamTag::Print (std::ostream &os) const
{
  os << m_beamId;
}

uint8_t
LoraBeamTag::GetBeamId () const
{
  return m_beamId;
}


void
LoraBeamTag::SetBeamId (uint8_t beamId)
{
  m_beamId = beamId;
}

} // namespace ns3
