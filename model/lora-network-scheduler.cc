/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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

#include "lora-network-scheduler.h"

NS_LOG_COMPONENT_DEFINE ("LoraNetworkScheduler");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoraNetworkScheduler);

TypeId
LoraNetworkScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraNetworkScheduler")
    .SetParent<Object> ()
    .AddAttribute ("FirstWindowAnswerDelay", "Delay to wait between end of reception of paquet and sending of anwser, to be in first window opportunity",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&LoraNetworkScheduler::m_firstWindowAnswerDelay),
                   MakeTimeChecker ())
    .AddAttribute ("SecondWindowAnswerDelay", "Delay to wait between end of reception of paquet and sending of anwser, to be in second window opportunity",
                   TimeValue (Seconds (2)),
                   MakeTimeAccessor (&LoraNetworkScheduler::m_secondWindowAnswerDelay),
                   MakeTimeChecker ())
    .AddTraceSource ("ReceiveWindowOpened",
                     "Trace source that is fired when a receive window opportunity happens.",
                     MakeTraceSourceAccessor (&LoraNetworkScheduler::m_receiveWindowOpened),
                     "ns3::Packet::TracedCallback")
    .AddConstructor<LoraNetworkScheduler> ()
    ;
  return tid;
}

TypeId
LoraNetworkScheduler::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

LoraNetworkScheduler::LoraNetworkScheduler ()
{
}

LoraNetworkScheduler::LoraNetworkScheduler (Ptr<LoraNetworkStatus> status,
                                            Ptr<LoraNetworkController> controller) :
  m_status (status),
  m_controller (controller),
  m_firstWindowAnswerDelay (Seconds (1)),
  m_secondWindowAnswerDelay (Seconds (2))
{
  ObjectBase::ConstructSelf (AttributeConstructionList ());
}

LoraNetworkScheduler::~LoraNetworkScheduler ()
{
}

void
LoraNetworkScheduler::OnReceivedPacket (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (packet);

  // Get the current packet's frame counter
  Ptr<Packet> packetCopy = packet->Copy ();
  LorawanMacHeader receivedMacHdr;
  packetCopy->RemoveHeader (receivedMacHdr);
  LoraFrameHeader receivedFrameHdr;
  receivedFrameHdr.SetAsUplink ();
  packetCopy->RemoveHeader (receivedFrameHdr);

  // Need to decide whether to schedule a receive window
  if (!m_status->GetEndDeviceStatus (packet)->HasReceiveWindowOpportunityScheduled ())
  {
    // Extract the address
    LoraDeviceAddress deviceAddress = receivedFrameHdr.GetAddress ();

    // Schedule OnReceiveWindowOpportunity event
    m_status->GetEndDeviceStatus (packet)->SetReceiveWindowOpportunity (
      Simulator::Schedule (m_firstWindowAnswerDelay, &LoraNetworkScheduler::OnReceiveWindowOpportunity, this, deviceAddress, 1));
  }
}

void
LoraNetworkScheduler::OnReceiveWindowOpportunity (LoraDeviceAddress deviceAddress, int window)
{
  NS_LOG_FUNCTION (deviceAddress);

  NS_LOG_DEBUG ("Opening receive window number " << window << " for device "
                                                 << deviceAddress);

  // Check whether we can send a reply to the device, again by using
  // NetworkStatus
  Address gwAddress = m_status->GetBestGatewayForDevice (deviceAddress, window);

  if (gwAddress == Address () && window == 1)
    {
      NS_LOG_DEBUG ("No suitable gateway found for first window.");

      std::cout << "gwAddress " << gwAddress << ", Address () " << Address () << std::endl;

      // No suitable GW was found, but there's still hope to find one for the
      // second window.
      // Schedule another OnReceiveWindowOpportunity event
      m_status->GetEndDeviceStatus (deviceAddress)->SetReceiveWindowOpportunity (
      Simulator::Schedule (m_secondWindowAnswerDelay,&LoraNetworkScheduler::OnReceiveWindowOpportunity, this, deviceAddress, 2));
    }
  else if (gwAddress == Address () && window == 2)
    {
      // No suitable GW was found and this was our last opportunity
      // Simply give up.
      NS_LOG_DEBUG ("Giving up on reply: no suitable gateway was found " <<
                   "on the second receive window");

      // Reset the reply
      // XXX Should we reset it here or keep it for the next opportunity?
      m_status->GetEndDeviceStatus (deviceAddress)->RemoveReceiveWindowOpportunity();
      m_status->GetEndDeviceStatus (deviceAddress)->InitializeReply ();
    }
  else
    {
      // A gateway was found

      std::cout << "gwAddress " << gwAddress << ", Address () " << Address () << std::endl;

      NS_LOG_DEBUG ("Found available gateway with address: " << gwAddress);

      m_controller->BeforeSendingReply (m_status->GetEndDeviceStatus (deviceAddress));

      // Check whether this device needs a response by querying m_status
      bool needsReply = m_status->NeedsReply (deviceAddress);

      if (needsReply)
        {
          NS_LOG_INFO ("A reply is needed");

          // Send the reply through that gateway
          m_status->SendThroughGateway (m_status->GetReplyForDevice (deviceAddress, window), gwAddress);

          // Reset the reply
          m_status->GetEndDeviceStatus (deviceAddress)->RemoveReceiveWindowOpportunity();
          m_status->GetEndDeviceStatus (deviceAddress)->InitializeReply ();
        }
    }
}
}
