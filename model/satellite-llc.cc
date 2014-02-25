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
#include "ns3/queue.h"
#include "ns3/satellite-queue.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mac48-address.h"

#include "satellite-llc.h"
#include "satellite-mac-tag.h"
#include "satellite-time-tag.h"
#include "satellite-encapsulator.h"
#include "satellite-return-link-encapsulator.h"
#include "satellite-scheduling-object.h"
#include "satellite-control-message.h"
#include "satellite-node-info.h"
#include "satellite-enums.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLlc);

TypeId
SatLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLlc")
    .SetParent<Object> ()
    .AddConstructor<SatLlc> ()
    .AddAttribute ("ControlTxQueue",
                   "A queue to use as the transmit queue for control packets in the device.",
                   PointerValue (),
                   MakePointerAccessor (&SatLlc::m_controlQueue),
                   MakePointerChecker<Queue> ())
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatLlc::m_packetTrace))
  ;
  return tid;
}

SatLlc::SatLlc ()
:m_nodeInfo (),
 m_requestManager (),
 m_encaps (),
 m_decaps (),
 m_controlQueue (),
 m_controlRcIndex (0)
{
  NS_LOG_FUNCTION (this);
}

SatLlc::~SatLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatLlc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rxCallback.Nullify ();

  if (m_controlQueue)
    {
      m_controlQueue->DoDispose ();
      m_controlQueue = 0;
    }

  EncapContainer_t::iterator it;

  for ( it = m_encaps.begin(); it != m_encaps.end(); ++it)
    {
      it->second->DoDispose ();
      it->second = 0;
    }
  m_encaps.clear ();

  for ( it = m_decaps.begin(); it != m_decaps.end(); ++it)
    {
      it->second->DoDispose ();
      it->second = 0;
    }
  m_decaps.clear ();

  if (m_requestManager)
    {
      m_requestManager->DoDispose ();
      m_requestManager = 0;
    }
  Object::DoDispose ();
}

void
SatLlc::SetQueue (Ptr<SatQueue> queue)
{
  NS_LOG_FUNCTION (this << queue);
  m_controlQueue = queue;
}

Ptr<SatQueue>
SatLlc::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);
  return m_controlQueue;
}

