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


#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-queue.h"
#include "satellite-time-tag.h"
#include "satellite-mac-tag.h"
#include "satellite-base-encapsulator.h"

NS_LOG_COMPONENT_DEFINE ("SatBaseEncapsulator");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatBaseEncapsulator);

SatBaseEncapsulator::SatBaseEncapsulator ()
  : m_sourceAddress (),
    m_destAddress (),
    m_flowId (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * Default constructor is not meant to be used!
   */
}

SatBaseEncapsulator::SatBaseEncapsulator (Mac48Address source, Mac48Address dest, uint8_t flowId)
  : m_sourceAddress (source),
    m_destAddress (dest),
    m_flowId (flowId)
{
  NS_LOG_FUNCTION (this);
}

SatBaseEncapsulator::~SatBaseEncapsulator ()
{
  NS_LOG_FUNCTION (this);
  m_txQueue = NULL;
  m_rxCallback.Nullify ();
}

TypeId SatBaseEncapsulator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBaseEncapsulator")
    .SetParent<Object> ()
  ;
  return tid;
}

void
SatBaseEncapsulator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_txQueue)
    {
      m_txQueue->DoDispose ();
    }
  m_rxCallback.Nullify ();
  m_ctrlCallback.Nullify ();
}

void
SatBaseEncapsulator::EnquePdu (Ptr<Packet> p, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // Add flow id tag
  SatFlowIdTag flowIdTag;
  flowIdTag.SetFlowId (m_flowId);
  p->AddPacketTag (flowIdTag);

  // Add MAC tag to identify the packet in lower layers
  SatMacTag mTag;
  mTag.SetDestAddress (dest);
  mTag.SetSourceAddress (m_sourceAddress);
  p->AddPacketTag (mTag);

  NS_LOG_INFO ("Tx Buffer: New packet added of size: " << p->GetSize ());

  if (!m_txQueue->Enqueue (p))
    {
      NS_LOG_INFO ("Packet is dropped!");
    }

  NS_LOG_INFO ("NumPackets = " << m_txQueue->GetNPackets () );
  NS_LOG_INFO ("NumBytes = " << m_txQueue->GetNBytes ());
}


Ptr<Packet>
SatBaseEncapsulator::NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << bytes);
  NS_LOG_INFO ("TxOpportunity for flowId: " << (uint32_t) m_flowId << " of " << bytes << " bytes");

  Ptr<Packet> packet;

  // No packets in buffer
  if (m_txQueue->IsEmpty ())
    {
      NS_LOG_INFO ("No data pending, return NULL packet");
      return packet;
    }

  // Peek the first PDU from the buffer.
  Ptr<const Packet> peekPacket = m_txQueue->Peek ();

  // Initialize with current packet size
  nextMinTxO = peekPacket->GetSize ();

  // If control packet fits into TxO
  if (peekPacket->GetSize () <= bytes)
    {
      // Peek the first PDU from the buffer.
      packet = m_txQueue->Dequeue ();

      if (!packet)
        {
          NS_FATAL_ERROR ("Packet not dequeued from txQueue even though the peek PDU should have been fit!");
        }

      // Update bytes left
      bytesLeft = m_txQueue->GetNBytes ();

      // Update the next min TxO
      Ptr<const Packet> nextPacket = m_txQueue->Peek ();
      if (nextPacket)
        {
          nextMinTxO = nextPacket->GetSize ();
        }
      else
        {
          nextMinTxO = 0;
        }
    }

  return packet;
}


void
SatBaseEncapsulator::ReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * The base encapsulator should not be used at receiving packets
   * at all! This functionality is implemented in the inherited classes.
   */
}

void
SatBaseEncapsulator::ReceiveAck (Ptr<SatArqAckMessage> ack)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * The base encapsulator should not be used at receiving control packets
   * at all! This functionality is implemented in the inherited classes.
   */
}

uint32_t
SatBaseEncapsulator::GetTxBufferSizeInBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_txQueue->GetNBytes ();
}

Time
SatBaseEncapsulator::GetHolDelay () const
{
  NS_LOG_FUNCTION (this);

  Time delay (Seconds (0.0));

  if (m_txQueue->GetNPackets () > 0)
    {
      // Peek the first PDU from the buffer.
      Ptr<const Packet> peekPacket = m_txQueue->Peek ();

      SatTimeTag timeTag;
      peekPacket->PeekPacketTag (timeTag);

      delay = Simulator::Now () - timeTag.GetSenderTimestamp ();
    }
  return delay;
}

void
SatBaseEncapsulator::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_rxCallback = cb;
}

void
SatBaseEncapsulator::SetCtrlMsgCallback (SatBaseEncapsulator::SendCtrlCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_ctrlCallback = cb;
}

void
SatBaseEncapsulator::SetQueue (Ptr<SatQueue> queue)
{
  NS_LOG_FUNCTION (this);

  m_txQueue = queue;
}

Ptr<SatQueue>
SatBaseEncapsulator::GetQueue ()
{
  NS_LOG_FUNCTION (this);

  return m_txQueue;
}

uint32_t
SatBaseEncapsulator::GetMinTxOpportunityInBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_txQueue->Peek ()->GetSize ();
}



} // namespace ns3
