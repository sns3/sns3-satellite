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

#include <algorithm>

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"

#include "satellite-return-link-encapsulator-arq.h"
#include "satellite-llc.h"
#include "satellite-mac-tag.h"
#include "satellite-encap-pdu-status-tag.h"
#include "satellite-rle-header.h"
#include "satellite-time-tag.h"
#include "satellite-queue.h"
#include "satellite-arq-header.h"

NS_LOG_COMPONENT_DEFINE ("SatReturnLinkEncapsulatorArq");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatReturnLinkEncapsulatorArq);


SatReturnLinkEncapsulatorArq::SatReturnLinkEncapsulatorArq ()
:m_seqNo (),
 m_txedBuffer (),
 m_retxBuffer (),
 m_retxBufferSize (0),
 m_txedBufferSize (0),
 m_maxRtnArqSegmentSize (38),
 m_maxNoOfRetransmissions (2),
 m_retransmissionTimer (Seconds (0.6)),
 m_arqWindowSize (10),
 m_arqHeaderSize (1),
 m_nextExpectedSeqNo (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}


SatReturnLinkEncapsulatorArq::SatReturnLinkEncapsulatorArq (Mac48Address source, Mac48Address dest, uint8_t flowId)
:SatReturnLinkEncapsulator (source, dest, flowId),
 m_seqNo (),
 m_txedBuffer (),
 m_retxBuffer (),
 m_retxBufferSize (0),
 m_txedBufferSize (0),
 m_maxRtnArqSegmentSize (38),
 m_maxNoOfRetransmissions (2),
 m_retransmissionTimer (Seconds (0.6)),
 m_arqWindowSize (10),
 m_arqHeaderSize (1),
 m_nextExpectedSeqNo (0)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_seqNo = Create<SatArqSequenceNumber> (m_arqWindowSize);

}

SatReturnLinkEncapsulatorArq::~SatReturnLinkEncapsulatorArq ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatReturnLinkEncapsulatorArq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatReturnLinkEncapsulatorArq")
    .SetParent<SatReturnLinkEncapsulator> ()
    .AddConstructor<SatReturnLinkEncapsulatorArq> ()
    .AddAttribute( "MaxRtnArqSegmentSize",
                   "Maximum return link ARQ segment size in Bytes.",
                   UintegerValue (38),
                   MakeUintegerAccessor (&SatReturnLinkEncapsulatorArq::m_maxRtnArqSegmentSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "MaxNoOfRetransmissions",
                   "Maximum number of retransmissions for a single RLE PDU.",
                   UintegerValue (2),
                   MakeUintegerAccessor (&SatReturnLinkEncapsulatorArq::m_maxNoOfRetransmissions),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "RetransmissionTimer",
                   "Retransmission time value.",
                   TimeValue (Seconds (0.6)),
                   MakeTimeAccessor (&SatReturnLinkEncapsulatorArq::m_retransmissionTimer),
                   MakeTimeChecker ())
    .AddAttribute( "WindowSize",
                   "Window size for ARQ.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&SatReturnLinkEncapsulatorArq::m_arqWindowSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "ArqHeaderSize",
                   "ARQ header size in Bytes.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SatReturnLinkEncapsulatorArq::m_arqHeaderSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "RxWaitingTime",
                   "Time to wait for a packet.",
                   TimeValue (Seconds (1.8)),
                   MakeTimeAccessor (&SatReturnLinkEncapsulatorArq::m_rxWaitingTimer),
                   MakeTimeChecker ())
  ;
  return tid;
}


TypeId
SatReturnLinkEncapsulatorArq::GetInstanceTypeId () const
{
  return GetTypeId ();
}


void
SatReturnLinkEncapsulatorArq::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_seqNo = NULL;
  SatReturnLinkEncapsulator::DoDispose ();
}


