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

#ifndef SATELLITE_ORBITER_NET_DEVICE_DVB_H
#define SATELLITE_ORBITER_NET_DEVICE_DVB_H

#include "satellite-channel.h"
#include "satellite-isl-arbiter.h"
#include "satellite-mac.h"
#include "satellite-phy.h"
#include "satellite-point-to-point-isl-net-device.h"
#include "satellite-signal-parameters.h"

#include "ns3/error-model.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/traced-callback.h"

#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

namespace ns3
{

/**
 * @ingroup satellite
 * @brief SatOrbiterNetDeviceDvb to be utilized in geostationary satellite.
 * SatOrbiterNetDeviceDvb holds a set of phy layers towards user and feeder
 * links; one pair of phy layers for each spot-beam. The SatNetDevice
 * implements a simple switching between all user and feeder links
 * modeling transparent payload.
 */
class SatOrbiterNetDeviceDvb : public SatOrbiterNetDevice
{
  public:
    /**
     * @brief Get the type ID
     * @return the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * Default constructor.
     */
    SatOrbiterNetDeviceDvb();

    /**
     * @brief Receive the packet from the lower layers, in network regeneration on return link
     * @param packet Packet received
     * @param userAddress MAC address of user that received this packet
     */
    void ReceivePacketUser(Ptr<Packet> packet, const Address& userAddress);

    /**
     * @brief Receive the packet from the lower layers, in network regeneration on forward link
     * @param packet Packet received
     * @param feederAddress MAC address of feeder that received this packet
     */
    void ReceivePacketFeeder(Ptr<Packet> packet, const Address& feederAddress);

    /**
     * @brief Send a control packet on the feeder link
     * @param msg The control message to send
     * @param dest The MAC destination
     * @param rxParams Strucutre storing additional parameters
     * @return True if success
     */
    virtual bool SendControlMsgToFeeder(Ptr<SatControlMessage> msg,
                                        const Address& dest,
                                        Ptr<SatSignalParameters> rxParams);

    /**
     * Receive a packet from ISL.
     * @param packet The packet to send
     * @param destination The MAC address of ground station that will receive the packet
     */
    void ReceiveFromIsl(Ptr<Packet> packet, Mac48Address destination);

    /**
     * Connect a UT to this satellite.
     * @param utAddress MAC address of the UT to connect
     * @param beamId beam used by satellite to reach this UT
     */
    virtual void ConnectUt(Mac48Address utAddress, uint32_t beamId);

    /**
     * Disconnect a UT to this satellite.
     * @param utAddress MAC address of the UT to disconnect
     * @param beamId beam used by satellite to reach this UT
     */
    virtual void DisconnectUt(Mac48Address utAddress, uint32_t beamId);
};

} // namespace ns3

#endif /* SATELLITE_ORBITER_NET_DEVICE_DVB_H */
