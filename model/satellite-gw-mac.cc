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
#include "satellite-scheduling-object.h"


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
                   TimeValue (Seconds (0.002)),
                   MakeTimeAccessor (&SatGwMac::m_tInterval),
                   MakeTimeChecker ())
  ;
  return tid;
}

SatGwMac::SatGwMac ()
{
  NS_LOG_FUNCTION (this);

  // Random variable used in scheduling
  m_random = CreateObject<UniformRandomVariable> ();
}

SatGwMac::~SatGwMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGwMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_schedContextCallback.Nullify ();
  SatMac::DoDispose ();
}

void
SatGwMac::StartScheduling()
{
  NS_ASSERT (m_tInterval.GetDouble() > 0.0);

  // Note, carrierId currently set by default to 0
  ScheduleNextTransmissionTime (m_tInterval, 0);
}

void
SatGwMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this);

  for (SatPhy::PacketContainer_t::iterator i = packets.begin(); i != packets.end(); i++ )
    {
      // Hit the trace hooks.  All of these hooks are in the same place in this
      // device because it is so simple, but this is not usually the case in
      // more complicated devices.
      m_snifferTrace (*i);
      m_promiscSnifferTrace (*i);
      m_macRxTrace (*i);

      // Remove packet tag
      SatMacTag macTag;
      bool mSuccess = (*i)->PeekPacketTag (macTag);
      if (!mSuccess)
        {
          NS_FATAL_ERROR ("MAC tag was not found from the packet!");
        }

      NS_LOG_LOGIC("Packet from " << macTag.GetSourceAddress() << " to " << macTag.GetDestAddress());
      NS_LOG_LOGIC("Receiver " << m_macAddress );

      // If the packet is intended for this receiver
      Mac48Address destAddress = Mac48Address::ConvertFrom (macTag.GetDestAddress());
      if (destAddress == m_macAddress || destAddress.IsBroadcast())
        {
          // Pass the source address to LLC
          m_rxCallback (*i, Mac48Address::ConvertFrom(macTag.GetSourceAddress ()));
        }
      else
        {
          NS_LOG_LOGIC("Packet intended for others received by MAC: " << m_macAddress );
        }
    }
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

  /**
   * TODO: This is a first skeleton implementation of the FWD link scheduler.
   * It is sending only one packet from each UT at a time, which a predefined
   * interval. In reality, the FWD link scheduler should be building BBFrames,
   * with possible GSE packets to several UTs.
   * In forward link txBytes should be either
   * - Short BBFrame = 16200 bits = 2025 Bytes
   * - Long BBFrame = 64800 bits = 8100 Bytes
   * The usage of either short or long BBFrame should be decided on-the-fly
   * based on buffered bytes in LLC layer.
   *
  */

  // Default size for Tx opportunity
  uint32_t txBytes (120);
  // Get scheduling objects from LLC
  std::vector< Ptr<SatSchedulingObject> > sos = m_schedContextCallback ();

  if ( !sos.empty ())
    {
      uint32_t ind;

      // Prioritize control
      if (sos.front()->IsControl () || sos.size() == 1)
        {
          ind = 0;
        }
      // Randomize users
      else
        {
          ind = (uint32_t)(m_random->GetInteger (0, sos.size()-1));
        }

      /**
       * Notify LLC of the Tx opportunity; returns a packet.
       * In addition, the function returns the bytes left after txOpportunity in
       * bytesLeft reference variable.
       */

      uint32_t bytesLeft (0);
      Ptr<Packet> p = m_txOpportunityCallback (txBytes, sos[ind]->GetMacAddress (), bytesLeft);

      if ( p )
        {
          /* TODO: The carrierId should be acquired from somewhere. Now
           * we assume only one carrier in forward link, so it is safe to use 0.
           * The BBFrame duration should be calculated based on BBFrame length and
           * used MODCOD.
           */
          Time DURATION (Seconds(0.001));
          SendPacket (p, carrierId, DURATION);
        }
    }

  // TODO: The next TransmitTime should be scheduled to be when just transmitted
  // packet transmission ends. This is dependent on the used BBFrame length and
  // used MODCOD.
  ScheduleNextTransmissionTime (m_tInterval, 0);
}

void
SatGwMac::SetSchedContextCallback (SatGwMac::SchedContextCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_schedContextCallback = cb;
}


} // namespace ns3
