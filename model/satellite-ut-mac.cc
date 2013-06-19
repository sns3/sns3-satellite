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
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"

#include "satellite-control-header.h"
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
SatUtMac::TransmitReady (void)
{
  NS_LOG_FUNCTION (this);

  if ( SatMac::PacketInQueue() )
    {
       Ptr<Packet> p = SatMac::GetPacketFromQueue();
       TransmitStart(p);
    }
}

void
SatUtMac::Receive (Ptr<Packet> packet, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << packet);

  bool deliverUp = true;

  if ( SatMac::PacketHasError(packet) == false )
    {
      MacAddressTag tag;

      // Fetch the packet tag
      if (packet->PeekPacketTag (tag))
        {
          // If the packet is intended for this receiver
          Mac48Address addr = Mac48Address::ConvertFrom (tag.GetAddress());

          if ( addr.IsBroadcast() )
            {
              SatCtrlHeader header;

              packet->RemoveHeader(header);

              if (header.GetMsgType() == SatCtrlHeader::TBTP_MSG)
                {
                  packet->RemovePacketTag (tag);
                  Simulator::Schedule (GetInterval(), &SatUtMac::TransmitReady, this);
                  deliverUp = false;
                }
            }

          if (deliverUp)
            {
              SatMac::Receive(packet,rxParams);
            }
        }
     }
}

} // namespace ns3
