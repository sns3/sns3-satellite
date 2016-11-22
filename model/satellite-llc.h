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
#include <map>
#include <ns3/object.h>
#include <ns3/traced-callback.h>
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-base-encapsulator.h>

namespace ns3 {

class Time;
class Address;
class Packet;
class SatControlMessage;
class SatSchedulingObject;
class SatNodeInfo;

/**
 * \ingroup satellite
 * \brief EncapKey class is used as a key in the encapsulator/decapsulator container. It
 * will hold the flow information related to one single encapsulator/decapsulator.
 */
class EncapKey : public SimpleRefCount <EncapKey>
{
public:
  Mac48Address  m_source;
  Mac48Address  m_destination;
  int8_t        m_flowId;

  EncapKey (const Mac48Address source, const Mac48Address dest, const uint8_t flowId)
    : m_source (source),
      m_destination (dest),
      m_flowId (flowId)
  {
  }
};

/**
 * \ingroup satellite
 * \brief EncapKeyCompare is used as a custom compare method within
 * EncapContainer map. Encap key has three member variables (source
 * address, dest address and flow id) and to be able to store them in
 * a map container, a custom compare method needs to be implemented.
 */
class EncapKeyCompare
{
public:
  bool operator() (Ptr<EncapKey> key1, Ptr<EncapKey> key2) const
  {
    if ( key1->m_source == key2->m_source )
      {
        if ( key1->m_destination == key2->m_destination )
          {
            return key1->m_flowId < key2->m_flowId;
          }
        else
          {
            return key1->m_destination < key2->m_destination;
          }
      }
    else
      {
        return key1->m_source < key2->m_source;
      }
  }
};

/**
 * \ingroup satellite
 * \brief SatLlc base class holds the UT specific SatBaseEncapsulator instances, which are responsible
 * of fragmentation, defragmentation, encapsulation and decapsulation. Encapsulator class is thus
 * capable of working in both transmission and reception side of the system. The SatLlc base class holds
 * base pointers of the encapsulators, but the actual encapsulator types depend on the simulation direction:
 *
 * At GW:
 * - Encapsulators are of type SatGenericStreamEncapsulator or SatGenericStreamEncapsulatorArq
 * - Decapsulators are of type SatReturnLinkEncapsulator or SatReturnLinkEncapsulatorArq
 * - There are as many encapsulators and decapsulators as there are UTs within the spot-beam.
 *
 * At UT
 * - Encapsulators are of type SatReturnLinkEncapsulator or SatReturnLinkEncapsulatorArq
 * - Decapsulators are of type SatGenericStreamEncapsulator or SatGenericStreamEncapsulatorArq
 * - There is only one encapsulator and one decapsulator
 *
 * Fragmentation is not allowed for control packets, thus the basic functionality of just buffering
 * control packets without encapsulation, decapsulation, fragmentation and packing is implemented to the
 * SatBaseEncapsulator class.
 *
 *  SatLlc is a pure virtual class, thus it is not supposed to be instantiated. A proper version of the
 *  SatLlc is inherited: SatUtLlc at the UT and SatGwLlc at the GW. There is no LLC layer at the satellite.
 */
class SatLlc : public Object
{
public:
  /**
   * Derived from Object
   */
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
  virtual ~SatLlc ();

  /**
   * Key = Ptr<EncapKey> (source, dest, flowId)
   * Value = Ptr<SatBaseEncapsulator>
   * Compare class = EncapKeyCompare
   */
  typedef std::map<Ptr<EncapKey>, Ptr<SatBaseEncapsulator>, EncapKeyCompare > EncapContainer_t;

  /**
   * \brief Receive callback used for sending packet to netdevice layer.
    * \param packet the packet received
    */
  typedef Callback<void,Ptr<const Packet> > ReceiveCallback;

  /**
   * \brief Callback to read control messages from container storing control messages.
   * Real length of the control messages are simulated in a packet, but not structure.
   * \param uint32_t ID of the message to read.
   * \return Pointer to read packet. (NULL if not found).
   */
  typedef Callback<Ptr<SatControlMessage>, uint32_t> ReadCtrlMsgCallback;

  /**
   * \brief Method to set read control message callback.
   * \param cb callback to invoke whenever a control message is wanted to read.
   */
  void SetReadCtrlCallback (SatLlc::ReadCtrlMsgCallback cb);

  /**
    * \brief Called from higher layer (SatNetDevice) to enque packet to LLC
    *
    * \param packet packet sent from above down to SatMac
    * \param dest Destination MAC address of the packet
    * \param flowId Flow identifier
    * \return Boolean indicating whether the enque operation succeeded
    */
  virtual bool Enque (Ptr<Packet> packet, Address dest, uint8_t flowId);

  /**
    * \brief Called from lower layer (MAC) to inform a Tx
    * opportunity of certain amount of bytes. Note, that this
    * method is not to be used in this class, but the real
    * implementation is located in inherited classes.
    *
    * \param bytes Size of the Tx opportunity
    * \param utAddr MAC address of the UT with the Tx opportunity
    * \param flowId Flow identifier
    * \param &bytesLeft Bytes left after TxOpportunity
    * \param &nextMinTxO Minimum TxO after this TxO
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t flowId, uint32_t &bytesLeft, uint32_t &nextMinTxO) = 0;

  /**
   * \brief Receive user data packet from lower layer.
   * \param packet Pointer to packet received.
   * \param source MAC address of the source
   * \param dest MAC address of the destination
   */
  virtual void Receive (Ptr<Packet> packet, Mac48Address source, Mac48Address dest);

