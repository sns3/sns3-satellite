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

#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include <stdint.h>
#include <string>
#include <map>
#include "ns3/traced-callback.h"

namespace ns3 {


class SatPhy;
class Node;
class ErrorModel;
class SatChannel;

/**
 * \defgroup satellite Satellite Models
 *
 */

/**
 * \ingroup satellite
 * SatGeoNetDevice to be utilized in the UT, GW and satellite.
 */
class SatGeoNetDevice : public NetDevice
{
public:
  static TypeId GetTypeId (void);
  SatGeoNetDevice ();

  /*
   * Receive the packet from the lower layers
   * \param packet Pointer to the packet to be received.
   * \param beamId the id of the beam where packet is from
   */
  void ReceiveUser (Ptr<Packet> packet, uint16_t beamId);

  /*
   * Receive the packet from the lower layers
   * \param packet Pointer to the packet to be received.
   * \param beamId the id of the beam where packet is from
   */
  void ReceiveFeeder (Ptr<Packet> packet, uint16_t beamId);

  /*
   * Add the User Phy object for the beam
   * \param phy user phy object to add.
   * \param beamId the id of the beam to use phy for
   */
  void AddUserPhy (Ptr<SatPhy> phy, uint16_t beamId);

  /*
   * Add the Feeder Phy object for the beam
   * \param phy feeder phy object to add.
   * \param beamId the id of the beam to use phy for
   */
  void AddFeederPhy (Ptr<SatPhy> phy, uint16_t beamId);

  /**
   * Attach a receive ErrorModel to the SatGeoNetDevice.
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
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  virtual Ptr<Channel> GetChannel (void) const;

protected:
  virtual void DoDispose (void);

private:
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
  Ptr<Node> m_node;
  uint16_t m_mtu;
  uint32_t m_ifIndex;
  uint16_t m_phyCount;
  Mac48Address m_address;
  Ptr<ErrorModel> m_receiveErrorModel;
  std::map<uint16_t, Ptr<SatPhy> > m_userPhy;
  std::map<uint16_t, Ptr<SatPhy> > m_feederPhy;

  /**
   * The trace source fired when the phy layer drops a packet it has received
   * due to the error model being active.  Although SatGeoNetDevice doesn't
   * really have a Phy model, we choose this trace source name for alignment
   * with other trace sources.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;
};

} // namespace ns3

#endif /* SATELLITE_GEO_NET_DEVICE_H */
