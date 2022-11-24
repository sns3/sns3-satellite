/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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
 * (Based on point-to-point network device)
 * Author: Andre Aguas    March 2020
 *         Simon          2020
 * Adapted to SNS-3 by: Bastien Tauran <bastien.tauran@viveris.fr>
 * 
 */

#ifndef SATELLITE_POINT_TO_POINT_ISL_NET_DEVICE_H
#define SATELLITE_POINT_TO_POINT_ISL_NET_DEVICE_H

#include <cstring>
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"

#include "ns3/satellite-geo-net-device.h"

namespace ns3 {

template <typename Item> class Queue;
class PointToPointIslChannel;
class ErrorModel;
class SatGeoNetDevice;

/**
 * \ingroup point-to-point
 * \class PointToPoinIsltNetDevice
 * \brief A Device for a Point to Point ISL Network Link.
 *
 * This PointToPointIslNetDevice class specializes the NetDevice abstract
 * base class.  Together with a PointToPointIslChannel (and a peer 
 * PointToPointIslNetDevice), the class models, with some level of 
 * abstraction, a generic point-to-point-isl or serial link.
 * Key parameters or objects that can be specified for this device 
 * include a queue, data rate, and interframe transmission gap (the 
 * propagation delay is set in the PointToPointIslChannel).
 */
class PointToPointIslNetDevice : public NetDevice
{
public:
  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId (void);

  /**
   * Construct a PointToPointIslNetDevice
   *
   * This is the constructor for the PointToPointIslNetDevice
   */
  PointToPointIslNetDevice ();

  /**
   * Destroy a PointToPointIslNetDevice
   *
   * This is the destructor for the PointToPointIslNetDevice.
   */
  virtual ~PointToPointIslNetDevice ();

  /**
   * Set the Data Rate used for transmission of packets.  The data rate is
   * set in the Attach () method from the corresponding field in the channel
   * to which the device is attached.  It can be overridden using this method.
   *
   * \param bps the data rate at which this object operates
   */
  void SetDataRate (DataRate bps);

  /**
   * Set the interframe gap used to separate packets.  The interframe gap
   * defines the minimum space required between packets sent by this device.
   *
   * \param t the interframe gap time
   */
  void SetInterframeGap (Time t);

  /**
   * Attach the device to a channel.
   *
   * \param ch Ptr to the channel to which this object is being attached.
   * \return true if the operation was successful (always true actually)
   */
  bool Attach (Ptr<PointToPointIslChannel> ch);

  /**
   * Attach a queue to the PointToPointIslNetDevice.
   *
   * The PointToPointIslNetDevice "owns" a queue that implements a queueing 
   * method such as DropTailQueue or RedQueue
   *
   * \param queue Ptr to the new queue.
   */
  void SetQueue (Ptr<Queue<Packet> > queue);

  /**
   * Get a copy of the attached Queue.
   *
   * \returns Ptr to the queue.
   */
  Ptr<Queue<Packet> > GetQueue (void) const;

  /**
   * Attach a receive ErrorModel to the PointToPointIslNetDevice.
   *
   * The PointToPointNetLaserDevice may optionally include an ErrorModel in
   * the packet receive chain.
   *
   * \param em Ptr to the ErrorModel.
   */
  void SetReceiveErrorModel (Ptr<ErrorModel> em);

  /**
   * Receive a packet from a connected PointToPointIslChannel.
   *
   * The PointToPointIslNetDevice receives packets from its connected channel
   * and forwards them up the protocol stack.  This is the public method
   * used by the channel to indicate that the last bit of a packet has 
   * arrived at the device.
   *
   * \param p Ptr to the received packet.
   */
  void Receive (Ptr<Packet> p);

  /**
   * Set the associated GeoNetDevice
   * \param geoNetDevice The device to attach to this instance
   */
  void SetGeoNetDevice (Ptr<SatGeoNetDevice> geoNetDevice);

  // The remaining methods are documented in ns3::NetDevice*

  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;

  virtual Ptr<Channel> GetChannel (void) const;

  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;

  virtual void SetDestinationNode (Ptr<Node> node);
  virtual Ptr<Node> GetDestinationNode (void) const;

  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;

  virtual bool IsLinkUp (void) const;

  virtual void AddLinkChangeCallback (Callback<void> callback);

  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;

  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;

  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;

  virtual bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);

  virtual bool NeedsArp (void) const;

  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

private:

  /**
   * \brief Assign operator
   *
   * The method is private, so it is DISABLED.
   *
   * \param o Other NetDevice
   * \return New instance of the NetDevice
   */
  PointToPointIslNetDevice& operator = (const PointToPointIslNetDevice &o);

