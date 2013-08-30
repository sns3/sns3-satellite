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

#ifndef IDEAL_NET_DEVICE_H
#define IDEAL_NET_DEVICE_H

#include "ns3/simple-net-device.h"
#include "ns3/simple-channel.h"
#include "ns3/packet.h"
#include "ns3/error-model.h"

namespace ns3 {

/**
 * \ingroup netdevice
 *
 * This device does not have a helper and assumes 48-bit mac addressing;
 * the default address assigned to each device is zero, so you must 
 * assign a real address to use it.  There is also the possibility to
 * add an ErrorModel if you want to force losses on the device.
 * 
 * \brief simple net device for simple things and testing
 */
class IdealNetDevice : public SimpleNetDevice
{
public:
  static TypeId GetTypeId (void);
  IdealNetDevice ();

  bool Receive (Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t protocol, const Address &from);

  bool ReceivePromisc (Ptr<NetDevice> dev, Ptr<const Packet> packet, uint16_t protocol,
                                const Address &from, const Address &to, NetDevice::PacketType packetType);
  bool NeedsArp (void) const;
  void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  void SetPromiscReceiveCallback (PromiscReceiveCallback cb);

private:
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscCallback;
};

} // namespace ns3

#endif /* IDEAL_NET_DEVICE_H */
