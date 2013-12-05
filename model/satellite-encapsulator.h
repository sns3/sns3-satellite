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

#ifndef SAT_ENCAPSULATOR_H
#define SAT_ENCAPSULATOR_H

#include <ns3/packet.h>
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"

#include "ns3/object.h"

namespace ns3 {


/**
 * \ingroup satellite
 * \brief SatEncapsulator base class. The actual implementations are located
 * in the inherited classes, e.g.
 * - SatGenericEncapsulator
 * - SatReturnLinkEncapsulator
 */
class SatEncapsulator : public Object
{

public:
  SatEncapsulator ();
  virtual ~SatEncapsulator ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

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

  virtual void TransmitPdu (Ptr<Packet> p) = 0;
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft) = 0;
  virtual void ReceivePdu (Ptr<Packet> p) = 0;
  virtual uint32_t GetTxBufferSizeInBytes () const = 0;
  virtual Time GetHolDelay () const = 0;

protected:

  ReceiveCallback m_rxCallback;

  /**
   * Used to inform of a PDU delivery
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

};


} // namespace ns3

#endif // SAT_ENCAPSULATOR_H
