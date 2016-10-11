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

#ifndef SATELLITE_SIMPLE_NET_DEVICE_H
#define SATELLITE_SIMPLE_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/satellite-simple-channel.h"
#include "ns3/packet.h"
#include "ns3/error-model.h"
#include "ns3/traced-callback.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Satellite simple net device for satellite public and backbone network use.
 *        This device is based on ns-3 SimpleNetDevice. Main difference is that this device
 *        needs ARP and it has multicast. Receive method DO NOT pass packet to upper layer when
 *        packet type is PACKET_OTHERHOST.
 *
 * This device does not have a helper and assumes 48-bit mac addressing;
 * the default address assigned to each device is zero, so you must
 * assign a real address to use it.  There is also the possibility to
 * add an ErrorModel, if you want to force losses on the device.
 *
 */
class SatSimpleNetDevice : public NetDevice
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatSimpleNetDevice ();

  /**
   * Destructor for SatSimpleNetDevice
   */
  ~SatSimpleNetDevice ();

  /**
   * Receive a packet from a connected SatSimpleChannel. The
   * SatSimpleNetDevice receives packets from its connected channel
   * and then forwards them by calling its rx callback method
   *
   * \param packet Packet received on the channel
   * \param protocol protocol number
   * \param to address packet should be sent to
   * \param from address packet was sent from
   */
  void Receive (Ptr<Packet> packet, uint16_t protocol, Mac48Address to, Mac48Address from);

  /**
   *
   * Attach a channel to this net device.  This will be the
   * channel the net device sends on
   *
   * \param channel channel to assign to this net device
   *
   */
  void SetChannel (Ptr<SatSimpleChannel> channel);

  /**
   * Attach a receive ErrorModel to the SimpleNetDevice.
   *
   * The SimpleNetDevice may optionally include an ErrorModel in
   * the packet receive chain.
   *
   * \see ErrorModel
   * \param em Ptr to the ErrorModel.
   */
  void SetReceiveErrorModel (Ptr<ErrorModel> em);

  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
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
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

protected:

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose (void);
private:
  Ptr<SatSimpleChannel>             m_channel;
  NetDevice::ReceiveCallback        m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  Ptr<Node>                         m_node;
  uint16_t                          m_mtu;
  uint32_t                          m_ifIndex;
  Mac48Address                      m_address;
  Ptr<ErrorModel>                   m_receiveErrorModel;
  /**
   * The trace source fired when the phy layer drops a packet it has received
   * due to the error model being active.  Although SimpleNetDevice doesn't
   * really have a Phy model, we choose this trace source name for alignment
   * with other trace sources.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;
};

} // namespace ns3

#endif /* SATELLITE_SIMPLE_NET_DEVICE_H */
