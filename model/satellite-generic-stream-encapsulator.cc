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
    m_gseHeaderSize (2),
    m_txFragmentId (0),
    m_currRxFragmentId (0),
    m_currRxPacketSize (0),
    m_currRxPacketFragmentBytes (0)

{
  NS_LOG_FUNCTION (this);

  /**
   * TODO: This is the current way of checking what is the GSE header size. Note,
   * that the header size is not correct (according to specs), thus it needs
   * to be corrected.
   */
  SatGseHeader gseHeader;
  m_gseHeaderSize = gseHeader.GetSerializedSize ();
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
                   UintegerValue (100 * 4095),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_maxTxBufferSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxGsePduSize",
                   "Maximum size of the GSE PDU (in Bytes)",
                   UintegerValue (4096),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_maxGsePduSize),
                   MakeUintegerChecker<uint32_t> ())
    //.AddAttribute ("GseHeaderSize",
    //               "GSE header size (in Bytes)",
    //               UintegerValue (2),
    //               MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_gseHeaderSize),
    //               MakeUintegerChecker<uint32_t> ())
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
  if (m_txBufferSize + p->GetSize () <= m_maxTxBufferSize)
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

  // No packets in buffer
  if ( m_txBuffer.size () == 0 )
    {
      NS_LOG_LOGIC ("No data pending");
      return packet;
    }

  // Tx opportunity bytes is not enough
  if (bytes <= m_gseHeaderSize)
    {
      NS_LOG_LOGIC ("TX opportunity too small = " << bytes);
      return packet;
    }

  // Take the first PDU from the buffer
  Ptr<Packet> nextPacket = (*(m_txBuffer.begin ()))->Copy ();

  // If the next PDU is full, increase the fragment id
  SatEncapPduStatusTag tag;
  nextPacket->PeekPacketTag (tag);
  if (tag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
    {
      // Build Data field
      uint32_t maxGsePacketSize = bytes - m_gseHeaderSize;

      /**
       * If the FULL PDU has to fit this BB frame since fragmentation
       * between BB frames is not allowed!
       */
      if (maxGsePacketSize >= nextPacket->GetSize ())
        {
          m_txFragmentId++;
        }
      else
        {
          NS_LOG_LOGIC ("FULL higher layer packet " << nextPacket->GetSize () <<  " does not fit into this Tx opportunity = " << bytes);
          return packet;
        }
    }

  m_txBufferSize -= (*(m_txBuffer.begin()))->GetSize ();
  m_txBuffer.erase (m_txBuffer.begin ());

  // Fragmentation
  double maxGseData = m_maxGsePduSize - m_gseHeaderSize;
  if ( nextPacket->GetSize () > maxGseData)
    {
      // Segment txBuffer.FirstBuffer and
      // Give back the remaining segment to the transmission buffer
      packet = nextPacket->CreateFragment (0, maxGseData);

      // Status tag of the new and remaining segments
      // Note: This is the only place where a PDU is segmented and
      // therefore its status can change
      SatEncapPduStatusTag oldTag, newTag;
      nextPacket->RemovePacketTag (oldTag);
      packet->RemovePacketTag (newTag);

      // Create new GSE header
      SatGseHeader gseHeader;
      gseHeader.SetGsePduLength (packet->GetSize());
      gseHeader.SetFragmentId (m_txFragmentId);

      if (oldTag.GetStatus () == SatEncapPduStatusTag::FULL_PDU)
        {
          gseHeader.SetStartIndicator ();
          gseHeader.SetTotalLength (nextPacket->GetSize());
          newTag.SetStatus (SatEncapPduStatusTag::START_PDU);
          oldTag.SetStatus (SatEncapPduStatusTag::END_PDU);
        }
      else if (oldTag.GetStatus () == SatEncapPduStatusTag::END_PDU)
        {
          // oldTag still is left with the END_PDU tag
          newTag.SetStatus (SatEncapPduStatusTag::CONTINUATION_PDU);
        }

      // Give back the remaining segment to the transmission buffer
      nextPacket->RemoveAtStart (maxGseData);

      nextPacket->AddPacketTag (oldTag);
      m_txBuffer.insert (m_txBuffer.begin (), nextPacket);
      m_txBufferSize += (*(m_txBuffer.begin()))->GetSize ();

      // Put status tag once it has been adjusted
      packet->AddPacketTag (newTag);

      // Add PDU header
      packet->AddHeader (gseHeader);
    }
  // Encapsulation
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

      gseHeader.SetGsePduLength (nextPacket->GetSize());
      gseHeader.SetFragmentId (m_txFragmentId);
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
  /**
   * Minimum valid Tx opportunity is the assumed header sizes + 1
   *
   */
  return m_gseHeaderSize + 1;
}


} // namespace ns3
