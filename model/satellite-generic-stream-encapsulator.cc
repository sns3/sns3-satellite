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
#include "satellite-gse-header.h"
#include "satellite-time-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatGenericStreamEncapsulator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGenericStreamEncapsulator);


SatGenericStreamEncapsulator::SatGenericStreamEncapsulator ()
  : SatBaseEncapsulator (),
    m_maxGsePduSize (4095),
    m_txFragmentId (0),
    m_currRxFragmentId (0),
    m_currRxPacketSize (0),
    m_currRxPacketFragmentBytes (0),
    m_minGseTxOpportunity (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}



SatGenericStreamEncapsulator::SatGenericStreamEncapsulator (Mac48Address source, Mac48Address dest, uint8_t flowId)
  : SatBaseEncapsulator (source, dest, flowId),
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
    .SetParent<SatBaseEncapsulator> ()
    .AddConstructor<SatGenericStreamEncapsulator> ()
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

  SatBaseEncapsulator::DoDispose ();
}

void
SatGenericStreamEncapsulator::EnquePdu (Ptr<Packet> p, Mac48Address /*dest*/)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // If the packet is smaller than the maximum size
  if (p->GetSize () > MAX_HL_PACKET_SIZE)
    {
      NS_FATAL_ERROR ("SatGenericStreamEncapsulator received too large HL PDU!");
    }

  // Mark the PDU with FULL_PDU tag
  SatEncapPduStatusTag tag;
  tag.SetStatus (SatEncapPduStatusTag::FULL_PDU);
  p->AddPacketTag (tag);

  NS_LOG_INFO ("Tx Buffer: New packet added of size: " << p->GetSize ());

  if (!m_txQueue->Enqueue (p))
    {
      NS_LOG_INFO ("Packet is dropped!");
    }

  NS_LOG_INFO ("NumPackets = " << m_txQueue->GetNPackets () );
  NS_LOG_INFO ("NumBytes = " << m_txQueue->GetNBytes ());
}

Ptr<Packet>
SatGenericStreamEncapsulator::NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << bytesLeft);
  NS_LOG_INFO ("TxOpportunity for UT: " << m_destAddress << " flowId: " << (uint32_t) m_flowId << " of " << bytes << " bytes");

  // GSE PDU
  Ptr<Packet> packet;

  // No packets in buffer
  if ( m_txQueue->GetNPackets () == 0 )
    {
      NS_LOG_INFO ("No data pending, return NULL packet");
      return packet;
    }

  packet = GetNewGsePdu (bytes, m_maxGsePduSize);

  if (packet)
    {
      // Add MAC tag to identify the packet in lower layers
      SatMacTag mTag;
      mTag.SetDestAddress (m_destAddress);
      mTag.SetSourceAddress (m_sourceAddress);
      packet->AddPacketTag (mTag);

      // Add flow id tag
      SatFlowIdTag flowIdTag;
      flowIdTag.SetFlowId (m_flowId);
      packet->AddPacketTag (flowIdTag);

      if (packet->GetSize () > bytes)
        {
          NS_FATAL_ERROR ("Created packet of size: " << packet->GetSize () << " is larger than the tx opportunity: " << bytes);
        }
    }

  // Update bytes lefts
  bytesLeft = GetTxBufferSizeInBytes ();

  // Update min TxO
  nextMinTxO = GetMinTxOpportunityInBytes ();

  return packet;
}

