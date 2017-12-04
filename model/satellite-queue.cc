/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
#include "ns3/enum.h"
#include "ns3/singleton.h"
#include "ns3/uinteger.h"
#include "satellite-queue.h"
#include "satellite-utils.h"
#include "satellite-const-variables.h"
#include "satellite-log.h"

NS_LOG_COMPONENT_DEFINE ("SatQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatQueue);


TypeId SatQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatQueue")
    .SetParent<Object> ()
    .AddConstructor<SatQueue> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this SatQueue.",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&SatQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Enqueue",
                     "Enqueue a packet in the queue.",
                     MakeTraceSourceAccessor (&SatQueue::m_traceEnqueue),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Dequeue",
                     "Dequeue a packet from the queue.",
                     MakeTraceSourceAccessor (&SatQueue::m_traceDequeue),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Drop",
                     "Drop a packet stored in the queue.",
                     MakeTraceSourceAccessor (&SatQueue::m_traceDrop),
                     "ns3::Packet::TracedCallback")
  ;

  return tid;
}

SatQueue::SatQueue ()
  : Object (),
    m_packets (),
    m_maxPackets (0),
    m_flowId (0),
    m_nBytes (0),
    m_nTotalReceivedBytes (0),
    m_nPackets (0),
    m_nTotalReceivedPackets (0),
    m_nTotalDroppedBytes (0),
    m_nTotalDroppedPackets (),
    m_nEnqueBytesSinceReset (0),
    m_nDequeBytesSinceReset (0),
    m_statResetTime (0)
{
  NS_LOG_FUNCTION (this);
}


SatQueue::SatQueue (uint8_t flowId)
  : Object (),
    m_packets (),
    m_maxPackets (0),
    m_flowId (flowId),
    m_nBytes (0),
    m_nTotalReceivedBytes (0),
    m_nPackets (0),
    m_nTotalReceivedPackets (0),
    m_nTotalDroppedBytes (0),
    m_nTotalDroppedPackets (),
    m_nEnqueBytesSinceReset (0),
    m_nDequeBytesSinceReset (0),
    m_statResetTime (Seconds (0.0))
{
  NS_LOG_FUNCTION (this);
}

SatQueue::~SatQueue ()
{
  NS_LOG_FUNCTION (this);
}

void SatQueue::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  for (EventCallbackContainer_t::iterator it = m_queueEventCallbacks.begin ();
       it != m_queueEventCallbacks.end ();
       ++it)
    {
      (*it).Nullify ();
    }

  DequeueAll ();
  Object::DoDispose ();
}

void
SatQueue::SetFlowId (uint32_t flowId)
{
  NS_LOG_FUNCTION (this << flowId);
  m_flowId = flowId;
}

bool
SatQueue::IsEmpty () const
{
  NS_LOG_FUNCTION (this);
  return m_packets.empty ();
}

bool
SatQueue::Enqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  NS_LOG_INFO ("Enque " << p->GetSize () << " bytes");

  if (m_packets.size () >= m_maxPackets)
    {
      NS_LOG_INFO ("Queue full (at max packets) -- dropping pkt");

      std::stringstream msg;
      msg << "SatQueue is full: packet dropped!";
      msg << " at: " << Now ().GetSeconds () << "s";
      msg << " MaxPackets: " << m_maxPackets;
      Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());

      Drop (p);
      return false;
    }

  bool emptyBeforeEnque = m_packets.empty ();

  m_nBytes += p->GetSize ();
  ++m_nPackets;

  m_nTotalReceivedBytes += p->GetSize ();
  ++m_nTotalReceivedPackets;

  m_nEnqueBytesSinceReset += p->GetSize ();

  m_packets.push_back (p);

  NS_LOG_INFO ("Number packets " << m_packets.size ());
  NS_LOG_INFO ("Number bytes " << m_nBytes);
  m_traceEnqueue (p);

  if (emptyBeforeEnque == true)
    {
      SendEvent (SatQueue::FIRST_BUFFERED_PKT);
    }
  else
    {
      SendEvent (SatQueue::BUFFERED_PKT);
    }

  return true;
}

