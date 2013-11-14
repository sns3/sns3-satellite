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

#ifndef SAT_GW_MAC_H
#define SAT_GW_MAC_H

#include <cstring>

#include "ns3/address.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/error-model.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"

#include "satellite-mac.h"
#include "satellite-net-device.h"
#include "satellite-signal-parameters.h"

namespace ns3 {

/**
 * \ingroup satellite
  * \brief GW specific Mac class for Sat Net Devices.
 *
 * This SatGwMac class specializes the Mac class with GW characteristics.
 */

class SatGwMac : public SatMac
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatGwMac
   *
   * This is the constructor for the SatGwMac
   *
   */
  SatGwMac ();

  /**
   * Destroy a SatGwMac
   *
   * This is the destructor for the SatGwMac.
   */
  ~SatGwMac ();

  /**
   * Starts scheduling of the sending. Called when MAC is wanted to take care of scheduling.
   */
  void StartScheduling();

  /**
   * Schedules one sending opportunity. Called for every sending opportunity scheduler.
   * /param Time transmitTime time when transmit possibility starts
   */
  void ScheduleTransmit ( Time transmitTime, uint32_t carrierId );

private:

  SatGwMac& operator = (const SatGwMac &);
  SatGwMac (const SatGwMac &);

  void DoDispose (void);

  /**
   * Start new sending if there is packet in queue, otherwise schedules next send moment.
   *
   * The TransmitReady method is used internally to schedule sending of a packet out on the phy.
   */
  void TransmitReady (uint32_t carrierId);

  /**
    * Start Sending a Packet Down the Wire.
    *
    * The TransmitStart method is the method that is used internally in the
    * SatGwMac to begin the process of sending a packet out on the phy layer.'
    *
    * \param p a reference to the packet to send
    * \param carrierId id of the carrier.
    * \returns true if success, false on failure
    */
   bool TransmitStart (Ptr<Packet> p,  uint32_t carrierId);

  /**
   * The interval that the Mac uses to throttle packet transmission
   */
  Time m_tInterval;

};

} // namespace ns3

#endif /* SAT_GW_MAC_H */
