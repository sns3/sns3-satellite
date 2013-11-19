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

#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/random-variable.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "satellite-ut-mac.h"

NS_LOG_COMPONENT_DEFINE ("SatUtMac");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatUtMac);

TypeId 
SatUtMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatUtMac> ()
    .AddAttribute ("SuperframeSequence", "Superframe sequence containing information of superframes.",
                    PointerValue (),
                    MakePointerAccessor (&SatUtMac::m_superframeSeq),
                    MakePointerChecker<SatSuperframeSeq> ())
    .AddAttribute ("Interval",
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (Seconds (0.001)),
                   MakeTimeAccessor (&SatUtMac::m_tInterval),
                   MakeTimeChecker ())
    .AddAttribute ("Cra",
                   "Constant Rate Assignment value for this UT Mac.",
                   DoubleValue (128),
                   MakeDoubleAccessor (&SatUtMac::m_cra),
                   MakeDoubleChecker<double> (0.0))
  ;

  return tid;
}

TypeId
SatUtMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatUtMac::SatUtMac ()
{
  NS_LOG_FUNCTION (this);
  
  // default constructor should not be used
  NS_ASSERT (false);
}

SatUtMac::SatUtMac (Ptr<SatSuperframeSeq> seq)
 : m_superframeSeq (seq)
{
	NS_LOG_FUNCTION (this);
}

SatUtMac::~SatUtMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatUtMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  SatMac::DoDispose ();
}

void
SatUtMac::StartScheduling()
{
  if ( m_tInterval.GetDouble() > 0 )
    {
      ScheduleTransmit (m_tInterval, 0);
    }
}

void
SatUtMac::ScheduleTransmit(Time transmitTime, uint32_t carrierId)
{
  Simulator::Schedule (transmitTime, &SatUtMac::TransmitReady, this, carrierId);
}

void
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_timingAdvanceCb = cb;
}

void
SatUtMac::ScheduleTimeSlots (SatTbtpHeader * tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);

  std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > > slots = tbtp->GetTimeslots (m_macAddress);

  if ( !slots.empty ())
    {
      double superframeDuration = m_superframeSeq->GetDuration_s (tbtp->GetSuperframeId ());

      // TODO: start time must be calculated using reference or global clock
      Time startTime = Seconds (superframeDuration * tbtp->GetSuperframeCounter ());

      // schedule time slots
      for ( std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > >::iterator it = slots.begin (); it != slots.end (); it++ )
        {
          Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
          Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf ((*it)->GetFrameId ());
          Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( (*it)->GetTimeSlotId () );

          Time slotStartTime = startTime + Seconds (timeSlotConf->GetStartTime_s ());
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, (*it)->GetFrameId (), timeSlotConf->GetCarrierId () );

          ScheduleTransmit (slotStartTime, carrierId);
        }
    }
}

bool
SatUtMac::TransmitStart (Ptr<Packet> p, uint32_t carrierId)
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
SatUtMac::TransmitReady (uint32_t carrierId)
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
      Simulator::Schedule (m_tInterval, &SatUtMac::TransmitReady, this, 0);
    }
}


void
SatUtMac::Receive (Ptr<Packet> packet, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << packet);

  bool deliverUp = true;

  SatMacTag macTag;
  packet->PeekPacketTag (macTag);

  SatControlMsgTag ctrlTag;
  packet->RemovePacketTag (ctrlTag);

  SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType ();
  if ( cType != SatControlMsgTag::SAT_NON_CTRL_MSG )
    {
      ReceiveSignalingPacket (packet, cType);
      deliverUp = false;
    }

  // deliver packet to parent SatMac, if not UT specific packet
  if (deliverUp)
    {
      SatMac::Receive (packet, rxParams);
    }
}

void
SatUtMac::ReceiveSignalingPacket (Ptr<Packet> packet, SatControlMsgTag::SatControlMsgType_t cType)
{
  switch (cType)
  {
    case SatControlMsgTag::SAT_TBTP_CTRL_MSG:
      {
        SatTbtpHeader tbtp;
        if ( packet->RemoveHeader (tbtp) > 0 )
          {
            ScheduleTimeSlots (&tbtp);
          }
        break;
      }
    case SatControlMsgTag::SAT_RA_CTRL_MSG:
    case SatControlMsgTag::SAT_CR_CTRL_MSG:
      {
        NS_FATAL_ERROR ("SatUtMac received a non-supported control packet!");
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatUtMac received a non-supported control packet!");
        break;
      }
  }
}

} // namespace ns3
