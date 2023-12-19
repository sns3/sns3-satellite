/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_UT_PHY_H
#define SATELLITE_UT_PHY_H

#include "satellite-channel-estimation-error-container.h"
#include "satellite-frame-conf.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

#include <ns3/address.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>

namespace ns3
{

class SatPhyRxCarrier;
class SatPhyRxCarrierUplink;
class SatPhyRxCarrierPerSlot;
class SatPhyRxCarrierPerFrame;
class SatPhyRxCarrierPerWindow;

/**
 * \ingroup satellite
 *
 * The SatUtPhy models the physical layer of the User Terminal of the satellite system.
 */
class SatUtPhy : public SatPhy
{
  public:
    /**
     * Default constructor, which is not used.
     */
    SatUtPhy(void);

    /**
     * Constructor.
     * \param params Creation parameters
     * \param linkResults Link results
     * \param parameters Rx carrier creation parameters
     * \param superFrameConf Superframe configuration
     */
    SatUtPhy(SatPhy::CreateParam_t& params,
             Ptr<SatLinkResults> linkResults,
             SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
             Ptr<SatSuperframeConf> superFrameConf,
             SatEnums::RegenerationMode_t forwardLinkRegenerationMode);

    /**
     * Destructor
     */
    virtual ~SatUtPhy();

    /**
     * inherited from Object
     */
    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const;
    /**
     * Initialization of SatUtPhy
     */
    virtual void DoInitialize(void);

    /**
     * Dispose of SatUtPhy
     */
    virtual void DoDispose(void);

    /**
     * \brief Get additional interference, used to compute final SINR at RX
     *
     * \return Additional interference
     */
    virtual double GetAdditionalInterference();

    /**
     * \brief Change underlying SatChannel to send and receive data from a new satellite and beam
     * \param satId the new satellite to listen/send to
     * \param beamId the new beam to listen/send to
     */
    void PerformHandover(uint32_t satId, uint32_t beamId);

    /**
     * \brief Inform whether or not the underlying Tx channel is properly configured for
     * transmission
     */
    bool IsTxPossible(void) const;

    /**
     * \brief Receives packets from lower layer.
     * \param rxParams Packet reception parameters
     * \param phyError Boolean indicating whether the packet successfully
     * received or not?
     */
    void Receive(Ptr<SatSignalParameters> rxParams, bool phyError);

    /**
     * \brief Updates the slices subscription list.
     * \param slice The new slice to subscribe. If zero, reset the subscription list
     * (except slice zero used for control messages).
     */
    void UpdateSliceSubscription(uint8_t slice);

  private:
    /**
     * \brief Update the underlying SatChannel to send and receive data from
     * the current beam (as described in the m_beamId attribute).
     */
    void AssignNewSatChannels();

    /**
     * \brief Get the link TX direction. Must be implemented by child clases.
     * \return The link TX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkTxDir();

    /**
     * \brief Get the link RX direction. Must be implemented by child clases.
     * \return The link RX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkRxDir();

    /**
     * Configured other system interference in dB.
     */
    double m_otherSysInterferenceCOverIDb;

    /**
     * Other system interference in linear.
     */
    double m_otherSysInterferenceCOverI;

    /**
     * Delay of antenna reconfiguration when performing handover
     */
    Time m_antennaReconfigurationDelay;

    /**
     * The list of slices subscribed for this UT.
     */
    std::set<uint8_t> m_slicesSubscribed;
};

} // namespace ns3

#endif /* SATELLITE_UT_PHY_H */
