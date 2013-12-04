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
 * Author: Jani Puttonen <sami.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mac48-address.h"

#include "satellite-llc.h"
#include "satellite-mac-tag.h"
#include "satellite-generic-encapsulator.h"
#include "satellite-scheduling-object.h"
#include "satellite-control-message.h"

NS_LOG_COMPONENT_DEFINE ("SatLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLlc);

TypeId
SatLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLlc")
    .SetParent<Object> ()
    .AddConstructor<SatLlc> ()
    .AddAttribute ("ControlTxQueue",
                   "A queue to use as the transmit queue for control packets in the device.",
                   PointerValue (),
                   MakePointerAccessor (&SatLlc::m_controlQueue),
                   MakePointerChecker<Queue> ())
    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("LlcTx",
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&SatLlc::m_llcTxTrace))
    .AddTraceSource ("LlcTxDrop",
                     "Trace source indicating a packet has been dropped by the device before transmission",
                     MakeTraceSourceAccessor (&SatLlc::m_llcTxDropTrace))
    .AddTraceSource ("LlcPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&SatLlc::m_llcPromiscRxTrace))
    .AddTraceSource ("LlcRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&SatLlc::m_llcRxTrace))
    #if 0
    // Not currently implemented for this device
    .AddTraceSource ("LlcRxDrop",
                     "Trace source indicating a packet was dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&SatLlc::m_llcRxDropTrace))
    #endif
  ;
  return tid;
}

SatLlc::SatLlc ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}

SatLlc::SatLlc (bool isUt)
:m_isUt (isUt)
{
  NS_LOG_FUNCTION (this);
}

SatLlc::~SatLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatLlc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rxCallback.Nullify ();

  if (m_controlQueue != 0)
    {
      m_controlQueue->DequeueAll();
    }
  m_controlQueue = 0;

  encapContainer_t::iterator it;

  for ( it = m_encaps.begin(); it != m_encaps.end(); ++it)
    {
      it->second = 0;
    }
  m_encaps.clear ();

  for ( it = m_decaps.begin(); it != m_decaps.end(); ++it)
    {
      it->second = 0;
    }
  m_decaps.clear ();

  Object::DoDispose ();
}

void
SatLlc::SetQueue (Ptr<Queue> queue)
{
  NS_LOG_FUNCTION (this << queue);
  m_controlQueue = queue;
}

Ptr<Queue>
SatLlc::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);
  return m_controlQueue;
}

bool
SatLlc::Enque (Ptr<Packet> packet, Address dest)
{
  NS_LOG_FUNCTION (this << packet << dest);
  NS_LOG_LOGIC ("p=" << packet );
  NS_LOG_LOGIC ("dest=" << dest );
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  m_llcTxTrace (packet);

  // Control PDUs use separate queue, since they do not need
  // encapsulation, fragmentation nor packing.
  if ( Mac48Address::ConvertFrom (dest).IsBroadcast () )
    {
      SatControlMsgTag cTag;
      bool cSuccess = packet->PeekPacketTag (cTag);
      if (cSuccess && cTag.GetMsgType() != SatControlMsgTag::SAT_NON_CTRL_MSG)
        {
          // Add MAC tag
          SatMacTag mTag;
          mTag.SetDestAddress (dest);
          mTag.SetSourceAddress (m_macAddress);
          packet->AddPacketTag (mTag);

          // Enque the control packet
          m_controlQueue->Enqueue (packet);
        }
      else
        {
          NS_FATAL_ERROR ("LLC does not support any other broadcast messages than control messages!");
        }
    }
  else
    {
      // UT: user own mac address
      // GW: use destination address
      Mac48Address mac = ( m_isUt ? m_macAddress : Mac48Address::ConvertFrom (dest) );
      encapContainer_t::iterator it = m_encaps.find (mac);

      if (it != m_encaps.end ())
        {
          it->second->TransmitPdu (packet);
        }
      else
        {
          NS_FATAL_ERROR ("Mac48Address not found in the encapsulator container!");
        }
    }

  return true;
}

