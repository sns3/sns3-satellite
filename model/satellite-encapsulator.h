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



class SatEncapsulator : public Object
{

public:
  SatEncapsulator ();
  virtual ~SatEncapsulator ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

protected:
  virtual void TransmitPdu (Ptr<Packet> p) = 0;

  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes) = 0;
  virtual void ReceivePdu (Ptr<Packet> p) = 0;

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
