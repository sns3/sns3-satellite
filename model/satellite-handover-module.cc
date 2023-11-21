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

#include "satellite-handover-module.h"

#include "geo-coordinate.h"
#include "satellite-mobility-model.h"

#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>

NS_LOG_COMPONENT_DEFINE("SatHandoverModule");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatHandoverModule);

TypeId
SatHandoverModule::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatHandoverModule")
            .SetParent<Object>()
            .AddConstructor<SatHandoverModule>()
            .AddAttribute("Timeout",
                          "Amount of time to wait before sending a new handover recommendation if "
                          "no TIM-U is received",
                          TimeValue(MilliSeconds(1000)),
                          MakeTimeAccessor(&SatHandoverModule::m_repeatRequestTimeout),
                          MakeTimeChecker())
            .AddAttribute("HandoverDecisionAlgorithm",
                          "Algorithm to use for handovers",
                          EnumValue(SatHandoverModule::SAT_N_CLOSEST_SAT),
                          MakeEnumAccessor(&SatHandoverModule::m_handoverDecisionAlgorithm),
                          MakeEnumChecker(SatHandoverModule::SAT_N_CLOSEST_SAT, "NClosestSats"))
            .AddAttribute("NumberClosestSats",
                          "Number of satellites to consider when using algorithm SAT_N_CLOSEST_SAT",
                          UintegerValue(1),
                          MakeUintegerAccessor(&SatHandoverModule::m_numberClosestSats),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("AntennaGainTrace",
                            "Trace antenna gains when checking for beam compliance",
                            MakeTraceSourceAccessor(&SatHandoverModule::m_antennaGainTrace),
                            "ns3::SatAntennaGainPattern::AntennaGainTrace");
    return tid;
}

TypeId
SatHandoverModule::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

void
SatHandoverModule::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_handoverCallback.Nullify();
    m_antennaGainPatterns = NULL;

    Object::DoDispose();
}

SatHandoverModule::SatHandoverModule()
    : m_antennaGainPatterns(NULL),
      m_lastMessageSentAt(0),
      m_repeatRequestTimeout(600),
      m_hasPendingRequest(false),
      m_askedSatId(0),
      m_askedBeamId(0)
{
    NS_LOG_FUNCTION(this);

    NS_FATAL_ERROR("SatHandoverModule default constructor should not be used!");
}

SatHandoverModule::SatHandoverModule(Ptr<Node> utNode,
                                     NodeContainer satellites,
                                     Ptr<SatAntennaGainPatternContainer> agpContainer)
    : m_handoverDecisionAlgorithm(SAT_N_CLOSEST_SAT),
      m_numberClosestSats(1),
      m_utNode(utNode),
      m_satellites(satellites),
      m_antennaGainPatterns(agpContainer),
      m_lastMessageSentAt(0),
      m_repeatRequestTimeout(600),
      m_hasPendingRequest(false),
      m_askedSatId(0),
      m_askedBeamId(0)
{
    NS_LOG_FUNCTION(this << agpContainer);
}

SatHandoverModule::~SatHandoverModule()
{
    NS_LOG_FUNCTION(this);
}

void
SatHandoverModule::SetHandoverRequestCallback(HandoverRequestCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);

    m_handoverCallback = cb;
}

uint32_t
SatHandoverModule::GetAskedSatId()
{
    return m_askedSatId;
}

uint32_t
SatHandoverModule::GetAskedBeamId()
{
    return m_askedBeamId;
}

void
SatHandoverModule::HandoverFinished()
{
    NS_LOG_FUNCTION(this);

    m_hasPendingRequest = false;
}