Ptr<Packet>
SatLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr)
{
  NS_LOG_FUNCTION (this << macAddr << bytes);

  Ptr<Packet> packet;

  // Prioritize control packets
  if (m_controlQueue->GetNPackets())
      // && m_controlQueue->GetNBytes() <= bytes)
    {
      NS_ASSERT (macAddr.IsBroadcast());
      packet = m_controlQueue->Dequeue ();
    }
  // Forward the txOpportunity to a certain encapsulator
  else
    {
      encapContainer_t::iterator it = m_encaps.find (macAddr);

      if (it != m_encaps.end ())
        {
          packet = it->second->NotifyTxOpportunity (bytes);
        }
      else
        {
          NS_FATAL_ERROR ("Mac48Address not found in the encapsulator container!");
        }
    }
  return packet;
}

void
SatLlc::Receive (Ptr<Packet> packet, Mac48Address macAddr)
{
  NS_LOG_FUNCTION (this << macAddr << packet);

  // Receive packet with a decapsulator instance which is handling the
  // packets for this specific id
  encapContainer_t::iterator it = m_decaps.find (macAddr);

  // Note: control messages should not be seen at the LLC layer, since
  // they are received already at the MAC layer.
  if (it != m_decaps.end ())
    {
      it->second->ReceivePdu (packet);
    }
  else
    {
      NS_FATAL_ERROR ("Mac48Address not found in the decapsulator container!");
    }
}

void
SatLlc::ReceiveHigherLayerPdu (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_llcRxTrace (packet);

  // Call a callback to receive the packet at upper layer
  m_rxCallback (packet);
}

void
SatLlc::AddEncap (Mac48Address macAddr, Ptr<SatEncapsulator> enc)
{
  NS_LOG_FUNCTION (this);

  encapContainer_t::iterator it = m_encaps.find (macAddr);

  if (it == m_encaps.end ())
    {
      m_encaps.insert(std::make_pair (macAddr, enc));
    }
  else
    {
      NS_FATAL_ERROR ("Encapsulator container already contains this MAC address!");
    }
}

void
SatLlc::AddDecap (Mac48Address macAddr, Ptr<SatEncapsulator> dec)
{
  NS_LOG_FUNCTION (this);

  encapContainer_t::iterator it = m_decaps.find (macAddr);

  if (it == m_decaps.end ())
    {
      m_decaps.insert(std::make_pair (macAddr, dec));
    }
  else
    {
      NS_FATAL_ERROR ("Decapsulator container already contains this MAC address!");
    }
}

void SatLlc::SetAddress( Mac48Address macAddress )
{
  NS_LOG_FUNCTION (this << macAddress);
  m_macAddress = macAddress;
}

std::vector< Ptr<SatSchedulingObject> > SatLlc::GetSchedulingContexts () const
{
  std::vector< Ptr<SatSchedulingObject> > schedObjects;

  // First fill the control scheduling object
  if (m_controlQueue->GetNPackets ())
    {
      Ptr<SatSchedulingObject> so =
          Create<SatSchedulingObject> (Mac48Address::GetBroadcast (), m_controlQueue->GetNBytes (), true);
      schedObjects.push_back (so);
    }

  // Then the user data
  for (encapContainer_t::const_iterator cit = m_encaps.begin ();
      cit != m_encaps.end ();
      ++cit)
    {
      uint32_t buf = cit->second->GetTxBufferSizeInBytes ();

      if (buf > 0)
        {
          Ptr<SatSchedulingObject> so =
              Create<SatSchedulingObject> (cit->first, buf);
          schedObjects.push_back (so);
        }
    }
  return schedObjects;
}

void
SatLlc::SetReceiveCallback (SatLlc::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}


} // namespace ns3


