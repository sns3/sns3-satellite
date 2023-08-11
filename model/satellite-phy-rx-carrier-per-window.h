/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Joaquin Muguerza <jmuguerza@toulouse.viveris.fr>
 */

#ifndef SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H
#define SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H

#include "satellite-crdsa-replica-tag.h"
#include "satellite-phy-rx-carrier-per-slot.h"
#include "satellite-phy-rx-carrier.h"
#include "satellite-rtn-link-time.h"

#include <ns3/singleton.h>

namespace ns3
{

class Address;
class SatPhy;
class SatSignalParameters;
class SatLinkResults;
class SatChannelEstimationErrorContainer;
class SatNodeInfo;
class SatPhyRxCarrier;
class SatPhyRxCarrierPerSlot;
class SatCrdsaReplicaTag;

/**
 * \ingroup satellite
 * \brief Inherited the functionality of ground station SatPhyRxCarriers
 *                              and extended it with CRDSA functionality.
 */
class SatPhyRxCarrierPerWindow : public SatPhyRxCarrierPerSlot
{
  public:
    /**
     * \brief Struct for storing the E-SSA packet specific Rx parameters
     */
    typedef struct
    {
        Ptr<SatSignalParameters> rxParams;
        Mac48Address destAddress;
        Mac48Address sourceAddress;
        bool hasBeenDecoded;
        bool hasBeenTreatedInWindow;
        bool failedSic;
        bool hasBeenUpdated;
        bool isInsideWindow;
        double meanSinr;
        double preambleMeanSinr;
        std::vector<std::pair<double, double>> gamma;
        Time arrivalTime;
        Time duration;
    } essaPacketRxParams_s;

    /**
     * Constructor.
     * \param carrierId ID of the carrier
     * \param carrierConf Carrier configuration
     * \param waveformConf Waveform configuration
     * \param randomAccessEnabled Is this a RA carrier
     */
    SatPhyRxCarrierPerWindow(uint32_t carrierId,
                             Ptr<SatPhyRxCarrierConf> carrierConf,
                             Ptr<SatWaveformConf> waveformConf,
                             bool randomAccessEnabled);

    /**
     * \brief Destructor
     */
    virtual ~SatPhyRxCarrierPerWindow();

    /**
     * Get the TypeId of the class.
     * \return TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Function for initializing the window end scheduling
     */
    void BeginEndScheduling();

    /**
     * \brief Method for querying the type of the carrier
     */
    inline virtual CarrierType GetCarrierType()
    {
        return CarrierType::RA_ESSA;
    }

    /**
     * \brief Callback signature for `WindowLoadTrace` trace source.
     */
    typedef void (*WindowLoadTraceCallback)(double windowLoad);

  protected:
    /**
     * Receive a slot.
     */
    virtual void ReceiveSlot(SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets);

    ///////// TRACED CALLBACKS /////////
    /**
     * \brief `EssaRxError` trace source.
     *
     * Fired when a packet is decoded through Random Access ESSA.
     *
     * Contains the following information:
     * - number of upper layer packets in the received packet.
     * - the MAC48 address of the sender; and
     * - whether a physical error has occured.
     */
    TracedCallback<uint32_t, const Address&, bool> m_essaRxErrorTrace;

    /**
     * \brief `EssaRxError` trace source.
     *
     * Fired when a packet is treated through Random Access ESSA.
     *
     * Contains the following information:
     * - number of upper layer packets in the received packet.
     * - the MAC48 address of the sender; and
     * - whether a it was treated or not (not detected).
     */
    TracedCallback<uint32_t, const Address&, bool> m_essaRxCollisionTrace;

    /**
     * \brief `WindowLoad` trace source.
     *
     *  Fired when a Window Load measurement is performed.
     *
     *  Contains te following information:
     *  - The load of the current window.
     */
    TracedCallback<double> m_windowLoadTrace;

    /**
     * \brief Dispose implementation
     */
    virtual void DoDispose();

