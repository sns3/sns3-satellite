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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "satellite-mac.h"
#include "satellite-phy.h"
#include "satellite-mac-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMac);

TypeId 
SatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMac")
    .SetParent<Object> ()
    .AddConstructor<SatMac> ()
    .AddAttribute ("TxQueue",
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&SatMac::m_queue),
                   MakePointerChecker<Queue> ())
    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("MacTx", 
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&SatMac::m_macTxTrace))
    .AddTraceSource ("MacTxDrop", 
                     "Trace source indicating a packet has been dropped by the device before transmission",
                     MakeTraceSourceAccessor (&SatMac::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&SatMac::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&SatMac::m_macRxTrace))
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("MacRxDrop", 
                     "Trace source indicating a packet was dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&SatMac::m_macRxDropTrace))
#endif

  ;
  return tid;
}

SatMac::SatMac ()
  : m_phy(0)
{
  NS_LOG_FUNCTION (this);
}

SatMac::~SatMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phy = 0;
  m_queue->DequeueAll();
  m_queue = 0;
  Object::DoDispose ();
}

void SatMac::SetAddress( Mac48Address macAddress )
{
  NS_LOG_FUNCTION (this << macAddress);
  m_macAddress = macAddress;
}

bool
SatMac::SetPhy (Ptr<SatPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phy = phy;
  return true;
}

Ptr<SatPhy>
SatMac::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
SatMac::SetQueue (Ptr<Queue> queue)
{
  NS_LOG_FUNCTION (this << queue);
  m_queue = queue;
}

Ptr<Queue>
SatMac::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

bool
SatMac::Send ( Ptr<Packet> packet, Address dest )
{
  NS_LOG_FUNCTION (this << packet << dest);
  NS_LOG_LOGIC ("p=" << packet );
  NS_LOG_LOGIC ("dest=" << dest );
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  // Add destination MAC address to the packet as a tag.
  SatMacTag tag;
  tag.SetDestAddress (dest);
  tag.SetSourceAddress (m_macAddress);
  packet->AddPacketTag (tag);

  m_macTxTrace (packet);

  if (m_queue->Enqueue (packet) == false)
    {
       // Enqueue may fail (overflow)
      m_macTxDropTrace (packet);
      return false;
    }
  else
    {
      return true;
    }

  return true;
}

void
SatMac::Receive (Ptr<Packet> packet, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this << packet);

  //
  // Hit the trace hooks.  All of these hooks are in the same place in this
  // device because it is so simple, but this is not usually the case in
  // more complicated devices.
  //
  m_snifferTrace (packet);
  m_promiscSnifferTrace (packet);

  m_macRxTrace (packet);

  SatMacTag msgTag;
  packet->RemovePacketTag (msgTag);

  NS_LOG_LOGIC("Packet to " << msgTag.GetDestAddress());
  NS_LOG_LOGIC("Receiver " << m_macAddress );

  // If the packet is intended for this receiver
  Mac48Address addr = Mac48Address::ConvertFrom (msgTag.GetDestAddress());

  if ( addr == m_macAddress || addr.IsBroadcast() )
    {
      m_rxCallback (packet);
    }
  else
    {
      NS_LOG_LOGIC("Packet intended for others received by MAC: " << m_macAddress );
    }
}

void
SatMac::SetReceiveCallback (SatMac::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

} // namespace ns3
