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

#include <ns3/satellite-isl-arbiter-unicast.h>

NS_LOG_COMPONENT_DEFINE ("SatIslArbiterUnicast");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatIslArbiterUnicast);

TypeId
SatIslArbiterUnicast::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatIslArbiterUnicast")
          .SetParent<SatIslArbiter> ()
  ;
  return tid;
}

SatIslArbiterUnicast::SatIslArbiterUnicast(Ptr<Node> node, std::map<uint32_t, uint32_t> nextHopMap)
 : SatIslArbiter (node)
{
  NS_LOG_FUNCTION (this << node);
  m_nextHopMap = nextHopMap;
}

int32_t
SatIslArbiterUnicast::Decide (int32_t sourceSatId, int32_t targetSatId, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << sourceSatId << targetSatId << pkt);

  if (m_nextHopMap.count (targetSatId) == 0)
    {
      return -1;
    }
  return m_nextHopMap[targetSatId];
}

std::string
SatIslArbiterUnicast::StringReprOfForwardingState ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream res;
  res << "Unicast state of node " << m_nodeId << std::endl;

  std::map<uint32_t, uint32_t>::iterator nextHopMapIterator;

  std::map<uint32_t, std::vector<uint32_t>> mapReversed;
  std::map<uint32_t, std::vector<uint32_t>>::iterator mapReversedIterator;

  for (nextHopMapIterator = m_nextHopMap.begin (); nextHopMapIterator != m_nextHopMap.end (); nextHopMapIterator++)
    {
      if (mapReversed.count (nextHopMapIterator->second) == 0)
        {
          mapReversed[nextHopMapIterator->second] = std::vector <uint32_t> ();
        }
      mapReversed[nextHopMapIterator->second].push_back (nextHopMapIterator->first);
    }

  for (mapReversedIterator = mapReversed.begin (); mapReversedIterator != mapReversed.end (); mapReversedIterator++)
    {
      res << mapReversedIterator->first << "  -> : {" << std::endl;
      bool first = true;
      for (uint32_t targetId : mapReversedIterator->second) {
          if (!first) {
              res << "," << std::endl;
          }
          res << "  " << targetId;
          first = false;
      }
      res << std::endl << "}" << std::endl;
    }

  return res.str();
}

}
