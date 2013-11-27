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

#ifndef SATELLITE_LLC_H_
#define SATELLITE_LLC_H_

#include <vector>
#include "ns3/object.h"
#include "ns3/queue.h"
#include "ns3/traced-callback.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"

#include "satellite-scheduling-object.h"
#include "satellite-generic-encapsulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Base Llc class for Sat Net Devices.
 *
 */
class SatLlc : public Object
{
public:
  static TypeId GetTypeId (void);

  SatLlc ();

  /**
   * Construct a SatLlc
   *
   * This is the constructor for the SatLlc
   * \param isUt boolean flag to indicate whether this LLC 
   * is attached to GW or UT.
   */
  SatLlc (bool isUt);

  /**
   * Destroy a SatLlc
   *
   * This is the destructor for the SatLlc.
   */
  ~SatLlc ();

  typedef std::map<Mac48Address, Ptr<SatGenericEncapsulator> > encapContainer_t;

  /**
   * Receive callback used for sending packet to netdevice layer.
    * \param packet the packet received
    */
  typedef Callback<void,Ptr<const Packet> > ReceiveCallback;

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
    *  Called from higher layer (SatNetDevice) to enque packet to LLC
    *
    * \param packet packet sent from above down to SatMac
    * \param dest Destination MAC address of the packet
    * \return whether the Send operation succeeded
    */
  virtual bool Enque(Ptr<Packet> packet, Address dest);

  /**
    *  Called from lower layer (MAC) to inform a tx
    *  opportunity of certain amount of bytes
    *
    * \param macAddr Mac address of the UT with tx opportunity
    * \param bytes Size of the Tx opportunity
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr);

  /**
   * Receive packet from lower layer.
   * \param macAddr MAC address of the UT (either as transmitter or receiver)
   * \param packet Pointer to packet received.
   */
  virtual void Receive (Ptr<Packet> packet, Mac48Address macAddr);

  /**
   * Receive HL PDU from encapsulator/decapsulator entity
   *
   * \param packet Pointer to packet received.
   */
  virtual void ReceiveHigherLayerPdu (Ptr<Packet> packet);

  /**
   * Set Receive callback to forward packet to upper layer
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetReceiveCallback (SatLlc::ReceiveCallback cb);

  /**
   * Add an encapsulator entry for the LLC
   * Key = UT's MAC address
   * Value = encap entity
   */
  void AddEncap (Mac48Address macAddr, Ptr<SatGenericEncapsulator> enc);

  /**
   * Add an decapsulator entry for the LLC
   * Key = UT's MAC address
   * Value = decap entity
   */
  void AddDecap (Mac48Address macAddr, Ptr<SatGenericEncapsulator> dec);

  /**
   * Set the address of this MAC
   * \param macAddress MAC address of this LLC
   */
  void SetAddress (Mac48Address macAddress);

  /**
   * Create and fill the scheduling objects based on LLC layer information.
   * Scheduling objects may be used at the MAC layer to assist in scheduling.
   * \return vector of scheduling object pointers
   */
  std::vector< Ptr<SatSchedulingObject> > GetSchedulingContexts () const;

protected:

  void DoDispose ();

private:

  /**
   * MAC address of this node
   */
  Mac48Address m_macAddress;

  // Map of encapsulators
  encapContainer_t m_encaps;

  // Map of decapsulators
  encapContainer_t m_decaps;

  /**
   * Boolean to identify whether this instance is attached to UT or GW.
   * TODO: probably the LLC should be split into UT and GW entities.
   */
  bool m_isUt;

  /**
   * The Queue which this SatMac uses as a packet source.
   * @see class Queue
   * @see class DropTailQueue
   */
  Ptr<Queue> m_controlQueue;

  /**
   * The upper layer package receive callback.
   */
  ReceiveCallback m_rxCallback;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_llcTxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * at the L3/L2 transition are dropped before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_llcTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a non-promiscuous trace (which doesn't mean a lot
   * here in the point-to-point device).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_llcRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but are dropped before being forwarded up to higher layers (at the L2/L3
   * transition).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_llcRxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a promiscuous trace (which doesn't mean a lot here
   * in the point-to-point device).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_llcPromiscRxTrace;

};

} // namespace ns3


#endif /* SATELLITE_LLC_H_ */
