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

#ifndef SAT_BASE_ENCAPSULATOR_H
#define SAT_BASE_ENCAPSULATOR_H

#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "ns3/object.h"

#include "satellite-queue.h"
#include "satellite-control-message.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief A base encapsulator implementation which does not support
 * encapsulation, fragmentation or packing. Base encapsulator is used
 * for flows which do not need encapsulation or fragmentation, such as
 * lower layer control flows. Base encapsulator basically just stores
 * packets in SatQueue and sends them as they are.
 *
 * Return Link Encapsulator (RLE) and Generic Stream Encapsulator (GSE),
 * as well as their ARQ supportive versions, are inherited from this base
 * class.
 */
class SatBaseEncapsulator : public Object
{

public:
  /**
   * Default constructor not used
   */
  SatBaseEncapsulator ();

  /**
   * Constructor
   * \param source Configured source MAC addressd
   * \param dest Configured destination MAC address
   * \param flowId Flow identifier
   */
  SatBaseEncapsulator (Mac48Address source, Mac48Address dest, uint8_t flowId);

  /**
   * Destructor for SatBaseEncapsulator
   */
  virtual ~SatBaseEncapsulator ();

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
   * Callback to send packet to lower layer.
    * \param Ptr<Packet> the packet received
    * \param Mac48Address Source MAC address
    * \param Mac48Address Destination MAC address
    */
  typedef Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> ReceiveCallback;

  /**
   * Control msg sending callback
   * \param msg        the message send
   * \param address    Packet destination address
   * \return bool
   */
  typedef Callback<bool, Ptr<SatControlMessage>, const Address& > SendCtrlCallback;

  /**
   * Set the used queue from outside
   * \param queue Transmission queue
   */
  void SetQueue (Ptr<SatQueue> queue);

  /**
   * Get the queue instance
   * \return queue
   */
  Ptr<SatQueue> GetQueue ();

  /**
   * Method to set receive callback.
    * \param cb callback to invoke whenever a packet has been received and must
    *        be forwarded to the higher layers.
    */
  void SetReceiveCallback (ReceiveCallback cb);

  /**
   * \param cb callback to send control messages.
   */
  void SetCtrlMsgCallback (SatBaseEncapsulator::SendCtrlCallback cb);

  /**
   * Enqueue a packet to txBuffer.
   * \param p To be buffered packet
   * \param dest Target MAC address
   */
  virtual void EnquePdu (Ptr<Packet> p, Mac48Address dest);

  /**
   * Notify a Tx opportunity to this base encapsulator. Note, that
   * this class does not do encapsulator nor do not support fragmentation.
   *
   * \param bytes Notified Tx opportunity bytes from lower layer
   * \param bytesLeft Bytes left after this TxOpportunity in txBuffer
   * \param &nextMinTxO Minimum TxO after this TxO
   * \return An raw control PDU
   */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft, uint32_t &nextMinTxO);

  /**
   * Receive a packet. Note, that base encapsulator does not support
   * packet reception, since it assumes that packet receptions are
   * terminated already at lower layers.
   * \param p packet pointer received from lower layer
   */
  virtual void ReceivePdu (Ptr<Packet> p);

  /**
   * Receive a control message (ARQ ACK)
   * \param ack Control message received (e.g. ARQ ACK)
   */
  virtual void ReceiveAck (Ptr<SatArqAckMessage> ack);

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

protected:
  /**
   * Source and destination mac addresses. Used to tag the Frame PDU
   * so that lower layers are capable of passing the packet to the
   * correct destination.
   */
  Mac48Address m_sourceAddress;
  Mac48Address m_destAddress;

  /**
   * Flow identifier
   */
  uint8_t m_flowId;

  /**
   * Used queue in satellite encapsulator
   */
  Ptr<SatQueue> m_txQueue;

  /**
   * Receive callback
   */
  ReceiveCallback m_rxCallback;

  /**
   * Callback to send control messages.
  */
  SendCtrlCallback m_ctrlCallback;

};


} // namespace ns3

#endif // SAT_BASE_ENCAPSULATOR_H
