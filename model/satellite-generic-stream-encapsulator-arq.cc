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

#include "satellite-generic-stream-encapsulator-arq.h"
#include "satellite-llc.h"
#include "satellite-mac-tag.h"
#include "satellite-encap-pdu-status-tag.h"
#include "satellite-queue.h"
#include "satellite-arq-header.h"
#include "satellite-arq-buffer-context.h"


NS_LOG_COMPONENT_DEFINE ("SatGenericStreamEncapsulatorArq");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGenericStreamEncapsulatorArq);


SatGenericStreamEncapsulatorArq::SatGenericStreamEncapsulatorArq ()
  : m_seqNo (),
    m_txedBuffer (),
    m_retxBuffer (),
    m_retxBufferSize (0),
    m_txedBufferSize (0),
    m_maxNoOfRetransmissions (2),
    m_retransmissionTimer (Seconds (0.6)),
    m_arqWindowSize (10),
    m_arqHeaderSize (1),
    m_nextExpectedSeqNo (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * Default constructor is not meant to be used!
   */
}


SatGenericStreamEncapsulatorArq::SatGenericStreamEncapsulatorArq (Mac48Address source, Mac48Address dest, uint8_t flowId)
  : SatGenericStreamEncapsulator (source, dest, flowId),
    m_seqNo (),
    m_txedBuffer (),
    m_retxBuffer (),
    m_retxBufferSize (0),
    m_txedBufferSize (0),
    m_maxNoOfRetransmissions (2),
    m_retransmissionTimer (Seconds (0.6)),
    m_arqWindowSize (10),
    m_arqHeaderSize (1),
    m_nextExpectedSeqNo (0)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  // ARQ sequence number generator
  m_seqNo = Create<SatArqSequenceNumber> (m_arqWindowSize);

}

SatGenericStreamEncapsulatorArq::~SatGenericStreamEncapsulatorArq ()
{
  NS_LOG_FUNCTION (this);
  m_seqNo = 0;
}

TypeId
SatGenericStreamEncapsulatorArq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGenericStreamEncapsulatorArq")
    .SetParent<SatGenericStreamEncapsulator> ()
    .AddConstructor<SatGenericStreamEncapsulatorArq> ()
    .AddAttribute ( "MaxNoOfRetransmissions",
                    "Maximum number of retransmissions for a single GSE PDU.",
                    UintegerValue (2),
                    MakeUintegerAccessor (&SatGenericStreamEncapsulatorArq::m_maxNoOfRetransmissions),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "RetransmissionTimer",
                    "Retransmission time value, i.e. how long to wait for ACK before retransmission.",
                    TimeValue (Seconds (0.6)),
                    MakeTimeAccessor (&SatGenericStreamEncapsulatorArq::m_retransmissionTimer),
                    MakeTimeChecker ())
    .AddAttribute ( "WindowSize",
                    "Window size for ARQ, i.e. how many simultaneous packets are allowed in the air.",
                    UintegerValue (10),
                    MakeUintegerAccessor (&SatGenericStreamEncapsulatorArq::m_arqWindowSize),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "ArqHeaderSize",
                    "ARQ header size in Bytes.",
                    UintegerValue (1),
                    MakeUintegerAccessor (&SatGenericStreamEncapsulatorArq::m_arqHeaderSize),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "RxWaitingTime",
                    "Time to wait for a packet at the reception (GW) before moving onwards with the packet reception.",
                    TimeValue (Seconds (1.8)),
                    MakeTimeAccessor (&SatGenericStreamEncapsulatorArq::m_rxWaitingTimer),
                    MakeTimeChecker ())
  ;
  return tid;
}


TypeId
SatGenericStreamEncapsulatorArq::GetInstanceTypeId () const
{
  return GetTypeId ();
}


