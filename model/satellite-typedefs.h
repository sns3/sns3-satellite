/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#ifndef SATELLITE_TYPEDEFS_H_
#define SATELLITE_TYPEDEFS_H_

#include <ns3/callback.h>
#include <ns3/ptr.h>
#include <ns3/satellite-enums.h>

namespace ns3 {

class Packet;
class Address;
class Time;

/**
 * \ingroup satellite
 *
 * \brief SatTypedefs class is for general typedefs used in satellite module.
 */
class SatTypedefs
{
public:
  /**
   * \brief Callback for carrier bandwidths
   * \param channelType     The type of the channel
   * \param carrierId       The id of the carrier
   * \param bandwidthType   The type of the bandwidth
   *
   * \return The bandwidth of the carrier.
   */
  typedef Callback<double, SatEnums::ChannelType_t, uint32_t, SatEnums::CarrierBandwidthType_t > CarrierBandwidthConverter_t;

  /**
   * \brief Common callback signature of a packet and a destination address.
   * \param packet the packet
   * \param destinationAddress the destination address
   */
  typedef void (*PacketDestinationAddressCallback)
    (Ptr<const Packet> packet, const Address &destinationAddress);

  /**
   * \brief Common callback signature of a packet and a source address.
   * \param packet the packet
   * \param sourceAddress the source address
   */
  typedef void (*PacketSenderAddressCallback)
    (Ptr<const Packet> packet, const Address &sourceAddress);

  /**
   * \brief Common callback signature for packet delay and address.
   * \param delay the packet delay
   * \param sourceAddress the source address
   */
  typedef void (*PacketDelayAddressCallback)
    (const Time &delay, const Address &sourceAddress);

  /**
   * \brief Common callback signature for `PacketTrace` trace sources.
   * \param now the current simulation time
   * \param eventType the type of this trace e.g., PACKET_SENT, PACKET_RECV,
   *                  PACKET_ENQUE, or PACKET_DROP
   * \param nodeType the type of the node where the event occured, e.g., NT_UT,
   *                 NT_SAT, NT_GW, NT_NCC, or NT_TER
   * \param nodeId the identifier of the node where the event occured
   * \param nodeMacAddress the MAC address of the node where the event occured
   * \param logLevel the log level used
   * \param linkDirection link direction, e.g., LD_FORWARD or LD_RETURN
   * \param packetInfo packet information in std::string for printing purpose,
   *                   typically contains the packet's Uid, source address,
   *                   and destination address
   * \todo Use const-reference for Time argument.
   */
  typedef void (*PacketTraceCallback)
    (Time                        now,
    SatEnums::SatPacketEvent_t  eventType,
    SatEnums::SatNodeType_t     nodeType,
    uint32_t                    nodeId,
    Mac48Address                nodeMacAddress,
    SatEnums::SatLogLevel_t     logLevel,
    SatEnums::SatLinkDir_t      linkDirection,
    std::string                 packetInfo);

  /**
   * \brief Common callback signature for scenario creation trace sources by
   *        helpers.
   * \param trace textual description of the creation being done
   */
  typedef void (*CreationCallback)
    (std::string trace);

  typedef void (*ServiceTimeCallback) (Time serviceTime);

private:
  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatTypedefs () = 0;
};


} // namespace ns3


#endif /* SATELLITE_TYPEDEFS_H_ */
