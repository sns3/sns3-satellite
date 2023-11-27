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

#ifndef SATELLITE_GEO_MAC_H
#define SATELLITE_GEO_MAC_H

#include "satellite-beam-scheduler.h"
#include "satellite-fwd-link-scheduler.h"
#include "satellite-geo-user-llc.h"
#include "satellite-mac.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

#include <ns3/address.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>

namespace ns3
{

/**
 * \ingroup satellite
 *
 * The SatGeoMac models the global link MAC layer of the satellite node.
 */
class SatGeoMac : public SatMac
{
  public:
    /**
     * Default constructor
     */
    SatGeoMac(void);

    /**
     * Construct a SatGeoMac
     *
     * This is the constructor for the SatGeoMac
     *
     * \param satId ID of sat for UT
     * \param beamId ID of beam for UT
     * \param forwardLinkRegenerationMode Forward link regeneration mode
     * \param returnLinkRegenerationMode Return link regeneration mode
     */
    SatGeoMac(uint32_t satId,
              uint32_t beamId,
              SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
              SatEnums::RegenerationMode_t returnLinkRegenerationMode);

    /**
     * Destructor for SatGeoMac
     */
    virtual ~SatGeoMac();

    /**
     * inherited from Object
     */
    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const;
    virtual void DoInitialize(void);

    /**
     * Dispose of this class instance
     */
    virtual void DoDispose(void);

    /**
     * Starts periodical transmissions. Called when MAC is wanted to take care of periodic sending.
     */
    void StartPeriodicTransmissions();

    /**
     * \brief Add new packet to the LLC queue.
     * \param packet Packets to be sent.
     */
    virtual void EnquePacket(Ptr<Packet> packet) = 0;

    /**
     * Method to set link scheduler (forward or SCPC)
     * \param The scheduler to use
     */
    void SetFwdScheduler(Ptr<SatFwdLinkScheduler> fwdScheduler);

    /**
     * Set the Geo LLC associated to this Geo MAC layer
     */
    void SetLlc(Ptr<SatGeoLlc> llc);

    virtual Time GetGuardTime() const;
    virtual void SetGuardTime(Time guardTime);

    typedef Callback<void, Ptr<SatSignalParameters>> TransmitCallback;

    void SetTransmitCallback(SatGeoMac::TransmitCallback cb);

    typedef Callback<void, SatPhy::PacketContainer_t, Ptr<SatSignalParameters>>
        ReceiveNetDeviceCallback;

    void SetReceiveNetDeviceCallback(SatGeoMac::ReceiveNetDeviceCallback cb);

    /**
     * Callback to get the SatBeamScheduler from the beam ID for handover
     */
    typedef Callback<Ptr<SatBeamScheduler>, uint32_t, uint32_t> BeamSchedulerCallback;

    /**
     * \brief Set the beam scheduler callback
     * \param cb Callback to get the SatBeamScheduler
     */
    void SetBeamSchedulerCallback(SatGeoMac::BeamSchedulerCallback cb);

    /**
     * Stop periodic transmission, until a pacquet in enqued.
     */
    virtual void StopPeriodicTransmissions();

    /**
     * Add a remote peer to this MAC
     *
     * \param address The MAC address of the peer
     * \return True if the peer has been added, false otherwise
     */
    virtual bool AddPeer(Mac48Address address) = 0;

    /**
     * Remove a remote peer from this MAC
     *
     * \param address The MAC address of the peer
     * \return True if the peer has been removed, false otherwise
     */
    virtual bool RemovePeer(Mac48Address address) = 0;

  protected:
    /**
     * Start sending a Packet Down the Wire.
     *
     * The StartTransmission method is used internally in the
     * SatGeoMac to begin the process of sending a packet out on the PHY layer.
     *
     * \param carrierId id of the carrier.
     * \returns true if success, false on failure
     */
    void StartTransmission(uint32_t carrierId);

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
    virtual void RxTraces(SatPhy::PacketContainer_t packets);

    /**
     * \brief Get the link TX direction. Must be implemented by child clases.
     * \return The link TX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkTxDir() = 0;

    /**
     * \brief Get the link RX direction. Must be implemented by child clases.
     * \return The link RX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkRxDir() = 0;

    /**
     * \brief Get the UT address associated to this RX packet.
     *        May be source or destination address depending on link used
     * \param packet The packet to consider
     * \return The address of associated UT
     */
    virtual Address GetRxUtAddress(Ptr<Packet> packet) = 0;

    /**
     * Indicates if at least one device is connected in this beam.
     *
     * \return True if at least a device is connected, false otherwise
     */
    virtual bool HasPeer() = 0;

    /**
     * If true, the periodic calls of StartTransmission are not called when no
     * devices are connected to this MAC
     */
    bool m_disableSchedulingIfNoDeviceConnected;

    /**
     * Scheduler for the forward link.
     */
    Ptr<SatFwdLinkScheduler> m_fwdScheduler;

    /**
     * Guard time for BB frames. The guard time is modeled by shortening
     * the duration of a BB frame by a m_guardTime set by an attribute.
     */
    Time m_guardTime;

    /**
     * ID of sat for UT
     */
    uint32_t m_satId;

    /**
     * ID of beam for UT
     */
    uint32_t m_beamId;

    /**
     * LLC layer linked to this MAC
     */
    Ptr<SatGeoLlc> m_llc;

    /**
     * Trace for transmitted BB frames.
     */
    TracedCallback<Ptr<SatBbFrame>> m_bbFrameTxTrace;

    TransmitCallback m_txCallback;
    ReceiveNetDeviceCallback m_rxNetDeviceCallback;

    /**
     * Indicated if periodic transmission is enabled.
     */
    bool m_periodicTransmissionEnabled;
};

} // namespace ns3

#endif /* SATELLITE_GEO_MAC_H */