  /**
   * \brief Receive HL PDU from encapsulator/decapsulator entity
   *
   * \param packet Pointer to packet received.
   * \param source MAC address of the source
   * \param dest MAC address of the destination
   */
  virtual void ReceiveHigherLayerPdu (Ptr<Packet> packet, Mac48Address source, Mac48Address dest);

  /**
   * \brief Set Receive callback to forward packet to upper layer
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetReceiveCallback (SatLlc::ReceiveCallback cb);

  /**
   * \brief Add an encapsulator entry for the LLC. This is called from the helpers
   * at initialization phase.
   * \param source Source MAC address
   * \param dest Destination MAC address
   * \param flowId Flow id of this encapsulator queue
   * \param enc Encapsulator pointer
   */
  void AddEncap (Mac48Address source, Mac48Address dest, uint8_t flowId, Ptr<SatBaseEncapsulator> enc);

  /**
   * \brief Add an decapsulator entry for the LLC. This is called from the helpers
   * at initialization phase.
   * \param source Source MAC address
   * \param dest Destination MAC address
   * \param flowId Flow id of this encapsulator queue
   * \param dec Decapsulator pointer
   */
  void AddDecap (Mac48Address source, Mac48Address dest, uint8_t flowId, Ptr<SatBaseEncapsulator> dec);

  /**
   * \brief Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Create and fill the scheduling objects based on LLC layer information.
   * Scheduling objects may be used at the MAC layer to assist in scheduling.
   * This is a pure virtual method to be implemented to inherited classes.
   * \param output reference to an output vector that will be filled with
   *               pointer to scheduling objects
   */
  virtual void GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const = 0;

  /**
   * \brief Are buffers empty?
   * \return Boolean to indicate whether the buffers are empty or not.
   */
  virtual bool BuffersEmpty () const;

  /**
   * \brief Are buffers empty?
   * \return Boolean to indicate whether the buffers are empty or not.
   */
  virtual bool ControlBuffersEmpty () const;

  /**
   * \brief Get the number of (new) bytes at LLC queue for a certain UT. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ). This is a pure
   * virtual method to be implemented to inherited classes.
   * \param utAddress the MAC address that identifies a particular UT node.
   * \return Number of bytes currently queued in the encapsulator(s)
   *         associated with the UT.
   */
  virtual uint32_t GetNBytesInQueue (Mac48Address utAddress) const = 0;

  /**
    \brief Get the number of (new) packets at LLC queues for a certain UT. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ). This is a pure
   * virtual method to be implemented to inherited classes.
   * \param utAddress the MAC address that identifies a particular UT node.
   * \return Number of packets currently queued in the encapsulator(s)
   *         associated with the UT.
   */
  virtual uint32_t GetNPacketsInQueue (Mac48Address utAddress) const = 0;

  /**
   * \brief Get the total number of (new) bytes in all encapsulators. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ).
   * \return Total number of bytes currently queued in all the encapsulators.
   */
  virtual uint32_t GetNBytesInQueue () const;

  /**
   * \brief Get the total number of (new) packets in all encapsulators. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ).
   * \return Total number of packets currently queued in all the encapsulators.
   */
  virtual uint32_t GetNPacketsInQueue () const;

  /**
   * \param cb callback to send control messages.
   */
  void SetCtrlMsgCallback (SatBaseEncapsulator::SendCtrlCallback cb);

  /**
   * \brief Set the GW address
   * \param address GW MAC address
   */
  void SetGwAddress (Mac48Address address);

protected:

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Virtual method to create a new encapsulator 'on-a-need-basis' dynamically.
   * Method is implemented in the inherited class which knows which type of encapsulator to create.
   * This is a pure virtual method to be implemented to inherited classes.
   * \param key Encapsulator key class
   */
  virtual void CreateEncap (Ptr<EncapKey> key) = 0;

  /**
   * \brief Virtual method to create a new decapsulator 'on-a-need-basis' dynamically.
   * Method is implemented in the inherited class which knows which type of decapsulator to create.
   * This is a pure virtual method to be implemented to inherited classes.
   * \param key Encapsulator key class
   */
  virtual void CreateDecap (Ptr<EncapKey> key) = 0;

  /**
   * \brief Receive a control msg (ARQ ACK) from lower layer.
   * \param ack ARQ ACK message
   * \param source Source MAC address
   * \param dest Destination MAC address
   */
  virtual void ReceiveAck (Ptr<SatArqAckMessage> ack, Mac48Address source, Mac48Address dest);

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

  /**
   * Map of encapsulator base pointers
   */
  EncapContainer_t m_encaps;

  /**
   * Map of decapsulator base pointers
   */
  EncapContainer_t m_decaps;

  /**
   * Is FWD link ARQ enabled
   */
  bool m_fwdLinkArqEnabled;

  /**
   * Is RTN link ARQ enabled
   */
  bool m_rtnLinkArqEnabled;

  /**
   * GW address
   */
  Mac48Address m_gwAddress;

  /**
   * The upper layer package receive callback.
   */
  ReceiveCallback m_rxCallback;

  /**
   * The read control message callback.
   */
  SatLlc::ReadCtrlMsgCallback m_readCtrlCallback;

  /**
   * Callback to send control messages. Note, that this is not
   * actually used by the LLC but the encapsulators. It is just
   * stored here.
  */
  SatBaseEncapsulator::SendCtrlCallback m_sendCtrlCallback;

};

} // namespace ns3


#endif /* SATELLITE_LLC_H_ */