Ptr<Packet>
SatQueue::Dequeue ()
{
  NS_LOG_FUNCTION (this);

  if (IsEmpty ())
    {
      NS_LOG_INFO ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();
  m_packets.pop_front ();

  m_nBytes -= p->GetSize ();
  --m_nPackets;

  m_nDequeBytesSinceReset += p->GetSize ();

  NS_LOG_INFO ("Popped " << p);
  NS_LOG_INFO ("Number packets " << m_packets.size ());
  NS_LOG_INFO ("Number bytes " << m_nBytes);
  m_traceDequeue (p);

  return p;
}

Ptr<const Packet>
SatQueue::Peek (void) const
{
  NS_LOG_FUNCTION (this);

  if (IsEmpty ())
    {
      NS_LOG_INFO ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_INFO ("Number packets " << m_packets.size ());
  NS_LOG_INFO ("Number bytes " << m_nBytes);

  return p;
}

void
SatQueue::PushFront (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  m_packets.push_front (p);

  ++m_nPackets;
  m_nBytes += p->GetSize ();

  m_nDequeBytesSinceReset -= p->GetSize ();
}

void
SatQueue::DequeueAll (void)
{
  NS_LOG_FUNCTION (this);
  while (!IsEmpty ())
    {
      Dequeue ();
    }
}

void
SatQueue::Drop (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  m_nTotalDroppedPackets++;
  m_nTotalDroppedBytes += p->GetSize ();

  NS_LOG_INFO ("m_traceDrop (p)");
  m_traceDrop (p);
}

void
SatQueue::AddQueueEventCallback (SatQueue::QueueEventCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_queueEventCallbacks.push_back (cb);
}

void
SatQueue::SendEvent (SatQueue::QueueEvent_t event)
{
  NS_LOG_FUNCTION (this);

  for (EventCallbackContainer_t::const_iterator it = m_queueEventCallbacks.begin ();
       it != m_queueEventCallbacks.end ();
       ++it)
    {
      if (!(*it).IsNull ())
        {
          (*it)(event, m_flowId);
        }
    }
}

uint32_t
SatQueue::GetNPackets () const
{
  NS_LOG_FUNCTION (this);

  return m_nPackets;
}

uint32_t
SatQueue::GetNBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_nBytes;
}

uint32_t
SatQueue::GetTotalReceivedBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_nTotalReceivedBytes;
}

uint32_t
SatQueue::GetTotalReceivedPackets () const
{
  NS_LOG_FUNCTION (this);

  return m_nTotalReceivedPackets;
}

uint32_t
SatQueue::GetTotalDroppedBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_nTotalDroppedBytes;
}

uint32_t
SatQueue::GetTotalDroppedPackets () const
{
  NS_LOG_FUNCTION (this);

  return m_nTotalDroppedPackets;
}

SatQueue::QueueStats_t
SatQueue::GetQueueStatistics (bool reset)
{
  NS_LOG_FUNCTION (this << reset);

  QueueStats_t queueStats;
  Time duration = Simulator::Now () - m_statResetTime;

  if (duration.IsStrictlyPositive ())
    {
      queueStats.m_incomingRateKbps = SatConstVariables::BITS_PER_BYTE * m_nEnqueBytesSinceReset / (double)(SatConstVariables::BITS_IN_KBIT) / duration.GetSeconds ();
      queueStats.m_outgoingRateKbps = SatConstVariables::BITS_PER_BYTE * m_nDequeBytesSinceReset / (double)(SatConstVariables::BITS_IN_KBIT) / duration.GetSeconds ();
      queueStats.m_volumeInBytes = m_nEnqueBytesSinceReset;
      queueStats.m_volumeOutBytes = m_nDequeBytesSinceReset;
      queueStats.m_queueSizeBytes = GetNBytes ();

      if (reset)
        {
          ResetShortTermStatistics ();
        }
    }
  return queueStats;
}

void
SatQueue::ResetStatistics ()
{
  NS_LOG_FUNCTION (this);

  m_nBytes = 0;
  m_nTotalReceivedBytes = 0;
  m_nPackets = 0;
  m_nTotalReceivedPackets = 0;
  m_nTotalDroppedBytes = 0;
  m_nTotalDroppedPackets = 0;
}

void
SatQueue::ResetShortTermStatistics ()
{
  NS_LOG_FUNCTION (this);

  m_nEnqueBytesSinceReset = 0;
  m_nDequeBytesSinceReset = 0;
  m_statResetTime = Simulator::Now ();
}

uint32_t
SatQueue::GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const
{
  NS_LOG_FUNCTION (this << maxPacketSizeBytes);

  uint32_t packets (0);
  for (PacketContainer_t::const_iterator it = m_packets.begin ();
       it != m_packets.end ();
       ++it)
    {
      if ((*it)->GetSize () <= maxPacketSizeBytes)
        {
          ++packets;
        }
      else
        {
          break;
        }
    }
  return packets;
}

} // namespace ns3



