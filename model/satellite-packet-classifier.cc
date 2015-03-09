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
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "satellite-packet-classifier.h"
#include "satellite-enums.h"
#include "satellite-control-message.h"

NS_LOG_COMPONENT_DEFINE ("SatPacketClassifier");

namespace ns3 {


SatPacketClassifier::SatPacketClassifier ()
{

}


SatPacketClassifier::~SatPacketClassifier ()
{

}

uint8_t
SatPacketClassifier::Classify (SatControlMsgTag::SatControlMsgType_t type, const Address& dest) const
{
  NS_LOG_FUNCTION (this << dest);

  // ACKs are treated as user data, thus they are classified to be using the best effort
  // flow id. Note, that in FWD link, the ACK is sent in RTN link back to the GW, thus
  // the UT needs to have a lower layer service configured for BE RC index to be able to
  // transmit the ACKs.
  if (type == SatControlMsgTag::SAT_ARQ_ACK)
    {
      return SatEnums::BE_FID;
    }

  // By default the control messages are classified to control fid
  return SatEnums::CONTROL_FID;
}


uint8_t
SatPacketClassifier::Classify (const Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) const
{
  NS_LOG_FUNCTION (this << packet->GetUid () << dest << protocolNumber);

  Ipv4Header ipv4Header;
  packet->PeekHeader (ipv4Header);

  uint8_t dscp = ipv4Header.GetDscp ();

  switch (dscp)
    {
    case Ipv4Header::DscpDefault:
      {
        return SatEnums::BE_FID;
        break;
      }
    case Ipv4Header::DSCP_AF11:
    case Ipv4Header::DSCP_AF12:
    case Ipv4Header::DSCP_AF13:
    case Ipv4Header::DSCP_AF21:
    case Ipv4Header::DSCP_AF22:
    case Ipv4Header::DSCP_AF23:
    case Ipv4Header::DSCP_AF31:
    case Ipv4Header::DSCP_AF32:
    case Ipv4Header::DSCP_AF33:
    case Ipv4Header::DSCP_AF41:
    case Ipv4Header::DSCP_AF42:
    case Ipv4Header::DSCP_AF43:
    case Ipv4Header::DSCP_CS1:
    case Ipv4Header::DSCP_CS2:
    case Ipv4Header::DSCP_CS3:
    case Ipv4Header::DSCP_CS4:
      {
        return SatEnums::AF_FID;
        break;
      }
    case Ipv4Header::DSCP_EF:
    case Ipv4Header::DSCP_CS5:
    case Ipv4Header::DSCP_CS6:
    case Ipv4Header::DSCP_CS7:
      {
        return SatEnums::EF_FID;
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported DSCP field value: " << dscp);
        break;
      }
    }

  return SatEnums::BE_FID;
}


} // namespace ns3



