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
#ifndef SAT_CHANNEL_H
#define SAT_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include <vector>

namespace ns3 {

class SatNetDevice;
class Packet;

/**
 * \ingroup channel
 * \brief A Sat channel, as a skeleton and very simple first
 */
class SatChannel : public Channel
{
public:
  static TypeId GetTypeId (void);
  SatChannel ();

  void Send (Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from,
             Ptr<SatNetDevice> sender, Time txTime);

  void Add (Ptr<SatNetDevice> device);

  // inherited from ns3::Channel
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:
  std::vector<Ptr<SatNetDevice> > m_devices;
  Time          m_delay;
  /**
   * The trace source for the packet transmission animation events that the
   * device can fire.
   * Arguments to the callback are the packet, transmitting
   * net device, receiving net device, transmission time and
   * packet receipt time.
   *
   * @see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet>, // Packet being transmitted
                 Ptr<NetDevice>,    // Transmitting NetDevice
                 Ptr<NetDevice>,    // Receiving NetDevice
                 Time,              // Amount of time to transmit the pkt
                 Time               // Last bit receive time (relative to now)
                 > m_txrxPointToPoint;
};

} // namespace ns3

#endif /* SIMPLE_CHANNEL_H */
