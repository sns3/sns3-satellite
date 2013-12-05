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
#include "satellite-bbframe.h"

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
  /**
   * Types for sorting algorithm used by forward link scheduler
   */
  typedef enum
  {
    NO_SORT,             //!< NO_SORT
    BUFFERING_DELAY_SORT,//!< BUFFERING_DELAY_SORT
    BUFFERING_LOAD_SORT, //!< BUFFERING_LOAD_SORT
    RANDOM_SORT,         //!< RANDOM_SORT
    PRIORITY_SORT        //!< PRIORITY_SORT
  } ScheduleSortingCriteria_t;

  /**
   * BBFrame usage modes.
   */
  typedef enum
  {
    SHORT_FRAMES,          //!< SHORT_FRAMES
    NORMAL_FRAMES,         //!< NORMAL_FRAMES
    SHORT_AND_NORMAL_FRAMES//!< SHORT_AND_NORMAL_FRAMES
  } BbFrameUsageMode_t;

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
    * Schedule BB Frames.
    */
   void ScheduleBbFrames ();

   /**
    * Create short or normal frame according to byteCount and
    * according to \member m_bbFrameUsageMode.
    *
    * \param Used MODCOD for frame.
    * \param byteCount
    * \return Pointer to created frame.
    */
   Ptr<SatBbFrame> CreateFrame (uint32_t modCod, uint32_t byteCount) const;

   /**
    * Create dummy frame. Dummy frame is sent when there is nothing else to send.
    */
   Ptr<SatBbFrame> CreateDummyFrame () const;

   /**
    * Random variable used in FWD link scheduling
    */
   Ptr<UniformRandomVariable> m_random;

  /**
   * The interval that the Mac uses to throttle packet transmission
   */
  Time m_tInterval;

  /**
   * Flag indicating if Dummy Frames are sent or not.
   * false means that only transmission time is simulated without sending.
   */
  bool m_dummyFrameSendingOn;

  /**
   * Threshold time of total transmissions in BB Frame container to trigger a scheduling round.
   */
  Time m_schedulingStartThresholdTime;

  /**
   * Threshold time of total transmissions in BB Frame container to stop a scheduling round.
   */
  Time m_schedulingStopThresholdTime;

  /**
   * Sorting criteria for scheduling objects received from LLC.
   */
  ScheduleSortingCriteria_t m_schedulingSortCriteria;

  /**
   * BBFrame usage mode.
   */
  BbFrameUsageMode_t m_bbFrameUsageMode;

  /**
   * The lower layer packet transmit callback.
   */
  SatGwMac::SchedContextCallback m_schedContextCallback;

  /**
   * The container for BB Frames.
   */
  std::list< Ptr<SatBbFrame> > m_bbFrameContainer;

};

} // namespace ns3

#endif /* SAT_GW_MAC_H */
