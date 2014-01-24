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
#include "ns3/mac48-address.h"

#include "satellite-generic-stream-encapsulator.h"
#include "satellite-llc.h"
#include "satellite-mac-tag.h"
#include "satellite-encap-pdu-status-tag.h"
#include "satellite-gse-headers.h"
#include "satellite-time-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatGenericStreamEncapsulator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGenericStreamEncapsulator);


SatGenericStreamEncapsulator::SatGenericStreamEncapsulator ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}

SatGenericStreamEncapsulator::SatGenericStreamEncapsulator (Mac48Address source, Mac48Address dest)
   :m_sourceAddress (source),
    m_destAddress (dest),
    m_maxTxBufferSize (100 * 4095),
    m_txBufferSize (0),
    m_maxGsePduSize (4095),
    m_txFragmentId (0),
    m_currRxFragmentId (0),
    m_currRxPacketSize (0),
    m_currRxPacketFragmentBytes (0),
    m_minGseTxOpportunity (0)
{
  NS_LOG_FUNCTION (this);

  SatGseHeader gseHeader;
  m_minGseTxOpportunity = gseHeader.GetMaxGseHeaderSizeInBytes () + 1;
}

SatGenericStreamEncapsulator::~SatGenericStreamEncapsulator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatGenericStreamEncapsulator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGenericStreamEncapsulator")
    .SetParent<SatEncapsulator> ()
    .AddConstructor<SatGenericStreamEncapsulator> ()
    .AddAttribute ("MaxTxBufferSize",
                   "Maximum size of the transmission buffer (in Bytes)",
                   UintegerValue (100 * 4096),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_maxTxBufferSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxGsePduSize",
                   "Maximum size of the GSE PDU (in Bytes)",
                   UintegerValue (4096),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_maxGsePduSize),
                   MakeUintegerChecker<uint32_t> ())
                   ;
  return tid;
}

void
SatGenericStreamEncapsulator::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  while (!m_txBuffer.empty ()) m_txBuffer.pop_back ();
  m_txBuffer.clear();

  while (!m_rxBuffer.empty ()) m_rxBuffer.pop_back ();
  m_rxBuffer.clear();

  SatEncapsulator::DoDispose ();
}

void
SatGenericStreamEncapsulator::TransmitPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // If the packet still fits into the buffer
  if (p->GetSize () > MAX_HL_PACKET_SIZE)
    {
      NS_FATAL_ERROR ("SatGenericStreamEncapsulator received too large HL PDU!");
    }
  else if (m_txBufferSize + p->GetSize () <= m_maxTxBufferSize)
    {
      // Store packet arrival time
      SatTimeTag timeTag (Simulator::Now ());
      p->AddPacketTag (timeTag);

      // Mark the PDU with FULL_PDU tag
      SatEncapPduStatusTag tag;
      tag.SetStatus (SatEncapPduStatusTag::FULL_PDU);
      p->AddPacketTag (tag);

      NS_LOG_LOGIC ("Tx Buffer: New packet added");
      m_txBuffer.push_back (p);
      m_txBufferSize += p->GetSize ();
      NS_LOG_LOGIC ("NumOfBuffers = " << m_txBuffer.size() );
      NS_LOG_LOGIC ("txBufferSize = " << m_txBufferSize);
    }
  else
    {
      // Discard full SDU
      NS_LOG_LOGIC ("TxBuffer is full. SDU discarded");
      NS_LOG_LOGIC ("MaxTxBufferSize = " << m_maxTxBufferSize);
      NS_LOG_LOGIC ("txBufferSize    = " << m_txBufferSize);
      NS_LOG_LOGIC ("packet size     = " << p->GetSize ());
    }

}