  private:
    typedef std::list<SatPhyRxCarrierPerWindow::essaPacketRxParams_s> packetList_t;
    /**
     * \brief Function for storing the received E-SSA packets
     */
    void AddEssaPacket(SatPhyRxCarrierPerWindow::essaPacketRxParams_s essaPacketParams);

    /**
     * \brief Function for calculating the normalized offered random access load
     * \return Normalized offered load
     */
    double CalculateNormalizedOfferedRandomAccessLoad();

    /**
     * \brief Function for measuring the random access load
     */
    void MeasureRandomAccessLoad();

    /**
     * \brief Calculate gamma and Interference vectors for a single packet
     */
    void EliminatePreviousInterferences(SatPhyRxCarrierPerWindow::essaPacketRxParams_s& packet);

    /**
     * \brief Calculate gamma and Interference vectors for a single packet
     */
    void CalculatePacketInterferenceVectors(SatPhyRxCarrierPerWindow::essaPacketRxParams_s& packet);

    /**
     * \brief Remove old packets from packet container
     */
    void CleanOldPackets(const Time windowStartTime);

    /**
     * \brief Function called when a window ends
     */
    void DoWindowEnd();

    /**
     * \brief Perform SIC for a given decoded packet
     */
    void DoSic(packetList_t::iterator processedPacket,
               std::pair<packetList_t::iterator, packetList_t::iterator> windowBounds);

    /**
     * \brief Get the effective SNIR of the packet using the Mutual Information function
     */
    double GetEffectiveSnir(const SatPhyRxCarrierPerWindow::essaPacketRxParams_s& packet);

    /**
     * \brief Get the packet with the highst SNIR on the list
     * \return True if a packet is returned, False otherwise
     */
    packetList_t::iterator GetHighestSnirPacket(
        const std::pair<packetList_t::iterator, packetList_t::iterator> windowBounds);

    /**
     * \brief Get the normalized start and end time between two interfering packets
     */
    std::pair<double, double> GetNormalizedPacketInterferenceTime(
        const SatPhyRxCarrierPerWindow::essaPacketRxParams_s& packet,
        const SatPhyRxCarrierPerWindow::essaPacketRxParams_s& interferingPacket);

    /**
     * \brief Get a pair of iterators, pointing to the first element in the window, and to the first
     * after the window
     */
    std::pair<packetList_t::iterator, packetList_t::iterator> GetWindowBounds(Time startTime,
                                                                              Time endTime);

    /**
     * \brief Function for processing a window
     */
    bool PacketCanBeDetected(const SatPhyRxCarrierPerWindow::essaPacketRxParams_s& packet);

    /**
     * \brief Function for processing a window
     */
    void ProcessWindow(Time startTime, Time endTime);

    /**
     * The duration of the sliding window
     */
    Time m_windowDuration;

    /**
     * The step of the sliding window
     */
    Time m_windowStep;

    /**
     * The delay before processing a sliding window
     */
    Time m_windowDelay;

    /**
     * The time of the first window
     */
    Time m_firstWindow;

    /**
     * The number of SIC iterations performed on each window
     */
    uint32_t m_windowSicIterations;

    // TODO: REMOVE!
    /**
     * The spreading factor of packets
     */
    uint32_t m_spreadingFactor;

    /**
     * \brief Has the window end scheduling been initialized
     */
    bool m_windowEndSchedulingInitialized;

    /**
     * \brief SNIR detection threshold for a packet (in magnitude)
     */
    // TODO: find a more appropiate place for this parameter
    double m_detectionThreshold;

    /**
     * \brief ESSA packet container
     */
    packetList_t m_essaPacketContainer;

    /**
     * \brief The number of payload bytes in the window
     */
    uint32_t m_payloadBytesInWindow;

    /**
     * \brief Enable Sic
     */
    uint32_t m_sicEnabled;
};

//////////////////////////////////////////////////////////

} // namespace ns3

#endif /* SATELLITE_PHY_RX_CARRIER_PER_WINDOW_H */
