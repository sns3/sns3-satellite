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
    .SetParent<Queue> ()
    .AddConstructor<SatQueue> ()
    .AddAttribute ("Mode",
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&SatQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this SatQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&SatQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this SatQueue.",
                   UintegerValue (100 * 65535),
                   MakeUintegerAccessor (&SatQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

SatQueue::SatQueue () :
  Queue (),
  m_packets (),
  m_bytesInQueue (0),
  m_lastResetTime (Seconds (0.0))

{
  NS_LOG_FUNCTION (this);
}

SatQueue::~SatQueue ()
{
  NS_LOG_FUNCTION (this);
}

void
SatQueue::SetMode (SatQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

SatQueue::QueueMode
SatQueue::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

bool
SatQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  if (m_mode == QUEUE_MODE_PACKETS && (m_packets.size () >= m_maxPackets))
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
    {
      NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (!m_queueEventCallback.IsNull() && m_packets.empty ())
    {
      m_queueEventCallback (SatQueue::FIRST_BUFFERED_PKT, 0);
    }

  m_bytesInQueue += p->GetSize ();
  m_packets.push (p);

  m_enquedBytesSinceReset += p->GetSize ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
  NS_LOG_LOGIC ("Number of bytes since last reset: " << m_enquedBytesSinceReset);

  return true;
}

Ptr<Packet>
SatQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();
  m_packets.pop ();
  m_bytesInQueue -= p->GetSize ();

  m_dequedBytesSinceReset += p->GetSize ();

  if (!m_queueEventCallback.IsNull() && m_packets.empty ())
    {
      m_queueEventCallback (SatQueue::BUFFER_EMPTY, 0);
    }

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);
  NS_LOG_LOGIC ("Number of bytes since last reset: " << m_dequedBytesSinceReset);

  return p;
}

Ptr<const Packet>
SatQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<Packet> p = m_packets.front ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

double
SatQueue::GetEnqueBitRate ()
{
  double bitrate (-1.0);
  if (Simulator::Now () <= m_lastResetTime)
    {
      Time duration = Simulator::Now () - m_lastResetTime;
      bitrate = 8.0 * m_enquedBytesSinceReset / duration.GetSeconds ();
    }

  NS_LOG_LOGIC ("Enque bitrate: " << bitrate);

  return bitrate;
}

double
SatQueue::GetDequeBitRate ()
{
  double bitrate (-1.0);
  if (Simulator::Now () <= m_lastResetTime)
    {
      Time duration = Simulator::Now () - m_lastResetTime;
      bitrate = 8.0 * m_dequedBytesSinceReset / duration.GetSeconds ();
    }

  NS_LOG_LOGIC ("Deque bitrate: " << bitrate);

  return bitrate;
}

void
SatQueue::ResetStatistics ()
{
  m_enquedBytesSinceReset = 0.0;
  m_dequedBytesSinceReset = 0.0;
  m_lastResetTime = Simulator::Now ();
}

void
SatQueue::SetQueueEventCallback (SatQueue::QueueEventCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_queueEventCallback = cb;
}


} // namespace ns3



