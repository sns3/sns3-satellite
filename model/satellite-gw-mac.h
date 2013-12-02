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
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "ns3/random-variable-stream.h"

#include "satellite-mac.h"
#include "satellite-net-device.h"
#include "satellite-signal-parameters.h"
#include "satellite-scheduling-object.h"
#include "satellite-phy.h"

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
   * Receive packet from lower layer.
   *
   * \param packets Pointers to packets received.
   */
  void Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/);

  /**
   * Callback to get scheduling contexts from upper layer
   * \param vector of scheduling contexts
   */
  typedef Callback<std::vector< Ptr<SatSchedulingObject> > > SchedContextCallback;

  /**
   * Method to set Tx opportunity callback.
    * \param cb callback to invoke whenever a packet has been received and must
    *        be forwarded to the higher layers.
    *
    */
  void SetSchedContextCallback (SatGwMac::SchedContextCallback cb);

private:

  SatGwMac& operator = (const SatGwMac &);
  SatGwMac (const SatGwMac &);

  void DoDispose (void);

  /**
   * Schedules the next transmission time.
   * \param txTime Next Tx opportunity
   * \param carrierId Carrier id for next transmission
   */
  void ScheduleNextTransmissionTime (Time txTime, uint32_t carrierId);

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
   void TransmitTime (uint32_t carrierId);

   /**
    * Random variable used in FWD link scheduling
    */
   Ptr<UniformRandomVariable> m_random;

  /**
   * The interval that the Mac uses to throttle packet transmission
   */
  Time m_tInterval;

  /**
   * The lower layer packet transmit callback.
   */
  SatGwMac::SchedContextCallback m_schedContextCallback;

};

} // namespace ns3

#endif /* SAT_GW_MAC_H */
