/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "satellite-mac.h"
#include "satellite-phy.h"
#include "satellite-net-device.h"
#include "satellite-channel.h"

NS_LOG_COMPONENT_DEFINE ("SatMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMac);

TypeId 
SatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMac")
    .SetParent<Object> ()
    .AddConstructor<SatMac> ()
    .AddAttribute ("ReceiveErrorModel", 
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&SatMac::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("Interval",
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (Seconds (0.001)),
                   MakeTimeAccessor (&SatMac::m_tInterval),
                   MakeTimeChecker ())

    //
    // Transmit queueing discipline for the device which includes its own set
    // of trace hooks.
    //
    .AddAttribute ("TxQueue", 
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&SatMac::m_queue),
                   MakePointerChecker<Queue> ())

    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("MacTx", 
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&SatMac::m_macTxTrace))
    .AddTraceSource ("MacTxDrop", 
                     "Trace source indicating a packet has been dropped by the device before transmission",
                     MakeTraceSourceAccessor (&SatMac::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&SatMac::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&SatMac::m_macRxTrace))
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("MacRxDrop", 
                     "Trace source indicating a packet was dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&SatMac::m_macRxDropTrace))
#endif

  ;
  return tid;
}

SatMac::SatMac ()
  :
    m_txMachineState (READY),
    m_phy(0),
    m_device(0),
    m_currentPkt (0)
{
  NS_LOG_FUNCTION (this);

  Simulator::Schedule (Seconds(0.01), &SatMac::TransmitReady, this);
}

SatMac::~SatMac ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
SatMac::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_receiveErrorModel = 0;
  m_currentPkt = 0;
  m_phy = 0;
  m_device = 0;
  Object::DoDispose ();
}

bool
SatMac::TransmitStart (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  //
  // This function is called to start the process of transmitting a packet.
  // We need to tell the channel that we've started wiggling the wire and
  // schedule an event that will be executed when the transmission is complete.
  //
  //NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
  //m_txMachineState = BUSY;
  //m_currentPkt = p;

  m_phy->SendPdu(p, Seconds(0));

  return true;
}

void
SatMac::TransmitReady (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  //
  // This function is called to when we're all done transmitting a packet.
  // We try and pull another packet off of the transmit queue.  If the queue
  // is empty, we are done, otherwise we need to start transmitting the
  // next packet.
  //
  //NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
  //m_txMachineState = READY;

  //NS_ASSERT_MSG (m_currentPkt != 0, "SatMac::TransmitReady(): m_currentPkt zero");
  //m_currentPkt = 0;

  if ( m_tInterval.GetDouble() > 0)
    {
      if ( m_queue->GetNPackets() != 0 )
          {
            Ptr<Packet> p = m_queue->Dequeue();
            TransmitStart(p);
          }

      Simulator::Schedule (m_tInterval, &SatMac::TransmitReady, this);
    }
}

bool
SatMac::SetPhy (Ptr<SatPhy> phy)
{
  NS_LOG_FUNCTION (this << &phy);
  m_phy = phy;
  return true;
}

Ptr<SatPhy>
SatMac::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

bool
SatMac::SetDevice (Ptr<SatNetDevice> dev)
{
  NS_LOG_FUNCTION (this << &dev);
  m_device = dev;
  return true;
}

Ptr<SatNetDevice>
SatMac::GetDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_device;
}

void
SatMac::SetQueue (Ptr<Queue> q)
{
  NS_LOG_FUNCTION (this << q);
  m_queue = q;
}

Ptr<Queue>
SatMac::GetQueue (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_queue;
}

void
SatMac::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) ) 
    {
      // 
      // If we have an error model and it indicates that it is time to lose a
      // corrupted packet, don't forward this packet up, let it go.
      //
      //m_phyRxDropTrace (packet);
    }
  else 
    {
      // 
      // Hit the trace hooks.  All of these hooks are in the same place in this 
      // device becuase it is so simple, but this is not usually the case in 
      // more complicated devices.
      //
      m_snifferTrace (packet);
      m_promiscSnifferTrace (packet);
      //m_phyRxEndTrace (packet);

      m_macRxTrace (packet);
      m_device->Receive(packet);
    }
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

bool
SatMac::Send ( Ptr<Packet> packet )
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("p=" << packet);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  m_macTxTrace (packet);

  if (m_queue->Enqueue (packet) == false)
    {
       // Enqueue may fail (overflow)
      m_macTxDropTrace (packet);
      return false;
    }
  else
    {
      return true;
    }

  return true;
}


void
SatMac::SetReceiveCallback (SatMac::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

} // namespace ns3
