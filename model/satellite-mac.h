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

#ifndef SAT_MAC_H
#define SAT_MAC_H

#include "satellite-control-message.h"
#include "satellite-handover-module.h"
#include "satellite-node-info.h"
#include "satellite-phy.h"
#include "satellite-queue.h"
#include "satellite-signal-parameters.h"

#include <ns3/address.h>
#include <ns3/callback.h>
#include <ns3/mac48-address.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>

#include <cstring>

namespace ns3
{

class Packet;
class SatBeamScheduler;

/**
 * \ingroup satellite
 * \brief Base MAC class for SatNetDevices. SatMac implements the common
 * MAC layer functionality for both UT and GW. More specialized implementations
 * are located at the inherited classes. Satellite does not have a MAC sublayer at all.
 * - SatUtMac
 * - SatGwMac
 */
class SatMac : public Object
{
  public:
    /**
     * Derived from Object
     */
    static TypeId GetTypeId(void);

    /**
     * Construct a SatMac
     *
     * This default constructor for the SatMac, which should not been used
     *
     */
    SatMac();

    /**
     * \brief Construct a SatMac
     * \param satId ID of sat for this MAC
     * \param beamId ID of beam for this MAC
     * \param forwardLinkRegenerationMode Forward link regeneration model
     * \param returnLinkRegenerationMode Return link regeneration model
     */
    SatMac(uint32_t satId,
           uint32_t beamId,
           SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
           SatEnums::RegenerationMode_t returnLinkRegenerationMode);

    /**
     * \brief Destroy a SatMac
     *
     * This is the destructor for the SatMac.
     */
    ~SatMac();

    /**
     * \brief Get sat ID of the object
     * \return sat ID
     */
    inline uint32_t GetSatId() const
    {
        return m_satId;
    }

    /**
     * \brief Get beam ID of the object
     * \return beam ID
     */
    inline uint32_t GetBeamId() const
    {
        return m_beamId;
    }

    /**
     * \brief Set sat ID of the object
     * \param satId sat ID
     */
    inline void SetSatId(uint32_t satId)
    {
        m_satId = satId;
    }

    /**
     * \brief Set beam ID of the object
     * \param beamId beam ID
     */
    inline void SetBeamId(uint32_t beamId)
    {
        m_beamId = beamId;
    }

    /**
     * \brief Get MAC address
     * \return The MAC address
     */
    inline Address GetAddress() const
    {
        return m_nodeInfo->GetMacAddress();
    }

    /**
     * \brief Set the handover module
     *
     * \param handoverModule The handover module
     */
    void SetHandoverModule(Ptr<SatHandoverModule> handoverModule);

    /**
     * \brief Callback to send packet to lower layer.
     * \param Container of the pointers to the packets received
     * \param uint32_t carrierId
     * \param  Time duration
     */
    typedef Callback<void, SatPhy::PacketContainer_t, uint32_t, Time, SatSignalParameters::txInfo_s>
        TransmitCallback;

    /**
     * \brief Method to set transmit callback.
     * \param cb callback to invoke whenever a packet needs to be sent
     * to lower layer (PHY)
     */
    void SetTransmitCallback(SatMac::TransmitCallback cb);

    /**
     * \brief Callback to receive packet by upper layer.
     * \param Source MAC address
     * \param Destination MAC address
     * \param packet the packet received
     */
    typedef Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> ReceiveCallback;

    /**
     * \brief Callback to receive packet by upper layer.
     * \param packet the packet received
     */
    typedef Callback<void, Ptr<const Packet>> LoraReceiveCallback;

    /**
     * \brief Method to set receive callback.
     * \param cb callback to invoke whenever a packet has been received and must
     *        be forwarded to the higher layers.
     */
    void SetReceiveCallback(SatMac::ReceiveCallback cb);

    /**
     * \brief Method to set receive callback.
     * \param cb callback to invoke whenever a packet has been received and must
     *        be forwarded to the higher layers.
     */
    void SetLoraReceiveCallback(SatMac::LoraReceiveCallback cb);

    /**
     * \brief Callback to read control messages from container storing control messages.
     * Real length of the control messages are simulated in a packet, but not structure.
     * \param uint32_t ID of the message to read.
     * \return Pointer to read packet. (NULL if not found).
     */
    typedef Callback<Ptr<SatControlMessage>, uint32_t> ReadCtrlMsgCallback;

    /**
     * \brief Callback to reserve an id and initially store the control
     * message.
     * \param Pointer to the message to be stored.
     * \return uint32_t ID of the stored message.
     */
    typedef Callback<uint32_t, Ptr<SatControlMessage>> ReserveCtrlMsgCallback;

