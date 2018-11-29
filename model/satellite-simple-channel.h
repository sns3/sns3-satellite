/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
#ifndef SATELLITE_SIMPLE_CHANNEL_H
#define SATELLITE_SIMPLE_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/mac48-address.h"
#include <vector>

namespace ns3 {

class SatSimpleNetDevice;
class Packet;

/**
 * \ingroup satellite
 * \brief The satellite simple channel, for satellite public and backbone networks.
 *        Based on ns-3 SimpleChannel (implementation is identically same than in SimpleChannel,
 *        but needed to re-implemented in order to use by SatSimpleNetDevice)
 */
class SatSimpleChannel : public Channel
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
  SatSimpleChannel ();

  /**
   * A packet is sent by a net device.  A receive event will be
   * scheduled for all net device connected to the channel other
   * than the net device who sent the packet
   *
   * \param p packet to be sent
   * \param protocol protocol number
   * \param to address to send packet to
   * \param from address the packet is coming from
   * \param sender net device who sent the packet
   *
   */
  virtual void Send (Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from,
                     Ptr<SatSimpleNetDevice> sender);

  /**
   * Attached a net device to the channel.
   *
   * \param device the device to attach to the channel
   */
  virtual void Add (Ptr<SatSimpleNetDevice> device);

  // inherited from ns3::Channel
  virtual std::size_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

private:
  std::vector<Ptr<SatSimpleNetDevice> > m_devices;
};

} // namespace ns3

#endif /* SATELLITE_SIMPLE_CHANNEL_H */
