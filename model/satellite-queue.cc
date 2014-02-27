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
#include "ns3/uinteger.h"
#include "satellite-queue.h"

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
                   UintegerValue (100),
                   MakeUintegerAccessor (&SatQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Enqueue",
                     "Enqueue a packet in the queue.",
                     MakeTraceSourceAccessor (&SatQueue::m_traceEnqueue))
    .AddTraceSource ("Dequeue",
                     "Dequeue a packet from the queue.",
                     MakeTraceSourceAccessor (&SatQueue::m_traceDequeue))
    .AddTraceSource ("Drop",
                     "Drop a packet stored in the queue.",
                     MakeTraceSourceAccessor (&SatQueue::m_traceDrop))
  ;

  return tid;
}

SatQueue::SatQueue ()
: Object (),
  m_packets (),
  m_maxPackets (0),
  m_rcIndex (0),
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

SatQueue::SatQueue (uint8_t rcIndex)
: Object (),
  m_packets (),
  m_maxPackets (0),
  m_rcIndex (rcIndex),
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
SatQueue::SetRcIndex (uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << rcIndex);
  m_rcIndex = rcIndex;
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
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("Enque " << p->GetSize () << " bytes");

  if (m_packets.size () >= m_maxPackets)
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- dropping pkt");
      Drop (p);
      return false;
    }

  if (m_packets.empty ())
    {
      SendEvent (SatQueue::FIRST_BUFFERED_PKT);
    }
  else
    {
      SendEvent (SatQueue::BUFFERED_PKT);
    }

  m_nBytes += p->GetSize ();
  ++m_nPackets;

  m_nTotalReceivedBytes += p->GetSize ();
  ++m_nTotalReceivedPackets;

  m_nEnqueBytesSinceReset += p->GetSize ();

  m_packets.push_back (p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_nBytes);
  m_traceEnqueue (p);

  return true;
}

Ptr<Packet>
SatQueue::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (IsEmpty())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();
  m_packets.pop_front ();

  m_nBytes -= p->GetSize ();
  --m_nPackets;

  m_nDequeBytesSinceReset += p->GetSize ();

  NS_LOG_LOGIC ("Popped " << p);
  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_nBytes);
  m_traceDequeue (p);

  return p;
}

Ptr<const Packet>
SatQueue::Peek (void) const
{
  NS_LOG_FUNCTION (this);

  if (IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_nBytes);

  return p;
}

void
SatQueue::PushFront (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
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
  NS_LOG_FUNCTION (this << p);

  m_nTotalDroppedPackets++;
  m_nTotalDroppedBytes += p->GetSize ();

  NS_LOG_LOGIC ("m_traceDrop (p)");
  m_traceDrop (p);
}

void
SatQueue::AddQueueEventCallback (SatQueue::QueueEventCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_queueEventCallbacks.push_back (cb);
}

void
SatQueue::SendEvent (QueueEvent_t event)
{
  for (EventCallbackContainer_t::const_iterator it = m_queueEventCallbacks.begin ();
      it != m_queueEventCallbacks.end ();
      ++it)
    {
      if (!(*it).IsNull())
        {
          (*it)(event, m_rcIndex);
        }
    }
}

uint32_t
SatQueue::GetNPackets () const
{
  return m_nPackets;
}

uint32_t
SatQueue::GetNBytes () const
{
  return m_nBytes;
}

uint32_t
SatQueue::GetTotalReceivedBytes () const
{
  return m_nTotalReceivedBytes;
}

uint32_t
SatQueue::GetTotalReceivedPackets () const
{
  return m_nTotalReceivedPackets;
}

uint32_t
SatQueue::GetTotalDroppedBytes () const
{
  return m_nTotalDroppedBytes;
}

uint32_t
SatQueue::GetTotalDroppedPackets () const
{
  return m_nTotalDroppedPackets;
}

SatQueue::QueueStats_t
SatQueue::GetQueueStatistics (bool reset)
{
  QueueStats_t queueStats;
  Time duration = Simulator::Now () - m_statResetTime;

  if (duration.IsPositive())
    {
      queueStats.m_incomingRateKbps = 8.0 * m_nEnqueBytesSinceReset / 1000.0 / duration.GetSeconds ();
      queueStats.m_outgoingRateKbps = 8.0 * m_nDequeBytesSinceReset / 1000.0 / duration.GetSeconds ();
      queueStats.m_volumeInBytes = m_nEnqueBytesSinceReset;
      queueStats.m_volumeInBytes = m_nDequeBytesSinceReset;
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
  m_nEnqueBytesSinceReset = 0;
  m_nDequeBytesSinceReset = 0;
  m_statResetTime = Simulator::Now ();
}

uint32_t
SatQueue::GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const
{
  uint32_t packets (0);
  for (PacketContainer_t::const_iterator it = m_packets.begin ();
      it != m_packets.end ();
      ++it)
    {
      if ((*it)->GetSize() <= maxPacketSizeBytes)
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