Ptr<Packet>
SatReturnLinkEncapsulatorArq::NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft)
{
  NS_LOG_FUNCTION (this << bytes);
  NS_LOG_LOGIC ("TxOpportunity for " << bytes << " bytes");

  // Payload adapted PDU = NULL
  Ptr<Packet> packet;

  /**
   * Give priority to the retransmissions. Whenever, the retransmission
   * timer is expired, packet is moved to the retransmission buffer from
   * the transmitted buffer.
   */
  if (!m_retxBuffer.empty ())
    {
      // Oldest seqNo sent first
      Ptr<SatArqBufferContext> context = m_retxBuffer.begin ()->second;

      // If the packet fits into the transmission opportunity
      if (context->m_pdu->GetSize () <= bytes)
        {
          // Pop the front
          m_retxBuffer.erase (m_retxBuffer.begin ());

          // Increase the retransmission counter
          context->m_retransmissionCount = context->m_retransmissionCount + 1;

          m_retxBufferSize -= context->m_pdu->GetSize ();
          m_txedBufferSize += context->m_pdu->GetSize ();

          // Store it back to the transmitted packet container.
          m_txedBuffer.insert (std::make_pair<uint8_t, Ptr<SatArqBufferContext> > (context->m_seqNo, context));

          // Create the retransmission event and store it to the context. Event is cancelled if a ACK
          // is received. However, if the event triggers, we shall send the packet again, if the packet still
          // has retransmissions left.
          EventId t = Simulator::Schedule (m_retransmissionTimer, &SatReturnLinkEncapsulatorArq::ArqReTxTimerExpired, this, context);
          context->m_waitingTimer = t;

          NS_LOG_LOGIC ("UT: " << m_sourceAddress << " sending a retransmission at: " << Now ().GetSeconds () << ": packetSize: " << context->m_pdu->GetSize () << " SeqNo: " << context->m_seqNo);

          return context->m_pdu;
        }
      else
        {
          NS_LOG_LOGIC ("Retransmission PDU: " << context->m_pdu->GetUid () << " size: " << context->m_pdu->GetSize () << " does not fit into TxO: " << bytes);
        }
    }

  // Check the transmission buffer. Sequence number needs to be
  // available for any new transmissions.
  else if (!m_txQueue->IsEmpty() && m_seqNo->SeqNoAvailable ())
    {

      packet = GetNewRlePdu (bytes, m_maxRtnArqSegmentSize, m_arqHeaderSize);

      /*
      // RLE (PPDU) header
      SatPPduHeader ppduHeader;

      // Peek the first PDU from the buffer.
      Ptr<const Packet> peekSegment = m_txQueue->Peek ();

      SatEncapPduStatusTag tag;
      bool found = peekSegment->PeekPacketTag (tag);
      if (!found)
        {
          NS_FATAL_ERROR ("EncapPduStatus tag not found from packet!");
        }

      // Tx opportunity bytes is not enough
      uint32_t headerSize = ppduHeader.GetHeaderSizeInBytes(tag.GetStatus()) + m_arqHeaderSize;
      if (bytes <= headerSize)
        {
          NS_LOG_LOGIC ("TX opportunity too small = " << bytes);
          return packet;
        }

      NS_LOG_LOGIC ("Size of the first packet in buffer: " << peekSegment->GetSize ());
      NS_LOG_LOGIC ("Encapsulation status of the first packet in buffer: " << tag.GetStatus());

      // Build Data field
       uint32_t maxSegmentSize = std::min(bytes, m_maxRtnArqSegmentSize) - headerSize;

       NS_LOG_LOGIC ("Maximum supported segment size: " << maxSegmentSize);

       // Fragmentation if the HL PDU does not fit into the burst or
       // the HL packet is too large.
       if ( peekSegment->GetSize () > maxSegmentSize )
         {
           NS_LOG_LOGIC ("Buffered packet is larger than the maximum segment size!");

           if (tag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
             {
               // Calculate again that the packet fits into the Tx opportunity
               headerSize = ppduHeader.GetHeaderSizeInBytes(SatEncapPduStatusTag::START_PDU) + m_arqHeaderSize;
               if (bytes <= headerSize)
                 {
                   NS_LOG_LOGIC ("Start PDU does not fit into the TxOpportunity anymore!");
                   return packet;
                 }

               maxSegmentSize = std::min(bytes, m_maxRtnArqSegmentSize) - headerSize;
               NS_LOG_LOGIC ("Recalculated maximum supported segment size: " << maxSegmentSize);

               // In case we have to fragment a FULL PDU, we need to increase
               // the fragment id.
               IncreaseFragmentId ();
             }
           // END_PDU
           else
             {
               // Calculate again that the packet fits into the Tx opportunity
               headerSize = ppduHeader.GetHeaderSizeInBytes(SatEncapPduStatusTag::CONTINUATION_PDU) + m_arqHeaderSize;
               if (bytes <= headerSize)
                 {
                   NS_LOG_LOGIC ("Continuation PDU does not fit into the TxOpportunity anymore!");
                   return packet;
                 }

               maxSegmentSize = std::min(bytes, m_maxRtnArqSegmentSize) - headerSize;
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
           NS_LOG_LOGIC ("Packing functionality TxO: " << bytes << " packet size: " << peekSegment->GetSize ());

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

       */

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

          // Get next available sequence number
          uint8_t seqNo = m_seqNo->NextSequenceNumber ();

          // Add ARQ header
          SatArqHeader arqHeader;
          arqHeader.SetSeqNo (seqNo);
          packet->AddHeader (arqHeader);

          // Create ARQ context and store it to Tx'ed buffer
          Ptr<SatArqBufferContext> arqContext = Create<SatArqBufferContext> ();
          arqContext->m_retransmissionCount = 0;
          arqContext->m_pdu = packet;
          arqContext->m_seqNo = seqNo;

          // Create the retransmission event and store it to the context. Event is cancelled if a ACK
          // is received. However, if the event triggers, we shall send the packet again, if the packet still
          // has retransmissions left.
          arqContext->m_waitingTimer = Simulator::Schedule (m_retransmissionTimer, &SatReturnLinkEncapsulatorArq::ArqReTxTimerExpired, this, arqContext);

          // Update the buffer status
          m_txedBufferSize += packet->GetSize ();
          m_txedBuffer.insert (std::make_pair<uint8_t, Ptr<SatArqBufferContext> > (seqNo, arqContext));

          // Update bytes lefts
          bytesLeft = GetTxBufferSizeInBytes ();

          if (packet->GetSize () > bytes)
            {
              NS_FATAL_ERROR ("Created packet of size: " << packet->GetSize () << " is larger than the tx opportunity: " << bytes);
            }

          NS_LOG_LOGIC ("UT: << " << m_sourceAddress << " sent a packet of size: " << packet->GetSize () << " with seqNo: " << (uint32_t)(seqNo) << " flowId: " << (uint32_t)(m_flowId) << " at: " << Now ().GetSeconds ());
          NS_LOG_LOGIC ("Queue size after TxOpportunity: " << m_txQueue->GetNBytes());
        }
    }
  NS_LOG_LOGIC ("No data pending");
  return packet;
}

