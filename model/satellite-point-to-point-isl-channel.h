/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 * (Based on point-to-point channel)
 * Author: Andre Aguas    March 2020
 * Adapted to SNS-3 by: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#ifndef SATELLITE_POINT_TO_POINT_ISL_CHANNEL_H
#define SATELLITE_POINT_TO_POINT_ISL_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/satellite-point-to-point-isl-net-device.h"

namespace ns3
{

class PointToPointIslNetDevice;
class Packet;

/**
 * \brief Point to Point ISL Channel
 *
 * Channel connecting two satellites
 *
 * This class represents a very simple point to point channel.  Think full
 * duplex RS-232 or RS-422 with null modem and no handshaking.  There is no
 * multi-drop capability on this channel -- there can be a maximum of two
 * point-to-point net devices connected.
 *
 * There are two "wires" in the channel.  The first device connected gets the
 * [0] wire to transmit on.  The second device gets the [1] wire.  There is a
 * state (IDLE, TRANSMITTING) associated with each wire.
 *
 * (PointToPointChannel with mobile nodes)
 *
 */
class PointToPointIslChannel : public Channel
{
  public:
    /**
     * \brief Get the TypeId
     *
     * \return The TypeId for this class
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Create a PointToPointIslChannel
     *
     */
    PointToPointIslChannel();

    /**
     * \brief Attach a given netdevice to this channel
     *
     * \param device pointer to the netdevice to attach to the channel
     */
    void Attach(Ptr<PointToPointIslNetDevice> device);

    /**
     * \brief Transmit a packet over this channel
     *
     * \param p Packet to transmit
     * \param src source PointToPointIslNetDevice
     * \param dst node at the other end of the channel
     * \param txTime transmission time
     * \returns true if successful (always true)
     */
    virtual bool TransmitStart(Ptr<const Packet> p,
                               Ptr<PointToPointIslNetDevice> src,
                               Ptr<Node> dst,
                               Time txTime);

    /**
     * \brief Get number of devices on this channel
     *
     * \returns number of devices on this channel
     */
    virtual std::size_t GetNDevices(void) const;

    /**
     * \brief Get PointToPointIslNetDevice corresponding to index i on this channel
     *
     * \param i Index number of the device requested
     *
     * \returns Ptr to PointToPointIslNetDevice requested
     */
    Ptr<PointToPointIslNetDevice> GetPointToPointIslDevice(std::size_t i) const;

    /**
     * \brief Get NetDevice corresponding to index i on this channel
     *
     * \param i Index number of the device requested
     *
     * \returns Ptr to NetDevice requested
     */
    virtual Ptr<NetDevice> GetDevice(std::size_t i) const;

  protected:
    /**
     * \brief Get the delay between two nodes on this channel
     *
     * \param senderMobility location of the sender
     * \param receiverMobility location of the receiver
     *
     * \returns Time delay
     */
    Time GetDelay(Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> receiverMobility) const;

    /**
     * \brief Check to make sure the link is initialized
     *
     * \returns true if initialized, asserts otherwise
     */
    bool IsInitialized(void) const;

    /**
     * \brief Get the source net-device
     *
     * \param i the link (direction) requested
     *
     * \returns Ptr to source PointToPointIslNetDevice for the
     *          specified link
     */
    Ptr<PointToPointIslNetDevice> GetSource(uint32_t i) const;

    /**
     * \brief Get the destination net-device
     *
     * \param i the link requested
     * \returns Ptr to destination PointToPointIslNetDevice for
     *          the specified link
     */
    Ptr<PointToPointIslNetDevice> GetDestination(uint32_t i) const;

  private:
    /** Each point to point link has exactly two net devices. */
    static const std::size_t N_DEVICES = 2;

    double m_propagationSpeed; //!< propagation speed on the channel
    std::size_t m_nDevices;    //!< Devices of this channel

    /** \brief Wire states
     *
     */
    enum WireState
    {
        /** Initializing state */
        INITIALIZING,
        /** Idle state (no transmission from NetDevice) */
        IDLE,
        /** Transmitting state (data being transmitted from NetDevice. */
        TRANSMITTING,
        /** Propagating state (data is being propagated in the channel. */
        PROPAGATING
    };

    /**
     * \brief Wire model for the PointToPointIslChannel
     */
    class Link
    {
      public:
        /** \brief Create the link, it will be in INITIALIZING state
         *
         */
        Link()
            : m_state(INITIALIZING),
              m_src(0),
              m_dst(0)
        {
        }

        WireState m_state;                   //!< State of the link
        Ptr<PointToPointIslNetDevice> m_src; //!< First NetDevice
        Ptr<PointToPointIslNetDevice> m_dst; //!< Second NetDevice
    };

    Link m_link[N_DEVICES]; //!< Link model
};

} // namespace ns3

#endif /* SATELLITE_POINT_TO_POINT_ISL_CHANNEL_H */
