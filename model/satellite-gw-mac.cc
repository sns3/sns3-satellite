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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"

#include "satellite-mac-tag.h"
#include "satellite-phy.h"
#include "satellite-net-device.h"
#include "satellite-signal-parameters.h"
#include "satellite-gw-mac.h"


NS_LOG_COMPONENT_DEFINE ("SatGwMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwMac);

TypeId
SatGwMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGwMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatGwMac> ()
    .AddAttribute ("Interval",
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (Seconds (0.001)),
                   MakeTimeAccessor (&SatGwMac::m_tInterval),
                   MakeTimeChecker ())
  ;
  return tid;
}

SatGwMac::SatGwMac ()
{
  NS_LOG_FUNCTION (this);
}

SatGwMac::~SatGwMac ()
{
  NS_LOG_FUNCTION (this);
}

void SatGwMac::StartScheduling()
{
  if ( m_tInterval.GetDouble() > 0 )
    {
      ScheduleTransmit (m_tInterval, 0);
    }
}

void SatGwMac::ScheduleTransmit(Time transmitTime, uint32_t carrierId)
{
  Simulator::Schedule (transmitTime, &SatGwMac::TransmitReady, this, carrierId);
}

void
SatGwMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  SatMac::DoDispose ();
}

bool
SatGwMac::TransmitStart (Ptr<Packet> p, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC (this << " transmit packet UID " << p->GetUid ());

  /* TODO: Now we are using only a static (time slot) duration
   * for packet transmissions and receptions.
   * The (time slot) durations for packet transmissions should be coming from:
   * - TBTP in return link
   * - GW scheduler in the forward link
   */
  Time DURATION (MicroSeconds(20));
  m_phy->SendPdu (p, carrierId, DURATION);

  return true;
}

void
SatGwMac::TransmitReady (uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);
  //
  // This function is called to when we're all done transmitting a packet.
  // We try and pull another packet off of the transmit queue.  If the queue
  // is empty, we are done, otherwise we need to start transmitting the
  // next packet.

  if ( PacketInQueue() )
    {
      Ptr<Packet> p = m_queue->Dequeue();
      TransmitStart(p, carrierId);
    }

  if ( m_tInterval.GetDouble() > 0)
    {
      Simulator::Schedule (m_tInterval, &SatGwMac::TransmitReady, this, 0);
    }
}

} // namespace ns3
