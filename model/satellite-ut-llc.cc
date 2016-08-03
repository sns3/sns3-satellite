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

#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/pointer.h>

#include "satellite-ut-llc.h"
#include <ns3/satellite-base-encapsulator.h>
#include <ns3/satellite-return-link-encapsulator.h>
#include <ns3/satellite-return-link-encapsulator-arq.h>
#include <ns3/satellite-generic-stream-encapsulator.h>
#include <ns3/satellite-generic-stream-encapsulator-arq.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-utils.h>
#include <ns3/satellite-request-manager.h>
#include <ns3/packet.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-scheduling-object.h>


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
                   "The request manager of this UT.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtLlc::GetRequestManager,
                                        &SatUtLlc::SetRequestManager),
                   MakePointerChecker<SatRequestManager> ())
  ;
  return tid;
}

SatUtLlc::SatUtLlc ()
  : m_requestManager ()
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

  m_macQueueEventCb.Nullify ();

  SatLlc::DoDispose ();
}

bool
SatUtLlc::Enque (Ptr<Packet> packet, Address dest, uint8_t flowId)
{
  NS_LOG_FUNCTION (this << packet << dest << (uint32_t) flowId);
  NS_LOG_INFO ("p=" << packet );
  NS_LOG_INFO ("dest=" << dest );
  NS_LOG_INFO ("UID is " << packet->GetUid ());

  Mac48Address destMacAddress = Mac48Address::ConvertFrom (dest);

  // all multicast traffic is delivered with GW address
  // in order to avoid supporting several encaps in UT
  // in return link there is only one receiver (GW) for the multicast traffic for the UT
  if ( destMacAddress.IsGroup ())
    {
      destMacAddress = m_gwAddress;
    }

  Ptr<EncapKey> key = Create<EncapKey> (m_nodeInfo->GetMacAddress (), destMacAddress, flowId);

  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it == m_encaps.end ())
    {
      /**
       * Encapsulator not found, thus create a new one. This method is
       * implemented in the inherited classes, which knows which type
       * of encapsulator to create.
       */
      CreateEncap (key);
      it = m_encaps.find (key);
    }

  it->second->EnquePdu (packet, Mac48Address::ConvertFrom (dest));

  SatEnums::SatLinkDir_t ld =
    (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
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
SatUtLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t rcIndex, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << utAddr << bytes << (uint32_t) rcIndex);

  Ptr<Packet> packet;
  Ptr<EncapKey> key = Create<EncapKey> (utAddr, m_gwAddress, rcIndex);
  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it != m_encaps.end ())
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft, nextMinTxO);

      if (packet)
        {
          SatEnums::SatLinkDir_t ld = SatEnums::LD_RETURN;

          // Add packet trace entry:
          m_packetTrace (Simulator::Now (),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_LLC,
                         ld,
                         SatUtils::GetPacketInfo (packet));
        }
    }
  /*
   * At the UT the UT scheduler may give a Tx opportunity to a encapsulator which
   * does not exist, since it does not exact knowledge that which encapsulators
   * have been dynamically created. Thus, it is not an error case, where the
   * encapsulator is not found!
  */

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
  NS_LOG_FUNCTION (this);

  return m_requestManager;
}

void
SatUtLlc::SetQueueStatisticsCallbacks ()
{
  NS_LOG_FUNCTION (this);

  // Control queue = rcIndex 0
  SatRequestManager::QueueCallback queueCb;
  for (EncapContainer_t::iterator it = m_encaps.begin ();
       it != m_encaps.end ();
       ++it)
    {
      // Set the callback for each RLE queue
      queueCb = MakeCallback (&SatQueue::GetQueueStatistics, it->second->GetQueue ());
      m_requestManager->AddQueueCallback (it->first->m_flowId, queueCb);
    }
}

uint32_t
SatUtLlc::GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const
{
  NS_LOG_FUNCTION (this << maxPacketSizeBytes);

  uint32_t packets (0);
  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end ();
       ++it)
    {
      packets += it->second->GetQueue ()->GetNumSmallerPackets (maxPacketSizeBytes);
    }
  return packets;
}

void
SatUtLlc::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this);

  m_requestManager->SetNodeInfo (nodeInfo);
  SatLlc::SetNodeInfo (nodeInfo);
}

void
SatUtLlc::CreateEncap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_source << key->m_destination << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> utEncap;

  if (m_rtnLinkArqEnabled)
    {
      utEncap = CreateObject<SatReturnLinkEncapsulatorArq> (key->m_source, key->m_destination, key->m_flowId);
    }
  else
    {
      utEncap = CreateObject<SatReturnLinkEncapsulator> (key->m_source, key->m_destination, key->m_flowId);
    }

  Ptr<SatQueue> queue = CreateObject<SatQueue> (key->m_flowId);
  queue->AddQueueEventCallback (m_macQueueEventCb);
  queue->AddQueueEventCallback (MakeCallback (&SatRequestManager::ReceiveQueueEvent, m_requestManager));

  // Set the callback for each RLE queue
  SatRequestManager::QueueCallback queueCb = MakeCallback (&SatQueue::GetQueueStatistics, queue);
  m_requestManager->AddQueueCallback (key->m_flowId, queueCb);

  utEncap->SetQueue (queue);

  NS_LOG_INFO ("Create encapsulator with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ")");

  // Store the encapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_encaps.insert (std::make_pair (key, utEncap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

void
SatUtLlc::CreateDecap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_source << key->m_destination << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> utDecap;

  if (m_fwdLinkArqEnabled)
    {
      utDecap = CreateObject<SatGenericStreamEncapsulatorArq> (key->m_source, key->m_destination, key->m_flowId);
    }
  else
    {
      utDecap = CreateObject<SatGenericStreamEncapsulator> (key->m_source, key->m_destination, key->m_flowId);
    }

  utDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, this));
  utDecap->SetCtrlMsgCallback (m_sendCtrlCallback);

  NS_LOG_INFO ("Create decapsulator with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ")");

  // Store the decapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_decaps.insert (std::make_pair (key, utDecap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

uint32_t
SatUtLlc::GetNBytesInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first->m_source == utAddress)
        {
          NS_ASSERT (it->second != 0);
          Ptr<SatQueue> queue = it->second->GetQueue ();
          NS_ASSERT (queue != 0);
          sum += queue->GetNBytes ();
        }
    }

  return sum;
}

uint32_t
SatUtLlc::GetNPacketsInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first->m_source == utAddress)
        {
          NS_ASSERT (it->second != 0);
          Ptr<SatQueue> queue = it->second->GetQueue ();
          NS_ASSERT (queue != 0);
          sum += queue->GetNPackets ();
        }
    }

  return sum;
}

void
SatUtLlc::GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * This is not yet implemented to the UT LLC
   */
}

void
SatUtLlc::SetMacQueueEventCallback (SatQueue::QueueEventCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_macQueueEventCb = cb;
}
} // namespace ns3