Ptr<Packet>
SatGenericStreamEncapsulator::NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft)
{
  NS_LOG_FUNCTION (this << bytesLeft);

  // GSE PDU
  Ptr<Packet> packet;

  // GSE header
  SatGseHeader gseHeader;

  // No packets in buffer
  if ( m_txBuffer.empty () )
    {
      NS_LOG_LOGIC ("No data pending");
      return packet;
    }

  // Take the first PDU from the buffer
  Ptr<Packet> nextPacket = (*(m_txBuffer.begin ()))->Copy ();

  SatEncapPduStatusTag tag;
  nextPacket->PeekPacketTag (tag);

  // Too small TxOpportunity!
  if (bytes <= gseHeader.GetGseHeaderSizeInBytes(tag.GetStatus ()))
    {
      NS_LOG_LOGIC ("TX opportunity too small = " << bytes);
      return packet;
    }

  uint32_t maxGsePayload = m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (tag.GetStatus ());

  if (tag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
    {
      // The FULL PDU has to fit this BB frame since fragmentation
      // between BB frames is not allowed!
      if (bytes < CalculateTotalPacketSizeWithHeaders (nextPacket->GetSize ()))
        {
          NS_LOG_LOGIC ("FULL higher layer packet " << nextPacket->GetSize () <<  " does not fit into this Tx opportunity = " << bytes);
          return packet;
        }
    }

  m_txBufferSize -= (*(m_txBuffer.begin()))->GetSize ();
  m_txBuffer.erase (m_txBuffer.begin ());

  // Fragmentation
  if ( nextPacket->GetSize () > maxGsePayload)
    {
      // Status tag of the old and new segment
      // Note: This is the only place where a PDU is segmented and
      // therefore its status can change
      SatEncapPduStatusTag oldTag, newTag;
      nextPacket->RemovePacketTag (oldTag);

      // Create new GSE header
      SatGseHeader gseHeader;

      if (oldTag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          IncreaseFragmentId ();
          gseHeader.SetStartIndicator ();
          gseHeader.SetTotalLength (nextPacket->GetSize());
          newTag.SetStatus (SatEncapPduStatusTag::START_PDU);
          oldTag.SetStatus (SatEncapPduStatusTag::END_PDU);
          maxGsePayload = m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::START_PDU);
        }
      else if (oldTag.GetStatus () == SatEncapPduStatusTag::END_PDU)
        {
          // oldTag still is left with the END_PDU tag
          newTag.SetStatus (SatEncapPduStatusTag::CONTINUATION_PDU);
          maxGsePayload = m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::CONTINUATION_PDU);
        }

      gseHeader.SetFragmentId (m_txFragmentId);

      // Create a fragment of correct size
      packet = nextPacket->CreateFragment (0, maxGsePayload);

      // Add proper payload length of the GSE packet
      gseHeader.SetGsePduLength (packet->GetSize());

      // Give back the remaining segment to the transmission buffer
      nextPacket->RemoveAtStart (maxGsePayload);

      // Add old tag back to the old packet
      nextPacket->AddPacketTag (oldTag);
      m_txBuffer.insert (m_txBuffer.begin (), nextPacket);
      m_txBufferSize += (*(m_txBuffer.begin()))->GetSize ();

      // Put status tag once it has been adjusted
      packet->AddPacketTag (newTag);

      // Add PDU header
      packet->AddHeader (gseHeader);
    }
  // Just encapsulation
  else
    {
      // Segment txBuffer.FirstBuffer and
      // Give back the remaining segment to the transmission buffer
      packet = nextPacket;

      // Create new GSE header
      SatGseHeader gseHeader;

      SatEncapPduStatusTag tag;
      packet->PeekPacketTag (tag);

      if (tag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          gseHeader.SetTotalLength (packet->GetSize());
          gseHeader.SetStartIndicator ();
        }
      // Fragment id is added to all fragmented packets
      else
        {
          gseHeader.SetFragmentId (m_txFragmentId);
        }

      gseHeader.SetGsePduLength (nextPacket->GetSize());
      gseHeader.SetEndIndicator ();

      // Add PDU header
      packet->AddHeader (gseHeader);
    }

  // Add MAC tag to identify the packet in lower layers
  SatMacTag mTag;
  mTag.SetDestAddress (m_destAddress);
  mTag.SetSourceAddress (m_sourceAddress);
  packet->AddPacketTag (mTag);

  // Update bytes left
  bytesLeft = GetTxBufferSizeInBytes ();

  return packet;
}

void
SatGenericStreamEncapsulator::IncreaseFragmentId ()
{
  ++m_txFragmentId;
  if (m_txFragmentId >= MAX_FRAGMENT_ID)
    {
      m_txFragmentId = 0;
    }
}

uint32_t
SatGenericStreamEncapsulator::CalculateTotalPacketSizeWithHeaders (uint32_t hlPacketSize) const
{
  SatGseHeader gseHeader;
  uint32_t totalPacketSize (0);

  // FULL PDU
  if (hlPacketSize <= m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::FULL_PDU))
    {
      totalPacketSize = hlPacketSize + gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::FULL_PDU);
    }
  // FRAGMENTATION
  else
    {
      // FIRST
      uint32_t firstPduPayload = m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::START_PDU);
      totalPacketSize += m_maxGsePduSize;
      uint32_t left = hlPacketSize - firstPduPayload;

      // CONTINUATION
      uint32_t contPdus = left / (m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::CONTINUATION_PDU));
      totalPacketSize += (contPdus * m_maxGsePduSize);
      left = left % (m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::CONTINUATION_PDU));

      // END
      if (left <= (m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::END_PDU)))
        {
          totalPacketSize += left + gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::END_PDU);
        }
      else
        {
          totalPacketSize += ( m_maxGsePduSize + gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::CONTINUATION_PDU));
          left -= m_maxGsePduSize - gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::CONTINUATION_PDU);
          totalPacketSize += left + gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::END_PDU);
        }
    }
  return totalPacketSize;
}