void
SatReturnLinkEncapsulatorArq::ArqReTxTimerExpired (Ptr<SatArqBufferContext> context)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("At UT: " << m_sourceAddress << " ARQ retransmission timer expired for: " << context->m_seqNo << " at: " << Now ().GetSeconds ());

  // Retransmission still possible
  if (context->m_retransmissionCount < m_maxNoOfRetransmissions)
    {
      NS_LOG_LOGIC ("Moving the ARQ context to retransmission buffer");

      // Erase from the Tx'ed buffer and insert to the reTx buffer
      std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_txedBuffer.find (context->m_seqNo);
      if (it != m_txedBuffer.end ())
        {
          m_txedBuffer.erase (it);

          m_retxBufferSize += context->m_pdu->GetSize ();

          // Push to the retransmission buffer
          m_retxBuffer.insert (std::make_pair<uint8_t, Ptr<SatArqBufferContext> > (context->m_seqNo, context));
        }
      else
        {
          NS_LOG_LOGIC ("Element not found anymore in the m_txedBuffer, thus ACK has been received already earlier");
        }
    }
  // Maximum retransmissions reached
  else
    {
      NS_LOG_LOGIC ("Maximum retransmissions reached, clean-up!");

      // Do clean-up
      CleanUp (context->m_seqNo);
    }
}