void
SatGenericStreamEncapsulatorArq::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_seqNo = 0;

  // Clean-up the Tx'ed buffer
  std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_txedBuffer.begin ();
  while (it != m_txedBuffer.end ())
    {
      it->second->DoDispose ();
      it->second = 0;
      ++it;
    }
  m_txedBuffer.clear ();

  // Clean-up the reTx buffer
  it = m_retxBuffer.begin ();
  while (it != m_retxBuffer.end ())
    {
      it->second->DoDispose ();
      it->second = 0;
      ++it;
    }
  m_retxBuffer.clear ();

  // Clean-up the reordering buffer
  std::map<uint32_t, Ptr<SatArqBufferContext> >::iterator it2 = m_reorderingBuffer.begin ();
  while (it2 != m_reorderingBuffer.end ())
    {
      it2->second->DoDispose ();
      it2->second = 0;
      ++it2;
    }
  m_reorderingBuffer.clear ();

  SatGenericStreamEncapsulator::DoDispose ();
}


Ptr<Packet>
SatGenericStreamEncapsulatorArq::NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << bytes);
  NS_LOG_INFO ("TxOpportunity for UT: " << m_destAddress << " flowId: " << (uint32_t) m_flowId << " of " << bytes << " bytes");

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

          std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_txedBuffer.find (context->m_seqNo);
          if (it != m_txedBuffer.end ())
            {
              NS_FATAL_ERROR ("Trying to add retransmission packet to txedBuffer even though it already exists there!");
            }

          // Store it back to the transmitted packet container.
          m_txedBuffer.insert (std::make_pair (context->m_seqNo, context));

          // Create the retransmission event and store it to the context. Event is cancelled if a ACK
          // is received. However, if the event triggers, we shall send the packet again, if the packet still
          // has retransmissions left.
          EventId t = Simulator::Schedule (m_retransmissionTimer, &SatGenericStreamEncapsulatorArq::ArqReTxTimerExpired, this, context->m_seqNo);
          context->m_waitingTimer = t;

          NS_LOG_INFO ("GW: << " << m_sourceAddress << " sent a retransmission packet of size: " << context->m_pdu->GetSize () << " with seqNo: " << (uint32_t)(context->m_seqNo) << " flowId: " << (uint32_t)(m_flowId) << " at: " << Now ().GetSeconds ());

          Ptr<Packet> copy = context->m_pdu->Copy ();
          return copy;
        }
      else
        {
          NS_LOG_INFO ("Retransmission PDU: " << context->m_pdu->GetUid () << " size: " << context->m_pdu->GetSize () << " does not fit into TxO: " << bytes);
        }
    }

  // Check the transmission buffer. Sequence number needs to be
  // available for any new transmissions.
  else if (!m_txQueue->IsEmpty () && m_seqNo->SeqNoAvailable ())
    {
      // Crate new GSE PDU
      packet = GetNewGsePdu (bytes, bytes, m_arqHeaderSize);

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
          Ptr<SatArqBufferContext> arqContext = CreateObject<SatArqBufferContext> ();
          arqContext->m_retransmissionCount = 0;
          Ptr<Packet> copy = packet->Copy ();
          arqContext->m_pdu = copy;
          arqContext->m_seqNo = seqNo;

          // Create the retransmission event and store it to the context. Event is cancelled if a ACK
          // is received. However, if the event triggers, we shall send the packet again, if the packet still
          // has retransmissions left.
          arqContext->m_waitingTimer = Simulator::Schedule (m_retransmissionTimer, &SatGenericStreamEncapsulatorArq::ArqReTxTimerExpired, this, seqNo);

          // Update the buffer status
          m_txedBufferSize += packet->GetSize ();
          m_txedBuffer.insert (std::make_pair (seqNo, arqContext));

          if (packet->GetSize () > bytes)
            {
              NS_FATAL_ERROR ("Created packet of size: " << packet->GetSize () << " is larger than the tx opportunity: " << bytes);
            }

          NS_LOG_INFO ("GW: << " << m_sourceAddress << " sent a packet of size: " << packet->GetSize () << " with seqNo: " << (uint32_t)(seqNo) << " flowId: " << (uint32_t)(m_flowId) << " at: " << Now ().GetSeconds ());
          NS_LOG_INFO ("Queue size after TxOpportunity: " << m_txQueue->GetNBytes ());
        }
    }

  // Update bytes lefts
  bytesLeft = GetTxBufferSizeInBytes ();

  // Update min TxO
  nextMinTxO = GetMinTxOpportunityInBytes ();

  return packet;
}

