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
 * Author: Jani Puttonen <sami.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mac48-address.h"

#include "satellite-llc.h"
#include "satellite-mac-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLlc);

TypeId
SatLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLlc")
    .SetParent<Object> ()
    .AddConstructor<SatLlc> ()
    .AddAttribute ("TxQueue",
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&SatLlc::m_queue),
                   MakePointerChecker<Queue> ())
    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("LlcTx",
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&SatLlc::m_llcTxTrace))
    .AddTraceSource ("LlcTxDrop",
                     "Trace source indicating a packet has been dropped by the device before transmission",
                     MakeTraceSourceAccessor (&SatLlc::m_llcTxDropTrace))
    .AddTraceSource ("LlcPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&SatLlc::m_llcPromiscRxTrace))
    .AddTraceSource ("LlcRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&SatLlc::m_llcRxTrace))
    #if 0
    // Not currently implemented for this device
    .AddTraceSource ("LlcRxDrop",
                     "Trace source indicating a packet was dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&SatLlc::m_llcRxDropTrace))
    #endif
  ;
  return tid;
}

SatLlc::SatLlc ()
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
  m_queue->DequeueAll();
  m_queue = 0;
  m_rxCallback.Nullify();
  Object::DoDispose ();
}

void
SatLlc::SetQueue (Ptr<Queue> queue)
{
  NS_LOG_FUNCTION (this << queue);
  m_queue = queue;
}

Ptr<Queue>
SatLlc::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

bool
SatLlc::Enque ( Ptr<Packet> packet, Address dest )
{
  NS_LOG_FUNCTION (this << packet << dest);
  NS_LOG_LOGIC ("p=" << packet );
  NS_LOG_LOGIC ("dest=" << dest );
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  // TODO: Think again the process of setting both destination
  // and source MAC addresses to the packet.
  SatMacTag tag;
  tag.SetDestAddress (dest);
  packet->AddPacketTag (tag);

  m_llcTxTrace (packet);

  if (m_queue->Enqueue (packet) == false)
    {
       // Enqueue may fail (overflow)
      m_llcTxDropTrace (packet);
      return false;
    }
  else
    {
      return true;
    }

  return true;
}

Ptr<Packet>
SatLlc::NotifyTxOpportunity(uint32_t /*bytes*/)
{
  Ptr<Packet> packet (NULL);
  if ( m_queue->GetNPackets() > 0)
    {
      return m_queue->Dequeue ();
    }
  return packet;
}

void
SatLlc::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  m_llcRxTrace (packet);

  // Call a callback to receive the packet at upper layer
  m_rxCallback (packet);
}


void
SatLlc::SetReceiveCallback (SatLlc::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}


} // namespace ns3