Ptr<Packet>
SatGenericStreamEncapsulator::GetNewGsePdu (uint32_t txOpportunityBytes, uint32_t maxGsePduSize, uint32_t additionalHeaderSize)
{
  NS_LOG_FUNCTION (this << txOpportunityBytes << maxGsePduSize << additionalHeaderSize);

  // GSE packet = NULL
  Ptr<Packet> packet;

  // GSE header
  SatGseHeader gseHeader;

  // Peek the first PDU from the buffer.
  Ptr<const Packet> peekPacket = m_txQueue->Peek ();

  SatEncapPduStatusTag peekTag;
  peekPacket->PeekPacketTag (peekTag);

  // Too small TxOpportunity!
  uint32_t headerSize = gseHeader.GetGseHeaderSizeInBytes (peekTag.GetStatus ()) + additionalHeaderSize;
  if (txOpportunityBytes <= headerSize)
    {
      NS_LOG_INFO ("TX opportunity too small = " << txOpportunityBytes);
      return packet;
    }

  // Build Data field
  uint32_t maxGsePayload = std::min (txOpportunityBytes, maxGsePduSize) - headerSize;

  NS_LOG_INFO ("GSE header size: " << gseHeader.GetGseHeaderSizeInBytes (peekTag.GetStatus ()));

  // Fragmentation
  if (peekPacket->GetSize () > maxGsePayload)
    {
      NS_LOG_INFO ("In fragmentation - packet size: " << peekPacket->GetSize () << " max GSE payload: " << maxGsePayload);

      // Now we can take the packe away from the queue
      Ptr<Packet> firstPacket = m_txQueue->Dequeue ();

      // Status tag of the old and new segment
      // Note: This is the only place where a PDU is segmented and
      // therefore its status can change
      SatEncapPduStatusTag oldTag, newTag;
      firstPacket->RemovePacketTag (oldTag);

      // Create new GSE header
      SatGseHeader gseHeader;

      if (oldTag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          IncreaseFragmentId ();
          gseHeader.SetStartIndicator ();
          gseHeader.SetTotalLength (firstPacket->GetSize ());
          newTag.SetStatus (SatEncapPduStatusTag::START_PDU);
          oldTag.SetStatus (SatEncapPduStatusTag::END_PDU);

          uint32_t newMaxGsePayload = std::min (txOpportunityBytes, maxGsePduSize) -
            gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::START_PDU) -
            additionalHeaderSize;

          NS_LOG_INFO ("Packet size: " << firstPacket->GetSize () << " max GSE payload: " << maxGsePayload);

          if (maxGsePayload > newMaxGsePayload)
            {
              NS_FATAL_ERROR ("Packet will fit into the time slot after all, since we changed to utilize START PDU GSE header");
            }
        }
      else if (oldTag.GetStatus () == SatEncapPduStatusTag::END_PDU)
        {
          // oldTag still is left with the END_PDU tag
          newTag.SetStatus (SatEncapPduStatusTag::CONTINUATION_PDU);

          uint32_t newMaxGsePayload = std::min (txOpportunityBytes, maxGsePduSize) -
            gseHeader.GetGseHeaderSizeInBytes (SatEncapPduStatusTag::CONTINUATION_PDU) -
            additionalHeaderSize;

          NS_LOG_INFO ("Packet size: " << firstPacket->GetSize () << " max GSE payload: " << maxGsePayload);

          if (maxGsePayload > newMaxGsePayload)
            {
              NS_FATAL_ERROR ("Packet will fit into the time slot after all, since we changed to utilize CONTINUATION PDU GSE header");
            }
        }

      gseHeader.SetFragmentId (m_txFragmentId);

      // Create a fragment of correct size
      Ptr<Packet> fragment = firstPacket->CreateFragment (0, maxGsePayload);

      NS_LOG_INFO ("Create fragment of size: " << fragment->GetSize ());

      // Add proper payload length of the GSE packet
      gseHeader.SetGsePduLength (fragment->GetSize ());

      // Give back the remaining segment to the transmission buffer
      firstPacket->RemoveAtStart (maxGsePayload);

      // Add old tag back to the old packet
      firstPacket->AddPacketTag (oldTag);

      // Push remainder packet to the queue
      m_txQueue->PushFront (firstPacket);

      // Put status tag once it has been adjusted
      fragment->AddPacketTag (newTag);

      // Add PDU header
      fragment->AddHeader (gseHeader);

      // GSE PDU
      packet = fragment;
    }
  // Just encapsulation
  else
    {
      NS_LOG_INFO ("In fragmentation - packet size: " << peekPacket->GetSize () << " max GSE payload: " << maxGsePayload);

      // Take the packe away from the queue
      Ptr<Packet> firstPacket = m_txQueue->Dequeue ();

      // Create new GSE header
      SatGseHeader gseHeader;

      SatEncapPduStatusTag tag;
      firstPacket->PeekPacketTag (tag);

      if (tag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          gseHeader.SetTotalLength (firstPacket->GetSize ());
          gseHeader.SetStartIndicator ();
        }
      // Fragment id is added to all fragmented packets
      else
        {
          gseHeader.SetFragmentId (m_txFragmentId);
        }

      gseHeader.SetGsePduLength (firstPacket->GetSize ());
      gseHeader.SetEndIndicator ();

      // Add PDU header
      firstPacket->AddHeader (gseHeader);

      // GSE PDU
      packet = firstPacket;
    }

  if (packet->GetSize () > txOpportunityBytes)
    {
      NS_FATAL_ERROR ("Created GSE PDU of size: " << packet->GetSize () << " is larger than the Tx opportunity: " << txOpportunityBytes);
    }

  return packet;
}

