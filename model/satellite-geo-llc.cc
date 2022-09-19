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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-geo-llc.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoLlc);

TypeId
SatGeoLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoLlc")
    .SetParent<SatLlc> ()
  ;
  return tid;
}

SatGeoLlc::SatGeoLlc ()
{
  NS_LOG_FUNCTION (this);

  m_encap = CreateObject<SatBaseEncapsulator> ();
}

SatGeoLlc::~SatGeoLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoLlc::DoDispose ()
{
  Object::DoDispose ();
}

bool
SatGeoLlc::Enque (Ptr<Packet> packet, Address dest, uint8_t flowId)
{
  NS_LOG_FUNCTION (this << packet << dest << (uint32_t) flowId);
  NS_LOG_INFO ("p=" << packet );
  NS_LOG_INFO ("dest=" << dest );
  NS_LOG_INFO ("UID is " << packet->GetUid ());

  m_encap->EnquePdu (packet, Mac48Address::ConvertFrom (dest));

  return true;
}

Ptr<Packet>
SatGeoLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t rcIndex, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << utAddr << bytes << (uint32_t) rcIndex);

  Ptr<Packet> packet = m_encap->NotifyTxOpportunity (bytes, bytesLeft, nextMinTxO);

  return packet;
}

} // namespace ns3


