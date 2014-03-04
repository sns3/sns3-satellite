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
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "satellite-mac.h"
#include "satellite-mac-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMac);

TypeId 
SatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMac")
    .SetParent<Object> ()
    .AddConstructor<SatMac> ()
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatMac::m_packetTrace))
  ;
  return tid;
}

SatMac::SatMac ()
 : m_nodeInfo (),
   m_beamId (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false); // this version of the constructor should not been used
}

SatMac::SatMac (uint32_t beamId)
 : m_nodeInfo (),
   m_beamId (beamId)
{
  NS_LOG_FUNCTION (this);
}

SatMac::~SatMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_txCallback.Nullify ();
  m_rxCallback.Nullify ();
  m_readCtrlCallback.Nullify ();
  m_writeCtrlCallback.Nullify ();

  Object::DoDispose ();
}


void
SatMac::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  m_nodeInfo = nodeInfo;
}

uint32_t
SatMac::WriteCtrlMsgToContainer (Ptr<SatControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  uint32_t id = 0;

  if ( m_writeCtrlCallback.IsNull () == false )
    {
      id = m_writeCtrlCallback (msg);
    }

  return id;
}

void
SatMac::ReceiveQueueEvent (SatQueue::QueueEvent_t /*event*/, uint8_t /*flowIndex*/)
{
  NS_LOG_FUNCTION (this);
}

void
SatMac::SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration)
{
  NS_LOG_FUNCTION (this);

  // Use call back to send packet to lower layer
  m_txCallback (packets, carrierId, duration);
}

void
SatMac::SetTransmitCallback (SatMac::TransmitCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_txCallback = cb;
}

void
SatMac::SetReceiveCallback (SatMac::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

void
SatMac::SetReadCtrlCallback (SatMac::ReadCtrlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_readCtrlCallback = cb;
}

void
SatMac::SetWriteCtrlCallback (SatMac::WriteCtrlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_writeCtrlCallback = cb;
}

} // namespace ns3
