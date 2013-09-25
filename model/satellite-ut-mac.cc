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

#include "satellite-control-message.h"
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
}

SatUtMac::~SatUtMac ()
{
  NS_LOG_FUNCTION (this);
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

          // TODO: now just put UT in line with random delay
          // start time to be read from frame configurations
          UniformVariable randomDelay(0.001, 0.005);
          double startTime = 0.01 * (double) slots[0]->GetTimeSlotId();
          startTime += randomDelay.GetValue();

          if ( !slots.empty() )
            {
              ScheduleTransmit ( Time::FromDouble (startTime, Time::S));
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