bool
SatHandoverModule::CheckForHandoverRecommendation(uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << satId << beamId);

    if (m_askedSatId == satId && m_askedBeamId == beamId)
    {
        // In case TIM-U was received successfuly, the last asked beam should
        // match the current beamId. So reset the timeout feature.
        // m_hasPendingRequest = false;
    }

    Time now = Simulator::Now();
    if (m_hasPendingRequest && (now - m_lastMessageSentAt > m_repeatRequestTimeout))
    {
        m_hasPendingRequest = false;
    }

    Ptr<SatMobilityModel> mobilityModel = GetObject<SatMobilityModel>();
    if (!mobilityModel)
    {
        NS_LOG_FUNCTION("Bailing out for lack of mobility model");
        return false;
    }

    // If current beam is still valid, do nothing
    GeoCoordinate coords = mobilityModel->GetGeoPosition();
    Ptr<SatAntennaGainPattern> agp = m_antennaGainPatterns->GetAntennaGainPattern(beamId);
    Ptr<SatMobilityModel> mobility = m_antennaGainPatterns->GetAntennaMobility(satId);
    if (agp->IsValidPosition(coords, m_antennaGainTrace, mobility))
    {
        NS_LOG_FUNCTION("Current beam is good, do nothing");
        // m_hasPendingRequest = false;
        return false;
    }

    // Current beam ID is no longer valid, check for better beam and ask for handover
    uint32_t bestSatId = m_askedSatId;
    uint32_t bestBeamId = m_askedBeamId;

    if (!m_hasPendingRequest || now - m_lastMessageSentAt > m_repeatRequestTimeout)
    {
        switch (m_handoverDecisionAlgorithm)
        {
        case SAT_N_CLOSEST_SAT: {
            std::pair<uint32_t, uint32_t> bestSatAndBeam = AlgorithmNClosest(coords);
            bestSatId = bestSatAndBeam.first;
            bestBeamId = bestSatAndBeam.second;
            break;
        }
        default:
            NS_FATAL_ERROR("Incorrect handover decision algorithm");
        }
    }

    if ((bestSatId != satId || bestBeamId != beamId) &&
        (!m_hasPendingRequest || now - m_lastMessageSentAt > m_repeatRequestTimeout))
    {
        NS_LOG_FUNCTION("Sending handover recommendation for beam " << bestBeamId << " on sat "
                                                                    << satId);
        m_handoverCallback(bestBeamId, bestSatId);
        m_lastMessageSentAt = now;
        m_hasPendingRequest = true;
        m_askedSatId = bestSatId;
        m_askedBeamId = bestBeamId;
        return true;
    }

    return false;
}

std::vector<uint32_t>
SatHandoverModule::GetNClosestSats(uint32_t numberOfSats)
{
    NS_LOG_FUNCTION(this << numberOfSats);

    std::map<double, uint32_t> satellites;

    Ptr<SatMobilityModel> utMobility = m_utNode->GetObject<SatMobilityModel>();
    Ptr<SatMobilityModel> satMobility;
    double distance;

    for (uint32_t i = 0; i < m_satellites.GetN(); i++)
    {
        satMobility = m_satellites.Get(i)->GetObject<SatMobilityModel>();
        distance = utMobility->GetDistanceFrom(satMobility);
        satellites.emplace(distance, i);
    }

    std::vector<uint32_t> closest;
    uint32_t i = 0;
    for (auto&& [dist, satId] : satellites)
    {
        if (i++ < numberOfSats)
        {
            closest.push_back(satId);
        }
        else
        {
            break;
        }
    }

    return closest;
}

std::pair<uint32_t, uint32_t>
SatHandoverModule::AlgorithmNClosest(GeoCoordinate coords)
{
    NS_LOG_FUNCTION(this);

    std::vector<uint32_t> satellites = GetNClosestSats(m_numberClosestSats);

    uint32_t bestSatId = satellites[0];
    uint32_t bestBeamId = m_antennaGainPatterns->GetBestBeamId(bestSatId, coords, true);
    double bestGain = m_antennaGainPatterns->GetBeamGain(bestSatId, bestBeamId, coords);

    uint32_t beamId;
    double gain;
    for (uint32_t i = 1; i < satellites.size(); i++)
    {
        beamId = m_antennaGainPatterns->GetBestBeamId(satellites[i], coords, true);
        gain = m_antennaGainPatterns->GetBeamGain(satellites[i], beamId, coords);

        if (beamId != 0)
        {
            if (gain > bestGain)
            {
                bestGain = gain;
                bestSatId = satellites[i];
                bestBeamId = beamId;
            }
        }
    }

    return std::make_pair(bestSatId, bestBeamId);
}

} // namespace ns3
