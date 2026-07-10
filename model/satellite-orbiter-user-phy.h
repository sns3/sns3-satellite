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
 *         Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_ORBITER_USER_PHY_H
#define SATELLITE_ORBITER_USER_PHY_H

#include "satellite-frame-conf.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

#include "ns3/address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <queue>
#include <stdint.h>
#include <tuple>

namespace ns3
{

class SatPhyRxCarrier;
class SatPhyRxCarrierUplink;
class SatPhyRxCarrierPerSlot;
class SatPhyRxCarrierPerFrame;
class SatPhyRxCarrierPerWindow;

/**
 * @ingroup satellite
 *
 * The SatOrbiterUserPhy models the user link physical layer of the
 * satellite node.
 */
class SatOrbiterUserPhy : public SatPhy
{
  public:
    /**
     * Default constructor
     */
    SatOrbiterUserPhy(void);

    SatOrbiterUserPhy(SatPhy::CreateParam_t& params,
                      Ptr<SatLinkResults> linkResults,
                      SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                      Ptr<SatSuperframeConf> superFrameConf);

    /**
     * Notifier called once the ObjectBase is fully constructed.
     *
     * This method is invoked once all member attributes have been
     * initialized. Subclasses can override this method to be notified
     * of this event but if they do this, they must chain up to their
     * parent's NotifyConstructionCompleted method.
     */
    virtual void NotifyConstructionCompleted() override;

    /**
     * Destructor for SatOrbiterUserPhy
     */
    virtual ~SatOrbiterUserPhy();

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
     * Send Pdu to the PHY tx module (for satellite switch packet forwarding)
     * @param rxParams Transmission parameters
     */
    virtual void SendPduWithParams(Ptr<SatSignalParameters> rxParams);

    /**
     * @brief Receives packets from lower layer.
     *
     * @param rxParams Packet reception parameters
     * @param phyError Boolean indicating whether the packet successfully
     * received or not? Note, that this parameter is not used in the satellite,
     * but exists since we are using a general interface defined in the parent
     * class.
     */
    virtual void Receive(Ptr<SatSignalParameters> rxParams, bool phyError);

    /**
     * @brief Get additional interference, used to compute final SINR at RX
     *
     * @return Additional interference
     */
    virtual double GetAdditionalInterference();

    /**
     * @brief Callback signature for `QueueSizeBytes` and `QueueSizePackets` trace source.
     * @param size number of bytes or number of packets of queue
     * @param from The MAC source address of packets
     */
    typedef void (*QueueSizeCallback)(uint32_t size, const Address& from);

    /**
     *
     */
    typedef Callback<bool, Ptr<SatControlMessage>, const Address&, Ptr<SatSignalParameters>>
        SendControlMsgToFeederCallback;

    /**
     * Callback to send ctrl packet on orbiter feeder
     */
    SatOrbiterUserPhy::SendControlMsgToFeederCallback m_txCtrlFeederCallback;

    /**
     * Set SendControlMsgToFeederCallback
     */
    void SetSendControlMsgToFeederCallback(SendControlMsgToFeederCallback cb);

  protected:
    /**
     * @brief Invoke the `Rx` trace source for each received packet.
     * @param packets Container of the pointers to the packets received.
     */
    virtual void RxTraces(SatPhy::PacketContainer_t packets);

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
     * Traced callback to monitor RTN feeder queue size in bytes.
     */
    TracedCallback<uint32_t, const Address&> m_queueSizeBytesTrace;

    /**
     * Traced callback to monitor RTN feeder queue size in packets.
     */
    TracedCallback<uint32_t, const Address&> m_queueSizePacketsTrace;

  private:
    /**
     *  Link results used for this physical layer.
     */
    Ptr<SatLinkResults> m_linkResults;

    /**
     *  RX carrier configuration parameters.
     */
    SatPhyRxCarrierConf::RxCarrierCreateParams_s m_parameters;

    /**
     *  Super frame cofiguration.
     */
    Ptr<SatSuperframeConf> m_superFrameConf;

    /**
     * Send a packet from the queue. Used only in REGENERATION_PHY mode.
     */
    void SendFromQueue();

    /**
     * Notify a packet has finished being sent. Used only in REGENERATION_PHY mode.
     */
    void EndTx();

    /**
     * Get destination address of packets.
     * @brief packets The packets from where extract destination
     * @return The destination MAC address
     */
    Address GetE2EDestinationAddress(SatPhy::PacketContainer_t packets);

    /**
     * Configured Adjacent Channel Interference (ACI) in dB.
     */
    double m_aciInterferenceCOverIDb;

    /**
     * Configured other system interference in dB.
     */
    double m_otherSysInterferenceCOverIDb;

    /**
     * Adjacent Channel Interference (ACI) in linear.
     */
    double m_aciInterferenceCOverI;

    /**
     * Other system interference in linear.
     */
    double m_otherSysInterferenceCOverI;

    /**
     * Regeneration mode on forward link.
     */
    SatEnums::RegenerationMode_t m_forwardLinkRegenerationMode;

    /**
     * Regeneration mode on return link.
     */
    SatEnums::RegenerationMode_t m_returnLinkRegenerationMode;

    /**
     * Simple FIFO queue to avoid collisions on TX in case of REGENERATION_PHY.
     * Second and third elements are respectively size in bytes and in packets.
     */
    std::queue<std::tuple<Ptr<SatSignalParameters>, uint32_t, uint32_t>> m_queue;

    /**
     * Size of FIFO queue in bytes
     */
    uint32_t m_queueSizeBytes;

    /**
     * Size of FIFO queue in packets
     */
    uint32_t m_queueSizePackets;

    /**
     * Maximum size of FIFO m_queue in bytes.
     */
    uint32_t m_queueSizeMax;

    /**
     * Indicates if a packet is already being sent.
     */
    bool m_isSending;
};

} // namespace ns3

#endif /* SATELLITE_ORBITER_USER_PHY_H */
