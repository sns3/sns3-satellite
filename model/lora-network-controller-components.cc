/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "ns3/lora-network-controller-components.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraNetworkControllerComponent");

NS_OBJECT_ENSURE_REGISTERED (LoraNetworkControllerComponent);

TypeId
LoraNetworkControllerComponent::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraNetworkControllerComponent")
    .SetParent<Object> ()
  ;
  return tid;
}

// Constructor and destructor
LoraNetworkControllerComponent::LoraNetworkControllerComponent ()
{
}
LoraNetworkControllerComponent::~LoraNetworkControllerComponent ()
{
}

////////////////////////////////
// LoraConfirmedMessagesComponent //
////////////////////////////////
TypeId
LoraConfirmedMessagesComponent::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraConfirmedMessagesComponent")
    .SetParent<LoraNetworkControllerComponent> ()
    .AddConstructor<LoraConfirmedMessagesComponent> ();
  return tid;
}

LoraConfirmedMessagesComponent::LoraConfirmedMessagesComponent ()
{
}
LoraConfirmedMessagesComponent::~LoraConfirmedMessagesComponent ()
{
}

void
LoraConfirmedMessagesComponent::OnReceivedPacket (Ptr<const Packet> packet,
                                                  Ptr<LoraEndDeviceStatus> status,
                                                  Ptr<LoraNetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this->GetTypeId () << packet << networkStatus);

  // Check whether the received packet requires an acknowledgment.
  LorawanMacHeader mHdr;
  LoraFrameHeader fHdr;
  fHdr.SetAsUplink ();
  Ptr<Packet> myPacket = packet->Copy ();
  myPacket->RemoveHeader (mHdr);
  myPacket->RemoveHeader (fHdr);

  NS_LOG_INFO ("Received packet Mac Header: " << mHdr);
  NS_LOG_INFO ("Received packet Frame Header: " << fHdr);

  if (mHdr.GetMType () == LorawanMacHeader::CONFIRMED_DATA_UP)
    {
      NS_LOG_INFO ("Packet requires confirmation");

      // Set up the ACK bit on the reply
      status->m_reply.frameHeader.SetAsDownlink ();
      status->m_reply.frameHeader.SetAck (true);
      status->m_reply.frameHeader.SetAddress (fHdr.GetAddress ());
      status->m_reply.macHeader.SetMType (LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
      status->m_reply.needsReply = true;

      // Note that the acknowledgment procedure dies here: "Acknowledgments
      // are only snt in response to the latest message received and are never
      // retransmitted". We interpret this to mean that only the current
      // reception window can be used, and that the Ack field should be
      // emptied in case transmission cannot be performed in the current
      // window. Because of this, in this component's OnFailedReply method we
      // void the ack bits.
    }
}

void
LoraConfirmedMessagesComponent::BeforeSendingReply (Ptr<LoraEndDeviceStatus> status,
                                                Ptr<LoraNetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this << status << networkStatus);
  // Nothing to do in this case
}

void
LoraConfirmedMessagesComponent::OnFailedReply (Ptr<LoraEndDeviceStatus> status,
                                           Ptr<LoraNetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this << networkStatus);

  // Empty the Ack bit.
  status->m_reply.frameHeader.SetAck (false);
}

////////////////////////
// LoraLinkCheckComponent //
////////////////////////
TypeId
LoraLinkCheckComponent::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraLinkCheckComponent")
    .SetParent<LoraNetworkControllerComponent> ()
    .AddConstructor<LoraLinkCheckComponent> ();
  return tid;
}

LoraLinkCheckComponent::LoraLinkCheckComponent ()
{
}
LoraLinkCheckComponent::~LoraLinkCheckComponent ()
{
}

void
LoraLinkCheckComponent::OnReceivedPacket (Ptr<const Packet> packet,
                                      Ptr<LoraEndDeviceStatus> status,
                                      Ptr<LoraNetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this->GetTypeId () << packet << networkStatus);

  // We will only act just before reply, when all Gateways will have received
  // the packet.
}

void
LoraLinkCheckComponent::BeforeSendingReply (Ptr<LoraEndDeviceStatus> status,
                                        Ptr<LoraNetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this << status << networkStatus);

  Ptr<Packet> myPacket = status->GetLastPacketReceivedFromDevice ()->Copy ();
  LorawanMacHeader mHdr;
  LoraFrameHeader fHdr;
  fHdr.SetAsUplink ();
  myPacket->RemoveHeader (mHdr);
  myPacket->RemoveHeader (fHdr);

  Ptr<LinkCheckReq> command = fHdr.GetLorawanMacCommand<LinkCheckReq> ();

  // GetMacCommand returns 0 if no command is found
  if (command)
    {
      status->m_reply.needsReply = true;

      // Get the number of gateways that received the packet and the best
      // margin
      uint8_t gwCount = status->GetLastReceivedPacketInfo ().gwList.size ();

      Ptr<LinkCheckAns> replyCommand = Create<LinkCheckAns> ();
      replyCommand->SetGwCnt (gwCount);
      status->m_reply.frameHeader.SetAsDownlink ();
      status->m_reply.frameHeader.AddCommand (replyCommand);
      status->m_reply.macHeader.SetMType (LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
    }
  else
    {
      // Do nothing
    }
}

void
LoraLinkCheckComponent::OnFailedReply (Ptr<LoraEndDeviceStatus> status,
                                   Ptr<LoraNetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this->GetTypeId () << networkStatus);
}
}