void
SatGenericStreamEncapsulator::IncreaseFragmentId ()
{
  NS_LOG_FUNCTION (this);

  ++m_txFragmentId;
  if (m_txFragmentId >= MAX_FRAGMENT_ID)
    {
      m_txFragmentId = 0;
    }
}

void
SatGenericStreamEncapsulator::ReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());

  // Remove encap PDU status tag
  SatEncapPduStatusTag statusTag;
  p->RemovePacketTag (statusTag);

  // Remove flow id tag
  SatFlowIdTag flowIdTag;
  p->RemovePacketTag (flowIdTag);

  // Sanity check
  SatMacTag mTag;
  bool success = p->RemovePacketTag (mTag);
  if (!success)
    {
      NS_FATAL_ERROR ("MAC tag not found in the packet!");
    }
  else if (mTag.GetDestAddress () != m_destAddress)
    {
      NS_FATAL_ERROR ("Packet was not intended for this receiver!");
    }

  // Decapsuling and defragmentation
  ProcessPdu (p);
}


void
SatGenericStreamEncapsulator::ProcessPdu (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet->GetSize ());

  // Remove PDU header
  SatGseHeader gseHeader;
  packet->RemoveHeader (gseHeader);

  // FULL_PDU
  if (gseHeader.GetStartIndicator () == true && gseHeader.GetEndIndicator () == true)
    {
      NS_LOG_INFO ("FULL PDU received");

      Reset ();

      m_rxCallback (packet, m_sourceAddress, m_destAddress);
    }

  // START_PDU
  else if (gseHeader.GetStartIndicator () == true && gseHeader.GetEndIndicator () == false)
    {
      NS_LOG_INFO ("START PDU received");

      Reset ();

      m_currRxFragmentId = gseHeader.GetFragmentId ();
      m_currRxPacketSize = gseHeader.GetTotalLength ();
      m_currRxPacketFragmentBytes = gseHeader.GetGsePduLength ();
      m_currRxPacketFragment = packet;
    }

  // CONTINUATION_PDU
  else if (gseHeader.GetStartIndicator () == false && gseHeader.GetEndIndicator () == false)
    {
      NS_LOG_INFO ("CONTINUATION PDU received");

      // Previous fragment found
      if (m_currRxPacketFragment && gseHeader.GetFragmentId () == m_currRxFragmentId)
        {
          m_currRxPacketFragmentBytes += gseHeader.GetGsePduLength ();
          m_currRxPacketFragment->AddAtEnd (packet);
        }
      else
        {
          Reset ();
          NS_LOG_INFO ("CONTINUATION PDU received while the START of the PDU may have been lost");
        }
    }

  // END_PDU
  else if (gseHeader.GetStartIndicator () == false && gseHeader.GetEndIndicator () == true)
    {
      NS_LOG_INFO ("END PDU received");

      // Previous fragment found
      if (m_currRxPacketFragment && gseHeader.GetFragmentId () == m_currRxFragmentId)
        {
          m_currRxPacketFragmentBytes += gseHeader.GetGsePduLength ();

          // The packet size is wrong!
          if (m_currRxPacketFragmentBytes != m_currRxPacketSize)
            {
              NS_LOG_INFO ("END PDU received, but the packet size of the HL PDU is wrong. Drop the HL packet!");
            }
          //   Receive the HL packet here
          else
            {
              m_currRxPacketFragment->AddAtEnd (packet);
              m_rxCallback (m_currRxPacketFragment, m_sourceAddress, m_destAddress);
            }
        }
      else
        {
          NS_LOG_INFO ("END PDU received while the START of the PDU may have been lost");
        }

      // Reset anyway
      Reset ();
    }
}

void SatGenericStreamEncapsulator::Reset ()
{
  NS_LOG_FUNCTION (this);

  m_currRxFragmentId = 0;
  m_currRxPacketSize = 0;
  m_currRxPacketFragment = 0;
  m_currRxPacketFragmentBytes = 0;
}


uint32_t
SatGenericStreamEncapsulator::GetMinTxOpportunityInBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_minGseTxOpportunity;
}


} // namespace ns3
