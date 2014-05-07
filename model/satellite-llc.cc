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
#include "satellite-base-encapsulator.h"
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
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatLlc::m_packetTrace))
  ;
  return tid;
}

SatLlc::SatLlc ()
:m_nodeInfo (),
 m_encaps (),
 m_decaps ()
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

  Object::DoDispose ();
}

bool
SatLlc::Enque (Ptr<Packet> packet, Address dest, uint8_t flowId)
{
  NS_LOG_FUNCTION (this << packet << dest);
  NS_LOG_LOGIC ("p=" << packet );
  NS_LOG_LOGIC ("dest=" << dest );
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  // UT: user own mac address
  // GW: use destination address
  Mac48Address mac = ( m_nodeInfo->GetNodeType () == SatEnums::NT_UT ? m_nodeInfo->GetMacAddress () : Mac48Address::ConvertFrom (dest) );
  EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (mac, flowId);

  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it != m_encaps.end ())
    {
      it->second->TransmitPdu (packet, Mac48Address::ConvertFrom (dest));
    }
  else
    {
      NS_FATAL_ERROR ("Key: (" << mac << ", " << flowId << ") not found in the encapsulator container!");
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
SatLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint8_t flowId, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << bytes << macAddr << flowId);
  NS_ASSERT (true);

  Ptr<Packet> packet;
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
  SatFlowIdTag flowIdTag;
  bool mSuccess = packet->PeekPacketTag (flowIdTag);
  if (mSuccess)
    {
      uint32_t flowId = flowIdTag.GetFlowId ();
      EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (macAddr, flowId);
      EncapContainer_t::iterator it = m_decaps.find (key);

      // Note: control messages should not be seen at the LLC layer, since
      // they are received already at the MAC layer and received as control
      // messages.
      if (flowId == SatEnums::CONTROL_FID)
        {
          NS_FATAL_ERROR ("Control messages should be terminated already at lower layer!");
        }
        
      if (it != m_decaps.end ())
        {
          it->second->ReceivePdu (packet);
        }
	    else
  	 	 {
	  	   NS_FATAL_ERROR ("MAC tag not found in the packet!");
  			}
    }
}


void
SatLlc::ReceiveAck (Ptr<SatArqAckMessage> ack, Mac48Address macAddr)
{
  NS_LOG_FUNCTION (this << macAddr);

  /**
   * Note, that the received ACK is forwarded to the proper encapsulator
   * instead of a decapsulator. The macAddress is the address of the UT
   * in case of both RTN and FWD links.
   */
  uint32_t flowId = ack->GetFlowId ();
  EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (macAddr, flowId);
  EncapContainer_t::iterator it = m_encaps.find (key);

  // Note: control messages should not be seen at the LLC layer, since
  // they are received already at the MAC layer.
  if (it != m_encaps.end ())
    {
      it->second->ReceiveAck (ack);
    }
  else
    {
      NS_FATAL_ERROR ("Key: (" << macAddr << ", " << flowId << ") not found in the encapsulator container!");
    }
}


void
SatLlc::ReceiveHigherLayerPdu (Ptr<Packet> packet, Mac48Address macAddr)
{
  NS_LOG_FUNCTION (this << packet);

  // Remove control msg tag
  SatControlMsgTag ctrlTag;
  bool cSuccess = packet->RemovePacketTag (ctrlTag);

  if (cSuccess)
    {
      if (ctrlTag.GetMsgType() != SatControlMsgTag::SAT_ARQ_ACK)
        {
          NS_FATAL_ERROR ("A control message other than ARQ ACK received at the LLC!");
        }

      // ARQ ACKs need to be forwarded to LLC/ARQ for processing
      uint32_t ackId = ctrlTag.GetMsgId ();

      Ptr<SatArqAckMessage> ack = DynamicCast<SatArqAckMessage> (m_readCtrlCallback (ackId));

      if ( ack == NULL )
        {
          NS_FATAL_ERROR ("ARQ ACK not found, check that control msg storage time is set long enough!");
        }

      ReceiveAck (ack, macAddr);
    }
  // Higher layer packet
  else
    {
      // Call a callback to receive the packet at upper layer
      m_rxCallback (packet);
    }
}

void
SatLlc::AddEncap (Mac48Address macAddr, Ptr<SatBaseEncapsulator> enc, uint8_t flowId)
{
  NS_LOG_FUNCTION (this << macAddr << flowId);

  EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (macAddr, flowId);
  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it == m_encaps.end ())
    {
      m_encaps.insert(std::make_pair (key, enc));
    }
  else
    {
      NS_FATAL_ERROR ("Encapsulator container already holds (" << macAddr << ", " << flowId << ") key!");
    }
}

void
SatLlc::AddDecap (Mac48Address macAddr, Ptr<SatBaseEncapsulator> dec, uint8_t flowId)
{
  NS_LOG_FUNCTION (this << macAddr << flowId);

  EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (macAddr, flowId);
  EncapContainer_t::iterator it = m_decaps.find (key);

  if (it == m_decaps.end ())
    {
      m_decaps.insert(std::make_pair (key, dec));
    }
  else
    {
      NS_FATAL_ERROR ("Decapsulator container already holds (" << macAddr << ", " << flowId << ") key!");
    }
}

void
SatLlc::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  m_nodeInfo = nodeInfo;
}

void SatLlc::GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const
{
  // Head of link queuing delay
  Time holDelay;

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
              Create<SatSchedulingObject> (cit->first.first, buf, minTxOpportunityInBytes, holDelay, cit->first.second);
          output.push_back (so);
        }
    }
}

void
SatLlc::SetReceiveCallback (SatLlc::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}


bool
SatLlc::BuffersEmpty () const
{
  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
      it != m_encaps.end ();
      ++it)
    {
      if (it->second->GetTxBufferSizeInBytes () > 0)
        {
          return false;
        }
    }
  return true;
}

uint32_t
SatLlc::GetNBytesInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first.first == utAddress)
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
SatLlc::GetNPacketsInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first.first == utAddress)
        {
          NS_ASSERT (it->second != 0);
          Ptr<SatQueue> queue = it->second->GetQueue ();
          NS_ASSERT (queue != 0);
          sum += queue->GetNPackets ();
        }
    }

  return sum;
}

uint32_t
SatLlc::GetNBytesInQueue () const
{
  NS_LOG_FUNCTION (this);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      NS_ASSERT (it->second != 0);
      Ptr<SatQueue> queue = it->second->GetQueue ();
      NS_ASSERT (queue != 0);
      sum += queue->GetNBytes ();
    }

  return sum;
}

uint32_t
SatLlc::GetNPacketsInQueue () const
{
  NS_LOG_FUNCTION (this);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      NS_ASSERT (it->second != 0);
      Ptr<SatQueue> queue = it->second->GetQueue ();
      NS_ASSERT (queue != 0);
      sum += queue->GetNPackets ();
    }

  return sum;
}

void
SatLlc::SetReadCtrlCallback (SatLlc::ReadCtrlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_readCtrlCallback = cb;
}


} // namespace ns3


