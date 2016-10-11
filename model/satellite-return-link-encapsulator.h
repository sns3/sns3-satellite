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

#ifndef SATELLITE_RETURN_LINK_ENCAPSULATOR
#define SATELLITE_RETURN_LINK_ENCAPSULATOR


#include <map>
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "satellite-base-encapsulator.h"
#include "satellite-control-message.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatReturnLinkEncapsulator class is used in the RTN link for RLE
 * encapsulation and fragmentation of higher layer packets. SatReturnLinkEncapsulator
 * object is flow (UT address - flow id) specific and its entities are located at both
 * UT (encapsulation, fragmentation) and GW (decapsulation, defragmentation).
 * Return link encapsulators/decapsulators are created dynamically when first
 * needed (when a packet is received for this flow).
 *
 * \see ETSI TS 103 179 V1.1.1 Satellite Earth Stations and Systems (SES);
 * Return Link Encapsulation (RLE) protocol
 */
class SatReturnLinkEncapsulator : public SatBaseEncapsulator
{
public:
  /**
   * Default constructor, not used
   */
  SatReturnLinkEncapsulator ();

  /**
   * Constructor
   */
  SatReturnLinkEncapsulator (Mac48Address source, Mac48Address dest, uint8_t rcIndex);

  /**
   * Destructor for SatReturnLinkEncapsulator
   */
  virtual ~SatReturnLinkEncapsulator ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * Enqueue a Higher Layer packet to txBuffer.
   * \param p To be buffered packet
   * \param dest Target MAC address
   */
  virtual void EnquePdu (Ptr<Packet> p, Mac48Address dest);

  /**
   * Notify a Tx opportunity to this encapsulator.
   * \param bytes Notified tx opportunity bytes from lower layer
   * \param bytesLeft Bytes left after this TxOpportunity in txBuffer
   * \param &nextMinTxO Minimum TxO after this TxO
   * \return Ptr<Packet> a Frame PDU
   */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft, uint32_t &nextMinTxO);

  /**
   * Receive a packet, thus decapsulate and defragment/deconcatenate
   * if needed. The decapsulated/defragmented HL PDU is forwarded back to
   * LLC and to upper layer.
   * \param p packet pointer received from lower layer
   */
  virtual void ReceivePdu (Ptr<Packet> p);

  /**
   * Receive a control msg (ARQ ACK)
   * \param ack ACK control message pointer received from lower layer
   */
  virtual void ReceiveAck (Ptr<SatArqAckMessage> ack);

  /**
   * Get minimum Tx opportunity in bytes, which takes the
   * assumed header sizes into account.
   * \return Minimum Tx opportunity
   */
  virtual uint32_t GetMinTxOpportunityInBytes () const;

protected:
  /**
   * Get new packet performs the RLE fragmentation and encapsulation
   * for a one single packet. Returns NULL packet if a suitable packet
   * is not created.
   * \return A RLE packet
   */
  Ptr<Packet> GetNewRlePdu (uint32_t txOpportunityBytes, uint32_t maxRlePduSize, uint32_t additionalHeaderSize = 0);

  /**
   * Process the reception of individual RLE PDUs
   * \param p Packet to be received
   */
  virtual void ProcessPdu (Ptr<Packet> p);

  /**
   * Method increases the fragment id by one. If the maximum fragment id is
   * reached, it is reset to zero.
   */
  void IncreaseFragmentId ();

  /**
   * Reset defragmentation variables
   */
  void Reset ();

  /**
   * Fragment id used in the packet transmissions
   */
  uint32_t m_txFragmentId;
  /**
   * Current fragment id in the reassembly process
   */
  uint32_t m_currRxFragmentId;

  /**
   * Current packet in the reassembly process
   */
  Ptr<Packet> m_currRxPacketFragment;

  /**
   * The total size of the ALPDU size reassembly process
   */
  uint32_t m_currRxPacketSize;

  /**
   * Currently received bytes of the fragmented packet
   */
  uint32_t m_currRxPacketFragmentBytes;

  /**
   * Minimum Tx opportunity
   */
  uint32_t m_minTxOpportunity;

  /**
   * The fragment is described with 3 bits, thus the
   * maximum fragment id is 8.
   */
  const uint32_t MAX_FRAGMENT_ID;

  /**
   * The maximum PPDU fragment size is described with 11 bits,
   * thus, the maximum fragment size is 2048 bytes.
   */
  const uint32_t MAX_PPDU_PACKET_SIZE;

  /**
   * The maximum packet size is described with 12 bits,
   * thus, the maximum HL packet size is 4096 bytes.
   */
  const uint32_t MAX_HL_PDU_PACKET_SIZE;

};


} // namespace ns3

#endif // SATELLITE_RETURN_LINK_ENCAPSULATOR
