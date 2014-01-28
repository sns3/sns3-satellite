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
#include "satellite-phy.h"

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
   * \param seq   Pointer to superframe sequence.
   * \param beamId Id of the beam.
   *
   */
  SatUtMac (Ptr<SatSuperframeSeq> seq, uint32_t beamId);

  /**
   * Destroy a SatUtMac
   *
   * This is the destructor for the SatUtMac.
   */
  ~SatUtMac ();

  /**
   * Receive packet from lower layer.
   *
   * \param packets Pointers to packets received.
   */
  virtual void Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/);

  /**
   * \return Timing advance as Time object.
   */
  typedef Callback<Time> TimingAdvanceCallback;

  /**
   * \param cb callback to invoke whenever a timing advance is needed by SatUtMac object.
   *
   */
  void SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb);

  /**
     * \param packet     the packet send
     * \param address    Packet destination address
     * \param protocol   protocol number to send packet.
     */
  typedef Callback<bool, Ptr<Packet>, const Address&, uint16_t > SendCallback;

  /**
   * \param cb callback to send control messages.
   *
   */
  void SetTxCallback (SatUtMac::SendCallback cb);

  /**
   * Update C/N0 information from lower layer.
   *
   * The SatUtMac receives C/N0 information of packet receptions from GW
   * to update this information to serving GW periodically.
   *
   * \param beamId  The id of the beam where C/N0 is from.
   * \param gwId  The id of the GW.
   * \param utId  The id (address) of the UT.
   * \param cno Value of the C/N0.
   */
  void CnoUpdated (uint32_t beamId, Address utId, Address gwId, double cno);

  /**
   * Set address of the GW (or its MAC) serving this UT.
   *
   * \param gwAddress Address of the GW.
   */
  void SetGwAddress (Mac48Address gwAddress);

protected:

   void DoDispose (void);

private:
   /**
    * Send capacity request update to GW.
    */
   void SendCapacityReq ();

   /**
    *  Schedules time slots according to received TBTP message.
    *
    * \param tbtp Pointer to TBTP message.
    */
   void ScheduleTimeSlots (Ptr<SatTbtpMessage> tbtp);

   /**
    * Schdules one Tx opportunity, i.e. time slot.
    * \param transmitTime time when transmit possibility starts
    * \param durationInSecs duration of the burst
    * \param payloadBytes payload in bytes
    * \param carrierId Carrier id used for the transmission
    */
   void ScheduleTxOpportunity (Time transmitTime, double durationInSecs, uint32_t payloadBytes, uint32_t carrierId);

   /**
    * Notify the upper layer about the Tx opportunity. If upper layer
    * returns a PDU, send it to lower layer.
    *
    * \param durationInSecs duration of the burst
    * \param payloadBytes payload in bytes
    * \param carrierId Carrier id used for the transmission
    */
   void TransmitTime (double durationInSecs, uint32_t payloadBytes, uint32_t carrierId);

  /**
   * Signaling packet receiver, which handles all the signaling packet
   * receptions.
   * \param packet Received signaling packet
   * \param cType Control message type
   */
  void ReceiveSignalingPacket (Ptr<Packet> packet, SatControlMsgTag ctrlTag);

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
   * Callback to send control messages.
  */
  SendCallback m_txCallback;

  /**
   * The configured Constant Rate Assignment (CRA) for this UT Mac.
   */
  double m_cra;

  /**
   * The last received C/N0 information from lower layer.
   */
  double m_lastCno;

  /**
   * Serving GW's address.
   */
  Mac48Address m_gwAddress;

  /**
   * Interval to send capacity requests.
   */
  Time m_crInterval;
};

} // namespace ns3

#endif /* SATELLITE_UT_MAC_H */
