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


#ifndef SATELLITE_ARQ_BUFFER_CONTEXT_H_
#define SATELLITE_ARQ_BUFFER_CONTEXT_H_

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/event-id.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief ARQ buffer context is holding information related to the ARQ transmission
 * or reception depending on whether packet(s) are being transmitted or received.
 * The SatArqBufferContext is used only when ARQ is enabled, i.e. encapsulator is
 * of type SatReturnLinkEncapsulatorArq or SatGenericStreamEncapsulatorArq.
 */
class SatArqBufferContext : public Object
{
public:

  /**
   * Default constructor.
   */
  SatArqBufferContext ();
  ~SatArqBufferContext ()
  {
  }

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

public:
  Ptr<Packet> m_pdu;
  uint32_t    m_seqNo;
  uint32_t    m_retransmissionCount;
  EventId     m_waitingTimer;
  bool        m_rxStatus;
};

} // namespace

#endif /* SATELLITE_ARQ_BUFFER_CONTEXT_H_ */
