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


#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-queue.h"
#include "satellite-encapsulator.h"


NS_LOG_COMPONENT_DEFINE ("SatEncapsulator");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatEncapsulator);

SatEncapsulator::SatEncapsulator ()
{
  NS_LOG_FUNCTION (this);
}

SatEncapsulator::~SatEncapsulator ()
{
  NS_LOG_FUNCTION (this);
  m_txQueue = NULL;
  m_rxCallback.Nullify ();
}

TypeId SatEncapsulator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatEncapsulator")
    .SetParent<Object> ()
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the MAC.",
                     MakeTraceSourceAccessor (&SatEncapsulator::m_txPdu))
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&SatEncapsulator::m_rxPdu))
    ;
  return tid;
}

void
SatEncapsulator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_txQueue)
    {
      m_txQueue->DoDispose ();
    }
  m_rxCallback.Nullify ();
}

void
SatEncapsulator::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

void
SatEncapsulator::SetQueue (Ptr<SatQueue> queue)
{
  m_txQueue = queue;
}

Ptr<SatQueue>
SatEncapsulator::GetQueue ()
{
  return m_txQueue;
}


} // namespace ns3
