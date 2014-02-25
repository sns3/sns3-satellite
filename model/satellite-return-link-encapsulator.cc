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

#include "satellite-return-link-encapsulator.h"
#include "satellite-llc.h"
#include "satellite-mac-tag.h"
#include "satellite-encap-pdu-status-tag.h"
#include "satellite-rle-header.h"
#include "satellite-time-tag.h"
#include "satellite-queue.h"

NS_LOG_COMPONENT_DEFINE ("SatReturnLinkEncapsulator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatReturnLinkEncapsulator);


SatReturnLinkEncapsulator::SatReturnLinkEncapsulator ()
:m_sourceAddress (),
 m_destAddress (),
 m_rcIndex (0),
 m_txFragmentId (0),
 m_currRxFragmentId (0),
 m_currRxPacketSize (0),
 m_currRxPacketFragmentBytes (0),
 m_minTxOpportunity (0),
 MAX_FRAGMENT_ID (8),
 MAX_PPDU_PACKET_SIZE (2048),
 MAX_HL_PDU_PACKET_SIZE (4096)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}


SatReturnLinkEncapsulator::SatReturnLinkEncapsulator (Mac48Address source, Mac48Address dest, uint8_t rcIndex)
   :m_sourceAddress (source),
    m_destAddress (dest),
    m_rcIndex (rcIndex),
    m_txFragmentId (0),
    m_currRxFragmentId (0),
    m_currRxPacketSize (0),
    m_currRxPacketFragmentBytes (0),
    m_minTxOpportunity (0),
    MAX_FRAGMENT_ID (8),
    MAX_PPDU_PACKET_SIZE (2048),
    MAX_HL_PDU_PACKET_SIZE (4096)
{
  NS_LOG_FUNCTION (this);

  SatPPduHeader ppduHeader;
  m_minTxOpportunity = ppduHeader.GetMaxHeaderSizeInBytes ();
}

SatReturnLinkEncapsulator::~SatReturnLinkEncapsulator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatReturnLinkEncapsulator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatReturnLinkEncapsulator")
    .SetParent<SatEncapsulator> ()
    .AddConstructor<SatReturnLinkEncapsulator> ()
  ;
  return tid;
}

void
SatReturnLinkEncapsulator::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  SatEncapsulator::DoDispose ();
}

void
SatReturnLinkEncapsulator::TransmitPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // If the packet is smaller than the maximum size
  if (p->GetSize () > MAX_HL_PDU_PACKET_SIZE)
    {
      NS_FATAL_ERROR ("SatReturnLinkEncapsulator received too large HL PDU!");
    }

  // Store packet arrival time
  SatTimeTag timeTag (Simulator::Now ());
  p->AddPacketTag (timeTag);

  // Mark the PDU with FULL_PDU tag
  SatEncapPduStatusTag tag;
  tag.SetStatus (SatEncapPduStatusTag::FULL_PDU);
  p->AddPacketTag (tag);

  /**
   * TODO: This is the place to encapsulate the HL packet
   * with Addressed Link (AL) header.
   */

  NS_LOG_LOGIC ("Tx Buffer: New packet added of size: " << p->GetSize ());

  if (!m_txQueue->Enqueue (p))
    {
      NS_LOG_LOGIC ("Packet is dropped!");
    }

  NS_LOG_LOGIC ("NumPackets = " << m_txQueue->GetNPackets() );
  NS_LOG_LOGIC ("NumBytes = " << m_txQueue->GetNBytes ());
}

