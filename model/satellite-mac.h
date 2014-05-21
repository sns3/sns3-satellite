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

#include "satellite-control-message.h"
#include "satellite-signal-parameters.h"
#include "satellite-phy.h"
#include "satellite-node-info.h"
#include "satellite-queue.h"


namespace ns3 {

class Packet;

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
   * This default constructor for the SatMac, which should not been used
   *
   */
  SatMac ();

  /**
   * Construct a SatMac
   *
   * This is the constructor for the SatMac
   * \param beamId
   */
  SatMac (uint32_t beamId);

  /**
   * Destroy a SatMac
   *
   * This is the destructor for the SatMac.
   */
  ~SatMac ();

  /**
   * Callback to send packet to lower layer.
   * \param Container of the pointers to the packets received
   * \param uint32_t carrierId
   * \param  Time duration
   */
  typedef Callback<void, SatPhy::PacketContainer_t, uint32_t, Time, SatSignalParameters::txInfo_s> TransmitCallback;

  /**
   * Method to set transmit callback.
   * \param cb callback to invoke whenever a packet needs to be sent
   * to lower layer (PHY)
   */
  void SetTransmitCallback (SatMac::TransmitCallback cb);

  /**
   * Callback to receive packet by upper layer.
   * \param MAC address related to the received packet connection
   * \param packet the packet received
   */
  typedef Callback<void, Ptr<Packet>, Mac48Address> ReceiveCallback;

  /**
   * Method to set receive callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetReceiveCallback (SatMac::ReceiveCallback cb);

  /**
   * Callback to read control messages from container storing control messages.
   * Real length of the control messages are simulated in a packet, but not structure.
   * \param uint32_t ID of the message to read.
   * \return Pointer to read packet. (NULL if not found).
   */
  typedef Callback<Ptr<SatControlMessage>, uint32_t> ReadCtrlMsgCallback;

  /**
   * Callback to reserve an id and initially store the control
   * message.
   * \param Pointer to the message to be stored.
   * \return uint32_t ID of the stored message.
   */
  typedef Callback<uint32_t, Ptr<SatControlMessage> > ReserveCtrlMsgCallback;

  /**
   * Callback to send a control message and allocate a recv ID for it.
   * \param uint32_t Internal ID used for initial storing of the control msg.
   * \return uint32_t ID for the control message read operation.
   */
  typedef Callback<uint32_t, uint32_t> SendCtrlMsgCallback;

  /**
   * Method to set read control message callback.
   * \param cb callback to invoke whenever a control message is wanted to read.
   */
  void SetReadCtrlCallback (SatMac::ReadCtrlMsgCallback cb);

  /**
   * Method to set reserve control message id callback.
   * \param cb callback to invoke whenever an id is wanted to
   * be reserved for a control message.
   */
  void SetReserveCtrlCallback (SatMac::ReserveCtrlMsgCallback cb);

  /**
   * Method to set send control message callback.
   * \param cb callback to invoke whenever a control message is wanted to sent.
   */
  void SetSendCtrlCallback (SatMac::SendCtrlMsgCallback cb);

  /**
   * Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * Reserve id and store the control message.
   *
   * \param msg Control message to store to the container.
   * \return Id of the internally stored message (send ID).
   */
  uint32_t ReserveIdAndStoreCtrlMsgToContainer (Ptr<SatControlMessage> msg);

  /**
   * Send the control message from the container.
   *
   * \param sendId Id of the internally stored message.
   * \return ID to be used in the control packet reception (recv ID).
   */
  uint32_t SendCtrlMsgFromContainer (uint32_t sendId);

  /**
   * Receive a queue event:
   * - FIRST_BUFFER_RCVD
   * - BUFFER_EMPTY
   * /param event Queue event from SatQueue
   * /param flowIndex Identifier of the queue
   */
  virtual void ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t flowIndex);

private:
  SatMac& operator = (const SatMac &);
  SatMac (const SatMac &);

protected:

  void DoDispose (void);

  /**
   * Send packet to lower layer by using a callback
   * \param packets Packets to be sent.
   */
  void SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s);

  /**
   * Invoke the `Rx` trace source for each received packet.
   * \param packets Container of the pointers to the packets received.
   */
  void RxTraces (SatPhy::PacketContainer_t packets);

  /**
   * The lower layer packet transmit callback.
   */
  SatMac::TransmitCallback m_txCallback;

  /**
   * The upper layer package receive callback.
   */
  SatMac::ReceiveCallback m_rxCallback;

  /**
   * The read control message callback.
   */
  SatMac::ReadCtrlMsgCallback m_readCtrlCallback;

  /**
   * The reserve control message id callback.
   */
  SatMac::ReserveCtrlMsgCallback m_reserveCtrlCallback;

  /**
   * The send control message callback.
   */
  SatMac::SendCtrlMsgCallback m_sendCtrlCallback;

  /**
   * `EnableStatisticsTags` attribute.
   */
  bool m_isStatisticsTagsEnabled;

  /**
   * Trace callback used for packet tracing.
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
   * Traced callback for all received packets, including the address of the
   * senders.
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  /**
   * Traced callback for all received packets, including delay information and
   * the address of the senders.
   */
  TracedCallback<Time, const Address &> m_rxDelayTrace;

  /**
   * Node info containing node related information, such as
   * node type, node id and MAC address (of the SatNetDevice)
   */
  Ptr<SatNodeInfo> m_nodeInfo;

  /**
   * The ID of the beam where mac belongs.
   */
  uint32_t m_beamId;
};

} // namespace ns3

#endif /* SAT_MAC_H */
