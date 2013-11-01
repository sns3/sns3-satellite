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
                    PointerValue(),
                    MakePointerAccessor(&SatUtMac::m_superframeSeq),
                    MakePointerChecker<SatSuperframeSeq> ())
  ;
  return tid;
}

TypeId
SatUtMac::GetInstanceTypeId (void) const
{
  return GetTypeId();
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
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_timingAdvanceCb = cb;
}

void
SatUtMac::ScheduleTimeSlots(std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > > slots)
{
  for ( std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > >::iterator it = slots.begin ();
      it != slots.end (); it++ )
    {
      Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
      Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf ((*it)->GetFrameId ());

      Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( (*it)->GetTimeSlotId() );

      uint32_t carrierId = m_superframeSeq->GetCarrierId(0, (*it)->GetFrameId (), timeSlotConf->GetCarrierId() );

      ScheduleTransmit ( Seconds(timeSlotConf->GetStartTime_s()), carrierId);
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

  // If the packet is intended for this receiver
  Mac48Address addr = Mac48Address::ConvertFrom (macTag.GetAddress());

  if ( addr.IsBroadcast() && ctrlTag.GetMsgType() == SatControlMsgTag::SAT_TBTP_CTRL_MSG )
    {
      SatTbtpHeader tbtp;

      if ( packet->RemoveHeader(tbtp) > 0 )
        {
          std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > > slots = tbtp.GetTimeslots (m_macAddress);

          deliverUp = false;

          // TODO: start time must be calculated using reference or global clock
          double superframeDuration = m_superframeSeq->GetDuration_s (tbtp.GetSuperframeId ());

          double startTime = superframeDuration * tbtp.GetSuperframeCounter();

          if ( !slots.empty() )
            {
              Simulator::Schedule (Seconds (startTime), &SatUtMac::ScheduleTimeSlots, this, slots);
            }
        }
    }

  // deliver packet to parent SatMac, if not UT specific packet
  if (deliverUp)
    {
      SatMac::Receive(packet, rxParams);
    }
}

} // namespace ns3
