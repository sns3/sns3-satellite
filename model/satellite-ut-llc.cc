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
    .AddAttribute ("SatRequestManager",
                   "The Satellite request manager attached to this UT LLC.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtLlc::GetRequestManager),
                   MakePointerChecker<SatRequestManager> ())
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
SatUtLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << macAddr << bytes);

  Ptr<Packet> packet;
  EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (macAddr, rcIndex);

  EncapContainer_t::iterator it = m_encaps.find (key);

  uint32_t bytesLeft (0);

  if (it != m_encaps.end ())
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft);
    }
  else
    {
      NS_FATAL_ERROR ("Key: (" << macAddr << ", " << rcIndex << ") not found in the encapsulator container!");
    }

  if (packet)
    {
      SatEnums::SatLinkDir_t ld = SatEnums::LD_RETURN;

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
SatUtLlc::SetRequestManager (Ptr<SatRequestManager> rm)
{
  NS_LOG_FUNCTION (this);
  m_requestManager = rm;
}

Ptr<SatRequestManager>
SatUtLlc::GetRequestManager () const
{
  return m_requestManager;
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

void
SatUtLlc::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);

  m_requestManager->SetNodeInfo (nodeInfo);
  SatLlc::SetNodeInfo (nodeInfo);
}

} // namespace ns3