void
SatReturnLinkEncapsulatorArq::CleanUp (uint8_t sequenceNumber)
{
  NS_LOG_FUNCTION (this << sequenceNumber);

  // Release sequence number
  m_seqNo->Release (sequenceNumber);

  // Clean-up the Tx'ed buffer
  std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_txedBuffer.find (sequenceNumber);
  if (it != m_txedBuffer.end ())
    {
      m_txedBufferSize -= it->second->m_pdu->GetSize ();
      it->second->m_waitingTimer.Cancel ();
      m_txedBuffer.erase (it);
    }

  // Clean-up the reTx buffer
  it = m_retxBuffer.find (sequenceNumber);
  if (it != m_retxBuffer.end ())
    {
      m_retxBufferSize -= it->second->m_pdu->GetSize ();
      it->second->m_waitingTimer.Cancel ();
      m_retxBuffer.erase (it);
    }
}


void
SatReturnLinkEncapsulatorArq::ReceiveAck (Ptr<SatArqAckMessage> ack)
{
  NS_LOG_FUNCTION (this);

  /**
   * Take the packet away from the transmitted or retransmission
   * buffer and release the sequence number for further use.
   */

  NS_LOG_LOGIC ("UT: " << m_sourceAddress << " received ACK with SN: " << (uint32_t)(ack->GetSequenceNumber ()) << " at: " << Now ().GetSeconds ());

  // Do clean-up
  CleanUp (ack->GetSequenceNumber ());
}


void
SatReturnLinkEncapsulatorArq::ReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize () << p->GetUid ());

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

  SatArqHeader arqHeader;
  p->RemoveHeader (arqHeader);
  uint8_t seqNo = arqHeader.GetSeqNo ();

  // Send ACK for the received RLE packet.
  SendAck (seqNo);

  std::map<uint8_t, Ptr<SatArqBufferContext > >::iterator it = m_reorderingBuffer.find (seqNo);

  // If the context is not found, then we create a new one. This imeans that all the
  // seqNos have been arriving in-order.
  if (it == m_reorderingBuffer.end ())
    {
      Ptr<SatArqBufferContext> arqContext = Create<SatArqBufferContext> ();
      arqContext->m_pdu = p;
      arqContext->m_rxStatus = true;
      arqContext->m_seqNo = seqNo;
      arqContext->m_retransmissionCount = 0;
      m_reorderingBuffer.insert (std::make_pair<uint8_t, Ptr<SatArqBufferContext> > (seqNo, arqContext));
    }
  // If the context is found, we have received at least one packet with higher SN that this one.
  // Then the context exists, but it has not yet received a packet.
  else
    {
      it->second->m_waitingTimer.Cancel ();
      it->second->m_pdu = p;
      it->second->m_rxStatus = true;
    }

  // SeqNo is NOT the one we are expecting
  if (seqNo != m_nextExpectedSeqNo)
    {
      if (seqNo < m_nextExpectedSeqNo)
        {
          NS_FATAL_ERROR ("A packet with sequence number from the past was received! SeqNo: " << seqNo << " nextExpectedSeqNo: " << m_nextExpectedSeqNo);
        }

      /**
       * We shall start a timer for all the sequence numbers preceeding this seq no, which
       * we have not yet received, if such timer is not already running.
       */
      for (uint8_t i = m_nextExpectedSeqNo; i < seqNo; ++i)
        {
          // If the timer is not already running
          std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_reorderingBuffer.find (i);

          // Found!
          if (it != m_reorderingBuffer.end ())
            {
              // If not already running!
              if (!it->second->m_waitingTimer.IsRunning ())
                {
                  EventId id = Simulator::Schedule (m_rxWaitingTimer, &SatReturnLinkEncapsulatorArq::RxWaitingTimerExpired, this, i);
                  it->second->m_waitingTimer = id;
                }
            }
          // Not found!
          else
            {
              Ptr<SatArqBufferContext> arqContext = Create<SatArqBufferContext> ();
              arqContext->m_pdu = NULL;
              arqContext->m_rxStatus = false;
              arqContext->m_seqNo = i;
              arqContext->m_retransmissionCount = 0;
              m_reorderingBuffer.insert (std::make_pair<uint8_t, Ptr<SatArqBufferContext> > (i, arqContext));

              EventId id = Simulator::Schedule (m_rxWaitingTimer, &SatReturnLinkEncapsulatorArq::RxWaitingTimerExpired, this, i);
              it->second->m_waitingTimer = id;
            }
        }
    }
  // SeqNo is the one we are expecting, thus try to reasseble and receive
  else
    {
      NS_LOG_LOGIC ("Received a packet with SeqNo we were expecting: " << m_nextExpectedSeqNo);
      ReassembleAndReceive();
    }
}

