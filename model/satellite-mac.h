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

#ifndef SAT_MAC_H
#define SAT_MAC_H

#include <cstring>

#include "ns3/address.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/queue.h"
#include "ns3/error-model.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"

#include "satellite-phy.h"
#include "satellite-net-device.h"
#include "satellite-signal-parameters.h"

namespace ns3 {




/**
 * \ingroup satellite
  * \brief Base Mac class for Sat Net Devices.
 *
 * This SatMac class specializes the Mac base class.
 * Key parameters or objects that can be specified for this object
 * include a queue and interval for periodic scheduling.
 *
 * Classed derived from this base class can add more realistic schedulers and
 * add more needed queues.
 */
class SatMac : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatMac
   *
   * This is the constructor for the SatMac
   *
   */
  SatMac ();

  /**
   * Destroy a SatMac
   *
   * This is the destructor for the SatMac.
   */
  ~SatMac ();

  /**
   * Starts scheduling of the sending. Called when MAC is wanted to take care of scheduling.
   */
  void StartScheduling();

  /**
   * Schdules one sending opportunity. Called for every sending opportunity scheduler.
   * /param Time transmitTime time when transmit possibility starts
   */
  void ScheduleTransmit ( Time transmitTime, uint32_t carrierId );

  /**
   * Attach the Phy object to SatMac.
   *
   * \param phy Ptr to the attached phy object.
   */
  bool SetPhy (Ptr<SatPhy> phy);

  /**
   * Get a copy of the attached Phy.
   *
   * \returns Ptr to the phy object.
   */
  Ptr<SatPhy> GetPhy (void) const;

  /**
   * Attach the a Net Device to SatMac.
   *
   * \param phy Ptr to the attached Net Device object.
   */

  void SetQueue (Ptr<Queue> queue);

  /**
   * Get a copy of the attached Queue.
   *
   * @returns Ptr to the queue.
   */
  Ptr<Queue> GetQueue (void) const;

   /**
   * Receive a packet from a connected Phy object.
   *
   * The SatMac receives packets from its connected phy object
   * and forwards them up the protocol stack.  This is the public method
   * used by the phy  object to indicate that the last bit of a packet has
   * arrived at the device.
   *
   * @see SatChannel
   * /param p Ptr to the received packet.
   * /param rxParams RX parameters
   */

  void Receive (Ptr<Packet> p, Ptr<SatSignalParameters> /*rxParams*/);

  /**
    * \param packet packet sent from above down to SatMac
    *
    *  Called from higher layer to send packet into Mac layer
    *  to the specified destination Address
    *
    * \return whether the Send operation succeeded
    */
  bool Send (Ptr<Packet> packet, Address dest);

  /**
    * \param packet the packet received
    */
  typedef Callback<void,Ptr<const Packet> > ReceiveCallback;

  /**
    * \param cb callback to invoke whenever a packet has been received and must
    *        be forwarded to the higher layers.
    *
    */
  void SetReceiveCallback (SatMac::ReceiveCallback cb);

  void SetAddress (Mac48Address macAddress);

protected:
  /**
    * Start Sending a Packet Down the Wire.
    *
    * The TransmitStart method is the method that is used internally in the
    * SatMac to begin the process of sending a packet out on the phy layer.'
    *
    * \param p a reference to the packet to send
    * \param carrierId id of the carrier.
    * \returns true if success, false on failure
    */
   bool TransmitStart (Ptr<Packet> p,  uint32_t carrierId);

   inline bool PacketInQueue() { return (bool)(m_queue->GetNPackets() > 0);}

protected:
   /**
    * The interval that the Mac uses to throttle packet transmission
    */
   Time m_tInterval;

   /**
    * MAC address of the this mac instance
    */
   Mac48Address m_macAddress;

private:
  SatMac& operator = (const SatMac &);
  SatMac (const SatMac &);

  void DoDispose (void);

  /**
   * Start new sending if there is packet in queue, otherwise schedules next send moment.
   *
   * The TransmitReady method is used internally to schedule sending of a packet out on the phy.
   */
  void TransmitReady (uint32_t carrierId);

  /**
   * The Phy obejct to which this SatMac has been attached.
   */
  Ptr<SatPhy> m_phy;

  /**
   * The Queue which this SatMac uses as a packet source.
   * Management of this Queue has been delegated to the SatMac
   * and it has the responsibility for deletion.
   * @see class Queue
   * @see class DropTailQueue
   */
  Ptr<Queue> m_queue;

  /**
   * The Net Device which owns this SatMac is attached to.
   */
  Ptr<SatNetDevice> m_device;

  /**
   * The upper layer package receive callback.
   */
  SatMac::ReceiveCallback m_rxCallback;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * at the L3/L2 transition are dropped before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3 
   * transition).  This is a promiscuous trace (which doesn't mean a lot here
   * in the point-to-point device).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3 
   * transition).  This is a non-promiscuous trace (which doesn't mean a lot 
   * here in the point-to-point device).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but are dropped before being forwarded up to higher layers (at the L2/L3 
   * transition).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * A trace source that emulates a non-promiscuous protocol sniffer connected 
   * to the device.  Unlike your average everyday sniffer, this trace source 
   * will not fire on PACKET_OTHERHOST events.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where 
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example, 
   * this would correspond to the point at which the packet is dispatched to 
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  /**
   * A trace source that emulates a promiscuous mode protocol sniffer connected
   * to the device.  This trace source fire on packets destined for any host
   * just like your average everyday packet sniffer.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where 
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example, 
   * this would correspond to the point at which the packet is dispatched to 
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;
};

} // namespace ns3

#endif /* SAT_MAC_H */
