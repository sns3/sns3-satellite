/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#ifndef SATELLITE_TBTP_CONTAINER_H_
#define SATELLITE_TBTP_CONTAINER_H_

#include <map>
#include "ns3/object.h"
#include "ns3/mac48-address.h"
#include "satellite-control-message.h"

namespace ns3 {

/**
 * \brief A container of received TBTPs. All the received TBTPs with
 * the calculated transmit time are stored here. The container may be
 * used to check whether the UT has known scheduled time slots in the
 * future.
 */
class SatTbtpContainer : public Object
{
public:
  SatTbtpContainer ();
  SatTbtpContainer (Time superFrameDuration);
  ~SatTbtpContainer ();

  static TypeId GetTypeId (void);

  virtual void DoDispose ();

  /**
   * Set the mac address of this node
   * /param address MAC address of this node
   */
  void SetMacAddress (Mac48Address address);

  /**
   * Add a TBTP message to the container
   * /param startTime TBTP superframe transmit start time
   * /param tbtp TBTP message
   */
  void Add (Time startTime, Ptr<SatTbtpMessage> tbtp);

  /**
   * Method of checking whether the UT has been scheduled time
   * slots into the future.
   * /return bool Boolean indicating whether UT has been scheduled
   * time slots into the future.
   */
  bool HasScheduledTimeSlots ();

private:

  /**
   * Function for removing the TBTPs which are in the past.
   */
  void RemovePastTbtps ();

  typedef std::multimap<Time, Ptr<SatTbtpMessage> > TbtpMap_t;
  TbtpMap_t m_tbtps;

  /**
   * Address of this UT
   */
  Mac48Address m_address;

  /**
   * Maximum stored TBTPs in the container
   */
  uint32_t m_maxStoredTbtps;

  /**
   * Number of received TBTPs
   */
  uint32_t m_rcvdTbtps;

  /**
   * Superframe duration
   */
  Time m_superFrameDuration;
};

} // namespace


#endif /* SATELLITE_TBTP_CONTAINER_H_ */