  /**
   * \brief Copy constructor
   *
   * The method is private, so it is DISABLED.

   * \param o Other NetDevice
   */
  PointToPointIslNetDevice (const PointToPointIslNetDevice &o);

  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

  /**
   * \returns the address of the remote device connected to this device
   * through the point to point channel.
   */
  Address GetRemote (void) const;

  /**
   * Adds the necessary headers and trailers to a packet of data in order to
   * respect the protocol implemented by the agent.
   * \param p packet
   * \param protocolNumber protocol number
   */
  void AddHeader (Ptr<Packet> p, uint16_t protocolNumber);

  /**
   * Removes, from a packet of data, all headers and trailers that
   * relate to the protocol implemented by the agent
   * \param p Packet whose headers need to be processed
   * \param param An integer parameter that can be set by the function
   * \return Returns true if the packet should be forwarded up the
   * protocol stack.
   */
  bool ProcessHeader (Ptr<Packet> p, uint16_t& param);

  /**
   * Start Sending a Packet Down the Wire.
   *
   * The TransmitStart method is the method that is used internally in the
   * PointToPointIslNetDevice to begin the process of sending a packet out on
   * the channel.  The corresponding method is called on the channel to let
   * it know that the physical device this class represents has virtually
   * started sending signals.  An event is scheduled for the time at which
   * the bits have been completely transmitted.
   *
   * \see PointToPointIslChannel::TransmitStart ()
   * \see TransmitComplete()
   * \param p a reference to the packet to send
   * \returns true if success, false on failure
   */
  bool TransmitStart (Ptr<Packet> p);

  /**
   * Stop Sending a Packet Down the Wire and Begin the Interframe Gap.
   *
   * The TransmitComplete method is used internally to finish the process
   * of sending a packet out on the channel.
   */
  void TransmitComplete (void);

  /**
   * \brief Make the link up and running
   *
   * It calls also the linkChange callback.
   */
  void NotifyLinkUp (void);

  /**
   * Enumeration of the states of the transmit machine of the net device.
   */
  enum TxMachineState
  {
    READY,   /**< The transmitter is ready to begin transmission of a packet */
    BUSY     /**< The transmitter is busy transmitting a packet */
  };

  static const uint16_t DEFAULT_MTU = 1500;             //!< Default MTU

  TxMachineState m_txMachineState;                      //!< The state of the Net Device transmit state machine
  DataRate m_dataRate;                                  //!< The data rate that the Net Device uses to simulate packet transmission timing
  Time m_tInterframeGap;                                //!< The interframe gap that the Net Device uses to throttle packet transmission
  Ptr<PointToPointIslChannel> m_channel;                //!< The PointToPointIslChannel to which this PointToPointIslNetDevice has been attached
  Ptr<Queue<Packet> > m_queue;                          //!< The Queue which this PointToPointIslNetDevice uses as a packet source.
                                                        //   Management of this Queue has been delegated to the PointToPointIslNetDevice
                                                        //   and it has the responsibility for deletion
  Ptr<ErrorModel> m_receiveErrorModel;                  //!< Error model for receive packet events
  Ptr<Node> m_node;                                     //!< Node owning this NetDevice
  Ptr<Node> m_destinationNode;                          //!< Node at the other end of the p2pIslLink
  Mac48Address m_address;                               //!< Mac48Address of this NetDevice
  NetDevice::ReceiveCallback m_rxCallback;              //!< Receive callback
  NetDevice::PromiscReceiveCallback m_promiscCallback;  //!< Receive callback
                                                        //   (promisc data)
  uint32_t m_ifIndex;                                   //!< Index of the interface
  bool m_linkUp;                                        //!< Identify if the link is up or not
  uint32_t m_mtu;                                       //!< The Maximum Transmission Unit
  Ptr<Packet> m_currentPkt;                             //!< Current packet processed

  Ptr<SatGeoNetDevice> m_geoNetDevice;                  //!< Satellite GEO Net Device associated to this instance

  /**
   * \brief PPP to Ethernet protocol number mapping
   * \param protocol A PPP protocol number
   * \return The corresponding Ethernet protocol number
   */
  static uint16_t PppToEther (uint16_t protocol);

  /**
   * \brief Ethernet to PPP protocol number mapping
   * \param protocol An Ethernet protocol number
   * \return The corresponding PPP protocol number
   */
  static uint16_t EtherToPpp (uint16_t protocol);
};

} // namespace ns3

#endif /* SATELLITE_POINT_TO_POINT_ISL_NET_DEVICE_H */
