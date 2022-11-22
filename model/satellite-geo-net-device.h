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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_GEO_NET_DEVICE_H
#define SATELLITE_GEO_NET_DEVICE_H

#include <map>
#include <stdint.h>
#include <string>

#include <ns3/error-model.h>
#include <ns3/output-stream-wrapper.h>
#include <ns3/net-device.h>
#include <ns3/mac48-address.h>
#include <ns3/traced-callback.h>

#include "satellite-phy.h"
#include "satellite-mac.h"
#include "satellite-channel.h"
#include "satellite-signal-parameters.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatGeoNetDevice to be utilized in geostationary satellite.
 * SatGeoNetDevice holds a set of phy layers towards user and feeder
 * links; one pair of phy layers for each spot-beam. The SatNetDevice
 * implements a simple switching between all user and feeder links
 * modeling transparent payload.
 */
class SatGeoNetDevice : public NetDevice
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
  SatGeoNetDevice ();

  /**
   * \brief Receive the packet from the lower layers, in network regeneration on return link
   * \param packet Packet received
   * \param userAddress MAC address of user that received this packet
   */
  void ReceivePacketUser (Ptr<Packet> packet, const Address& userAddress);

  /**
   * \brief Receive the packet from the lower layers, in network regeneration on forward link
   * \param packet Packet received
   * \param feederAddress MAC address of feeder that received this packet
   */
  void ReceivePacketFeeder (Ptr<Packet> packet, const Address& feederAddress);

  /**
   * \brief Receive the packet from the lower layers
   * \param packets Container of pointers to the packets to be received.
   * \param rxParams Packet transmission parameters
   */
  void ReceiveUser (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams);

  /**
   * Receive the packet from the lower layers
   * \param packets Container of pointers to the packets to be received.
   * \param rxParams Packet transmission parameters
   */
  void ReceiveFeeder (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams);

  /**
   * \brief Send a control packet on the feeder link
   * \param msg The control message to send
   * \param dest The MAC destination
   * \param rxParams Strucutre storing additional parameters
   * \return True if success
   */
  bool SendControlMsgToFeeder (Ptr<SatControlMessage> msg, const Address& dest, Ptr<SatSignalParameters> rxParams);

  /**
   * Add the User Phy object for the beam
   * \param phy user phy object to add.
   * \param beamId the id of the beam to use phy for
   */
  void AddUserPhy (Ptr<SatPhy> phy, uint32_t beamId);

  /**
   * Add the Feeder Phy object for the beam
   * \param phy feeder phy object to add.
   * \param beamId the id of the beam to use phy for
   */
  void AddFeederPhy (Ptr<SatPhy> phy, uint32_t beamId);

  /**
   * Get the User Phy object for the beam
   * \param beamId the id of the beam to use phy for
   * \return The User Phy
   */
  Ptr<SatPhy> GetUserPhy (uint32_t beamId);

  /**
   * Get the Feeder Phy object for the beam
   * \param beamId the id of the beam to use phy for
   * \return The Feeder Phy
   */
  Ptr<SatPhy> GetFeederPhy (uint32_t beamId);

  /**
   * Get all User Phy objects attached to this satellite
   * \return All the User Phy
   */
  std::map<uint32_t, Ptr<SatPhy> > GetUserPhy ();

  /**
   * Get all Feeder Phy objects attached to this satellite
   * \return All the Feeder Phy
   */
  std::map<uint32_t, Ptr<SatPhy> > GetFeederPhy ();

  /**
   * Add the User MAC object for the beam
   * \param mac user MAC object to add.
   * \param beamId the id of the beam to use MAC for
   */
  void AddUserMac (Ptr<SatMac> mac, uint32_t beamId);

  /**
   * Add the Feeder MAC object for the beam
   * \param mac feeder MAC created for this beam
   * \param macUsed feeder MAC that will be used to send data
   * \param beamId the id of the beam to use MAC for
   */
  void AddFeederMac (Ptr<SatMac> mac, Ptr<SatMac> macUsed, uint32_t beamId);

  /**
   * Get the User MAC object for the beam
   * \param beamId the id of the beam to use MAC for
   * \return The User MAC
   */
  Ptr<SatMac> GetUserMac (uint32_t beamId);

  /**
   * Get the Feeder MAC object for the beam
   * \param beamId the id of the beam to use MAC for
   * \return The Feeder MAC
   */
  Ptr<SatMac> GetFeederMac (uint32_t beamId);

  /**
   * Get all User MAC objects attached to this satellite
   * \return All the User MAC
   */
  std::map<uint32_t, Ptr<SatMac> > GetUserMac ();

  /**
   * Get all Feeder MAC objects attached to this satellite that are in use
   * \return All the Feeder MAC used to send on return feeder link
   */
  std::map<uint32_t, Ptr<SatMac> > GetFeederMac ();

  /**
   * Get all Feeder MAC objects attached to this satellite
   * \return All the Feeder MAC. They may or not be used to send on return feeder
   */
  std::map<uint32_t, Ptr<SatMac> > GetAllFeederMac ();

  /**
   * Add an entry in the database to get satellite feeder address from beam ID
   * \param beamId Beam ID
   * \param satelliteFeederAddress MAC address on the satellite feeder
   */
  void AddFeederPair (uint32_t beamId, Mac48Address satelliteFeederAddress);

  /**
   * Get satellite feeder entry from associated beam ID
   * \param beamId Beam ID
   * \return satellite feeder MAC associated to this beam
   */
  Mac48Address GetSatelliteFeederAddress (uint32_t beamId);

  /**
   * Attach a receive ErrorModel to the SatGeoNetDevice.
   * \param em Ptr to the ErrorModel.
   */
  void SetReceiveErrorModel (Ptr<ErrorModel> em);

  /**
   * Set the forward link regeneration mode.
   * \param forwardLinkRegenerationMode The regeneration mode.
   */
  void SetForwardLinkRegenerationMode (SatEnums::RegenerationMode_t forwardLinkRegenerationMode);

  /**
   * Set the return link regeneration mode.
   * \param returnLinkRegenerationMode The regeneration mode.
   */
  void SetReturnLinkRegenerationMode (SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  void SetNodeId (uint32_t nodeId);

  /**
   * Connect a GW to this satellite.
   * \param gwAddress MAC address of the GW to connect
   */
  void ConnectGw (Mac48Address gwAddress);

  /**
   * Disconnect a GW to this satellite.
   * \param gwAddress MAC address of the GW to disconnect
   */
  void DisconnectGw (Mac48Address gwAddress);

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
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  virtual Ptr<Channel> GetChannel (void) const;

protected:
  /**
   * Dispose of this class instance
   */
  virtual void DoDispose (void);

private:
  /**
   * Get UT MAC address associated to this packet.
   * May be source or destination depending on link
   */
  Address GetRxUtAddress (Ptr<Packet> packet, SatEnums::SatLinkDir_t ld);

  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  Mac48Address m_address;
  Ptr<ErrorModel> m_receiveErrorModel;
  std::map<uint32_t, Ptr<SatPhy> > m_userPhy;
  std::map<uint32_t, Ptr<SatPhy> > m_feederPhy;
  std::map<uint32_t, Ptr<SatMac> > m_userMac;
  std::map<uint32_t, Ptr<SatMac> > m_feederMac;

  std::map<uint32_t, Ptr<SatMac> > m_allFeederMac;

  std::map<uint32_t, Mac48Address > m_addressMapFeeder;

  SatEnums::RegenerationMode_t m_forwardLinkRegenerationMode;
  SatEnums::RegenerationMode_t m_returnLinkRegenerationMode;
  uint32_t m_nodeId;

  bool m_isStatisticsTagsEnabled;

  std::map <Mac48Address, Time> m_lastDelays;

  /**
   * Set containing all connected GWs
   */
  std::set <Mac48Address> m_gwConnected;

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
   * Traced callback for all received packets on feeder, including the address of the
   * senders.
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_rxFeederTrace;

  /**
   * Traced callback for all received packets on user, including the address of the
   * senders.
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_rxUserTrace;

  /**
   * Traced callback for all received packets, including feeder link delay information and
   * the address of the senders.
   */
  TracedCallback<const Time &, const Address &> m_rxFeederLinkDelayTrace;

  /**
   * Traced callback for all received packets, including feeder link jitter information and
   * the address of the senders.
   */
  TracedCallback<const Time &, const Address &> m_rxFeederLinkJitterTrace;

  /**
   * Traced callback for all received packets, including user link delay information and
   * the address of the senders.
   */
  TracedCallback<const Time &, const Address &> m_rxUserLinkDelayTrace;

  /**
   * Traced callback for all received packets, including user link jitter information and
   * the address of the senders.
   */
  TracedCallback<const Time &, const Address &> m_rxUserLinkJitterTrace;

};

} // namespace ns3

#endif /* SATELLITE_GEO_NET_DEVICE_H */
