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
#include "satellite-frame-conf.h"
#include "satellite-superframe-sequence.h"

namespace ns3 {


/**
 * \ingroup satellite
 * This class sorts time slots within TBTP into increasing order based
 * on start time.
 */
class SortTimeSlots
{
public:
  SortTimeSlots ()
  {
  }

  /**
   * Operator overload for sorting function
   * \param p1 Time slot configuration
   * \param p2 Time slot configuration
   * \return Start time of p1 < Start time of p2
   */
  bool operator() (Ptr<SatTimeSlotConf> p1, Ptr<SatTimeSlotConf> p2)
  {
    return p1->GetStartTime () < p2->GetStartTime ();
  }
};

/**
 * \ingroup satellite
 * \brief A container of received TBTPs. All the received TBTPs with
 * the calculated transmit time are stored here. The container may be
 * used to check whether the UT has known scheduled time slots in the
 * future.
 */
class SatTbtpContainer : public Object
{
public:

  /**
   * Default constructor.
   */
  SatTbtpContainer ();

  /**
   * Constructor with initialization parameters.
   * \param seq 
   */
  SatTbtpContainer (Ptr<SatSuperframeSeq> seq);

  /**
   * Destructor for SatTbtpContainer
   */
  ~SatTbtpContainer ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Set the MAC address of this node
   * \param address MAC address of this node
   */
  void SetMacAddress (Mac48Address address);

  /**
   * \brief Add a TBTP message to the container
   * \param startTime TBTP superframe transmit start time
   * \param tbtp TBTP message
   */
  void Add (Time startTime, Ptr<SatTbtpMessage> tbtp);

  /**
   * \brief Method of checking whether the UT has been scheduled time
   * slots into the future.
   * \return bool Boolean indicating whether UT has been scheduled
   * time slots into the future.
   */
  bool HasScheduledTimeSlots ();

private:
  /**
   * \brief Function for removing the TBTPs which are in the past.
   */
  void RemovePastTbtps ();

  typedef std::multimap<Time, Ptr<SatTbtpMessage> > TbtpMap_t;
  TbtpMap_t m_tbtps;

  /**
   * Address of this UT
   */
  Mac48Address m_address;

  /**
   * Superframe sequence
   */
  Ptr<SatSuperframeSeq> m_superframeSeq;

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
