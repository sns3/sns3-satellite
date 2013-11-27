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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_UT_MAC_H
#define SATELLITE_UT_MAC_H

#include "satellite-mac.h"
#include "ns3/satellite-superframe-sequence.h"
#include "satellite-control-message.h"

namespace ns3 {

/**
 * \ingroup satellite
  * \brief UT specific Mac class for Sat Net Devices.
 *
 * This SatUtMac class specializes the Mac class with UT characteristics.
 * The class implements UT specific scheduler and will implement Ut specif queus later..
 *
 */
class SatUtMac : public SatMac
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  SatUtMac ();
  /**
   * Construct a SatUtMac
   *
   * This is the constructor for the SatUtMac
   *
   * \param seq Pointer to superframe sequence.
   *
   */
  SatUtMac (Ptr<SatSuperframeSeq> seq);

  /**
   * Destroy a SatUtMac
   *
   * This is the destructor for the SatUtMac.
   */
  ~SatUtMac ();

  /**
   * Receive packet from lower layer.
   *
   * \param packet Pointer to packet received.
   */
  virtual void Receive (Ptr<Packet> packet, Ptr<SatSignalParameters> /*rxParams*/);

  /**
   * \return Timing advance as Time object.
   */
  typedef Callback<Time> TimingAdvanceCallback;

  /**
   * \param cb callback to invoke whenever a timing advance is needed by SatUtMac object.
   *
   */
  void SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb);

protected:

   void DoDispose (void);

private:

   /**
    *  Schedules time slots according to received TBTP message.
    *
    * \param tbtp Pointer to TBTP message.
    */
   void ScheduleTimeSlots (SatTbtpHeader * tbtp);

   /**
    * Schdules one Tx opportunity, i.e. time slot.
    * \param transmitTime time when transmit possibility starts
    * \param carrierId Carrier id used for the transmission
    * \param timeSlotConf Time Slot configuration of the scheduled time slot
    */
   void ScheduleTxOpportunity ( Time transmitTime, uint32_t carrierId, Ptr<SatTimeSlotConf> timeSlotConf );

   /**
    * Notify the upper layer about the Tx opportunity. If upper layer
    * returns a PDU, send it to lower layer.
    *
    * \param carrierId Carrier id used for the transmission
    * \param timeSlotConf Time Slot configuration of the scheduled time slot
    */
   void TransmitTime (uint32_t carrierId, Ptr<SatTimeSlotConf> timeSlotConf);

  /**
   * Signaling packet receiver, which handles all the signaling packet
   * receptions.
   * \param packet Received signaling packet
   * \param cType Control message type
   */
  void ReceiveSignalingPacket (Ptr<Packet> packet, SatControlMsgTag::SatControlMsgType_t cType);

  SatUtMac& operator = (const SatUtMac &);
  SatUtMac (const SatUtMac &);

  /**
   * Used superframe sequence for the return link
   */
  Ptr<SatSuperframeSeq> m_superframeSeq;

  /**
   * Callback for getting the timing advance information
   */
  TimingAdvanceCallback m_timingAdvanceCb;

  /**
   * The configured Constant Rate Assignment (CRA) for this UT Mac.
   */
  double m_cra;
};

} // namespace ns3

#endif /* SATELLITE_UT_MAC_H */
