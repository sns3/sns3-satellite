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

#ifndef LORA_NETWORK_SCHEDULER_H
#define LORA_NETWORK_SCHEDULER_H

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/core-module.h"

#include "ns3/lora-device-address.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-network-controller.h"
#include "ns3/lora-network-status.h"

namespace ns3 {

class LoraNetworkStatus;         // Forward declaration
class LoraNetworkController;     // Forward declaration

class LoraNetworkScheduler : public Object
{
public:
  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId (void) const;

  LoraNetworkScheduler ();
  LoraNetworkScheduler (Ptr<LoraNetworkStatus> status,
                        Ptr<LoraNetworkController> controller);
  virtual ~LoraNetworkScheduler ();

  /**
   * Method called by NetworkServer to inform the Scheduler of a newly arrived
   * uplink packet. This function schedules the OnReceiveWindowOpportunity
   * events 1 and 2 seconds later.
   */
  void OnReceivedPacket (Ptr<const Packet> packet);

  /**
   * Method that is scheduled after packet arrivals in order to act on
   * receive windows 1 and 2 seconds later receptions.
   */
  void OnReceiveWindowOpportunity (LoraDeviceAddress deviceAddress, int window);

private:
  TracedCallback<Ptr<const Packet> > m_receiveWindowOpened;
  Ptr<LoraNetworkStatus> m_status;
  Ptr<LoraNetworkController> m_controller;

  /**
   * Delay to wait between end of reception of paquet and sending of anwser, to be in first window opportunity
   */
  Time m_firstWindowAnswerDelay;

  /**
   * Delay to wait between end of reception of paquet and sending of anwser, to be in second window opportunity
   */
  Time m_secondWindowAnswerDelay;
};

} /* namespace ns3 */

#endif /* LORA_NETWORK_SCHEDULER_H */
