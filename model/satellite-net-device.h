/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#ifndef SATELLITE_NET_DEVICE_H
#define SATELLITE_NET_DEVICE_H

#include <stdint.h>
#include <string>

#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/net-device.h>
#include <ns3/mac48-address.h>
#include <ns3/traced-callback.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-packet-classifier.h>

namespace ns3 {


class SatPhy;
class SatMac;
class SatLlc;
class Node;
class ErrorModel;
class SatNodeInfo;
class SatControlMessage;


/**
 * \ingroup satellite
 * SatNetDevice to be utilized in the UT and GW nodes.
 */
class SatNetDevice : public NetDevice
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
  SatNetDevice ();

  /*
   * \brief Receive the packet from mac layer
   * \param packet Pointer to the packet to be received.
   */
  void Receive (Ptr<const Packet> packet);

  /*
   * \brief Attach the SatPhy physical layer to this netdevice.
   * \param phy SatPhy pointer to be added
   */
  void SetPhy (Ptr<SatPhy> phy);

  /**
   * \brief Get a Phy pointer
   *
   * \returns Ptr to the SatPhy object.
   */
  Ptr<SatPhy> GetPhy (void) const;

  /*
   * \brief Attach the SatMac mac layer to this netdevice.
   * \param mac SatMac pointer to be added
   */
  void SetMac (Ptr<SatMac> mac);

  /**
   * \brief Get a Mac pointer
   *
   * \returns Ptr to the SatMac object.
   */
  Ptr<SatMac> GetMac (void) const;

  /**
   * \brief Attach the SatLlc llc layer to this netdevice.
   * \param llc SatLlc pointer to be added
   */
  void SetLlc (Ptr<SatLlc> llc);

  /**
   * \brief Get Llc pointer
   *
   * \returns Ptr to the SatLlc object.
   */
  Ptr<SatLlc> GetLlc (void) const;

  /**
   * \brief Set the packet classifier class
   * \param classifier
   */
  void SetPacketClassifier (Ptr<SatPacketClassifier> classifier);

  /**
   * \brief Get a pointer to packet classifier class
   * \return Ptr<SatPacketClassifier Packet classifier
   */
  Ptr<SatPacketClassifier> GetPacketClassifier () const;

  /**
 * \brief Attach a receive ErrorModel to the SatNetDevice. Note,
 * that this method is not used in the satellite module, since the error
 * model is located within the implemented sublayers (PHY).
 * \param em Ptr to the ErrorModel.
 */
  void SetReceiveErrorModel (Ptr<ErrorModel> em);

  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
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
  bool SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  virtual Ptr<Channel> GetChannel (void) const;

  /**
   * \brief Set the node info
   * \param nodeInfo containing node specific information
   */
  void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Toggle the state of the device. This is used for beam hopping
   * purposes only, thus by default the netdevice is assumed to be enabled.
   * \param enabled Flag indicating whether the ND is considered enabled or disabled
   */
  void ToggleState (bool enabled);

protected:

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose (void);

private:
  Ptr<SatPhy> m_phy;
  Ptr<SatMac> m_mac;
  Ptr<SatLlc> m_llc;
  bool m_isStatisticsTagsEnabled;  ///< `EnableStatisticsTags` attribute.
  Ptr<SatPacketClassifier> m_classifier;
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  Mac48Address m_address;
  Ptr<ErrorModel> m_receiveErrorModel;

  Ptr<SatNodeInfo> m_nodeInfo;

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
   * Traced callback for all packets received to be transmitted
   */
  TracedCallback<Ptr<const Packet> > m_txTrace;

  /**
   * Traced callback for all signalling (control message) packets sent,
   * including the destination address.
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_signallingTxTrace;

  /**
   * Traced callback for all received packets, including the address of the
   * senders.
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  /**
   * Traced callback for all received packets, including delay information and
   * the address of the senders.
   */
  TracedCallback<const Time &, const Address &> m_rxDelayTrace;

};

} // namespace ns3

#endif /* SATELLITE_NET_DEVICE_H */
