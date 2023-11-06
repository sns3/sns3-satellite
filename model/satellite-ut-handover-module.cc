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

#include "satellite-ut-handover-module.h"

#include "geo-coordinate.h"
#include "satellite-mobility-model.h"

#include <ns3/log.h>
#include <ns3/simulator.h>

NS_LOG_COMPONENT_DEFINE("SatUtHandoverModule");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatUtHandoverModule);

TypeId
SatUtHandoverModule::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatUtHandoverModule")
            .SetParent<Object>()
            .AddConstructor<SatUtHandoverModule>()
            .AddAttribute("Timeout",
                          "Amount of time to wait before sending a new handover recommendation if "
                          "no TIM-U is received",
                          TimeValue(MilliSeconds(600)),
                          MakeTimeAccessor(&SatUtHandoverModule::m_repeatRequestTimeout),
                          MakeTimeChecker())
            .AddTraceSource("AntennaGainTrace",
                            "Trace antenna gains when checking for beam compliance",
                            MakeTraceSourceAccessor(&SatUtHandoverModule::m_antennaGainTrace),
                            "ns3::SatAntennaGainPattern::AntennaGainTrace");
    return tid;
}

TypeId
SatUtHandoverModule::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

void
SatUtHandoverModule::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_handoverCallback.Nullify();
    m_antennaGainPatterns = NULL;

    Object::DoDispose();
}

SatUtHandoverModule::SatUtHandoverModule()
    : m_antennaGainPatterns(NULL),
      m_lastMessageSentAt(0),
      m_repeatRequestTimeout(600),
      m_hasPendingRequest(false),
      m_askedSatId(0),
      m_askedBeamId(0)
{
    NS_LOG_FUNCTION(this);

    NS_FATAL_ERROR("SatUtHandoverModule default constructor should not be used!");
}

SatUtHandoverModule::SatUtHandoverModule(Ptr<Node> utNode, NodeContainer satellites, Ptr<SatAntennaGainPatternContainer> agpContainer)
    : m_utNode(utNode),
      m_satellites(satellites),
      m_antennaGainPatterns(agpContainer),
      m_askedSatId(0),
      m_askedBeamId(0)
{
    NS_LOG_FUNCTION(this << agpContainer);
}

SatUtHandoverModule::~SatUtHandoverModule()
{
    NS_LOG_FUNCTION(this);
}

void
SatUtHandoverModule::SetHandoverRequestCallback(HandoverRequestCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);

    m_handoverCallback = cb;
}

uint32_t
SatUtHandoverModule::GetAskedBeamId()
{
    return m_askedBeamId;
}

bool
SatUtHandoverModule::CheckForHandoverRecommendation(uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << satId << beamId);

    if (m_askedSatId == satId && m_askedBeamId == beamId)
    {
        // In case TIM-U was received successfuly, the last asked beam should
        // match the current beamId. So reset the timeout feature.
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
        m_hasPendingRequest = false;
        return false;
    }

    // Current beam ID is no longer valid, check for better beam and ask for handover
    uint32_t bestSatId = m_askedSatId;
    uint32_t bestBeamId = m_askedBeamId;

    Time now = Simulator::Now();
    if (!m_hasPendingRequest || now - m_lastMessageSentAt > m_repeatRequestTimeout)
    {
        bestSatId = GetClosestSat();
        bestBeamId = m_antennaGainPatterns->GetBestBeamId(bestSatId, coords, false);
    }

    if ((bestSatId != satId || bestBeamId != beamId) &&
        (!m_hasPendingRequest || now - m_lastMessageSentAt > m_repeatRequestTimeout))
    {
        NS_LOG_FUNCTION("Sending handover recommendation for beam " << bestBeamId << " on sat " << satId);
        m_handoverCallback(bestBeamId, bestSatId);
        m_lastMessageSentAt = now;
        m_hasPendingRequest = true;
        m_askedSatId = bestSatId;
        m_askedBeamId = bestBeamId;
        return true;
    }

    return false;
}

bool SatUtHandoverModule::GetClosestSat()
{
    NS_LOG_FUNCTION(this);

    Ptr<SatMobilityModel> utMobility = m_utNode->GetObject<SatMobilityModel>();
    Ptr<SatMobilityModel> satMobility = m_satellites.Get(0)->GetObject<SatMobilityModel>();

    uint32_t closestSatId = m_satellites.Get(0)->GetId();
    double distanceClosest = utMobility->GetDistanceFrom(satMobility);

    double distance;

    for(uint32_t i = 1; i < m_satellites.GetN(); i++)
    {
        satMobility = m_satellites.Get(i)->GetObject<SatMobilityModel>();

        distance = utMobility->GetDistanceFrom(satMobility);
        if(distance < distanceClosest)
        {
            distanceClosest = distance;
            closestSatId = m_satellites.Get(i)->GetId();
        }
    }

    return closestSatId;
}

} // namespace ns3
