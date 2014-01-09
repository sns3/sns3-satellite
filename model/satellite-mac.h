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
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "ns3/mac48-address.h"

#include "satellite-signal-parameters.h"
#include "satellite-phy.h"
#include "satellite-node-info.h"


namespace ns3 {

/**
 * \ingroup satellite
  * \brief Base Mac class for Sat Net Devices.
 *
 * This SatMac class specializes the Mac base class.
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
   * Callback to send packet to lower layer.
   * \param Coitainer of the pointers to the packets received
   * \param uint32_t carrierId
   * \param  Time duration
   */
  typedef Callback<void, SatPhy::PacketContainer_t, uint32_t, Time> TransmitCallback;

  /**
   * Callback to receive packet by upper layer.
   * \param MAC address related to the received packet connection
   * \param packet the packet received
   */
  typedef Callback<void, Ptr<Packet>, Mac48Address> ReceiveCallback;

  /**
   * Method to set transmit callback.
   * \param cb callback to invoke whenever a packet needs to be sent
   * to lower layer (PHY)
   */
  void SetTransmitCallback (SatMac::TransmitCallback cb);

  /**
   * Method to set receive callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetReceiveCallback (SatMac::ReceiveCallback cb);

  /**
   * Callback to notify upper layer about Tx opportunity.
   * \param Mac48Address address
   * \param uint32_t payload size in bytes
   * \return packet Packet to be transmitted to PHY
   */
  typedef Callback< Ptr<Packet>, uint32_t, Mac48Address, uint32_t& > TxOpportunityCallback;

  /**
   * Method to set Tx opportunity callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetTxOpportunityCallback (SatMac::TxOpportunityCallback cb);

  /**
   * Set the node info
   * \param nodeInfo containing node specific information
   */
  void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

private:
  SatMac& operator = (const SatMac &);
  SatMac (const SatMac &);

protected:

  void DoDispose (void);

  /**
   * Send packet to lower layer by using a callback
   * \param packets Packets to be sent.
   */
  void SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration);

  /**
   * The lower layer packet transmit callback.
   */
  SatMac::TransmitCallback m_txCallback;

  /**
   * The upper layer package receive callback.
   */
  SatMac::ReceiveCallback m_rxCallback;

  /**
   * Callback to notify the txOpportunity to upper layer
   * Returns a packet
   * Attributes: payload in bytes
   */
  SatMac::TxOpportunityCallback m_txOpportunityCallback;

  /**
   * Trace callback used for packet tracing:
   */
  TracedCallback< Time,
                  SatEnums::SatPacketEvent_t,
                  SatEnums::SatNodeType_t,
                  uint32_t,
                  Mac48Address,
                  SatEnums::SatLogLevel_t,
                  SatEnums::SatLinkDir_t,
                  std::string
                  > m_packetTrace;

  /**
   * Node info containing node related information, such as
   * node type, node id and MAC address (of the SatNetDevice)
   */
  Ptr<SatNodeInfo> m_nodeInfo;

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