    /**
     * \brief Callback to send a control message and allocate a recv ID for it.
     * \param uint32_t Internal ID used for initial storing of the control msg.
     * \return uint32_t ID for the control message read operation.
     */
    typedef Callback<uint32_t, uint32_t> SendCtrlMsgCallback;

    /**
     * Callback to get the SatBeamScheduler from the beam ID for handover
     */
    typedef Callback<Ptr<SatBeamScheduler>, uint32_t, uint32_t> BeamSchedulerCallback;

    /**
     * \brief Callback to update ISL routes when handovers are performed.
     */
    typedef Callback<void> UpdateIslCallback;

    /**
     * \brief Method to set read control message callback.
     * \param cb callback to invoke whenever a control message is wanted to read.
     */
    void SetReadCtrlCallback(SatMac::ReadCtrlMsgCallback cb);

    /**
     * \brief Method to set reserve control message id callback.
     * \param cb callback to invoke whenever an id is wanted to
     * be reserved for a control message.
     */
    void SetReserveCtrlCallback(SatMac::ReserveCtrlMsgCallback cb);

    /**
     * \brief Method to set send control message callback.
     * \param cb callback to invoke whenever a control message is wanted to sent.
     */
    void SetSendCtrlCallback(SatMac::SendCtrlMsgCallback cb);

    /**
     * \brief Set the beam scheduler callback
     * \param cb Callback to get the SatBeamScheduler
     */
    void SetBeamSchedulerCallback(SatMac::BeamSchedulerCallback cb);

    /**
     * \brief Method to set update ISL callback.
     * \param cb callback to invoke whenever ISL routes need to be updated.
     */
    void SetUpdateIslCallback(SatMac::UpdateIslCallback cb);

    /**
     * \brief Callback to update routing and ARP tables after handover
     * \param Address the address of this device
     * \param Address the address of the new gateway
     */
    typedef Callback<void, Address, Address> RoutingUpdateCallback;

    /**
     * \brief Method to set the routing update callback
     * \param cb callback to invoke to update routing
     */
    void SetRoutingUpdateCallback(SatMac::RoutingUpdateCallback cb);

    /**
     * \brief Set the node info
     * \param nodeInfo containing node specific information
     */
    virtual void SetNodeInfo(Ptr<SatNodeInfo> nodeInfo);

    /**
     * \brief Reserve id and store the control message.
     *
     * \param msg Control message to store to the container.
     * \return Id of the internally stored message (send ID).
     */
    uint32_t ReserveIdAndStoreCtrlMsgToContainer(Ptr<SatControlMessage> msg);

    /**
     * \brief Send the control message from the container.
     *
     * \param sendId Id of the internally stored message.
     * \return ID to be used in the control packet reception (recv ID).
     */
    uint32_t SendCtrlMsgFromContainer(uint32_t sendId);

    /**
     * \brief Receive a queue event:
     * - FIRST_BUFFER_RCVD
     * - BUFFER_EMPTY
     * \param event Queue event from SatQueue
     * \param flowIndex Identifier of the queue
     */
    virtual void ReceiveQueueEvent(SatQueue::QueueEvent_t event, uint8_t flowIndex);

    /**
     * \brief Enable the MAC layer, i.e. allow it to send data to the PHY layer.
     * This is used in beam hopping implementation.
     */
    virtual void Enable();

    /**
     * \brief Disable the MAC layer, i.e. disallow it to send data to the PHY layer.
     * This is used in beam hopping implementation.
     */
    virtual void Disable();

    /**
     * Set the satellite MAC address on the other side of this link (if regenerative satellite).
     */
    virtual void SetSatelliteAddress(Address satelliteAddress);

  private:
    SatMac& operator=(const SatMac&);
    SatMac(const SatMac&);

  protected:
    /**
     * Dispose of SatMac
     */
    void DoDispose(void);

    /**
     * \brief Set SatMacTimeTag of packets
     * \param packets Container of the pointers to the packets to tag.
     */
    void SetTimeTag(SatPhy::PacketContainer_t packets);

    /**
     * \brief Send packets to lower layer by using a callback
     * \param packets Packets to be sent.
     * \param carrierId ID of the carrier used for transmission.
     * \param duration Duration of the physical layer transmission.
     * \param txInfo Additional parameterization for burst transmission.
     */
    virtual void SendPacket(SatPhy::PacketContainer_t packets,
                            uint32_t carrierId,
                            Time duration,
                            SatSignalParameters::txInfo_s txInfo);

