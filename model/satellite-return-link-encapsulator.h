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
#include "satellite-encapsulator.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatReturnLinkEncapsulator class used in return link for
 * encapsulation, fragmentation and packing of higher layer packets.
 * The SatReturnLinkEncapsulator object is UT specific and its entities
 * are located at both UT (encapsulation, fragmentation, packing) and
 * GW (decapsulation, defragmentation, reassembly).
 */
class SatReturnLinkEncapsulator : public SatEncapsulator
{
public:

  /**
   * Default constructor, not used
   */
  SatReturnLinkEncapsulator ();

  /**
   * Constuctor
   */
  SatReturnLinkEncapsulator (Mac48Address source, Mac48Address dest);
  virtual ~SatReturnLinkEncapsulator ();

  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Enqueue a Higher Layer packet to txBuffer.
   * \param p To be buffered packet
   */
  virtual void TransmitPdu (Ptr<Packet> p);

  /**
   * Notify a Tx opportunity to this encapsulator.
   * HL packet = IP packet
   * AL PDU = Addressed link PDU
   * PPDU = Payload Adapted PDU
   * FPDU = Frame PDU
   * \param bytes Notified tx opportunity bytes from lower layer
   * \param bytesLeft Bytes left after this TxOpportunity in txBuffer
   * \return Ptr<Packet> a Frame PDU
   */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft);

  /**
   * Receive a packet, thus decapsulate and defragment/deconcatenate
   * if needed. The formulated HL PDU is forwarded back to LLC and
   * to upper layer.
   * \param p packet pointer received from lower layer
   */
  virtual void ReceivePdu (Ptr<Packet> p);

  /**
   * Get the buffered packets for this encapsulator
   * \return uint32_t buffered bytes
   */
  virtual uint32_t GetTxBufferSizeInBytes () const;
  
  /**
   * Get Head-of-Line packet buffering delay.
   * \return Time HoL buffering delay
   */
  virtual Time GetHolDelay () const;

  /**
   * Get minimum Tx opportunity in bytes, which takes the
   * assumed header sizes into account.
   * \return uint32_t minimum tx opportunity
   */
  virtual uint32_t GetMinTxOpportunityInBytes () const;

  /**
   * Callback to send packet to lower layer.
    * \param Ptr<Packet> the packet received
    */
  typedef Callback<void, Ptr<Packet> > ReceiveCallback;

  /**
   * Method to set receive callback.
    * \param cb callback to invoke whenever a packet has been received and must
    *        be forwarded to the higher layers.
    */
  void SetReceiveCallback (ReceiveCallback cb);

private:

  /**
   * Method increases the fragment id by one. If the maximum fragment id is
   * reached, it is reset to zero.
   */
  void IncreaseFragmentId ();


  void Reassemble ();

  void Reset ();

  /**
   * Source and destination mac addresses. Used to tag the Frame PDU
   * so that lower layers are capable of passing the packet to the
   * correct destination.
   */
  Mac48Address m_sourceAddress;
  Mac48Address m_destAddress;

  /**
   * Maximum buffer size in bytes
   */
  uint32_t m_maxTxBufferSize;

  /**
   * Buffered bytes
   */
  uint32_t m_txBufferSize;

  /**
   * Physical PDU header size
   */
  uint32_t m_ppduHeaderSize;

  /**
   * Frame PDU header size
   */
  uint32_t m_fpduHeaderSize;

  /**
   * Fragment id used in the packet transmissions
   */
  uint32_t m_txFragmentId;

  /**
   * Transmission buffer
   */
  std::list < Ptr<Packet> > m_txBuffer;

  /**
   * Reception buffer
   */
  std::list < Ptr<Packet> >  m_rxBuffer;

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
   * The fragment is described with 3 bits, thus the
   * maximum fragment id is 8.
   */
  static const uint32_t MAX_FRAGMENT_ID = 8;

  /**
   * The maximum PPDU fragment size is described with 11 bits,
   * thus, the maximum fragment size is 2048 bytes.
   */
  static const uint32_t MAX_PPDU_PACKET_SIZE = 2048;

  /**
   * The maximum packet size is described with 12 bits,
   * thus, the maximum HL packet size is 4096 bytes.
   */
  static const uint32_t MAX_HL_PDU_PACKET_SIZE = 4096;

};


} // namespace ns3

#endif // SATELLITE_RETURN_LINK_ENCAPSULATOR
