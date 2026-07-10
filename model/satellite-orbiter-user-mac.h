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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_ORBITER_USER_MAC_H
#define SATELLITE_ORBITER_USER_MAC_H

#include "satellite-fwd-link-scheduler.h"
#include "satellite-mac.h"
#include "satellite-orbiter-mac.h"
#include "satellite-orbiter-user-llc.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

#include "ns3/address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <set>
#include <stdint.h>

namespace ns3
{

/**
 * @ingroup satellite
 *
 * The SatOrbiterUserMac models the user link MAC layer of the
 * satellite node.
 */
class SatOrbiterUserMac : public SatOrbiterMac
{
  public:
    /**
     * Default constructor
     */
    SatOrbiterUserMac(void);

    /**
     * Construct a SatOrbiterUserMac
     *
     * This is the constructor for the SatOrbiterUserMac
     *
     * @param satId ID of sat for UT
     * @param beamId ID of beam for UT
     */
    SatOrbiterUserMac(uint32_t satId, uint32_t beamId);

    /**
     * Destructor for SatOrbiterUserMac
     */
    virtual ~SatOrbiterUserMac();

    /**
     * inherited from Object
     */
    static TypeId GetTypeId(void);
    virtual void DoInitialize(void);

    /**
     * Dispose of this class instance
     */
    virtual void DoDispose(void);

    /**
     * @brief Add new packet to the LLC queue.
     * @param packet Packets to be sent.
     */
    virtual void EnquePacket(Ptr<Packet> packet);

    /**
     * Receive packet from lower layer.
     *
     * @param packets Pointers to packets received.
     * @param rxParams The parameters associated to these packets.
     */
    void Receive(SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams);

    void ReceiveSignalingPacket(Ptr<Packet> packet);

    /**
     * Add a remote peer to this MAC
     *
     * @param address The MAC address of the peer
     * @return True if the peer has been added, false otherwise
     */
    virtual bool AddPeer(Mac48Address address);

    /**
     * Remove a remote peer from this MAC
     *
     * @param address The MAC address of the peer
     * @return True if the peer has been removed, false otherwise
     */
    virtual bool RemovePeer(Mac48Address address);

  private:
    /**
     * @brief Get the link TX direction. Must be implemented by child clases.
     * @return The link TX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkTxDir();

    /**
     * @brief Get the link RX direction. Must be implemented by child clases.
     * @return The link RX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkRxDir();

    /**
     * @brief Get the UT address associated to this RX packet.
     *        In this class, this is the source address
     * @param packet The packet to consider
     * @return The address of associated UT
     */
    virtual Address GetRxUtAddress(Ptr<Packet> packet);

    /**
     * Indicates if at least one device is connected in this beam.
     *
     * @return True if at least a device is connected, false otherwise
     */
    virtual bool HasPeer();

    /**
     * List of UT MAC connected to this MAC.
     */
    std::set<Mac48Address> m_peers;
};

} // namespace ns3

#endif /* SATELLITE_ORBITER_USER_MAC_H */
