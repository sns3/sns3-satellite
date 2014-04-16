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

#ifndef SATELLITE_GENERIC_STREAM_ENCAPSULATOR
#define SATELLITE_GENERIC_STREAM_ENCAPSULATOR


#include <map>
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "satellite-base-encapsulator.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatGenericStreamEncapsulator class is used in the FWD link for
 * GSE encapsulation and fragmentation of higher layer packets. The
 * SatGenericStreamEncapsulator object is UT specific and its entities
 * are located at both GW (encapsulation, fragmentation) and
 * UT (decapsulation, defragmentation).
 */
class SatGenericStreamEncapsulator : public SatBaseEncapsulator
{
public:

  /**
   * Default constructor, not used
   */
  SatGenericStreamEncapsulator ();

  /**
   * Constuctor
   */
  SatGenericStreamEncapsulator (Mac48Address source, Mac48Address dest, uint8_t flowId);
  virtual ~SatGenericStreamEncapsulator ();

  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Enqueue a Higher Layer packet to txBuffer.
   * \param p To be buffered packet
   * \param mac Target MAC address
   */
  virtual void TransmitPdu (Ptr<Packet> p, Mac48Address mac);

  /**
   * Notify a Tx opportunity to this encapsulator.
   * \param bytes Notified tx opportunity bytes from lower layer
   * \param bytesLeft Bytes left after this TxOpportunity in SatQueue
   * \return A GSE PDU
   */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft);

  /**
   * Receive a packet, thus decapsulate and defragment/deconcatenate
   * if needed. The decapsulated/defragmented HL PDU is forwarded back
   * to LLC and to upper layer.
   * \param p packet pointer received from lower layer
   */
  virtual void ReceivePdu (Ptr<Packet> p);

  /**
   * Get minimum Tx opportunity in bytes, which takes the
   * assumed header sizes into account.
   * \return uint32_t minimum tx opportunity
   */
  virtual uint32_t GetMinTxOpportunityInBytes () const;

protected:

  /**
   * Get new packet performs the GSE fragmentation and encapsulation
   * for a one single packet. Returns NULL packet if a suitable packet
   * is not created.
   * \return Ptr<Packet> GSE packet
   */
  Ptr<Packet> GetNewGsePdu (uint32_t txOpportunityBytes, uint32_t maxGsePduSize, uint32_t additionalHeaderSize = 0);

  /**
   * Method increases the fragment id by one. If the maximum fragment id is
   * reached, it is reset to zero.
   */
  void IncreaseFragmentId ();

  /**
   * Process the reception of individual GSE PDUs
   * \param p Packet to be received
   */
  virtual void ProcessPdu (Ptr<Packet> p);

  /**
   * Reset defragmentation variables
   */
  void Reset ();

  /**
   * Maximum GSE PDU size
   */
  uint32_t m_maxGsePduSize;

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
   * If the GSE opportunity is smaller than this, a NULL
   * packet is returned.
   */
  uint32_t m_minGseTxOpportunity;

  /**
   * The fragment is described with 8 bits, thus the
   * maximum fragment id is 256.
   */
  static const uint32_t MAX_FRAGMENT_ID = 256;

  /**
   * The maximum packet size is described with 16 bits,
   * thus, the maximum HL packet size is 65536 bytes.
   */
  static const uint32_t MAX_HL_PACKET_SIZE = 65536;

};


} // namespace ns3

#endif // SATELLITE_GENERIC_STREAM_ENCAPSULATOR