bool
SatLlc::Enque (Ptr<Packet> packet, Address dest, uint8_t tos)
{
  NS_LOG_FUNCTION (this << packet << dest);
  NS_LOG_LOGIC ("p=" << packet );
  NS_LOG_LOGIC ("dest=" << dest );
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  SatControlMsgTag cTag;
  bool cSuccess = packet->PeekPacketTag (cTag);

  // Control PDUs use separate queue, since they do not need
  // encapsulation, fragmentation nor packing.
  if (cSuccess && cTag.GetMsgType() != SatControlMsgTag::SAT_NON_CTRL_MSG)
     {
        // Store packet arrival time
        SatTimeTag timeTag (Simulator::Now ());
        packet->AddPacketTag (timeTag);

        // Add MAC tag
        SatMacTag mTag;
        mTag.SetDestAddress (dest);
        mTag.SetSourceAddress (m_nodeInfo->GetMacAddress ());
        packet->AddPacketTag (mTag);

        SatRcIndexTag rcTag;
        rcTag.SetRcIndex (m_controlRcIndex);
        packet->AddPacketTag (rcTag);

        // Enque the control packet
        m_controlQueue->Enqueue (packet);
     }
  else if (cSuccess)
    {
      NS_FATAL_ERROR ("LLC does not support any other broadcast messages than control messages!");
    }
  else
    {
      /**
       * TODO: Here we should do the mapping between ToS and RC indeces! Currently
       * all the non-control packets are just using RC index 1.
       */
      uint32_t rcIndex (1);

      // UT: user own mac address
      // GW: use destination address
      Mac48Address mac = ( m_nodeInfo->GetNodeType () == SatEnums::NT_UT ? m_nodeInfo->GetMacAddress () : Mac48Address::ConvertFrom (dest) );
      EncapKey_t key = std::make_pair<Mac48Address, uint32_t> (mac, rcIndex);

      EncapContainer_t::iterator it = m_encaps.find (key);

      if (it != m_encaps.end ())
        {
          it->second->TransmitPdu (packet);
        }
      else
        {
          NS_FATAL_ERROR ("Key: (" << mac << ", " << rcIndex << ") not found in the encapsulator container!");
        }
    }

  SatEnums::SatLinkDir_t ld =
      (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

  // Add packet trace entry:
  m_packetTrace (Simulator::Now(),
                 SatEnums::PACKET_ENQUE,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_LLC,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  return true;
}

Ptr<Packet>
SatLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint32_t &bytesLeft )
{
  NS_LOG_FUNCTION (this << macAddr << bytes);

  Ptr<Packet> packet;

  // Prioritize control packets
  if (m_controlQueue->GetNPackets() )
    {
      uint32_t cPacketSize = m_controlQueue->Peek ()->GetSize();

      // If the control packet fits into the time slot
      if (cPacketSize <= bytes)
        {
          bytesLeft = m_controlQueue->GetNBytes() - cPacketSize;
          packet = m_controlQueue->Dequeue ();
        }
    }
  // Forward the txOpportunity to a certain encapsulator
  else
    {
      uint32_t rcIndex (1);
      EncapKey_t key = std::make_pair<Mac48Address, uint32_t> (macAddr, rcIndex);
      EncapContainer_t::iterator it = m_encaps.find (key);

      if (it != m_encaps.end ())
        {
          packet = it->second->NotifyTxOpportunity (bytes, bytesLeft);
        }
      else
        {
          NS_FATAL_ERROR ("Key: (" << macAddr << ", " << rcIndex << ") not found in the encapsulator container!");
        }
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
SatLlc::Receive (Ptr<Packet> packet, Mac48Address macAddr)
{
  NS_LOG_FUNCTION (this << macAddr << packet);

  SatEnums::SatLinkDir_t ld =
      (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_FORWARD : SatEnums::LD_RETURN;

  // Add packet trace entry:
  m_packetTrace (Simulator::Now(),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_LLC,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  // Receive packet with a decapsulator instance which is handling the
  // packets for this specific id
  SatRcIndexTag rcTag;
  bool mSuccess = packet->PeekPacketTag (rcTag);
  if (mSuccess)
    {
      uint32_t rcIndex = rcTag.GetRcIndex ();
      EncapKey_t key = std::make_pair<Mac48Address, uint32_t> (macAddr, rcIndex);
      EncapContainer_t::iterator it = m_decaps.find (key);

      // Note: control messages should not be seen at the LLC layer, since
      // they are received already at the MAC layer.
      if (it != m_decaps.end ())
        {
          it->second->ReceivePdu (packet);
        }
      else
        {
          NS_FATAL_ERROR ("Key: (" << macAddr << ", " << rcIndex << ") not found in the encapsulator container!");
        }
    }
  else
    {
      NS_FATAL_ERROR ("MAC tag not found in the packet!");
    }
}

void
SatLlc::ReceiveHigherLayerPdu (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Call a callback to receive the packet at upper layer
  m_rxCallback (packet);
}

void
SatLlc::AddRequestManager (Ptr<SatRequestManager> rm)
{
  NS_LOG_FUNCTION (this);
  m_requestManager = rm;
}


void
SatLlc::AddEncap (Mac48Address macAddr, Ptr<SatEncapsulator> enc, uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << macAddr << rcIndex);

  EncapKey_t key = std::make_pair<Mac48Address, uint32_t> (macAddr, rcIndex);
  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it == m_encaps.end ())
    {
      // Create a new transmission queue for the encapsulator
      Ptr<SatQueue> queue = CreateObject<SatQueue> (rcIndex);

      // Callback to Request manager
      SatQueue::QueueEventCallback rmCb = MakeCallback (&SatRequestManager::ReceiveQueueEvent, m_requestManager);
      queue->AddQueueEventCallback (rmCb);
      enc->SetQueue (queue);

      m_encaps.insert(std::make_pair (key, enc));
    }
  else
    {
      NS_FATAL_ERROR ("Encapsulator container already holds (" << macAddr << ", " << rcIndex << ") key!");
    }
}

void
SatLlc::AddDecap (Mac48Address macAddr, Ptr<SatEncapsulator> dec, uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << macAddr << rcIndex);

  EncapKey_t key = std::make_pair<Mac48Address, uint32_t> (macAddr, rcIndex);
  EncapContainer_t::iterator it = m_decaps.find (key);

  if (it == m_decaps.end ())
    {
      m_decaps.insert(std::make_pair (key, dec));
    }
  else
    {
      NS_FATAL_ERROR ("Decapsulator container already holds (" << macAddr << ", " << rcIndex << ") key!");
    }
}

void
SatLlc::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  m_nodeInfo = nodeInfo;
}

std::vector< Ptr<SatSchedulingObject> > SatLlc::GetSchedulingContexts () const
{
  // Head of link queuing delay
  Time holDelay;

  std::vector< Ptr<SatSchedulingObject> > schedObjects;

  // First fill the control scheduling object
  if (m_controlQueue->GetNPackets ())
    {
      // Calculate HoL delay
      SatTimeTag tag;
      Ptr<const Packet> p = m_controlQueue->Peek ();
      p->PeekPacketTag (tag);
      holDelay = Simulator::Now () - tag.GetSenderTimestamp ();

      Ptr<SatSchedulingObject> so =
          Create<SatSchedulingObject> (Mac48Address::GetBroadcast (), m_controlQueue->GetNBytes (), p->GetSize(), holDelay, 0 );
      schedObjects.push_back (so);
    }

  // Then the user data
  for (EncapContainer_t::const_iterator cit = m_encaps.begin ();
      cit != m_encaps.end ();
      ++cit)
    {
      uint32_t buf = cit->second->GetTxBufferSizeInBytes ();

      if (buf > 0)
        {
          holDelay = cit->second->GetHolDelay ();
          uint32_t minTxOpportunityInBytes = cit->second->GetMinTxOpportunityInBytes ();
          Ptr<SatSchedulingObject> so =
              Create<SatSchedulingObject> (cit->first.first, buf, minTxOpportunityInBytes, holDelay, 1);
          schedObjects.push_back (so);
        }
    }
  return schedObjects;
}

void
SatLlc::SetReceiveCallback (SatLlc::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}


void
SatLlc::SetQueueSatisticsCallbacks ()
{
  // Control queue = rcIndex 0
  SatRequestManager::QueueCallback queueCb = MakeCallback (&SatQueue::GetQueueStatistics, m_controlQueue);
  m_requestManager->AddQueueCallback ((uint8_t)(0), queueCb);

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
SatLlc::GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const
{
  uint32_t packets (0);
  m_controlQueue->GetNumSmallerPackets (maxPacketSizeBytes);
  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
      it != m_encaps.end ();
      ++it)
    {
      packets += it->second->GetQueue()->GetNumSmallerPackets (maxPacketSizeBytes);
    }
  return packets;
}

bool
SatLlc::BuffersEmpty () const
{
  if (!m_controlQueue->IsEmpty ())
    {
      return false;
    }

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
      it != m_encaps.end ();
      ++it)
    {
      if (!(it->second->GetQueue ()->IsEmpty ()))
        {
          return false;
        }
    }
  return true;
}

} // namespace ns3


