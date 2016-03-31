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
#include <ns3/satellite-queue.h>
#include <ns3/nstime.h>

#include "satellite-llc.h"
#include <ns3/satellite-time-tag.h>
#include <ns3/satellite-scheduling-object.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-utils.h>
#include <ns3/satellite-typedefs.h>


NS_LOG_COMPONENT_DEFINE ("SatLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLlc);

TypeId
SatLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLlc")
    .SetParent<Object> ()
    .AddAttribute ("FwdLinkArqEnabled",
                   "Enable ARQ in forward link.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatLlc::m_fwdLinkArqEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("RtnLinkArqEnabled",
                   "Enable ARQ in return link.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatLlc::m_rtnLinkArqEnabled),
                   MakeBooleanChecker ())
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatLlc::m_packetTrace),
                     "ns3::PacketTraceCallback")
  ;
  return tid;
}

SatLlc::SatLlc ()
  : m_nodeInfo (),
    m_encaps (),
    m_decaps (),
    m_fwdLinkArqEnabled (false),
    m_rtnLinkArqEnabled (false),
    m_gwAddress ()
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

  for ( it = m_encaps.begin (); it != m_encaps.end (); ++it)
    {
      it->second->DoDispose ();
      it->second = 0;
    }
  m_encaps.clear ();

  for ( it = m_decaps.begin (); it != m_decaps.end (); ++it)
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
  NS_LOG_FUNCTION (this << packet << dest << (uint32_t) flowId);
  NS_LOG_INFO ("p=" << packet );
  NS_LOG_INFO ("dest=" << dest );
  NS_LOG_INFO ("UID is " << packet->GetUid ());

  Ptr<EncapKey> key = Create<EncapKey> (m_nodeInfo->GetMacAddress (), Mac48Address::ConvertFrom (dest), flowId);

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

  // Store packet arrival time
  SatTimeTag timeTag (Simulator::Now ());
  packet->AddPacketTag (timeTag);

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

void
SatLlc::Receive (Ptr<Packet> packet, Mac48Address source, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << source << dest << packet);

  SatEnums::SatLinkDir_t ld =
    (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_FORWARD : SatEnums::LD_RETURN;

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
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
      Ptr<EncapKey> key = Create<EncapKey> (source, dest, flowId);
      EncapContainer_t::iterator it = m_decaps.find (key);

      // Control messages not received by this method
      if (flowId == SatEnums::CONTROL_FID)
        {
          NS_FATAL_ERROR ("Control messages should not be received by SatLlc::Receive () method!");
        }

      if (it == m_decaps.end ())
        {
          /**
           * Decapsulator not found, thus create a new one. This method is
           * implemented in the inherited classes, which knows which type
           * of decapsulator to create.
           */
          CreateDecap (key);
          it = m_decaps.find (key);
        }

      it->second->ReceivePdu (packet);
    }
}


void
SatLlc::ReceiveAck (Ptr<SatArqAckMessage> ack, Mac48Address source, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << source << dest);

  /**
   * Note, that the received ACK is forwarded to the proper encapsulator
   * instead of a decapsulator. The source and destination MAC addresses reflect
   * to the ACK direction, thus they are turned around to represent the other direction
   * when mapping it to the encapsulator.
   */
  uint32_t flowId = ack->GetFlowId ();

  Ptr<EncapKey> key = Create<EncapKey> (dest, source, flowId);
  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it != m_encaps.end ())
    {
      it->second->ReceiveAck (ack);
    }
  else
    {
      NS_FATAL_ERROR ("Encapsulator not found for key (" << source << ", " << dest << ", " << (uint32_t) flowId << ")");
    }
}


void
SatLlc::ReceiveHigherLayerPdu (Ptr<Packet> packet, Mac48Address source, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << packet << source << dest);

  // Remove time tag
  SatTimeTag timeTag;
  packet->RemovePacketTag (timeTag);

  // Remove control msg tag
  SatControlMsgTag ctrlTag;
  bool cSuccess = packet->RemovePacketTag (ctrlTag);

  if (cSuccess)
    {
      if (ctrlTag.GetMsgType () != SatControlMsgTag::SAT_ARQ_ACK)
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

      ReceiveAck (ack, source, dest);
    }
  // Higher layer packet
  else
    {
      // Call a callback to receive the packet at upper layer
      m_rxCallback (packet);
    }
}

void
SatLlc::AddEncap (Mac48Address source, Mac48Address dest, uint8_t flowId, Ptr<SatBaseEncapsulator> enc)
{
  NS_LOG_FUNCTION (this << source << dest << (uint32_t) flowId);

  Ptr<EncapKey> key = Create<EncapKey> (source, dest, flowId);
  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it == m_encaps.end ())
    {
      NS_LOG_INFO ("Add encapsulator with key (" << source << ", " << dest << ", " << (uint32_t) flowId << ")");

      std::pair<EncapContainer_t::iterator, bool> result = m_encaps.insert (std::make_pair (key, enc));
      if (result.second == false)
        {
          NS_FATAL_ERROR ("Insert to map with key (" << source << ", " << dest << ", " << (uint32_t) flowId << ") failed!");
        }
    }
  else
    {
      NS_FATAL_ERROR ("Encapsulator container already holds key (" << source << ", " << dest << ", " <<  (uint32_t) flowId << ") key!");
    }
}

void
SatLlc::AddDecap (Mac48Address source, Mac48Address dest, uint8_t flowId, Ptr<SatBaseEncapsulator> dec)
{
  NS_LOG_FUNCTION (this << source << dest << (uint32_t) flowId);

  Ptr<EncapKey> key = Create<EncapKey> (source, dest, flowId);
  EncapContainer_t::iterator it = m_decaps.find (key);

  if (it == m_decaps.end ())
    {
      NS_LOG_INFO ("Add Decapsulator with key (" << source << ", " << dest << ", " << (uint32_t) flowId << ")");

      std::pair<EncapContainer_t::iterator, bool> result = m_decaps.insert (std::make_pair (key, dec));
      if (result.second == false)
        {
          NS_FATAL_ERROR ("Insert to map with key (" << source << ", " << dest << ", " << (uint32_t) flowId << ") failed!");
        }
    }
  else
    {
      NS_FATAL_ERROR ("Decapsulator container already holds (" << source << ", " << dest << ", " << (uint32_t) flowId << ") key!");
    }
}

void
SatLlc::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  m_nodeInfo = nodeInfo;
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

bool
SatLlc::ControlBuffersEmpty () const
{
  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end ();
       ++it)
    {
      if (it->first->m_flowId == SatEnums::CONTROL_FID)
        {
          if (it->second->GetTxBufferSizeInBytes () > 0)
            {
              return false;
            }
        }
    }
  return true;
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

void
SatLlc::SetCtrlMsgCallback (SatBaseEncapsulator::SendCtrlCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_sendCtrlCallback = cb;
}

void
SatLlc::SetGwAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_gwAddress = address;
}

} // namespace ns3


