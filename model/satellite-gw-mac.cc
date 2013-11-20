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
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"

#include "satellite-mac-tag.h"
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
  NS_ASSERT (m_tInterval.GetDouble() > 0.0);

  // Note, carrierId currently set by default to 0
  ScheduleNextTransmissionTime (m_tInterval, 0);
}

void
SatGwMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  SatMac::DoDispose ();
}

void
SatGwMac::ScheduleNextTransmissionTime (Time txTime, uint32_t carrierId)
{
  Simulator::Schedule (txTime, &SatGwMac::TransmitTime, this, 0);
}

void
SatGwMac::TransmitTime (uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);

  // TODO: In forward link txBytes should be either
  // - Short BBFrame = 16200 bits = 2025 Bytes
  // - Long BBFrame = 64800 bits = 8100 Bytes
  // This should be decided on-the-fly based on buffered bytes
  // in LLC layer.
  uint32_t txBytes (8100);
  Ptr<Packet> p = m_txOpportunityCallback (txBytes);

  if ( p )
    {
      /* TODO: The carrierId should be acquired from somewhere. Now
       * we assume only one carrier in forward link, so it is safe to use 0.
       * The duration should be calculated based on BBFrame length and
       * used MODCOD.
       */
      Time DURATION (MicroSeconds(20));
      SendPacket (p, carrierId, DURATION);
    }

  // TODO: The next TransmitTime should be scheduled to be when just transmitted
  // packet transmission ends. This is dependent on the used BBFrame length and
  // used MODCOD.
  ScheduleNextTransmissionTime (m_tInterval, 0);
}

} // namespace ns3
