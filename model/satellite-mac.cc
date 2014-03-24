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

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/packet.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/uinteger.h>
#include <ns3/nstime.h>
#include <ns3/pointer.h>
#include <ns3/satellite-mac-tag.h>
#include <ns3/satellite-address-tag.h>
#include <ns3/satellite-time-tag.h>
#include "satellite-mac.h"

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
    .AddTraceSource ("Rx",
                     "A packet received",
                     MakeTraceSourceAccessor (&SatMac::m_rxTrace))
    .AddTraceSource ("RxDelay",
                     "A packet is received with delay information",
                     MakeTraceSourceAccessor (&SatMac::m_rxDelayTrace))
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
SatMac::SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this);

  // Add a SatMacTimeTag tag for packet delay computation at the receiver end.
  for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
       it != packets.end (); ++it)
    {
      (*it)->AddPacketTag (SatMacTimeTag (Simulator::Now ()));
    }

  // Use call back to send packet to lower layer
  m_txCallback (packets, carrierId, duration, txInfo);
}

void
SatMac::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  for (SatPhy::PacketContainer_t::const_iterator it1 = packets.begin ();
       it1 != packets.end (); ++it1)
    {
      Address addr; // invalid address.
      bool isTaggedWithAddress = false;
      ByteTagIterator it2 = (*it1)->GetByteTagIterator ();

      while (!isTaggedWithAddress && it2.HasNext ())
        {
          ByteTagIterator::Item item = it2.Next ();

          if (item.GetTypeId () == SatAddressTag::GetTypeId ())
            {
              NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                 << " start=" << item.GetStart ()
                                 << " end=" << item.GetEnd ());
              SatAddressTag addrTag;
              item.GetTag (addrTag);
              addr = addrTag.GetSourceAddress ();
              isTaggedWithAddress = true; // this will exit the while loop.
            }
        }

      m_rxTrace (*it1, addr);

      SatMacTimeTag timeTag;
      if ((*it1)->RemovePacketTag (timeTag))
        {
          NS_LOG_DEBUG (this << " contains a SatMacTimeTag tag");
          m_rxDelayTrace (Simulator::Now () - timeTag.GetSenderTimestamp (),
                          addr);
        }

    } // end of `for it1 = packets.begin () -> packets.end ()`

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
