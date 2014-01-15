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
#include "satellite-rle-pdu-status-tag.h"
#include "satellite-rle-headers.h"
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
    m_maxAlPduSize (4095),
    m_ppduHeaderSize (2),
    m_fpduHeaderSize (1),
    m_txFragmentId (0),
    m_currRxFragmentId (0),
    m_currRxPacketSize (0),
    m_currRxPacketFragmentBytes (0)

{
  NS_LOG_FUNCTION (this);
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
    .AddAttribute ("MaxAdressedLinkPduSize",
                   "Maximum size of the Addressed Link PDU (in Bytes)",
                   UintegerValue (4095),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_maxAlPduSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("PayloadAdaptedPduHeaderSize",
                   "Payload adapted PDU header size (in Bytes)",
                   UintegerValue (2),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_ppduHeaderSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("FramePduHeaderSize",
                   "Frame PDU header size (in Bytes)",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SatGenericStreamEncapsulator::m_fpduHeaderSize),
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
  if (m_txBufferSize + p->GetSize () <= m_maxTxBufferSize)
    {
      // Store packet arrival time
      SatTimeTag timeTag (Simulator::Now ());
      p->AddPacketTag (timeTag);

      // Mark the PDU with FULL_PDU tag
      SatRlePduStatusTag tag;
      tag.SetStatus (SatRlePduStatusTag::FULL_PPDU);
      p->AddPacketTag (tag);

      /**
       * TODO: This is the place to encapsulate the HL packet
       * with Addressed Link (AL) header.
       */

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
  NS_LOG_FUNCTION (this << bytes);

  // Frame PDU
  Ptr<Packet> packet;

  // Tx opportunity bytes is not enough
  if (bytes <= (m_ppduHeaderSize + m_fpduHeaderSize))
    {
      NS_LOG_LOGIC ("TX opportunity too small = " << bytes);
      return packet;
    }

  // Build Data field
  uint32_t nextSegmentSize = bytes - (m_ppduHeaderSize + m_fpduHeaderSize);
  uint32_t dataFieldTotalSize = 0;
  uint32_t dataFieldAddedSize = 0;
  std::vector < Ptr<Packet> > dataField;

  // No packets in buffer
  if ( m_txBuffer.size () == 0 )
    {
      NS_LOG_LOGIC ("No data pending");
      return packet;
    }

  // Frame PDU
  packet = Create<Packet> ();

  // Frame header
  SatFPduHeader fpduHeader;

  // Take the first PDU from the buffer
  Ptr<Packet> firstSegment = (*(m_txBuffer.begin ()))->Copy ();
  m_txBufferSize -= (*(m_txBuffer.begin()))->GetSize ();
  m_txBuffer.erase (m_txBuffer.begin ());

  // If the next PDU is full, increase the fragment id
  SatRlePduStatusTag tag;
  firstSegment->PeekPacketTag (tag);
  if (tag.GetStatus () == SatRlePduStatusTag::FULL_PPDU)
    {
      m_txFragmentId++;
    }

  // While we have a first segment
  while ( firstSegment && (firstSegment->GetSize () > 0) && (nextSegmentSize > 0) )
    {
      // Fragmentation functionality
      if ( (firstSegment->GetSize () > nextSegmentSize) ||
           (firstSegment->GetSize () > m_maxAlPduSize) )
        {
          uint32_t currSegmentSize = std::min (firstSegment->GetSize (), nextSegmentSize);

          // Segment txBuffer.FirstBuffer and
          // Give back the remaining segment to the transmission buffer
          Ptr<Packet> newSegment = firstSegment->CreateFragment (0, currSegmentSize);

          // Status tag of the new and remaining segments
          // Note: This is the only place where a PDU is segmented and
          // therefore its status can change
          SatRlePduStatusTag oldTag, newTag;
          firstSegment->RemovePacketTag (oldTag);
          newSegment->RemovePacketTag (newTag);

          // Create new PPDU header
          SatPPduHeader ppduHeader;
          ppduHeader.SetPPduLength (newSegment->GetSize());
          ppduHeader.SetFragmentId (m_txFragmentId);

          if (oldTag.GetStatus () == SatRlePduStatusTag::FULL_PPDU)
            {
              ppduHeader.SetStartIndicator ();
              ppduHeader.SetTotalLength (firstSegment->GetSize());
              newTag.SetStatus (SatRlePduStatusTag::START_PPDU);
              oldTag.SetStatus (SatRlePduStatusTag::END_PPDU);
            }
          else if (oldTag.GetStatus () == SatRlePduStatusTag::END_PPDU)
            {
              // oldTag still is left with the END_PPDU tag
              newTag.SetStatus (SatRlePduStatusTag::CONTINUATION_PPDU);
            }

          // Give back the remaining segment to the transmission buffer
          firstSegment->RemoveAtStart (currSegmentSize);

          // If bytes left after fragmentation
          if (firstSegment->GetSize () > 0)
            {
              firstSegment->AddPacketTag (oldTag);
              m_txBuffer.insert (m_txBuffer.begin (), firstSegment);
              m_txBufferSize += (*(m_txBuffer.begin()))->GetSize ();
            }
          else
            {
              NS_FATAL_ERROR ("The full segment was taken even though we are in the fragmentation part of the code!");
            }

          // Segment is completely taken or
          // the remaining segment is given back to the transmission buffer
          firstSegment = 0;

          // Put status tag once it has been adjusted
          newSegment->AddPacketTag (newTag);

          // Add Segment to Data field
          dataFieldAddedSize = newSegment->GetSize ();
          dataFieldTotalSize += dataFieldAddedSize;

          // Add PPDU header
          newSegment->AddHeader (ppduHeader);

          // Add PPDU to dataField
          dataField.push_back (newSegment);

          // no LengthIndicator for the last one
          nextSegmentSize -= dataFieldAddedSize;

          // Add PPDU info to the FPDU header
          fpduHeader.PushPPduLength (newSegment->GetSize());

          newSegment = 0;
        }
      // Packing functionality, for either a FULL_PPDU or END_PPDU
      else
        {
          // Add txBuffer.FirstBuffer to DataField
          dataFieldAddedSize = firstSegment->GetSize ();
          dataFieldTotalSize += dataFieldAddedSize;
          dataField.push_back (firstSegment);

          nextSegmentSize -= dataFieldAddedSize;

          // Add PPDU header
          SatPPduHeader ppduHeader;
          ppduHeader.SetEndIndicator ();
          ppduHeader.SetFragmentId (m_txFragmentId);
          ppduHeader.SetPPduLength (firstSegment->GetSize());

          SatRlePduStatusTag tag;
          firstSegment->PeekPacketTag (tag);
          if (tag.GetStatus() == SatRlePduStatusTag::FULL_PPDU)
            {
              ppduHeader.SetStartIndicator ();
            }

          // Add PPDU header
          firstSegment->AddHeader (ppduHeader);

          // Add PPDU info to the FPDU header
          fpduHeader.PushPPduLength (firstSegment->GetSize());

          // If there are still space in TxOpportunity AND
          // bytes left in the txBuffer
          if (nextSegmentSize > (m_ppduHeaderSize + m_fpduHeaderSize) &&
              !m_txBuffer.empty ())
            {
              // Get another segment
              firstSegment = (*(m_txBuffer.begin ()))->Copy ();
              m_txBufferSize -= (*(m_txBuffer.begin()))->GetSize ();
              m_txBuffer.erase (m_txBuffer.begin ());
              NS_LOG_LOGIC ("        txBufferSize = " << m_txBufferSize );

              // If the next PDU is full, increase the fragment id
              SatRlePduStatusTag tag;
              firstSegment->PeekPacketTag (tag);
              if (tag.GetStatus () == SatRlePduStatusTag::FULL_PPDU)
                {
                  m_txFragmentId++;
                }
            }
          // Else end loop
          else
            {
              firstSegment = 0;
            }
        }
    }

  // Build encapsulated PDU with DataField and Header
  std::vector< Ptr<Packet> >::iterator it;
  it = dataField.begin ();

  while (it < dataField.end ())
    {
      NS_LOG_LOGIC ("Adding SDU/segment to packet, length = " << (*it)->GetSize ());

      packet->AddAtEnd (*it);
      it++;
    }

  // Add the Frame PDU header
  packet->AddHeader (fpduHeader);

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

  // Remove frame PDU header
  SatFPduHeader fpduHeader;
  p->RemoveHeader (fpduHeader);

  // Get number of packed PPdus
  uint32_t numPPdus = fpduHeader.GetNumPPdus ();

  // If several packets have been packed, create original
  // fragments (= PPDUs)
  if (numPPdus > 1)
    {
      for (uint32_t i = 0; i < numPPdus; ++i)
        {
          uint32_t lengthIndicator = fpduHeader.GetPPduLength (i);

          // Sanity check
         if (p->GetSize() < lengthIndicator)
            {
              NS_FATAL_ERROR ("Packet size is smaller than the fragment size!");
            }

          Ptr<Packet> data_field = p->CreateFragment (0, lengthIndicator);
          p->RemoveAtStart (lengthIndicator);
          m_rxBuffer.push_back (data_field);
        }
    }
  // Only one PPDU per FPDU
  else
    {
      m_rxBuffer.push_back(p);
    }

  // Try to reassemble the received packets
  Reassemble ();
}


void
SatGenericStreamEncapsulator::Reassemble ()
{
  while (!m_rxBuffer.empty ())
    {
      // Remove PPDU header
      SatPPduHeader ppduHeader;
      m_rxBuffer.front()->RemoveHeader (ppduHeader);

      /**
       * TODO: The PHY packet errors have to be checked from FPDUs and distributed
       * to HL PDUs.
       */

      // FULL_PPDU
      if (ppduHeader.GetStartIndicator() == true && ppduHeader.GetEndIndicator() == true)
        {
          m_rxCallback(m_rxBuffer.front ());
          m_rxBuffer.pop_front();
        }

      // START_PPDU
      else if (ppduHeader.GetStartIndicator() == true && ppduHeader.GetEndIndicator() == false)
        {
          if (m_currRxPacketFragment)
            {
              NS_FATAL_ERROR ("Trying to defragment a new packet while we have previous packet in process!");
            }

          m_currRxFragmentId = ppduHeader.GetFragmentId ();
          m_currRxPacketSize = ppduHeader.GetTotalLength ();
          m_currRxPacketFragmentBytes = ppduHeader.GetPPduLength ();
          m_currRxPacketFragment = m_rxBuffer.front ();
          m_rxBuffer.pop_front();
        }

      // CONTINUATION_PPDU
      else if (ppduHeader.GetStartIndicator() == false && ppduHeader.GetEndIndicator() == false)
        {
          if (!m_currRxPacketFragment)
            {
              NS_FATAL_ERROR ("Trying to defragment end of packet while we do not have packet in process!");
            }
          if (ppduHeader.GetFragmentId () != m_currRxFragmentId)
            {
              NS_FATAL_ERROR ("Fragmenting wrong fragment id!");
            }
          m_currRxPacketFragmentBytes += ppduHeader.GetPPduLength ();
          m_currRxPacketFragment->AddAtEnd (m_rxBuffer.front ());
          m_rxBuffer.pop_front();
        }

      // END_PPDU
      else if (ppduHeader.GetStartIndicator() == false && ppduHeader.GetEndIndicator() == true)
        {
          if (!m_currRxPacketFragment)
            {
              NS_FATAL_ERROR ("Trying to defragment end of packet while we do not have packet in process!");
            }
          if (ppduHeader.GetFragmentId () != m_currRxFragmentId)
            {
              NS_FATAL_ERROR ("Fragmenting wrong fragment id!");
            }

          m_currRxPacketFragmentBytes += ppduHeader.GetPPduLength ();

          if (m_currRxPacketFragmentBytes != m_currRxPacketSize)
            {
              NS_FATAL_ERROR ("Total packet size wrong!");
            }

          m_currRxPacketFragment->AddAtEnd (m_rxBuffer.front ());
          m_rxBuffer.pop_front();

          m_rxCallback (m_currRxPacketFragment);
          m_currRxPacketFragment = 0;
          m_currRxPacketFragmentBytes = 0;
        }
    }
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
  return m_ppduHeaderSize + m_fpduHeaderSize + 1;
}


} // namespace ns3
