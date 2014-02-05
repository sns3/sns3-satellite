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

#include "satellite-node-info.h"
#include "satellite-scheduling-object.h"
#include "satellite-encapsulator.h"
#include "satellite-request-manager.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatLlc class holds the UT specific SatEncapsulator instances, which are responsible
 * of fragmentation, defragmentation, encapsulation and decapsulation. Packets are enqued by
 * the SatNetDevice by using the Enque () method and classified to a correct encapsulator object or
 * to a global control queue. Fragmentation is not allowed for control packets. Lower layer (MAC)
 * requests the packets by using the NotifyTxOpportunity () method.
 *
 * At GW:
 * - Encapsulators are of type SatGenericStreamEncapsulator
 * - Decapsulators are of type SatReturnLinkEncapsulator
 * - There are as many encapsulators and decapsulators as there are UTs within the spot-beam.
 *
 * At UT
 * - Encapsulators are of type SatReturnLinkEncapsulator
 * - Decapsulators are of type SatGenericStreamEncapsulator
 * - There is only one encapsulator and one decapsulator
 *
 */
class SatLlc : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatLlc
   */
  SatLlc ();

  /**
   * Destroy a SatLlc
   *
   * This is the destructor for the SatLlc.
   */
  ~SatLlc ();

  typedef std::pair<Mac48Address, uint32_t> EncapKey_t;
  typedef std::map<EncapKey_t, Ptr<SatEncapsulator> > EncapContainer_t;

  /**
   * Receive callback used for sending packet to netdevice layer.
    * \param packet the packet received
    */
  typedef Callback<void,Ptr<const Packet> > ReceiveCallback;

  /**
   * Attach the a Net Device to SatMac.
   *
   * \param queue Ptr to the attached Net Device object.
   */
  void SetQueue (Ptr<Queue> queue);

  /**
   * Get a copy of the attached Queue.
   *
   * \return Ptr to the queue.
   */
  Ptr<Queue> GetQueue (void) const;

  /**
    *  Called from higher layer (SatNetDevice) to enque packet to LLC
    *
    * \param packet packet sent from above down to SatMac
    * \param dest Destination MAC address of the packet
    * \param tos Type-of-Service of the IPv4 header
    * \return whether the Send operation succeeded
    */
  virtual bool Enque(Ptr<Packet> packet, Address dest, uint8_t tos);

  /**
    *  Called from lower layer (MAC) to inform a tx
    *  opportunity of certain amount of bytes
    *
    * \param macAddr Mac address of the UT with tx opportunity
    * \param bytes Size of the Tx opportunity
    * \param &bytesLeft Bytes left after TxOpportunity
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint32_t &bytesLeft);

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

  void AddRequestManager (Ptr<SatRequestManager> rm);

  /**
   * Add an encapsulator entry for the LLC
   * Key = UT's MAC address
   * Value = encap entity
   * \param macAddr UT's MAC address
   * \param enc encap entity
   */
  void AddEncap (Mac48Address macAddr, Ptr<SatEncapsulator> enc);

  /**
   * Add an decapsulator entry for the LLC
   * Key = UT's MAC address
   * Value = decap entity
   * \param macAddr UT's MAC address
   * \param dec decap entity
   */
  void AddDecap (Mac48Address macAddr, Ptr<SatEncapsulator> dec);

  /**
   * Set the node info
   * \param nodeInfo containing node specific information
   */
  void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * Create and fill the scheduling objects based on LLC layer information.
   * Scheduling objects may be used at the MAC layer to assist in scheduling.
   * \return vector of scheduling object pointers
   */
  std::vector< Ptr<SatSchedulingObject> > GetSchedulingContexts () const;

  /**
   * Get the KPIs related to a specific queue
   * @return
   */
  double GetQueueKpis (uint32_t rcIndex);

protected:
  /**
   * \brief
   */
  void DoDispose ();

  /**
   * Trace callback used for packet tracing:
   */
  TracedCallback<Time,
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

private:

  Ptr<SatRequestManager> m_requestManager;

  // Map of encapsulator base pointers
  EncapContainer_t m_encaps;

  // Map of decapsulator base pointers
  EncapContainer_t m_decaps;

  /**
   * The Queue which this SatLlc uses as a packet source.
   */
  Ptr<Queue> m_controlQueue;

  /**
   * The upper layer package receive callback.
   */
  ReceiveCallback m_rxCallback;

};

} // namespace ns3


#endif /* SATELLITE_LLC_H_ */
