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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SAT_FWD_LINK_SCHEDULER_TIME_SLICING_H
#define SAT_FWD_LINK_SCHEDULER_TIME_SLICING_H

#include "ns3/satellite-fwd-link-scheduler.h"
#include "ns3/pointer.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatFwdLinkSchedulerTimeSlicing schedules BB frames for forward link. It classifies the packets
 *        into different time slices depending on its destination MAC address.
 *
 *        SatFwdLinkSchedulerTimeSlicing communicated through callback functions to request scheduling objects and
 *        notifying TX opportunities.
 *
 *        GW MAC requests frames from scheduler through method GetNextFrame.
 *
 */

class SatFwdLinkSchedulerTimeSlicing : public SatFwdLinkScheduler
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Construct a SatFwdLinkScheduler
   *
   * This the default constructor for the SatFwdLinkScheduler is not supported.
   *
   */
  SatFwdLinkSchedulerTimeSlicing ();

  /**
   * Actual constructor of a SatFwdLinkScheduler
   *
   * \param conf BB Frame configuration
   * \param address MAC address
   * \param carrierBandwidthInHz Carrier bandwidth where scheduler is associated to [Hz].
   */
  SatFwdLinkSchedulerTimeSlicing (Ptr<SatBbFrameConf> conf, Mac48Address address, double carrierBandwidthInHz);

  /**
   * Destroy a SatFwdLinkScheduler
   *
   * This is the destructor for the SatFwdLinkScheduler.
   */
  ~SatFwdLinkSchedulerTimeSlicing ();

  /**
   * Get next frame to be transmitted.
   *
   * \return Pointer to frame
   */
  virtual Ptr<SatBbFrame> GetNextFrame ();

  /**
   * Callback to notify upper layer about Tx opportunity.
   * \param Ptr<SatControlMessage> The control message to send.
   * \param Address& the destination MAC address.
   * \return True
   */
  typedef Callback<bool, Ptr<SatControlMessage>, const Address& > SendControlMsgCallback;

  /**
   * Method to set the control message sender callback.
   * \param cb callback to invoke whenever a control packet has to be sent. Should be about time-slice subscriptions.
   */
  void SetSendControlMsgCallback (SatFwdLinkSchedulerTimeSlicing::SendControlMsgCallback cb);

private:

  /**
   * Do dispose actions.
   */
  void DoDispose (void);

  /**
   * Schedule BB Frames.
   */
  void ScheduleBbFrames ();

  /**
   *  Handles periodic timer timeouts.
   */
  void PeriodicTimerExpired ();

  /**
   * Gets scheduling object in sorted order according to configured sorting criteria.
   *
   * \param output reference to a vector which will be filled with pointers to
   *               the scheduling objects available for scheduling.
   */
  void GetSchedulingObjects (std::vector< Ptr<SatSchedulingObject> > & output);

  /*
   * Give the total sending time of all the BBFrames in all the slices.
   * \return The total duration.
   */
  Time GetTotalDuration ();

  /*
   * Send a control packet to the UT to inform which slices to subscribe
   * \param address The MAC address of the UT
   * \param slices The slices id the destination must subscribe to.
   */
  void SendTimeSliceSubscription (Mac48Address address, std::vector<uint8_t> slices);

  /**
   * The containers for BBFrames. The keys are the slices and the values the associated container
   */
  std::map<uint8_t, Ptr<SatBbFrameContainer>> m_bbFrameContainers;

  /**
   * The container for control BBFrames that are broadcasted to all UT
   */
  Ptr<SatBbFrameContainer> m_bbFrameCtrlContainer;

  /**
   * The association between a destination MAC address and its slice.
   */
  std::map<Mac48Address, uint8_t> m_slicesMapping;

  /**
   * The number of slices
   */
  uint8_t m_numberOfSlices;

  /**
   * The slice of the last MAC address discovered. Used to associated destination MAC address
   * to a slice following a Round Robin law.
   */
  uint8_t m_lastSliceAssigned;

  /**
   * The last slice from which a BBFrame has been sent. used for Round Robin scheduling.
   */
  uint8_t m_lastSliceDequeued;

  /**
   * The control message sender callback.
   */
  SatFwdLinkSchedulerTimeSlicing::SendControlMsgCallback m_sendControlMsgCallback;

};

} // namespace ns3

#endif /* SAT_FWD_LINK_SCHEDULER_TIME_SLICING_H */
