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

#include "ns3/simulator.h"
#include "ns3/log.h"

#include "satellite-ut-llc.h"
#include "satellite-return-link-encapsulator.h"
#include "satellite-node-info.h"
#include "satellite-enums.h"
#include "satellite-utils.h"
#include "satellite-queue.h"


NS_LOG_COMPONENT_DEFINE ("SatUtLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLlc);

TypeId
SatUtLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtLlc")
    .SetParent<SatLlc> ()
    .AddConstructor<SatUtLlc> ()
  ;
  return tid;
}

SatUtLlc::SatUtLlc ()
:m_requestManager ()
{
  NS_LOG_FUNCTION (this);
}

SatUtLlc::~SatUtLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatUtLlc::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  if (m_requestManager)
    {
      m_requestManager->DoDispose ();
      m_requestManager = 0;
    }
  SatLlc::DoDispose ();
}


Ptr<Packet>
SatUtLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint32_t &bytesLeft)
{
  NS_LOG_FUNCTION (this << macAddr << bytes);

  Ptr<Packet> packet;

  /**
   * TODO: This is not the final implementation! The NotifyTxOpportunity
   * will be enhanced with the flow id which to serve. The decision is passed
   * then to the scheduler (UT/NCC or forward link).
   */
  EncapKey_t key;
  // Check whether there are some control messages
  if (m_nodeInfo->GetNodeType () == SatEnums::NT_UT)
    {
      key = std::make_pair<Mac48Address, uint8_t> (macAddr, m_controlFlowIndex);
    }
  else if (m_nodeInfo->GetNodeType () == SatEnums::NT_GW)
    {
      key = std::make_pair<Mac48Address, uint8_t> (Mac48Address::GetBroadcast (), m_controlFlowIndex);
    }

  EncapContainer_t::iterator it = m_encaps.find (key);

  if (!it->second->GetQueue ()->IsEmpty())
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft);
    }

  key = std::make_pair<Mac48Address, uint8_t> (macAddr, 1);
  it = m_encaps.find (key);
  if (!packet)
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft);
    }

  if (packet)
    {
      SatEnums::SatLinkDir_t ld =
          (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

      // Add packet trace entry:
      m_packetTrace (Simulator::Now(),
                     SatEnums::PACKET_SENT,
                     m_nodeInfo->GetNodeType (),
                     m_nodeInfo->GetNodeId (),
                     m_nodeInfo->GetMacAddress (),
                     SatEnums::LL_LLC,
                     ld,
                     SatUtils::GetPacketInfo (packet));
    }

  return packet;
}

void
SatUtLlc::AddRequestManager (Ptr<SatRequestManager> rm)
{
  NS_LOG_FUNCTION (this);
  m_requestManager = rm;
}

void
SatUtLlc::SetQueueStatisticsCallbacks ()
{
  // Control queue = rcIndex 0
  SatRequestManager::QueueCallback queueCb;
  for (EncapContainer_t::iterator it = m_encaps.begin ();
      it != m_encaps.end ();
      ++it)
    {
      // Set the callback for each RLE queue
      queueCb = MakeCallback (&SatQueue::GetQueueStatistics, it->second->GetQueue ());
      m_requestManager->AddQueueCallback (it->first.second, queueCb);
    }
 }

uint32_t
SatUtLlc::GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const
{
  uint32_t packets (0);
  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
      it != m_encaps.end ();
      ++it)
    {
      packets += it->second->GetQueue()->GetNumSmallerPackets (maxPacketSizeBytes);
    }
  return packets;
}

} // namespace ns3