Ptr<Packet>
SatReturnLinkEncapsulator::NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft)
{
  NS_LOG_FUNCTION (this << bytes);
  NS_LOG_LOGIC ("TxOpportunity for " << bytes << " bytes");

  // Payload adapted PDU = NULL
  Ptr<Packet> packet;

  NS_LOG_LOGIC ("Queue size before TxOpportunity: " << m_txQueue->GetNBytes());

  // No packets in buffer
  if ( m_txQueue->GetNPackets () == 0 )
    {
      NS_LOG_LOGIC ("No data pending, return NULL packet");
      return packet;
    }

  // RLE (PPDU) header
  SatPPduHeader ppduHeader;

  // Peek the first PDU from the buffer.
  Ptr<const Packet> peekSegment = m_txQueue->Peek ();

  SatEncapPduStatusTag tag;
  peekSegment->PeekPacketTag (tag);

  NS_LOG_LOGIC ("Size of the first packet in buffer: " << peekSegment->GetSize ());
  NS_LOG_LOGIC ("Encapsulation status of the first packet in buffer: " << tag.GetStatus());

  // Tx opportunity bytes is not enough
  if (bytes <= ppduHeader.GetHeaderSizeInBytes(tag.GetStatus()))
    {
      NS_LOG_LOGIC ("TX opportunity too small = " << bytes);
      return packet;
    }

  NS_LOG_LOGIC ("Header size of " << tag.GetStatus () << " packet: " << ppduHeader.GetHeaderSizeInBytes(tag.GetStatus()));

  // Build Data field
  uint32_t maxSegmentSize = std::min(bytes, MAX_PPDU_PACKET_SIZE) - ppduHeader.GetHeaderSizeInBytes(tag.GetStatus());

  NS_LOG_LOGIC ("Maximum supported segment size: " << maxSegmentSize);

  // Fragmentation if the HL PDU does not fit into the burst or
  // the HL packet is too large.
  if ( peekSegment->GetSize () > maxSegmentSize )
    {
      NS_LOG_LOGIC ("Buffered packet is larger than the maximum segment size!");

      if (tag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          // Calculate again that the packet fits into the Tx opportunity
          uint32_t headerSize = ppduHeader.GetHeaderSizeInBytes(SatEncapPduStatusTag::START_PDU);
          if (bytes <= headerSize)
            {
              NS_LOG_LOGIC ("Start PDU does not fit into the TxOpportunity anymore!");
              return packet;
            }

          maxSegmentSize = std::min(bytes, MAX_PPDU_PACKET_SIZE) - headerSize;
          NS_LOG_LOGIC ("Recalculated maximum supported segment size: " << maxSegmentSize);

          // In case we have to fragment a FULL PDU, we need to increase
          // the fragment id.
          IncreaseFragmentId ();
        }
      else
        {
          // Calculate again that the packet fits into the Tx opportunity
          uint32_t headerSize = ppduHeader.GetHeaderSizeInBytes(SatEncapPduStatusTag::CONTINUATION_PDU);
          if (bytes <= headerSize)
            {
              NS_LOG_LOGIC ("Continuation PDU does not fit into the TxOpportunity anymore!");
              return packet;
            }

          maxSegmentSize = std::min(bytes, MAX_PPDU_PACKET_SIZE) - headerSize;
          NS_LOG_LOGIC ("Recalculated maximum supported segment size: " << maxSegmentSize);
        }

      // Now we can take the packe away from the queue
      Ptr<Packet> firstSegment = m_txQueue->Dequeue ();

      // Create a new fragment
      Ptr<Packet> newSegment = firstSegment->CreateFragment (0, maxSegmentSize);

      // Status tag of the new and remaining segments
      // Note: This is the only place where a PDU is segmented and
      // therefore its status can change
      SatEncapPduStatusTag oldTag, newTag;
      firstSegment->RemovePacketTag (oldTag);
      newSegment->RemovePacketTag (newTag);

      // Create new PPDU header
      ppduHeader.SetPPduLength (newSegment->GetSize());
      ppduHeader.SetFragmentId (m_txFragmentId);

      if (oldTag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          ppduHeader.SetStartIndicator ();
          ppduHeader.SetTotalLength (firstSegment->GetSize());

          newTag.SetStatus (SatEncapPduStatusTag::START_PDU);
          oldTag.SetStatus (SatEncapPduStatusTag::END_PDU);
        }
      else if (oldTag.GetStatus () == SatEncapPduStatusTag::END_PDU)
        {
          // oldTag still is left with the END_PPDU tag
          newTag.SetStatus (SatEncapPduStatusTag::CONTINUATION_PDU);
        }

      // Give back the remaining segment to the transmission buffer
      firstSegment->RemoveAtStart (maxSegmentSize);

      // If bytes left after fragmentation
      if (firstSegment->GetSize () > 0)
        {
          NS_LOG_LOGIC ("Returning the remaining " << firstSegment->GetSize () << " bytes to buffer");
          firstSegment->AddPacketTag (oldTag);
          m_txQueue->PushFront (firstSegment);
        }
      else
        {
          NS_FATAL_ERROR ("The full segment was taken even though we are in the fragmentation part of the code!");
        }

      // Put status tag once it has been adjusted
      newSegment->AddPacketTag (newTag);

      // Add PPDU header
      newSegment->AddHeader (ppduHeader);

      // PPDU
      packet = newSegment;

      NS_LOG_LOGIC ("Created a fragment of size: " << packet->GetSize ());
    }
  // Packing functionality, for either a FULL_PPDU or END_PPDU
  else
    {
      if (tag.GetStatus() == SatEncapPduStatusTag::FULL_PDU)
        {
          ppduHeader.SetStartIndicator ();
        }
      else
        {
          ppduHeader.SetFragmentId (m_txFragmentId);
        }

      // Take the packe away from the queue
      Ptr<Packet> firstSegment = m_txQueue->Dequeue ();

      ppduHeader.SetEndIndicator ();
      ppduHeader.SetPPduLength (firstSegment->GetSize());

      // Add PPDU header
      firstSegment->AddHeader (ppduHeader);

      // PPDU
      packet = firstSegment;

      NS_LOG_LOGIC ("Packed a packet of size: " << packet->GetSize ());
    }

  // Add MAC tag to identify the packet in lower layers
  SatMacTag mTag;
  mTag.SetDestAddress (m_destAddress);
  mTag.SetSourceAddress (m_sourceAddress);
  packet->AddPacketTag (mTag);

  SatRcIndexTag rcTag;
  rcTag.SetRcIndex (m_rcIndex);
  packet->AddPacketTag (rcTag);

  // Update bytes lefts
  bytesLeft = GetTxBufferSizeInBytes ();

  if (packet)
    {
      if (packet->GetSize () > bytes)
        {
          NS_FATAL_ERROR ("Created packet of size: " << packet->GetSize () << " is larger than the tx opportunity: " << bytes);
        }
      NS_LOG_LOGIC ("Created packet size: " << packet->GetSize ());
    }
  NS_LOG_LOGIC ("Queue size after TxOpportunity: " << m_txQueue->GetNBytes());

  return packet;
}