    /**
     * \brief Invoke the `Rx` trace source for each received packet.
     * \param packets Container of the pointers to the packets received.
     */
    void RxTraces(SatPhy::PacketContainer_t packets);

    /**
     * The lower layer packet transmit callback.
     */
    SatMac::TransmitCallback m_txCallback;

    /**
     * The upper layer package receive callback.
     */
    SatMac::ReceiveCallback m_rxCallback;

    /**
     * The upper layer package receive callback.
     */
    SatMac::LoraReceiveCallback m_rxLoraCallback;

    /**
     * The read control message callback.
     */
    SatMac::ReadCtrlMsgCallback m_readCtrlCallback;

    /**
     * The reserve control message id callback.
     */
    SatMac::ReserveCtrlMsgCallback m_reserveCtrlCallback;

    /**
     * The send control message callback.
     */
    SatMac::SendCtrlMsgCallback m_sendCtrlCallback;

    /**
     * Callback to get the SatBeamScheduler linked to a beam ID
     */
    SatMac::BeamSchedulerCallback m_beamSchedulerCallback;

    /**
     * The update ISL routes callback.
     */
    SatMac::UpdateIslCallback m_updateIslCallback;

    /**
     * `EnableStatisticsTags` attribute.
     */
    bool m_isStatisticsTagsEnabled;

    /**
     * Use of version 2 of NCR dates.
     * Version 1 use date of frame N-2.
     * Version 2 use date of frame N.
     */
    bool m_ncrV2;

    /**
     * Trace callback used for packet tracing.
     */
    TracedCallback<Time,
                   SatEnums::SatPacketEvent_t,
                   SatEnums::SatNodeType_t,
                   uint32_t,
                   Mac48Address,
                   SatEnums::SatLogLevel_t,
                   SatEnums::SatLinkDir_t,
                   std::string>
        m_packetTrace;

    /**
     * Traced callback for all received packets, including the address of the
     * senders.
     */
    TracedCallback<Ptr<const Packet>, const Address&> m_rxTrace;

    /**
     * Traced callback for all received packets, including delay information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxDelayTrace;

    /**
     * Traced callback for all received packets, including link delay information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxLinkDelayTrace;

    /**
     * Traced callback for all received packets, including jitter information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxJitterTrace;

    /**
     * Traced callback for all received packets, including link jitter information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxLinkJitterTrace;

    /**
     * Traced callback for beam being disabled and including service time.
     */
    TracedCallback<Time> m_beamServiceTrace;

    /**
     * Callback to update routing and ARP tables after a beam handover
     */
    SatMac::RoutingUpdateCallback m_routingUpdateCallback;

    /**
     * Node info containing node related information, such as
     * node type, node id and MAC address (of the SatNetDevice)
     */
    Ptr<SatNodeInfo> m_nodeInfo;

    /**
     * The ID of the sat where mac belongs.
     */
    uint32_t m_satId;

    /**
     * The ID of the beam where mac belongs.
     */
    uint32_t m_beamId;

    /**
     * Module used to perform handovers
     */
    Ptr<SatHandoverModule> m_handoverModule;

    /**
     * Flag indicating whether the MAC is enabled, i.e. it is capable/allowed to
     * transmit data to PHY layer. This is used for beam hopping, where forward link
     * is only supported currently, thus, it is set to true by default.
     */
    bool m_txEnabled;

    /**
     * Time of the last beam enable event.
     */
    Time m_beamEnabledTime;

    /**
     * Last delay measurement. Used to compute jitter.
     */
    Time m_lastDelay;

    /**
     * List of NCR control messages created but not sent yet.
     */
    std::queue<Ptr<SatNcrMessage>> m_ncrMessagesToSend;

    /**
     * Store last 3 SOF date for Forward messages, to insert in NCR packets.
     */
    std::queue<Time> m_lastSOF;

    /**
     * Regeneration mode on forward link.
     */
    SatEnums::RegenerationMode_t m_forwardLinkRegenerationMode;

    /**
     * Regeneration mode on return link.
     */
    SatEnums::RegenerationMode_t m_returnLinkRegenerationMode;

    /**
     * Indicate if satellite is regeneration (at least LINK level) for TX
     */
    bool m_isRegenerative;

    /**
     * MAC address of satellite on other side of the link. Use in regenerative cases.
     */
    Address m_satelliteAddress;

    /**
     * Last delay measurement for link. Used to compute link jitter.
     */
    Time m_lastLinkDelay;
};

} // namespace ns3

#endif /* SAT_MAC_H */
