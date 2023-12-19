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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SATELLITE_HANDOVER_MODULE_H
#define SATELLITE_HANDOVER_MODULE_H

#include "satellite-antenna-gain-pattern-container.h"

#include <ns3/callback.h>
#include <ns3/node-container.h>
#include <ns3/node.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

namespace ns3
{

/**
 * \ingroup satellite
 * \brief UT handover module
 */
class SatHandoverModule : public Object
{
  public:
    typedef enum
    {
        SAT_N_CLOSEST_SAT,
    } HandoverDecisionAlgorithm_t;

    /**
     * \brief Handover recommendation message sending callback
     * \param uint32_t The satellite ID this UT want to change to
     * \param uint32_t The beam ID this UT want to change to
     */
    typedef Callback<void, uint32_t, uint32_t> HandoverRequestCallback;

    /**
     * Derived from Object
     */
    static TypeId GetTypeId(void);

    /**
     * Derived from Object
     */
    virtual TypeId GetInstanceTypeId(void) const;

    /**
     * Dispose of this class instance
     */
    virtual void DoDispose();

    /**
     * Default constructor, which is not used.
     */
    SatHandoverModule();

    /**
     * Construct a SatHandoverModule
     * \param node The UT node linked to this module
     * \param satellites The list of satellites used in the scenario
     * \param agpContainer the antenna gain patterns of the simulation
     */
    SatHandoverModule(Ptr<Node> utNode,
                      NodeContainer satellites,
                      Ptr<SatAntennaGainPatternContainer> agpContainer);

    /**
     * Destroy a SatHandoverModule
     */
    ~SatHandoverModule();

    /**
     * \brief Set the handover recommendation message sending callback.
     * \param cb callback to send handover recommendation messages
     */
    void SetHandoverRequestCallback(SatHandoverModule::HandoverRequestCallback cb);

    /**
     * \brief Get the best sat ID
     * \return The best sat ID
     */
    uint32_t GetAskedSatId();

    /**
     * \brief Get the best beam ID
     * \return The best beam ID
     */
    uint32_t GetAskedBeamId();

    /**
     * \brief Method to call when a handover has been performed.
     */
    void HandoverFinished();

    /**
     * \brief Inspect whether or not the given beam is still suitable for
     * the underlying mobility model.
     * \param satId The current satellite ID the underlying mobility model is emitting in
     * \param beamId The current beam ID the underlying mobility model is emitting in
     * \return whether or not an handover recommendation has been sent
     */
    bool CheckForHandoverRecommendation(uint32_t satId, uint32_t beamId);

  private:
    /**
     * Get the N closest satellites to the UT node
     *
     * \param numberOfSats Number of satellites to get
     * \return Closest satellites IDs
     */
    std::vector<uint32_t> GetNClosestSats(uint32_t numberOfSats);

    /**
     * Handover algorithm choosing best beam between N closest satellites
     *
     * \param coords Coordiantes of UT
     * \return A pair containing satellite ID and beam ID
     */
    std::pair<uint32_t, uint32_t> AlgorithmNClosest(GeoCoordinate coords);

    /**
     * Algorithm used to detect if handover is needed
     */
    HandoverDecisionAlgorithm_t m_handoverDecisionAlgorithm;

    /**
     * Number of satellites to consider when using algorithm SAT_N_CLOSEST_SAT
     */
    uint32_t m_numberClosestSats;

    HandoverRequestCallback m_handoverCallback;

    Ptr<Node> m_utNode;
    NodeContainer m_satellites;
    Ptr<SatAntennaGainPatternContainer> m_antennaGainPatterns;

    Time m_lastMessageSentAt;
    Time m_repeatRequestTimeout;
    bool m_hasPendingRequest;
    uint32_t m_askedSatId;
    uint32_t m_askedBeamId;

    TracedCallback<double> m_antennaGainTrace;
};

} // namespace ns3

#endif /* SATELLITE_HANDOVER_MODULE_H */