void
SatReturnLinkEncapsulatorArq::ReassembleAndReceive ()
{
  NS_LOG_FUNCTION (this);

  // Start from the expected sequence number iterator
  std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_reorderingBuffer.find (m_nextExpectedSeqNo);

  // If not found
  if (it == m_reorderingBuffer.end ())
    {
      NS_FATAL_ERROR ("Expected sequence number was not found in the reordering buffer!");
    }

  /**
   * As long as the PDU is the next expected one, process the PDU
   * and erase it.
   */
  while (it != m_reorderingBuffer.end () && it->first == m_nextExpectedSeqNo && it->second->m_rxStatus == true)
    {
      // If timer is running, cancel it.
      if (it->second->m_waitingTimer.IsRunning ())
        {
          it->second->m_waitingTimer.Cancel ();
        }

      // Process the PDU
      ProcessPdu (it->second->m_pdu);

      std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator currIt = it++;

      m_reorderingBuffer.erase (currIt);

      // Start from the beginning
      ++m_nextExpectedSeqNo;
      if (m_nextExpectedSeqNo >= std::numeric_limits<uint8_t>::max ())
        {
          m_nextExpectedSeqNo = 0;
          it = m_reorderingBuffer.begin ();
        }
      // Take the next iterator! Note, that it may also be m_reorderingBuffer.end ()
      else
        {
          NS_LOG_LOGIC ("Advance the iterator");
        }

      NS_LOG_LOGIC ("Starting to process a packet, increase the next expected seqNo to: " << m_nextExpectedSeqNo);
    }
}


void
SatReturnLinkEncapsulatorArq::RxWaitingTimerExpired (uint8_t seqNo)
{
  NS_LOG_FUNCTION (this << seqNo);

  // Find waiting timer, erase it and mark the packet received.
  std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_reorderingBuffer.find (seqNo);
  if (it != m_reorderingBuffer.end ())
    {
      it->second->m_waitingTimer.Cancel ();
      it->second->m_rxStatus = true;
    }
  else
    {
      NS_FATAL_ERROR ("Rx waiting timer is not running anymore even though it expired!");
    }

  ReassembleAndReceive ();
}


uint32_t
SatReturnLinkEncapsulatorArq::GetTxBufferSizeInBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_txQueue->GetNBytes () + m_txedBufferSize + m_retxBufferSize;
}

void
SatReturnLinkEncapsulatorArq::SendAck (uint8_t seqNo) const
{
  NS_LOG_FUNCTION (this << seqNo);

  NS_LOG_LOGIC ("GW: " << m_destAddress << " send ACK to UT: " << m_sourceAddress << " with flowId: " << (uint32_t)(m_flowId) << " with SN: " << (uint8_t)(seqNo) << " at: " << Now ().GetSeconds ());

  /**
   * RLE sends the ACK control message via a callback to SatNetDevice of the GW to the
   * GSE control buffer. The transmission assumptions for the control messages are currently
   * - Default (most robust) MODCOD
   * - Strict priority
   */
  if ( !m_ctrlCallback.IsNull ())
    {
      Ptr<SatArqAckMessage> ack = Create<SatArqAckMessage> ();
      ack->SetSequenceNumber (seqNo);
      ack->SetFlowId (m_flowId);

      // Source address (UT) is used here, since the in RTN the GW is
      // sending the ACK to the UT.
      m_ctrlCallback (ack, m_sourceAddress);
    }
  else
    {
      NS_FATAL_ERROR ("Unable to send ACK, since the Ctrl callback is NULL!");
    }
}

} // namespace ns3