void
SatGenericStreamEncapsulatorArq::ArqReTxTimerExpired (uint8_t seqNo)
{
  NS_LOG_FUNCTION (this << (uint32_t) seqNo);

  NS_LOG_INFO ("At GW: " << m_sourceAddress << " ARQ retransmission timer expired for: " << (uint32_t)(seqNo) << " at: " << Now ().GetSeconds ());

  std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_txedBuffer.find (seqNo);

  if (it != m_txedBuffer.end ())
    {
      NS_ASSERT (seqNo == it->second->m_seqNo);
      NS_ASSERT (it->second->m_pdu);

      // Retransmission still possible
      if (it->second->m_retransmissionCount < m_maxNoOfRetransmissions)
        {
          NS_LOG_INFO ("Moving the ARQ context to retransmission buffer");

          Ptr<SatArqBufferContext> context = it->second;

          m_txedBuffer.erase (it);
          m_retxBufferSize += context->m_pdu->GetSize ();

          // Push to the retransmission buffer
          m_retxBuffer.insert (std::make_pair (seqNo, context));
        }
      // Maximum retransmissions reached
      else
        {
          NS_LOG_INFO ("For GW: " << m_sourceAddress << " max retransmissions reached for " << (uint32_t)(seqNo) << " at: " << Now ().GetSeconds ());

          // Do clean-up
          CleanUp (seqNo);
        }
    }
  else
    {
      NS_LOG_INFO ("Element not found anymore in the m_txedBuffer, thus ACK has been received already earlier");
    }
}

void
SatGenericStreamEncapsulatorArq::CleanUp (uint8_t sequenceNumber)
{
  NS_LOG_FUNCTION (this << (uint32_t) sequenceNumber);

  // Release sequence number
  m_seqNo->Release (sequenceNumber);

  // Clean-up the Tx'ed buffer
  std::map<uint8_t, Ptr<SatArqBufferContext> >::iterator it = m_txedBuffer.find (sequenceNumber);
  if (it != m_txedBuffer.end ())
    {
      NS_LOG_INFO ("Sequence no: " << (uint32_t) sequenceNumber << " clean up from txedBuffer!");
      m_txedBufferSize -= it->second->m_pdu->GetSize ();
      it->second->DoDispose ();
      it->second = 0;
      m_txedBuffer.erase (it);
    }

  // Clean-up the reTx buffer
  it = m_retxBuffer.find (sequenceNumber);
  if (it != m_retxBuffer.end ())
    {
      NS_LOG_INFO ("Sequence no: " << (uint32_t) sequenceNumber << " clean up from retxBuffer!");
      m_retxBufferSize -= it->second->m_pdu->GetSize ();
      it->second->DoDispose ();
      it->second = 0;
      m_retxBuffer.erase (it);
    }
}


void
SatGenericStreamEncapsulatorArq::ReceiveAck (Ptr<SatArqAckMessage> ack)
{
  NS_LOG_FUNCTION (this);

  /**
   * Take the packet away from the transmitted or retransmission
   * buffer and release the sequence number for further use.
   */

  NS_LOG_INFO ("GW: " << m_sourceAddress << " received ACK with SN: " << (uint32_t)(ack->GetSequenceNumber ()) << " at: " << Now ().GetSeconds ());

  // Do clean-up
  CleanUp (ack->GetSequenceNumber ());
}