uint32_t
SatGenericStreamEncapsulator::GetTxBufferSizeInBytes () const
{
  return m_txBufferSize;
}

void
SatGenericStreamEncapsulator::ReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // Sanity check
  SatMacTag mTag;
  bool success = p->RemovePacketTag (mTag);
  if (!success)
    {
      NS_FATAL_ERROR ("MAC tag not found in the packet!");
    }
  else if (mTag.GetDestAddress() != m_destAddress)
    {
      NS_FATAL_ERROR ("Packet was not intended for this receiver!");
    }

  m_rxBuffer.push_back (p);

  // Try to reassemble the received packets
  Reassemble ();
}


void
SatGenericStreamEncapsulator::Reassemble ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (!m_rxBuffer.empty ());

  while (!m_rxBuffer.empty ())
    {
      // Remove PDU header
      SatGseHeader gseHeader;
      m_rxBuffer.front()->RemoveHeader (gseHeader);

      // FULL_PDU
      if (gseHeader.GetStartIndicator() == true && gseHeader.GetEndIndicator() == true)
        {
          NS_LOG_LOGIC ("FULL PDU received");

          Reset ();

          m_rxCallback(m_rxBuffer.front ());
          m_rxBuffer.pop_front();
        }

      // START_PDU
      else if (gseHeader.GetStartIndicator() == true && gseHeader.GetEndIndicator() == false)
        {
          NS_LOG_LOGIC ("START PDU received");

          m_currRxFragmentId = gseHeader.GetFragmentId ();
          m_currRxPacketSize = gseHeader.GetTotalLength ();
          m_currRxPacketFragmentBytes = gseHeader.GetGsePduLength ();
          m_currRxPacketFragment = m_rxBuffer.front ();
          m_rxBuffer.pop_front();
        }

      // CONTINUATION_PDU
      else if (gseHeader.GetStartIndicator() == false && gseHeader.GetEndIndicator() == false)
        {
          NS_LOG_LOGIC ("CONTINUATION PDU received");

          // Previous fragment found
          if (m_currRxPacketFragment && gseHeader.GetFragmentId () == m_currRxFragmentId)
            {
              m_currRxPacketFragmentBytes += gseHeader.GetGsePduLength ();
              m_currRxPacketFragment->AddAtEnd (m_rxBuffer.front ());
            }
          else
            {
              Reset ();
              NS_LOG_LOGIC ("CONTINUATION PDU received while the START of the PDU may have been lost");
            }
          m_rxBuffer.pop_front();
        }

      // END_PDU
      else if (gseHeader.GetStartIndicator() == false && gseHeader.GetEndIndicator() == true)
        {
          NS_LOG_LOGIC ("END PDU received");

          // Previous fragment found
          if (m_currRxPacketFragment && gseHeader.GetFragmentId () == m_currRxFragmentId)
            {
              m_currRxPacketFragmentBytes += gseHeader.GetGsePduLength ();

              // The packet size is wrong!
              if (m_currRxPacketFragmentBytes != m_currRxPacketSize)
                {
                  NS_LOG_LOGIC ("END PDU received, but the packet size of the HL PDU is wrong. Drop the HL packet!");
                }
              // Receive the HL packet here
              else
                {
                  m_currRxPacketFragment->AddAtEnd (m_rxBuffer.front ());
                  m_rxBuffer.pop_front();
                  m_rxCallback (m_currRxPacketFragment);
                }
            }
          else
            {
              NS_LOG_LOGIC ("END PDU received while the START of the PDU may have been lost");
            }

          // Reset anyway
          Reset ();
        }
    }
}

void SatGenericStreamEncapsulator::Reset ()
{
  m_currRxFragmentId = 0;
  m_currRxPacketSize = 0;
  m_currRxPacketFragment = 0;
  m_currRxPacketFragmentBytes = 0;
}


void
SatGenericStreamEncapsulator::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

Time
SatGenericStreamEncapsulator::GetHolDelay () const
{
  Time holDelay;
  SatTimeTag timeTag;
  if (m_txBuffer.front ()->PeekPacketTag(timeTag))
    {
      holDelay = Simulator::Now () - timeTag.GetSenderTimestamp ();
    }
  else
    {
      NS_FATAL_ERROR ("SatTimeTag not found in the packet!");
    }

  return holDelay;
}


uint32_t
SatGenericStreamEncapsulator::GetMinTxOpportunityInBytes () const
{
  return m_minGseTxOpportunity;
}


} // namespace ns3