uint32_t
SatReturnLinkEncapsulator::GetTxBufferSizeInBytes () const
{
  return m_txQueue->GetNBytes ();
}

void
SatReturnLinkEncapsulator::ReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // Sanity check
  SatMacTag mTag;
  bool mSuccess = p->RemovePacketTag (mTag);
  if (!mSuccess)
    {
      NS_FATAL_ERROR ("MAC tag not found in the packet!");
    }
  else if (mTag.GetDestAddress() != m_destAddress)
    {
      NS_FATAL_ERROR ("Packet was not intended for this receiver!");
    }

  // Remove PPDU header
  SatPPduHeader ppduHeader;
  p->RemoveHeader (ppduHeader);

  // FULL_PPDU
  if (ppduHeader.GetStartIndicator() == true && ppduHeader.GetEndIndicator() == true)
    {
      NS_LOG_LOGIC ("FULL PPDU received");

      Reset ();

      m_rxCallback (p);
    }

  // START_PPDU
  else if (ppduHeader.GetStartIndicator() == true && ppduHeader.GetEndIndicator() == false)
    {
      NS_LOG_LOGIC ("START PPDU received");

      m_currRxFragmentId = ppduHeader.GetFragmentId ();
      m_currRxPacketSize = ppduHeader.GetTotalLength ();
      m_currRxPacketFragmentBytes = ppduHeader.GetPPduLength ();
      m_currRxPacketFragment = p;
    }

  // CONTINUATION_PPDU
  else if (ppduHeader.GetStartIndicator() == false && ppduHeader.GetEndIndicator() == false)
    {
      NS_LOG_LOGIC ("CONTINUATION PPDU received");

      // Previous fragment found
      if (m_currRxPacketFragment && ppduHeader.GetFragmentId () == m_currRxFragmentId)
        {
          m_currRxPacketFragmentBytes += ppduHeader.GetPPduLength ();
          m_currRxPacketFragment->AddAtEnd (p);
        }
      else
        {
          Reset ();
          NS_LOG_LOGIC ("CONTINUATION PPDU received while the START of the PPDU may have been lost");
        }
    }

  // END_PPDU
  else if (ppduHeader.GetStartIndicator() == false && ppduHeader.GetEndIndicator() == true)
    {
      NS_LOG_LOGIC ("END PPDU received");

      // Previous fragment found
      if (m_currRxPacketFragment && ppduHeader.GetFragmentId () == m_currRxFragmentId)
        {
          m_currRxPacketFragmentBytes += ppduHeader.GetPPduLength ();

          // The packet size is wrong!
          if (m_currRxPacketFragmentBytes != m_currRxPacketSize)
            {
              NS_LOG_LOGIC ("END PDU received, but the packet size of the HL PDU is wrong. Drop the HL packet!");
            }
          // Receive the HL packet here
          else
            {
              m_currRxPacketFragment->AddAtEnd (p);
              m_rxCallback (m_currRxPacketFragment);
            }
        }
      else
        {
          NS_LOG_LOGIC ("END PPDU received while the START of the PPDU may have been lost");
        }

      // Reset anyway
      Reset ();
    }
}


void
SatReturnLinkEncapsulator::IncreaseFragmentId ()
{
  ++m_txFragmentId;
  if (m_txFragmentId >= MAX_FRAGMENT_ID)
    {
      m_txFragmentId = 0;
    }
}


void SatReturnLinkEncapsulator::Reset ()
{
  m_currRxFragmentId = 0;
  m_currRxPacketSize = 0;
  m_currRxPacketFragment = 0;
  m_currRxPacketFragmentBytes = 0;
}

void
SatReturnLinkEncapsulator::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

Time
SatReturnLinkEncapsulator::GetHolDelay () const
{
  Time holDelay;
  SatTimeTag timeTag;
  if (m_txQueue->Peek ()->PeekPacketTag(timeTag))
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
SatReturnLinkEncapsulator::GetMinTxOpportunityInBytes () const
{
  return m_minTxOpportunity;
}


} // namespace ns3