void
SatGenericStreamEncapsulatorArq::ReceivePdu (Ptr<Packet> p)
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
  bool mSuccess = p->RemovePacketTag (mTag);
  if (!mSuccess)
    {
      NS_FATAL_ERROR ("MAC tag not found in the packet!");
    }
  else if (mTag.GetDestAddress () != m_destAddress)
    {
      NS_FATAL_ERROR ("Packet was not intended for this receiver!");
    }

  SatArqHeader arqHeader;
  p->RemoveHeader (arqHeader);
  uint8_t seqNo = arqHeader.GetSeqNo ();

  NS_LOG_INFO ("GW: " << m_sourceAddress << " received a packet with SeqNo: " << (uint32_t)(seqNo) << " at: " << Now ().GetSeconds ());

  // Send ACK for the received GSE packet.
  SendAck (seqNo);

  // Convert the 8-bit sequence number to continuous 32-bit sequence number
  uint32_t sn = ConvertSeqNo (seqNo);

  NS_LOG_INFO ("8bit SN: " << (uint32_t)(seqNo) << " 32bit SN: " << sn);

  // If the received SN is valid. If we receive a SN from the past
  // nothing is needed to be done.
  if (sn >= m_nextExpectedSeqNo)
    {
      std::map<uint32_t, Ptr<SatArqBufferContext > >::iterator it = m_reorderingBuffer.find (sn);

      // If the context is not found, then we create a new one.
      if (it == m_reorderingBuffer.end ())
        {
          NS_LOG_INFO ("GW: " << m_sourceAddress << " created a new ARQ buffer entry for SeqNo: " << sn << " at: " << Now ().GetSeconds ());
          Ptr<SatArqBufferContext> arqContext = CreateObject<SatArqBufferContext> ();
          arqContext->m_pdu = p;
          arqContext->m_rxStatus = true;
          arqContext->m_seqNo = sn;
          arqContext->m_retransmissionCount = 0;
          m_reorderingBuffer.insert (std::make_pair (sn, arqContext));
        }
      // If the context is found, update it.
      else
        {
          NS_LOG_INFO ("GW: " << m_sourceAddress << " reset an existing ARQ entry for SeqNo: " << sn << " at " << Now ().GetSeconds ());
          it->second->m_waitingTimer.Cancel ();
          it->second->m_pdu = p;
          it->second->m_rxStatus = true;
        }

      NS_LOG_INFO ("Received a packet with SeqNo: " << sn << ", expecting: " << m_nextExpectedSeqNo);

      // If this is not the SN we expect
      if (sn != m_nextExpectedSeqNo)
        {
          // Add context
          for (uint32_t i = m_nextExpectedSeqNo; i < sn; ++i)
            {
              std::map<uint32_t, Ptr<SatArqBufferContext > >::iterator it2 = m_reorderingBuffer.find (i);

              NS_LOG_INFO ("Finding context for " << i);

              // If context not found
              if (it2 == m_reorderingBuffer.end ())
                {
                  NS_LOG_INFO ("Context NOT found for SeqNo: " << i);

                  Ptr<SatArqBufferContext> arqContext = CreateObject<SatArqBufferContext> ();
                  arqContext->m_pdu = NULL;
                  arqContext->m_rxStatus = false;
                  arqContext->m_seqNo = i;
                  arqContext->m_retransmissionCount = 0;
                  m_reorderingBuffer.insert (std::make_pair (i, arqContext));
                  EventId id = Simulator::Schedule (m_rxWaitingTimer, &SatGenericStreamEncapsulatorArq::RxWaitingTimerExpired, this, i);
                  arqContext->m_waitingTimer = id;
                }
            }
        }
      // An expected sequence number received, reassemble and receive.
      else
        {
          ReassembleAndReceive ();
        }
    }
  else
    {
      NS_LOG_INFO ("GW: " << m_sourceAddress << " received a packet with SeqNo: " << sn << " which is already received!");
    }
}

uint32_t
SatGenericStreamEncapsulatorArq::ConvertSeqNo (uint8_t seqNo) const
{
  NS_LOG_FUNCTION (this << (uint32_t) seqNo);

  uint32_t globalSeqNo (0);

  // Calculate the rounds and current seq no from m_nextExpectedSeqNo
  uint32_t rounds = (m_nextExpectedSeqNo / std::numeric_limits<uint8_t>::max ());
  uint32_t rawSeqNo = m_nextExpectedSeqNo % std::numeric_limits<uint8_t>::max ();

  NS_LOG_INFO ("Input: " << (uint32_t)(seqNo) << " rounds: " << rounds << " rawSeqNo: " << rawSeqNo << " windowSize: " << m_arqWindowSize << " next expected: " << m_nextExpectedSeqNo);

  // Received sequence number is higher than the expected one.
  if (seqNo >= rawSeqNo)
    {
      // If seqNo is from previous round
      if ((seqNo - rawSeqNo) > 2 * m_arqWindowSize)
        {
          rounds--;
        }
    }
  // seqNo < rawSeqNo
  else
    {
      if ((rawSeqNo - seqNo) > 2 * m_arqWindowSize)
        {
          rounds++;
        }
    }

  globalSeqNo = rounds * std::numeric_limits<uint8_t>::max () + seqNo;

  return globalSeqNo;
}

