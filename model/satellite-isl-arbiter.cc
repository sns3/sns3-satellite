/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Inspired and adapted from Hypatia: https://github.com/snkas/hypatia
 *
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/singleton.h>
#include <ns3/satellite-id-mapper.h>

#include "ns3/satellite-isl-arbiter.h"

NS_LOG_COMPONENT_DEFINE ("SatIslArbiter");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatIslArbiter);

TypeId
SatIslArbiter::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatIslArbiter")
          .SetParent<Object> ()
  ;
  return tid;
}

SatIslArbiter::SatIslArbiter(Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);

  m_nodeId = node->GetId ();
  m_satGeoNetDevice = nullptr;

  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      Ptr<SatGeoNetDevice> nd = DynamicCast<SatGeoNetDevice> (node->GetDevice (i));
      if (nd)
        {
          NS_ASSERT_MSG (m_satGeoNetDevice != nullptr, "Satellite node must not have more than one SatGeoNetDevice");
          m_satGeoNetDevice = nd;
        }
    }
  NS_ASSERT_MSG (m_satGeoNetDevice == nullptr, "Satellite node must not have one SatGeoNetDevice");
}

int32_t
SatIslArbiter::BaseDecide(Ptr<Packet> pkt, Mac48Address destination)
{
  NS_LOG_FUNCTION (this << pkt << destination);

  int32_t targetId = Singleton<SatIdMapper>::Get ()->GetSatIdWithMacIsl (destination);

  if (targetId == -1)
    {
      NS_FATAL_ERROR ("Cannot resolve target node ID");
    }

  return Decide(m_nodeId, targetId, pkt);
}

} // namespace ns3