void
SatGenericStreamEncapsulatorArq::ReassembleAndReceive ()
{
  NS_LOG_FUNCTION (this);

  // Start from the expected sequence number iterator
  std::map<uint32_t, Ptr<SatArqBufferContext> >::iterator it = m_reorderingBuffer.begin ();

  NS_LOG_INFO ("Process SeqNo: " << it->first << ", expected: " << m_nextExpectedSeqNo << ", status: " << it->second->m_rxStatus);

  /**
   * As long as the PDU is the next expected one, process the PDU
   * and erase it.
   */
  while (it != m_reorderingBuffer.end () && it->first == m_nextExpectedSeqNo && it->second->m_rxStatus == true)
    {
      NS_LOG_INFO ("Process SeqNo: " << it->first << ", expected: " << m_nextExpectedSeqNo << ", status: " << it->second->m_rxStatus);

      // If PDU == NULL, it means that the RxWaitingTimer has expired
      // without PDU being received
      if (it->second->m_pdu)
        {
          // Process the PDU
          ProcessPdu (it->second->m_pdu);
        }

      it->second->DoDispose ();
      it->second = 0;
      m_reorderingBuffer.erase (it);
      it = m_reorderingBuffer.begin ();

      // Increase the seq no
      ++m_nextExpectedSeqNo;

      NS_LOG_INFO ("Increasing SeqNo to " << m_nextExpectedSeqNo);
    }
}


void
SatGenericStreamEncapsulatorArq::RxWaitingTimerExpired (uint32_t seqNo)
{
  NS_LOG_FUNCTION (this << (uint32_t) seqNo);

  NS_LOG_INFO ("For GW: " << m_sourceAddress << " max waiting time reached for SeqNo: " << seqNo << " at: " << Now ().GetSeconds ());
  NS_LOG_INFO ("Mark the PDU received and move forward!");

  // Find waiting timer, erase it and mark the packet received.
  std::map<uint32_t, Ptr<SatArqBufferContext> >::iterator it = m_reorderingBuffer.find (seqNo);
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
SatGenericStreamEncapsulatorArq::GetTxBufferSizeInBytes () const
{
  NS_LOG_FUNCTION (this);

  return m_txQueue->GetNBytes () + m_retxBufferSize;
}

void
SatGenericStreamEncapsulatorArq::SendAck (uint8_t seqNo) const
{
  NS_LOG_FUNCTION (this << (uint32_t) seqNo);

  NS_LOG_INFO ("GW: " << m_destAddress << " send ACK to GW: " << m_sourceAddress << " with flowId: " << (uint32_t)(m_flowId) << " with SN: " << (uint32_t)(seqNo) << " at: " << Now ().GetSeconds ());

  /**
   * GSE sends the ACK control message via a callback to SatNetDevice of the GW to the
   * RLE control buffer. The transmission assumptions for the control messages are currently
   * - Default (most robust) MODCOD
   * - Strict priority
   */
  if ( !m_ctrlCallback.IsNull ())
    {
      Ptr<SatArqAckMessage> ack = Create<SatArqAckMessage> ();
      ack->SetSequenceNumber (seqNo);
      ack->SetFlowId (m_flowId);

      // Source address (GW) is used here, since the in FWD link the UT is
      // sending the ACK to the GW.
      m_ctrlCallback (ack, m_sourceAddress);
    }
  else
    {
      NS_FATAL_ERROR ("Unable to send ACK, since the Ctrl callback is NULL!");
    }
}

} // namespace ns3
