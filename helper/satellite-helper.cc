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

#include "satellite-helper.h"

#include <ns3/arp-cache.h>
#include <ns3/csma-helper.h>
#include <ns3/double.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-interface.h>
#include <ns3/ipv4-routing-table-entry.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/log.h>
#include <ns3/lora-device-address-generator.h>
#include <ns3/lora-forwarder-helper.h>
#include <ns3/lora-network-server-helper.h>
#include <ns3/mobility-helper.h>
#include <ns3/names.h>
#include <ns3/queue.h>
#include <ns3/satellite-env-variables.h>
#include <ns3/satellite-handover-module.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-log.h>
#include <ns3/satellite-lora-conf.h>
#include <ns3/satellite-point-to-point-isl-net-device.h>
#include <ns3/satellite-position-allocator.h>
#include <ns3/satellite-position-input-trace-container.h>
#include <ns3/satellite-rtn-link-time.h>
#include <ns3/satellite-sgp4-mobility-model.h>
#include <ns3/satellite-traced-mobility-model.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/system-path.h>
#include <ns3/type-id.h>

#include <sys/stat.h>

NS_LOG_COMPONENT_DEFINE("SatHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatHelper);

TypeId
SatHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatHelper")
            .SetParent<Object>()
            .AddConstructor<SatHelper>()
            .AddAttribute("UtCount",
                          "The count of created UTs in beam (full or user-defined GEO scenario)",
                          UintegerValue(3),
                          MakeUintegerAccessor(&SatHelper::m_utsInBeam),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("GwUsers",
                          "The number of created GW users (full or user-defined scenario)",
                          UintegerValue(5),
                          MakeUintegerAccessor(&SatHelper::m_gwUsers),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("UtUsers",
                          "The number of created UT users per UT (full or user-defined scenario)",
                          UintegerValue(3),
                          MakeUintegerAccessor(&SatHelper::m_utUsers),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute(
                "BeamNetworkAddress",
                "Initial network number to use "
                "during allocation of satellite devices "
                "(mask set by attribute 'BeamNetworkMask' will be used as the network mask)",
                Ipv4AddressValue("40.1.0.0"),
                MakeIpv4AddressAccessor(&SatHelper::m_beamNetworkAddress),
                MakeIpv4AddressChecker())
            .AddAttribute("BeamNetworkMask",
                          "Network mask to use during allocation of satellite devices.",
                          Ipv4MaskValue("255.255.0.0"),
                          MakeIpv4MaskAccessor(&SatHelper::m_beamNetworkMask),
                          MakeIpv4MaskChecker())
            .AddAttribute(
                "GwNetworkAddress",
                "Initial network number to use "
                "during allocation of GW, router, and GW users "
                "(mask set by attribute 'GwNetworkMask' will be used as the network mask)",
                Ipv4AddressValue("90.1.0.0"),
                MakeIpv4AddressAccessor(&SatHelper::m_gwNetworkAddress),
                MakeIpv4AddressChecker())
            .AddAttribute("GwNetworkMask",
                          "Network mask to use during allocation of GW, router, and GW users.",
                          Ipv4MaskValue("255.255.0.0"),
                          MakeIpv4MaskAccessor(&SatHelper::m_gwNetworkMask),
                          MakeIpv4MaskChecker())
            .AddAttribute(
                "UtNetworkAddress",
                "Initial network number to use "
                "during allocation of UT and UT users "
                "(mask set by attribute 'UtNetworkMask' will be used as the network mask)",
                Ipv4AddressValue("10.1.0.0"),
                MakeIpv4AddressAccessor(&SatHelper::m_utNetworkAddress),
                MakeIpv4AddressChecker())
            .AddAttribute("UtNetworkMask",
                          "Network mask to use during allocation of UT and UT users.",
                          Ipv4MaskValue("255.255.0.0"),
                          MakeIpv4MaskAccessor(&SatHelper::m_utNetworkMask),
                          MakeIpv4MaskChecker())
            .AddAttribute("PacketTraceEnabled",
                          "Packet tracing enable status.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatHelper::m_packetTraces),
                          MakeBooleanChecker())
            .AddAttribute("ScenarioCreationTraceEnabled",
                          "Scenario creation trace output enable status.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatHelper::m_creationTraces),
                          MakeBooleanChecker())
            .AddAttribute("DetailedScenarioCreationTraceEnabled",
                          "Detailed scenario creation trace output enable status.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatHelper::m_detailedCreationTraces),
                          MakeBooleanChecker())
            .AddAttribute("ScenarioCreationTraceFileName",
                          "File name for the scenario creation trace output",
                          StringValue("CreationTraceScenario"),
                          MakeStringAccessor(&SatHelper::m_scenarioCreationFileName),
                          MakeStringChecker())
            .AddAttribute("UtCreationTraceFileName",
                          "File name for the UT creation trace output",
                          StringValue("CreationTraceUt"),
                          MakeStringAccessor(&SatHelper::m_utCreationFileName),
                          MakeStringChecker())
            .AddTraceSource("Creation",
                            "Creation traces",
                            MakeTraceSourceAccessor(&SatHelper::m_creationDetailsTrace),
                            "ns3::SatTypedefs::CreationCallback")
            .AddTraceSource("CreationSummary",
                            "Creation summary traces",
                            MakeTraceSourceAccessor(&SatHelper::m_creationSummaryTrace),
                            "ns3::SatTypedefs::CreationCallback");
    return tid;
}

TypeId
SatHelper::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatHelper::SatHelper()
{
    NS_LOG_FUNCTION(this);

    NS_FATAL_ERROR("Constructor not in use");
}

SatHelper::SatHelper(std::string scenarioPath)
    : m_satConstellationEnabled(false),
      m_scenarioCreated(false),
      m_creationTraces(false),
      m_detailedCreationTraces(false),
      m_packetTraces(false),
      m_utsInBeam(0),
      m_gwUsers(0),
      m_utUsers(0),
      m_utPositionsByBeam(),
      m_mobileUtsByBeam(),
      m_mobileUtsUsersByBeam()
{
    NS_LOG_FUNCTION(this << scenarioPath);

    m_scenarioPath = scenarioPath;

    m_rtnConfFileName = m_scenarioPath + "/beams/rtnConf.txt";
    m_fwdConfFileName = m_scenarioPath + "/beams/fwdConf.txt";

    m_gwPosFileName = m_scenarioPath + "/positions/gw_positions.txt";
    m_geoPosFileName = m_scenarioPath + "/positions/geo_positions.txt";
    m_utPosFileName = m_scenarioPath + "/positions/ut_positions.txt";

    m_waveformConfDirectoryName = m_scenarioPath + "/waveforms";

    ReadStandard(m_scenarioPath + "/standard/standard.txt");

    if (Singleton<SatEnvVariables>::Get()->IsValidFile(m_scenarioPath + "/positions/tles.txt"))
    {
        NS_ASSERT_MSG(!Singleton<SatEnvVariables>::Get()->IsValidFile(
                          m_scenarioPath + "/positions/geo_positions.txt"),
                      "position subfolder of scenario cannot have both contain tles.txt and "
                      "geo_positions.txt");
        m_satConstellationEnabled = true;
    }
    else if (!Singleton<SatEnvVariables>::Get()->IsValidFile(m_scenarioPath +
                                                             "/positions/geo_positions.txt"))
    {
        NS_FATAL_ERROR("position subfolder of scenario must contain tles.txt or geo_positions.txt");
    }

    // uncomment next line, if attributes are needed already in construction phase
    ObjectBase::ConstructSelf(AttributeConstructionList());

    Singleton<SatEnvVariables>::Get()->Initialize();
    Singleton<SatIdMapper>::Get()->Reset();

    m_satConf = CreateObject<SatConf>();

    if (m_standard == SatEnums::LORA)
    {
        SatLoraConf satLoraConf;
        satLoraConf.setSatConfAttributes(m_satConf);
    }

    NodeContainer geoNodes;
    std::vector<std::pair<uint32_t, uint32_t>> isls;

    if (m_satConstellationEnabled)
    {
        if (m_satConf->GetForwardLinkRegenerationMode() != SatEnums::REGENERATION_NETWORK)
        {
            NS_FATAL_ERROR("Forward regeneration must be network when using constellations");
        }
        if (m_satConf->GetReturnLinkRegenerationMode() != SatEnums::REGENERATION_NETWORK)
        {
            NS_FATAL_ERROR("Return regeneration must be network when using constellations");
        }

        std::vector<std::string> tles;

        LoadConstellationTopology(tles, isls);

        m_antennaGainPatterns =
            CreateObject<SatAntennaGainPatternContainer>(tles.size(),
                                                         m_scenarioPath + "/antennapatterns");

        for (uint32_t i = 0; i < tles.size(); i++)
        {
            // create Geo Satellite node, set mobility to it
            Ptr<Node> geoSatNode = CreateObject<Node>();

            SetSatMobility(geoSatNode, tles[i]);

            Ptr<SatMobilityModel> mobility = geoSatNode->GetObject<SatMobilityModel>();
            m_antennaGainPatterns->ConfigureBeamsMobility(i, mobility);

            geoNodes.Add(geoSatNode);
        }
    }
    else
    {
        m_antennaGainPatterns =
            CreateObject<SatAntennaGainPatternContainer>(1, m_scenarioPath + "/antennapatterns");

        // In case of constellations, all satellites have the same features, read in same
        // configuration file
        m_satConf->Initialize(m_rtnConfFileName,
                              m_fwdConfFileName,
                              m_gwPosFileName,
                              m_geoPosFileName,
                              m_utPosFileName,
                              m_waveformConfDirectoryName);

        // create Geo Satellite node, set mobility to it
        Ptr<Node> geoSatNode = CreateObject<Node>();

        SetGeoSatMobility(geoSatNode);

        Ptr<SatMobilityModel> mobility = geoSatNode->GetObject<SatMobilityModel>();
        m_antennaGainPatterns->ConfigureBeamsMobility(0, mobility);

        geoNodes.Add(geoSatNode);
    }

    m_beamHelper =
        CreateObject<SatBeamHelper>(m_standard,
                                    geoNodes,
                                    isls,
                                    MakeCallback(&SatConf::GetCarrierBandwidthHz, m_satConf),
                                    m_satConf->GetRtnLinkCarrierCount(),
                                    m_satConf->GetFwdLinkCarrierCount(),
                                    m_satConf->GetSuperframeSeq(),
                                    m_satConf->GetForwardLinkRegenerationMode(),
                                    m_satConf->GetReturnLinkRegenerationMode());

    m_beamHelper->SetAntennaGainPatterns(m_antennaGainPatterns);

    Ptr<SatRtnLinkTime> rtnTime = Singleton<SatRtnLinkTime>::Get();
    rtnTime->Initialize(m_satConf->GetSuperframeSeq());

    SatBeamHelper::CarrierFreqConverter converterCb =
        MakeCallback(&SatConf::GetCarrierFrequencyHz, m_satConf);
    m_beamHelper->SetAttribute("CarrierFrequencyConverter", CallbackValue(converterCb));

    m_userHelper = CreateObject<SatUserHelper>();
    SatUserHelper::PropagationDelayCallback delayModelCb =
        MakeCallback(&SatBeamHelper::GetPropagationDelayModel, m_beamHelper);
    m_userHelper->SetAttribute("PropagationDelayGetter", CallbackValue(delayModelCb));
}

void
SatHelper::CreatePredefinedScenario(PreDefinedScenario_t scenario)
{
    NS_LOG_FUNCTION(this);

    switch (scenario)
    {
    case SIMPLE:
        CreateSimpleScenario();
        break;

    case LARGER:
        CreateLargerScenario();
        break;

    case FULL:
        CreateFullScenario();
        break;

    default:
        NS_FATAL_ERROR("Not supported predefined scenario.");
        break;
    }
}

void
SatHelper::EnableCreationTraces()
{
    NS_LOG_FUNCTION(this);

    AsciiTraceHelper asciiTraceHelper;

    std::stringstream outputPathCreation;
    std::stringstream outputPathUt;
    outputPathCreation << Singleton<SatEnvVariables>::Get()->GetOutputPath() << "/"
                       << m_scenarioCreationFileName << ".log";
    outputPathUt << Singleton<SatEnvVariables>::Get()->GetOutputPath() << "/"
                 << m_utCreationFileName << ".log";

    m_creationTraceStream = asciiTraceHelper.CreateFileStream(outputPathCreation.str());
    m_utTraceStream = asciiTraceHelper.CreateFileStream(outputPathUt.str());

    TraceConnectWithoutContext("CreationSummary",
                               MakeCallback(&SatHelper::CreationSummarySink, this));

    if (m_detailedCreationTraces)
    {
        EnableDetailedCreationTraces();
    }
}

void
SatHelper::EnablePacketTrace()
{
    m_beamHelper->EnablePacketTrace();
}

void
SatHelper::LoadConstellationTopology(std::vector<std::string>& tles,
                                     std::vector<std::pair<uint32_t, uint32_t>>& isls)
{
    NS_LOG_FUNCTION(this);

    m_scenarioPath + "beams/rtnConf.txt";

    m_satConf->Initialize(m_rtnConfFileName,
                          m_fwdConfFileName,
                          m_gwPosFileName,
                          m_geoPosFileName,
                          m_utPosFileName,
                          m_waveformConfDirectoryName,
                          true);

    tles = m_satConf->LoadTles(m_scenarioPath + "/positions/tles.txt",
                               m_scenarioPath + "/positions/start_date.txt");

    if (Singleton<SatEnvVariables>::Get()->IsValidFile(m_scenarioPath + "/positions/isls.txt"))
    {
        isls = m_satConf->LoadIsls(m_scenarioPath + "/positions/isls.txt");
    }
}

void
SatHelper::EnableDetailedCreationTraces()
{
    NS_LOG_FUNCTION(this);

    CallbackBase creationCb =
        MakeBoundCallback(&SatHelper::CreationDetailsSink, m_creationTraceStream);
    TraceConnect("Creation", "SatHelper", creationCb);

    m_userHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
    m_beamHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
}

uint32_t
SatHelper::GetClosestSat(GeoCoordinate position)
{
    NS_LOG_FUNCTION(this);

    return m_beamHelper->GetClosestSat(position);
}

Ipv4Address
SatHelper::GetUserAddress(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this);

    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
    return ipv4->GetAddress(1, 0)
        .GetLocal(); // Get Ipv4InterfaceAddress of interface csma interface.
}

NodeContainer
SatHelper::GetUtUsers() const
{
    NS_LOG_FUNCTION(this);

    return m_userHelper->GetUtUsers();
}

NodeContainer
SatHelper::GetUtUsers(Ptr<Node> utNode) const
{
    return m_userHelper->GetUtUsers(utNode);
}

NodeContainer
SatHelper::GetUtUsers(NodeContainer utNodes) const
{
    NodeContainer total;
    for (NodeContainer::Iterator i = utNodes.Begin(); i != utNodes.End(); i++)
    {
        total.Add(GetUtUsers(*i));
    }
    return total;
}

NodeContainer
SatHelper::GetGwUsers() const
{
    NS_LOG_FUNCTION(this);

    return m_userHelper->GetGwUsers();
}

Ptr<SatBeamHelper>
SatHelper::GetBeamHelper() const
{
    NS_LOG_FUNCTION(this);
    return m_beamHelper;
}

Ptr<SatGroupHelper>
SatHelper::GetGroupHelper() const
{
    NS_LOG_FUNCTION(this);
    return m_groupHelper;
}

void
SatHelper::SetGroupHelper(Ptr<SatGroupHelper> groupHelper)
{
    NS_LOG_FUNCTION(this << groupHelper);
    m_groupHelper = groupHelper;
}

void
SatHelper::SetAntennaGainPatterns(Ptr<SatAntennaGainPatternContainer> antennaGainPatterns)
{
    NS_LOG_FUNCTION(this);
    m_antennaGainPatterns = antennaGainPatterns;
}

Ptr<SatAntennaGainPatternContainer>
SatHelper::GetAntennaGainPatterns()
{
    return m_antennaGainPatterns;
}

Ptr<SatUserHelper>
SatHelper::GetUserHelper() const
{
    NS_LOG_FUNCTION(this);
    return m_userHelper;
}

uint32_t
SatHelper::GetBeamCount() const
{
    NS_LOG_FUNCTION(this);

    return m_satConf->GetBeamCount();
}

void
SatHelper::CreateSimpleScenario()
{
    NS_LOG_FUNCTION(this);

    SatBeamUserInfo beamInfo = SatBeamUserInfo(1, 1);
    BeamUserInfoMap_t beamUserInfos;
    beamUserInfos[std::make_pair(0, 8)] = beamInfo;

    DoCreateScenario(beamUserInfos, 1);

    m_creationSummaryTrace("*** Simple Scenario Creation Summary ***");
}

void
SatHelper::CreateLargerScenario()
{
    NS_LOG_FUNCTION(this);

    // install one user for UTs in beams 12 and 22
    SatBeamUserInfo beamInfo = SatBeamUserInfo(1, 1);
    BeamUserInfoMap_t beamUserInfos;

    beamUserInfos[std::make_pair(0, 12)] = beamInfo;
    beamUserInfos[std::make_pair(0, 22)] = beamInfo;

    // install two users for UT1 and one for UT2 in beam 3
    beamInfo.SetUtUserCount(0, 2);
    beamInfo.AppendUt(1);

    beamUserInfos[std::make_pair(0, 3)] = beamInfo;

    DoCreateScenario(beamUserInfos, 1);

    m_creationSummaryTrace("*** Larger Scenario Creation Summary ***");
}

void
SatHelper::CreateFullScenario()
{
    NS_LOG_FUNCTION(this);

    uint32_t beamCount = m_satConf->GetBeamCount();
    BeamUserInfoMap_t beamUserInfos;

    for (uint32_t i = 1; i < (beamCount + 1); i++)
    {
        BeamUserInfoMap_t::iterator beamInfo = m_beamUserInfos.find(std::make_pair(0, i));
        SatBeamUserInfo info;

        if (beamInfo != m_beamUserInfos.end())
        {
            info = beamInfo->second;
        }
        else
        {
            info = SatBeamUserInfo(m_utsInBeam, this->m_utUsers);
        }

        beamUserInfos[std::make_pair(0, i)] = info;
    }

    DoCreateScenario(beamUserInfos, m_gwUsers);

    m_creationSummaryTrace("*** Full Scenario Creation Summary ***");
}

void
SatHelper::CreateUserDefinedScenario(BeamUserInfoMap_t& infos)
{
    NS_LOG_FUNCTION(this);

    // create as user wants
    DoCreateScenario(infos, m_gwUsers);

    m_creationSummaryTrace("*** User Defined Scenario Creation Summary ***");
}

void
SatHelper::SetCustomUtPositionAllocator(Ptr<SatListPositionAllocator> posAllocator)
{
    NS_LOG_FUNCTION(this);
    m_utPositions = posAllocator;
}

void
SatHelper::SetUtPositionAllocatorForBeam(uint32_t beamId,
                                         Ptr<SatListPositionAllocator> posAllocator)
{
    NS_LOG_FUNCTION(this << beamId);
    m_utPositionsByBeam[beamId] = posAllocator;
}

void
SatHelper::CreateUserDefinedScenarioFromListPositions(uint32_t satId,
                                                      BeamUserInfoMap_t& infos,
                                                      std::string inputFileUtListPositions,
                                                      bool checkBeam)
{
    NS_LOG_FUNCTION(this);

    uint32_t positionIndex = 1;

    // construct list position allocator and fill it with position
    // configured through SatConf

    m_utPosFileName = inputFileUtListPositions;

    m_satConf->SetUtPositionsPath(m_utPosFileName);

    m_utPositions = CreateObject<SatListPositionAllocator>();

    for (BeamUserInfoMap_t::iterator it = infos.begin(); it != infos.end(); it++)
    {
        for (uint32_t i = 0; i < it->second.GetUtCount(); i++)
        {
            if (positionIndex > m_satConf->GetUtCount())
            {
                NS_FATAL_ERROR("Not enough positions available in SatConf for UTs!!!");
            }

            GeoCoordinate position = m_satConf->GetUtPosition(positionIndex);
            m_utPositions->Add(position);
            positionIndex++;

            // if requested, check that the given beam is the best in the configured position
            if (checkBeam)
            {
                uint32_t bestBeamId = m_antennaGainPatterns->GetBestBeamId(satId, position, false);

                if (bestBeamId != it->first.second)
                {
                    NS_FATAL_ERROR("The beam: " << it->first << " is not the best beam ("
                                                << bestBeamId
                                                << ") for the position: " << position);
                }
            }
        }
    }

    // create as user wants
    DoCreateScenario(infos, m_gwUsers);

    m_creationSummaryTrace("*** User Defined Scenario with List Positions Creation Summary ***");
}

void
SatHelper::LoadConstellationScenario(BeamUserInfoMap_t& info,
                                     GetNextUtUserCountCallback getNextUtUserCountCallback)
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT_MSG(info.size() > 0, "There must be at least one beam satellite");

    m_antennaGainPatterns->SetEnabledBeams(info);

    for (uint32_t i = 0; i < m_satConf->GetUtCount(); i++)
    {
        GeoCoordinate position = m_satConf->GetUtPosition(i + 1);

        uint32_t satId = m_beamHelper->GetClosestSat(position);

        uint32_t bestBeamId = m_antennaGainPatterns->GetBestBeamId(satId, position, false);

        std::vector<std::pair<GeoCoordinate, uint32_t>> positions =
            info.at(std::pair(satId, bestBeamId)).GetPositions();
        positions.push_back(std::make_pair(position, 0));
        info.at(std::pair(satId, bestBeamId)).SetPositions(positions);
        uint32_t nbUsers = getNextUtUserCountCallback();
        info.at(std::pair(satId, bestBeamId)).AppendUt(nbUsers);
    }

    for (uint32_t i = 0; i < m_satConf->GetGwCount(); i++)
    {
        GeoCoordinate position = m_satConf->GetGwPosition(i + 1);
        uint32_t satId = m_beamHelper->GetClosestSat(position);

        m_gwSats[i] = satId;
    }

    m_groupHelper->SetSatConstellationEnabled();
}

void
SatHelper::DoCreateScenario(BeamUserInfoMap_t& beamInfos, uint32_t gwUsers)
{
    NS_LOG_FUNCTION(this);

    if (m_scenarioCreated)
    {
        Singleton<SatLog>::Get()->AddToLog(
            SatLog::LOG_WARNING,
            "",
            "Scenario tried to re-create with SatHelper. Creation can be done only once.");
    }
    else
    {
        SetNetworkAddresses(beamInfos, gwUsers);

        if (m_creationTraces)
        {
            EnableCreationTraces();
        }

        m_beamHelper->SetNccRoutingCallback(
            MakeCallback(&SatUserHelper::UpdateGwRoutes, m_userHelper));

        InternetStackHelper internet;

        // create all possible GW nodes, set mobility to them and install to Internet
        NodeContainer gwNodes;
        gwNodes.Create(m_satConf->GetGwCount());
        internet.Install(gwNodes);

        // Create beams explicitly required for this scenario
        for (BeamUserInfoMap_t::iterator info = beamInfos.begin(); info != beamInfos.end(); info++)
        {
            uint32_t satId = info->first.first;
            uint32_t beamId = info->first.second;

            // create UTs of the beam, set mobility to them
            std::vector<std::pair<GeoCoordinate, uint32_t>> positionsAndGroupId =
                info->second.GetPositions();
            NodeContainer uts;
            uts.Create(info->second.GetUtCount() - positionsAndGroupId.size());
            SetUtMobility(uts, satId, beamId);

            NodeContainer utsFromPosition;
            utsFromPosition.Create(positionsAndGroupId.size());
            SetUtMobilityFromPosition(utsFromPosition, satId, beamId, positionsAndGroupId);
            uts.Add(utsFromPosition);

            // Add mobile UTs starting at this beam
            std::map<uint32_t, NodeContainer>::iterator mobileUts = m_mobileUtsByBeam.find(beamId);
            if (mobileUts != m_mobileUtsByBeam.end())
            {
                uts.Add(mobileUts->second);
                m_mobileUtsByBeam.erase(mobileUts);
            }

            // install the whole fleet to Internet
            internet.Install(uts);

            for (uint32_t i = 0; i < info->second.GetUtCount(); ++i)
            {
                // create and install needed users
                m_userHelper->InstallUt(uts.Get(i), info->second.GetUtUserCount(i));
            }

            std::pair<std::multimap<uint32_t, uint32_t>::iterator,
                      std::multimap<uint32_t, uint32_t>::iterator>
                mobileUsers;
            mobileUsers = m_mobileUtsUsersByBeam.equal_range(beamId);
            std::multimap<uint32_t, uint32_t>::iterator it = mobileUsers.first;
            for (uint32_t i = info->second.GetUtCount(); i < uts.GetN() && it != mobileUsers.second;
                 ++i, ++it)
            {
                // create and install needed mobile users
                m_userHelper->InstallUt(uts.Get(i), it->second);
            }

            std::vector<uint32_t> rtnConf =
                m_satConf->GetBeamConfiguration(beamId, SatEnums::LD_RETURN);
            std::vector<uint32_t> fwdConf =
                m_satConf->GetBeamConfiguration(beamId, SatEnums::LD_FORWARD);

            /**
             * GW and beam ids are assumed to be the same for both directions
             * currently!
             */
            NS_ASSERT(rtnConf[SatConf::GW_ID_INDEX] == fwdConf[SatConf::GW_ID_INDEX]);
            NS_ASSERT(rtnConf[SatConf::BEAM_ID_INDEX] == fwdConf[SatConf::BEAM_ID_INDEX]);

            // gw index starts from 1 and we have stored them starting from 0
            Ptr<Node> gwNode = gwNodes.Get(rtnConf[SatConf::GW_ID_INDEX] - 1);

            SetGwMobility(satId, gwNode, rtnConf[SatConf::GW_ID_INDEX]);

            if (m_satConstellationEnabled)
            {
                for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); it++)
                {
                    if ((*it)->GetObject<SatHandoverModule>() == nullptr)
                    {
                        (*it)->AggregateObject(
                            CreateObject<SatHandoverModule>(*it,
                                                            GeoSatNodes(),
                                                            m_antennaGainPatterns));
                    }
                }
            }

            std::pair<Ptr<NetDevice>, NetDeviceContainer> netDevices =
                m_beamHelper->Install(uts,
                                      gwNode,
                                      rtnConf[SatConf::GW_ID_INDEX],
                                      satId,
                                      rtnConf[SatConf::BEAM_ID_INDEX],
                                      rtnConf[SatConf::U_FREQ_ID_INDEX],
                                      rtnConf[SatConf::F_FREQ_ID_INDEX],
                                      fwdConf[SatConf::U_FREQ_ID_INDEX],
                                      fwdConf[SatConf::F_FREQ_ID_INDEX],
                                      MakeCallback(&SatUserHelper::UpdateUtRoutes, m_userHelper));

            m_utsDistribution.insert(netDevices);

            if (m_satConstellationEnabled)
            {
                uint32_t gwId = rtnConf[SatConf::GW_ID_INDEX] - 1;
                uint32_t gwSatId = m_gwSats[gwId];
                if (satId == gwSatId)
                {
                    DynamicCast<SatGeoNetDevice>(
                        m_beamHelper->GetGeoSatNodes().Get(gwSatId)->GetDevice(0))
                        ->ConnectGw(Mac48Address::ConvertFrom(netDevices.first->GetAddress()));
                }
            }
            else
            {
                DynamicCast<SatGeoNetDevice>(m_beamHelper->GetGeoSatNodes().Get(0)->GetDevice(0))
                    ->ConnectGw(Mac48Address::ConvertFrom(netDevices.first->GetAddress()));
                m_userHelper->PopulateBeamRoutings(uts,
                                                   netDevices.second,
                                                   gwNode,
                                                   netDevices.first);
            }

            for (uint32_t utIndex = 0; utIndex < uts.GetN(); utIndex++)
            {
                DynamicCast<SatGeoNetDevice>(
                    m_beamHelper->GetGeoSatNodes().Get(satId)->GetDevice(0))
                    ->ConnectUt(
                        Mac48Address::ConvertFrom(netDevices.second.Get(utIndex)->GetAddress()));
            }
        }

        m_mobileUtsByBeam
            .clear(); // Release unused resources (mobile UTs starting in non-existent beams)

        if (m_satConstellationEnabled)
        {
            SetBeamRoutingConstellations();
        }

        m_userHelper->InstallGw(m_beamHelper->GetGwNodes(), gwUsers);

        if (m_satConstellationEnabled)
        {
            m_beamHelper->InstallIsls();
            m_beamHelper->SetIslRoutes();

            SetGwAddressInUts();

            for (uint32_t i = 0; i < UtNodes().GetN(); i++)
            {
                Ptr<Node> ut = UtNodes().Get(i);

                for (uint32_t j = 0; j < ut->GetNDevices(); j++)
                {
                    Ptr<SatNetDevice> netDevice = DynamicCast<SatNetDevice>(ut->GetDevice(j));
                    if (netDevice)
                    {
                        Ptr<SatUtMac> mac = DynamicCast<SatUtMac>(netDevice->GetMac());
                        mac->SetUpdateIslCallback(
                            MakeCallback(&SatBeamHelper::SetIslRoutes, m_beamHelper));
                    }
                }
            }
        }

        if (m_standard == SatEnums::LORA)
        {
            // Create the LoraDeviceAddress of the end devices
            uint8_t nwkId = 54;
            uint32_t nwkAddr = 1864;
            Ptr<LoraDeviceAddressGenerator> addrGen =
                CreateObject<LoraDeviceAddressGenerator>(nwkId, nwkAddr);

            Ptr<Node> utNode;
            for (uint32_t indexUt = 0; indexUt < UtNodes().GetN(); indexUt++)
            {
                utNode = UtNodes().Get(indexUt);
                Ptr<SatLorawanNetDevice> dev =
                    utNode->GetDevice(2)->GetObject<SatLorawanNetDevice>();
                dev->GetMac()->GetObject<LorawanMacEndDeviceClassA>()->SetDeviceAddress(
                    addrGen->NextAddress());
                ;
            }

            Ptr<LoraNetworkServerHelper> loraNetworkServerHelper =
                CreateObject<LoraNetworkServerHelper>();
            Ptr<LoraForwarderHelper> forHelper = CreateObject<LoraForwarderHelper>();

            loraNetworkServerHelper->SetGateways(GwNodes());
            loraNetworkServerHelper->SetEndDevices(UtNodes());

            NodeContainer networkServer;
            networkServer.Create(1);

            loraNetworkServerHelper->Install(networkServer);

            forHelper->Install(GwNodes());
        }

        if (m_packetTraces)
        {
            EnablePacketTrace();
        }

        m_scenarioCreated = true;
    }

    m_beamHelper->Init();
}

void
SatHelper::SetGwAddressInUts()
{
    NS_LOG_FUNCTION(this);

    // Loop on each UT
    for (uint32_t i = 0; i < m_beamHelper->GetUtNodes().GetN(); i++)
    {
        Ptr<Node> ut = m_beamHelper->GetUtNodes().Get(i);
        Mac48Address gwAddress = GetGwAddressInSingleUt(ut->GetId());

        Ptr<SatUtMac> satUtMac;
        for (uint32_t ndId = 0; ndId < ut->GetNDevices(); ndId++)
        {
            Ptr<SatNetDevice> utNd = DynamicCast<SatNetDevice>(ut->GetDevice(ndId));
            if (utNd)
            {
                satUtMac = DynamicCast<SatUtMac>(utNd->GetMac());
                break;
            }
        }

        satUtMac->SetGwAddress(gwAddress);
        satUtMac->SetGetGwAddressInUtCallback(
            MakeCallback(&SatHelper::GetGwAddressInSingleUt, this));
    }
}

Mac48Address
SatHelper::GetGwAddressInSingleUt(uint32_t utId)
{
    NS_LOG_FUNCTION(this << utId);

    // Get UT, GW attached to this UT, satellite linked to this GW and beam ID used by the
    // satellite connected to the UT
    Ptr<Node> ut;
    for (uint32_t i = 0; i < m_beamHelper->GetUtNodes().GetN(); i++)
    {
        if (m_beamHelper->GetUtNodes().Get(i)->GetId() == utId)
        {
            ut = m_beamHelper->GetUtNodes().Get(i);
            break;
        }
    }
    NS_ASSERT_MSG(ut != nullptr, "Cannot find UT with ID of " << utId);

    Ptr<SatUtMac> satUtMac;
    uint32_t utBeamId = 0;
    uint32_t utSatNetDeviceCount = 0;
    for (uint32_t ndId = 0; ndId < ut->GetNDevices(); ndId++)
    {
        Ptr<SatNetDevice> utNd = DynamicCast<SatNetDevice>(ut->GetDevice(ndId));
        if (utNd)
        {
            utSatNetDeviceCount++;
            satUtMac = DynamicCast<SatUtMac>(utNd->GetMac());
            utBeamId = satUtMac->GetBeamId();
        }
    }
    NS_ASSERT_MSG(utSatNetDeviceCount == 1, "UT must have exactly one SatNetDevice");
    NS_ASSERT_MSG(satUtMac != nullptr, "UT must have a SatUtMac for beam");

    std::vector<uint32_t> rtnConf = m_satConf->GetBeamConfiguration(utBeamId, SatEnums::LD_RETURN);
    Ptr<Node> gw = m_beamHelper->GetGwNode(rtnConf[SatConf::GW_ID_INDEX]);
    uint32_t gwSatId =
        GetClosestSat(GeoCoordinate(gw->GetObject<SatMobilityModel>()->GetPosition()));

    // Get feeder MAC used on sat on GW side, and corresponding beam ID used for downlink (can
    // be different than UT beam ID)
    uint32_t usedBeamId = 0;
    uint32_t gwSatGeoNetDeviceCount = 0;
    for (uint32_t ndId = 0; ndId < m_beamHelper->GetGeoSatNodes().Get(gwSatId)->GetNDevices();
         ndId++)
    {
        Ptr<SatGeoNetDevice> gwNd = DynamicCast<SatGeoNetDevice>(
            m_beamHelper->GetGeoSatNodes().Get(gwSatId)->GetDevice(ndId));
        if (gwNd)
        {
            gwSatGeoNetDeviceCount++;
            usedBeamId = gwNd->GetFeederMac(utBeamId)->GetBeamId();
        }
    }
    NS_ASSERT_MSG(gwSatGeoNetDeviceCount == 1, "SAT must have exactly one SatGeoNetDevice");
    NS_ASSERT_MSG(usedBeamId != 0, "Incorrect beam ID");

    // Get GW MAC for usedBeamId, and corresponding MAC address
    Mac48Address gwAddress;
    uint32_t gwSatNetDeviceCount = 0;
    for (uint32_t ndId = 0; ndId < gw->GetNDevices(); ndId++)
    {
        Ptr<SatNetDevice> gwNd = DynamicCast<SatNetDevice>(gw->GetDevice(ndId));
        if (gwNd && gwNd->GetMac()->GetBeamId() == usedBeamId &&
            gwNd->GetMac()->GetSatId() == gwSatId)
        {
            gwSatNetDeviceCount++;
            gwAddress = Mac48Address::ConvertFrom(gwNd->GetAddress());
        }
    }
    NS_ASSERT_MSG(gwSatNetDeviceCount == 1,
                  "GW must have exactly one SatNetDevice for beam "
                      << usedBeamId << " and satellite " << gwSatId);

    return gwAddress;
}

void
SatHelper::SetBeamRoutingConstellations()
{
    NS_LOG_FUNCTION(this);

    for (uint32_t gwId = 0; gwId < GwNodes().GetN(); gwId++)
    {
        Ptr<Node> gw = GwNodes().Get(gwId);
        for (uint32_t ndId = 0; ndId < gw->GetNDevices(); ndId++)
        {
            if (DynamicCast<SatNetDevice>(gw->GetDevice(ndId)))
            {
                Ptr<SatNetDevice> gwNd = DynamicCast<SatNetDevice>(gw->GetDevice(ndId));

                NetDeviceContainer utNd = m_utsDistribution[gwNd];
                NodeContainer ut;
                for (uint32_t i = 0; i < utNd.GetN(); i++)
                {
                    ut.Add(utNd.Get(i)->GetNode());
                }
                m_userHelper->PopulateBeamRoutings(ut, utNd, gw, gwNd);
            }
        }
    }
}

void
SatHelper::LoadMobileUTsFromFolder(const std::string& folderName, Ptr<RandomVariableStream> utUsers)
{
    NS_LOG_FUNCTION(this << folderName << utUsers);

    if (!(Singleton<SatEnvVariables>::Get()->IsValidDirectory(folderName)))
    {
        NS_LOG_INFO("Directory '" << folderName
                                  << "' does not exist, no mobile UTs will be created.");
        return;
    }

    for (std::string& filename : SystemPath::ReadFiles(folderName))
    {
        std::string filepath = folderName + "/" + filename;
        if (Singleton<SatEnvVariables>::Get()->IsValidDirectory(filepath))
        {
            NS_LOG_INFO("Skipping directory '" << filename << "'");
            continue;
        }

        Ptr<Node> utNode = LoadMobileUtFromFile(filepath);
        uint32_t bestBeamId = utNode->GetObject<SatTracedMobilityModel>()->GetBestBeamId();

        // Store Node in the container for the starting beam
        std::map<uint32_t, NodeContainer>::iterator it = m_mobileUtsByBeam.find(bestBeamId);
        if (it == m_mobileUtsByBeam.end())
        {
            std::pair<std::map<uint32_t, NodeContainer>::iterator, bool> inserted =
                m_mobileUtsByBeam.insert(std::make_pair(bestBeamId, NodeContainer(utNode)));
            NS_ASSERT_MSG(inserted.second,
                          "Failed to create a new beam when reading UT mobility files");
        }
        else
        {
            it->second.Add(utNode);
        }

        // Store amount of users for this UT
        m_mobileUtsUsersByBeam.insert(std::make_pair(bestBeamId, utUsers->GetInteger()));
    }

    for (auto& mobileUtsForBeam : m_mobileUtsByBeam)
    {
        NS_LOG_INFO("Installing Mobility Observers for mobile UTs starting in beam "
                    << mobileUtsForBeam.first);
        InstallMobilityObserver(0, mobileUtsForBeam.second);
    }
}

Ptr<Node>
SatHelper::LoadMobileUtFromFile(const std::string& filename)
{
    NS_LOG_FUNCTION(this << filename);

    if (Singleton<SatEnvVariables>::Get()->IsValidFile(
            Singleton<SatEnvVariables>::Get()->LocateDataDirectory() + "/" + filename))
    {
        NS_FATAL_ERROR(filename << " is not a valid file name");
    }

    GeoCoordinate initialPosition =
        Singleton<SatPositionInputTraceContainer>::Get()->GetPosition(filename,
                                                                      GeoCoordinate::SPHERE);
    uint32_t satId = m_beamHelper->GetClosestSat(initialPosition);

    // Create Node, Mobility and aggregate them
    Ptr<SatTracedMobilityModel> mobility =
        CreateObject<SatTracedMobilityModel>(satId, filename, m_antennaGainPatterns);

    Ptr<Node> utNode = CreateObject<Node>();
    utNode->AggregateObject(mobility);
    utNode->AggregateObject(
        CreateObject<SatHandoverModule>(utNode, GeoSatNodes(), m_antennaGainPatterns));
    return utNode;
}

Ptr<Node>
SatHelper::LoadMobileUtFromFile(uint32_t satId, const std::string& filename)
{
    NS_LOG_FUNCTION(this << satId << filename);

    if (Singleton<SatEnvVariables>::Get()->IsValidFile(
            Singleton<SatEnvVariables>::Get()->LocateDataDirectory() + "/" + filename))
    {
        NS_FATAL_ERROR(filename << " is not a valid file name");
    }

    // Create Node, Mobility and aggregate them
    Ptr<SatTracedMobilityModel> mobility =
        CreateObject<SatTracedMobilityModel>(satId, filename, m_antennaGainPatterns);

    Ptr<Node> utNode = CreateObject<Node>();
    utNode->AggregateObject(mobility);
    utNode->AggregateObject(
        CreateObject<SatHandoverModule>(utNode, GeoSatNodes(), m_antennaGainPatterns));
    return utNode;
}

void
SatHelper::SetGwMobility(uint32_t satId, Ptr<Node> gw, uint32_t gwIndex)
{
    NS_LOG_FUNCTION(this << satId << gw << gwIndex);

    if (gw->GetObject<SatHandoverModule>() != nullptr)
    {
        return;
    }

    NodeContainer gwNodes = NodeContainer(gw);
    MobilityHelper mobility;
    Ptr<SatListPositionAllocator> gwPosAllocator = CreateObject<SatListPositionAllocator>();

    gwPosAllocator->Add(m_satConf->GetGwPosition(gwIndex));

    mobility.SetPositionAllocator(gwPosAllocator);
    mobility.SetMobilityModel("ns3::SatConstantPositionMobilityModel");
    mobility.Install(gwNodes);

    InstallMobilityObserver(satId, gwNodes);

    Ptr<SatHandoverModule> ho =
        CreateObject<SatHandoverModule>(gw, GeoSatNodes(), m_antennaGainPatterns);
    NS_LOG_DEBUG("Created Handover Module " << ho << " for GW node " << gw);
    gw->AggregateObject(ho);
}

void
SatHelper::SetUtMobility(NodeContainer uts, uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this);

    MobilityHelper mobility;

    Ptr<SatPositionAllocator> allocator;

    // if position allocator (list) for UTs is created by helper already use it,
    // in other case use the spot beam position allocator
    if (m_utPositionsByBeam.find(beamId) != m_utPositionsByBeam.end())
    {
        allocator = m_utPositionsByBeam[beamId];
    }
    else if (m_utPositions != NULL)
    {
        allocator = m_utPositions;
    }
    else
    {
        // Create new position allocator
        allocator = GetBeamAllocator(beamId);
    }

    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::SatConstantPositionMobilityModel");
    mobility.Install(uts);

    InstallMobilityObserver(satId, uts);

    for (uint32_t i = 0; i < uts.GetN(); ++i)
    {
        GeoCoordinate position = uts.Get(i)->GetObject<SatMobilityModel>()->GetGeoPosition();
        NS_LOG_INFO(
            "Installing mobility observer on Ut Node at "
            << position << " with antenna gain of "
            << m_antennaGainPatterns->GetAntennaGainPattern(beamId)->GetAntennaGain_lin(
                   position,
                   m_beamHelper->GetGeoSatNodes().Get(satId)->GetObject<SatMobilityModel>()));
    }
}

void
SatHelper::SetUtMobilityFromPosition(
    NodeContainer uts,
    uint32_t satId,
    uint32_t beamId,
    std::vector<std::pair<GeoCoordinate, uint32_t>> positionsAndGroupId)
{
    NS_LOG_FUNCTION(this << beamId);

    MobilityHelper mobility;

    Ptr<SatListPositionAllocator> allocator = CreateObject<SatListPositionAllocator>();

    NS_ASSERT_MSG(uts.GetN() == positionsAndGroupId.size(),
                  "Inconsistent number of nodes and positions");

    for (uint32_t i = 0; i < positionsAndGroupId.size(); i++)
    {
        allocator->Add(positionsAndGroupId[i].first);
        m_groupHelper->AddNodeToGroupAfterScenarioCreation(positionsAndGroupId[i].second,
                                                           uts.Get(i));
    }

    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::SatConstantPositionMobilityModel");
    mobility.Install(uts);

    InstallMobilityObserver(satId, uts);

    for (uint32_t i = 0; i < uts.GetN(); ++i)
    {
        GeoCoordinate position = uts.Get(i)->GetObject<SatMobilityModel>()->GetGeoPosition();
        NS_LOG_INFO(
            "Installing mobility observer on Ut Node at "
            << position << " with antenna gain of "
            << m_antennaGainPatterns->GetAntennaGainPattern(beamId)->GetAntennaGain_lin(
                   position,
                   m_beamHelper->GetGeoSatNodes().Get(satId)->GetObject<SatMobilityModel>()));
    }
}

Ptr<SatSpotBeamPositionAllocator>
SatHelper::GetBeamAllocator(uint32_t beamId)
{
    NS_LOG_FUNCTION(this << beamId);

    GeoCoordinate satPosition;
    if (m_satConstellationEnabled)
    {
        satPosition =
            m_beamHelper->GetGeoSatNodes().Get(0)->GetObject<SatMobilityModel>()->GetPosition();
    }
    else
    {
        satPosition = m_satConf->GetGeoSatPosition();
    }
    Ptr<SatSpotBeamPositionAllocator> beamAllocator =
        CreateObject<SatSpotBeamPositionAllocator>(beamId, m_antennaGainPatterns, satPosition);

    Ptr<UniformRandomVariable> altRnd = CreateObject<UniformRandomVariable>();
    altRnd->SetAttribute("Min", DoubleValue(0.0));
    altRnd->SetAttribute("Max", DoubleValue(500.0));
    beamAllocator->SetAltitude(altRnd);
    return beamAllocator;
}

void
SatHelper::SetGeoSatMobility(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    MobilityHelper mobility;

    Ptr<SatListPositionAllocator> geoSatPosAllocator = CreateObject<SatListPositionAllocator>();
    geoSatPosAllocator->Add(m_satConf->GetGeoSatPosition());

    mobility.SetPositionAllocator(geoSatPosAllocator);
    mobility.SetMobilityModel("ns3::SatConstantPositionMobilityModel");
    mobility.Install(node);
}

void
SatHelper::SetSatMobility(Ptr<Node> node, std::string tle)
{
    NS_LOG_FUNCTION(this);

    Ptr<Object> object = node;
    Ptr<SatSGP4MobilityModel> model = object->GetObject<SatSGP4MobilityModel>();
    if (model == nullptr)
    {
        ObjectFactory mobilityFactory;
        mobilityFactory.SetTypeId("ns3::SatSGP4MobilityModel");
        model = mobilityFactory.Create()->GetObject<SatSGP4MobilityModel>();
        if (model == nullptr)
        {
            NS_FATAL_ERROR("The requested mobility model is not a mobility model: \""
                           << mobilityFactory.GetTypeId().GetName() << "\"");
        }
        model->SetStartDate(m_satConf->GetStartTimeStr());
        object->AggregateObject(model);
    }

    model->SetTleInfo(tle);
}

void
SatHelper::InstallMobilityObserver(uint32_t satId, NodeContainer nodes) const
{
    NS_LOG_FUNCTION(this);

    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); i++)
    {
        Ptr<SatMobilityObserver> observer = (*i)->GetObject<SatMobilityObserver>();

        if (observer == nullptr)
        {
            Ptr<SatMobilityModel> ownMobility = (*i)->GetObject<SatMobilityModel>();
            Ptr<SatMobilityModel> satMobility =
                m_beamHelper->GetGeoSatNodes().Get(satId)->GetObject<SatMobilityModel>();

            NS_ASSERT(ownMobility != NULL);
            NS_ASSERT(satMobility != NULL);

            observer = CreateObject<SatMobilityObserver>(
                ownMobility,
                satMobility,
                m_beamHelper->GetReturnLinkRegenerationMode() != SatEnums::TRANSPARENT);

            (*i)->AggregateObject(observer);
        }
    }
}

void
SatHelper::SetMulticastGroupRoutes(Ptr<Node> source,
                                   NodeContainer receivers,
                                   Ipv4Address sourceAddress,
                                   Ipv4Address groupAddress)
{
    NS_LOG_FUNCTION(this);

    MulticastBeamInfo_t beamInfo;
    Ptr<NetDevice> routerUserOutputDev;
    Ptr<Node> sourceUtNode = m_userHelper->GetUtNode(source);

    // Construct multicast info from source UT node and receivers. In case that sourceUtNode is
    // NULL, source is some GW user. As a result is given flag indicating if traffic shall be
    // forwarded to source's own network

    if (ConstructMulticastInfo(sourceUtNode, receivers, beamInfo, routerUserOutputDev))
    {
        // Some multicast receiver belongs to same group with source

        // select destination node
        Ptr<Node> destNode = m_userHelper->GetRouter();

        if (sourceUtNode)
        {
            destNode = sourceUtNode;
        }

        // add default route for multicast group to source's network
        SetMulticastRouteToSourceNetwork(source, destNode);
    }

    // set routes outside source's network only when there are receivers
    if (!beamInfo.empty() || (sourceUtNode && routerUserOutputDev))
    {
        Ptr<Node> routerNode = m_userHelper->GetRouter();

        Ptr<NetDevice> routerInputDev = NULL;
        Ptr<NetDevice> gwOutputDev = NULL;

        // set multicast routes to satellite network utilizing beam helper
        NetDeviceContainer gwInputDevices =
            m_beamHelper->AddMulticastGroupRoutes(beamInfo,
                                                  sourceUtNode,
                                                  sourceAddress,
                                                  groupAddress,
                                                  (bool)routerUserOutputDev,
                                                  gwOutputDev);

        Ipv4StaticRoutingHelper multicast;

        // select input device in IP router
        if (gwOutputDev)
        {
            // traffic coming from some GW to router (satellite network and source is UT)
            // find matching input device using GW output device
            routerInputDev = FindMatchingDevice(gwOutputDev, routerNode);
        }
        else if (!sourceUtNode)
        {
            // traffic is coming form user network (some GW user)

            // find matching device using source node
            std::pair<Ptr<NetDevice>, Ptr<NetDevice>> devices;

            if (FindMatchingDevices(source, routerNode, devices))
            {
                routerInputDev = devices.second;
            }
        }

        NetDeviceContainer routerOutputDevices;

        if (routerUserOutputDev)
        {
            routerOutputDevices.Add(routerUserOutputDev);
        }

        for (NetDeviceContainer::Iterator it = gwInputDevices.Begin(); it != gwInputDevices.End();
             it++)
        {
            Ptr<NetDevice> matchingDevice = FindMatchingDevice(*it, routerNode);

            if (matchingDevice)
            {
                routerOutputDevices.Add(matchingDevice);
            }
        }

        // Add multicast route over IP router
        // Input device is getting traffic from user network (GW users) or from some GW
        // Output devices are forwarding traffic to user network (GW users) and/or GWs
        if (routerInputDev && (routerOutputDevices.GetN() > 0))
        {
            multicast.AddMulticastRoute(routerNode,
                                        sourceAddress,
                                        groupAddress,
                                        routerInputDev,
                                        routerOutputDevices);
        }
    }
}

void
SatHelper::CreationDetailsSink(Ptr<OutputStreamWrapper> stream,
                               std::string context,
                               std::string info)
{
    *stream->GetStream() << context << ", " << info << std::endl;
}

void
SatHelper::CreationSummarySink(std::string title)
{
    NS_LOG_FUNCTION(this);

    *m_creationTraceStream->GetStream() << CreateCreationSummary(title);
    *m_utTraceStream->GetStream() << m_beamHelper->GetUtInfo();
}

std::string
SatHelper::CreateCreationSummary(std::string title)
{
    NS_LOG_FUNCTION(this);

    std::ostringstream oss;

    oss << std::endl << std::endl << title << std::endl << std::endl;
    oss << "--- User Info ---" << std::endl << std::endl;
    oss << "Created GW users: " << m_userHelper->GetGwUserCount() << ", ";
    oss << "Created UT users: " << m_userHelper->GetUtUserCount() << std::endl << std::endl;
    oss << m_userHelper->GetRouterInfo() << std::endl << std::endl;
    oss << m_beamHelper->GetBeamInfo() << std::endl;

    return oss.str();
}

void
SatHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_userHelper = NULL;
    m_beamHelper->DoDispose();
    m_beamHelper = NULL;
    m_antennaGainPatterns = NULL;
    m_utPositionsByBeam.clear();
    m_mobileUtsByBeam.clear();
    m_mobileUtsUsersByBeam.clear();
}

void
SatHelper::PrintTopology(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);

    os << "Satellite topology" << std::endl;
    os << "==================" << std::endl;

    os << "Satellites" << std::endl;
    NodeContainer satNodes = m_beamHelper->GetGeoSatNodes();
    for (uint32_t i = 0; i < satNodes.GetN(); i++)
    {
        Ptr<Node> node = satNodes.Get(i);
        os << "  SAT: ID = " << satNodes.Get(i)->GetId();
        os << ", at " << GeoCoordinate(node->GetObject<SatMobilityModel>()->GetPosition())
           << std::endl;
        os << "    Devices to ground stations " << std::endl;
        for (uint32_t j = 0; j < node->GetNDevices(); j++)
        {
            Ptr<SatGeoNetDevice> geoNetDevice = DynamicCast<SatGeoNetDevice>(node->GetDevice(j));
            if (geoNetDevice)
            {
                os << "      " << geoNetDevice->GetAddress() << std::endl;
                std::map<uint32_t, Ptr<SatMac>> feederMac = geoNetDevice->GetAllFeederMac();
                for (std::map<uint32_t, Ptr<SatMac>>::iterator it = feederMac.begin();
                     it != feederMac.end();
                     it++)
                {
                    os << "        Feeder at " << it->second->GetAddress() << ", beam " << it->first
                       << std::endl;
                }
                std::map<uint32_t, Ptr<SatMac>> userMac = geoNetDevice->GetUserMac();
                for (std::map<uint32_t, Ptr<SatMac>>::iterator it = userMac.begin();
                     it != userMac.end();
                     it++)
                {
                    os << "        User at " << it->second->GetAddress() << ", beam " << it->first
                       << std::endl;
                }
                std::set<Mac48Address> gwConnected = geoNetDevice->GetGwConnected();
                os << "      Feeder connected to" << std::endl;
                for (std::set<Mac48Address>::iterator it = gwConnected.begin();
                     it != gwConnected.end();
                     it++)
                {
                    os << "        " << *it << std::endl;
                }
            }
        }
        os << "    ISLs " << std::endl;
        for (uint32_t j = 0; j < node->GetNDevices(); j++)
        {
            Ptr<PointToPointIslNetDevice> islNetDevice =
                DynamicCast<PointToPointIslNetDevice>(node->GetDevice(j));
            if (islNetDevice)
            {
                os << "      " << islNetDevice->GetAddress() << " to SAT "
                   << islNetDevice->GetDestinationNode()->GetId() << std::endl;
            }
        }
    }

    os << "GWs" << std::endl;
    NodeContainer gwNodes = m_beamHelper->GetGwNodes();
    for (uint32_t i = 0; i < gwNodes.GetN(); i++)
    {
        Ptr<Node> node = gwNodes.Get(i);
        os << "  GW: ID = " << gwNodes.Get(i)->GetId();
        os << ", at " << GeoCoordinate(node->GetObject<SatMobilityModel>()->GetPosition())
           << std::endl;
        os << "  Devices " << std::endl;
        for (uint32_t j = 0; j < node->GetNDevices(); j++)
        {
            Ptr<SatNetDevice> netDevice = DynamicCast<SatNetDevice>(node->GetDevice(j));
            if (netDevice)
            {
                Ptr<SatMac> mac = netDevice->GetMac();
                os << "    " << mac->GetAddress() << ", sat: " << mac->GetSatId()
                   << ", beam: " << mac->GetBeamId() << std::endl;
            }
        }
    }

    os << "UTs" << std::endl;
    NodeContainer utNodes = m_beamHelper->GetUtNodes();
    for (uint32_t i = 0; i < utNodes.GetN(); i++)
    {
        Ptr<Node> node = utNodes.Get(i);
        os << "  UT: ID = " << utNodes.Get(i)->GetId();
        os << ", at " << GeoCoordinate(node->GetObject<SatMobilityModel>()->GetPosition())
           << std::endl;
        os << "  Devices " << std::endl;
        for (uint32_t j = 0; j < node->GetNDevices(); j++)
        {
            Ptr<SatNetDevice> netDevice = DynamicCast<SatNetDevice>(node->GetDevice(j));
            if (netDevice)
            {
                Ptr<SatUtMac> mac = DynamicCast<SatUtMac>(netDevice->GetMac());
                os << "    " << mac->GetAddress() << ", sat: " << mac->GetSatId()
                   << ", beam: " << mac->GetBeamId();
                os << ". Linked to GW " << mac->GetGwAddress() << std::endl;
            }
        }
    }

    os << "GW users" << std::endl;
    NodeContainer gwUserNodes = m_userHelper->GetGwUsers();
    for (uint32_t i = 0; i < gwUserNodes.GetN(); i++)
    {
        Ptr<Node> node = gwUserNodes.Get(i);
        os << "  GW user: ID = " << gwUserNodes.Get(i)->GetId() << std::endl;
    }

    os << "UT users" << std::endl;
    NodeContainer utUserNodes = m_userHelper->GetUtUsers();
    for (uint32_t i = 0; i < utUserNodes.GetN(); i++)
    {
        Ptr<Node> node = utUserNodes.Get(i);
        os << "  UT user: ID = " << utUserNodes.Get(i)->GetId() << std::endl;
    }

    os << "==================" << std::endl;
    os << std::endl;
}

bool
SatHelper::FindMatchingDevices(Ptr<Node> nodeA,
                               Ptr<Node> nodeB,
                               std::pair<Ptr<NetDevice>, Ptr<NetDevice>>& matchingDevices)
{
    bool found = false;

    for (uint32_t i = 1; ((i < nodeA->GetNDevices()) && !found); i++)
    {
        Ptr<NetDevice> devA = nodeA->GetDevice(i);
        Ptr<NetDevice> devB = FindMatchingDevice(devA, nodeB);

        if (devB)
        {
            matchingDevices = std::make_pair(devA, devB);
            found = true;
        }
    }

    return found;
}

Ptr<NetDevice>
SatHelper::FindMatchingDevice(Ptr<NetDevice> devA, Ptr<Node> nodeB)
{
    Ptr<NetDevice> matchingDevice = NULL;

    Ipv4Address addressA =
        devA->GetNode()->GetObject<Ipv4L3Protocol>()->GetAddress(devA->GetIfIndex(), 0).GetLocal();
    Ipv4Mask maskA =
        devA->GetNode()->GetObject<Ipv4L3Protocol>()->GetAddress(devA->GetIfIndex(), 0).GetMask();

    Ipv4Address netAddressA = addressA.CombineMask(maskA);

    for (uint32_t j = 1; j < nodeB->GetNDevices(); j++)
    {
        Ipv4Address addressB = nodeB->GetObject<Ipv4L3Protocol>()->GetAddress(j, 0).GetLocal();
        Ipv4Mask maskB = nodeB->GetObject<Ipv4L3Protocol>()->GetAddress(j, 0).GetMask();

        Ipv4Address netAddressB = addressB.CombineMask(maskB);

        if (netAddressA == netAddressB)
        {
            matchingDevice = nodeB->GetDevice(j);
        }
    }

    return matchingDevice;
}

void
SatHelper::SetMulticastRouteToSourceNetwork(Ptr<Node> source, Ptr<Node> dest)
{
    NS_LOG_FUNCTION(this);

    std::pair<Ptr<NetDevice>, Ptr<NetDevice>> devices;

    if (FindMatchingDevices(source, dest, devices))
    {
        Ipv4StaticRoutingHelper multicast;
        Ptr<Ipv4StaticRouting> staticRouting =
            multicast.GetStaticRouting(source->GetObject<ns3::Ipv4>());

        // check if default multicast route already exists
        bool defaultMulticastRouteExists = false;
        Ipv4Address defMulticastNetwork = Ipv4Address("224.0.0.0");
        Ipv4Mask defMulticastNetworkMask = Ipv4Mask("240.0.0.0");

        for (uint32_t i = 0; i < staticRouting->GetNRoutes(); i++)
        {
            if (staticRouting->GetRoute(i).GetDestNetwork() == defMulticastNetwork &&
                staticRouting->GetRoute(i).GetDestNetworkMask() == defMulticastNetworkMask)
            {
                defaultMulticastRouteExists = true;
            }
        }

        // add default multicast route only if it does not exist already
        if (!defaultMulticastRouteExists)
        {
            multicast.SetDefaultMulticastRoute(source, devices.first);
        }
    }
}

bool
SatHelper::ConstructMulticastInfo(Ptr<Node> sourceUtNode,
                                  NodeContainer receivers,
                                  MulticastBeamInfo_t& beamInfo,
                                  Ptr<NetDevice>& routerUserOutputDev)
{
    NS_LOG_FUNCTION(this);

    bool routeToSourceNertwork = false;

    routerUserOutputDev = NULL;

    // go through all receivers
    for (uint32_t i = 0; i < receivers.GetN(); i++)
    {
        Ptr<Node> receiverNode = receivers.Get(i);
        Ptr<Node> utNode = m_userHelper->GetUtNode(receiverNode);

        // check if user is connected to UT or GW

        if (utNode) // connected to UT
        {
            uint32_t beamId = m_beamHelper->GetUtBeamId(utNode);

            if (beamId != 0) // beam ID is found
            {
                if (sourceUtNode == utNode)
                {
                    // Source UT node is same than current UT node. Set flag to indicate that
                    // multicast group traffic shall be routed to source own network.
                    routeToSourceNertwork = true;
                }
                else
                {
                    // store other UT nodes beam ID and pointer to multicast group info for later
                    // routing
                    MulticastBeamInfo_t::iterator it = beamInfo.find(beamId);

                    // find or create first storage for the beam
                    if (it == beamInfo.end())
                    {
                        std::pair<MulticastBeamInfo_t::iterator, bool> result =
                            beamInfo.insert(std::make_pair(beamId, MulticastBeamInfoItem_t()));

                        if (result.second)
                        {
                            it = result.first;
                        }
                        else
                        {
                            NS_FATAL_ERROR("Cannot insert beam to map container");
                        }
                    }

                    // Add to UT node to beam storage (map)
                    it->second.insert(utNode);
                }
            }
            else
            {
                NS_FATAL_ERROR("UT node's beam ID is invalid!!");
            }
        }
        else if (m_userHelper->IsGwUser(receiverNode)) // connected to GW
        {
            if (!routerUserOutputDev)
            {
                if (sourceUtNode)
                {
                    std::pair<Ptr<NetDevice>, Ptr<NetDevice>> devices;

                    if (FindMatchingDevices(receiverNode, m_userHelper->GetRouter(), devices))
                    {
                        routerUserOutputDev = devices.second;
                    }
                }
                else
                {
                    routeToSourceNertwork = true;
                }
            }
        }
        else
        {
            NS_FATAL_ERROR("Multicast receiver node is expected to be connected UT or GW node!!!");
        }
    }

    return routeToSourceNertwork;
}

void
SatHelper::SetNetworkAddresses(BeamUserInfoMap_t& info, uint32_t gwUsers) const
{
    NS_LOG_FUNCTION(this);

    std::set<uint32_t> networkAddresses;
    std::pair<std::set<uint32_t>::const_iterator, bool> addressInsertionResult;

    // test first that configured initial addresses per configured address space
    // are not same by inserting them to set container
    networkAddresses.insert(m_beamNetworkAddress.Get());
    addressInsertionResult = networkAddresses.insert(m_gwNetworkAddress.Get());

    if (!addressInsertionResult.second)
    {
        NS_FATAL_ERROR("GW network address is invalid (same as Beam network address)");
    }

    addressInsertionResult = networkAddresses.insert(m_utNetworkAddress.Get());

    if (!addressInsertionResult.second)
    {
        NS_FATAL_ERROR("UT network address is invalid (same as Beam or GW network address)");
    }

    // calculate values to check needed network and host address counts
    uint32_t utNetworkAddressCount = 0; // network addresses needed in UT network
    uint32_t utHostAddressCount = 0;    // host addresses needed in UT network
    uint32_t beamHostAddressCount = 0;  // host addresses needed in Beam network
    uint32_t gwNetworkAddressCount = 1; // network addresses needed in GW network. Initially one
                                        // network needed between GW users and Router needed
    std::set<uint32_t> gwIds; // to find out the additional network addresses needed in GW network

    for (BeamUserInfoMap_t::const_iterator it = info.begin(); it != info.end(); it++)
    {
        uint32_t beamUtCount = it->second.GetUtCount();
        utNetworkAddressCount += beamUtCount;

        if (beamUtCount > beamHostAddressCount)
        {
            beamHostAddressCount = beamUtCount;
        }

        for (uint32_t i = 0; i < beamUtCount; i++)
        {
            if (it->second.GetUtUserCount(i) > utHostAddressCount)
            {
                utHostAddressCount = it->second.GetUtUserCount(i);
            }
        }

        // try to add GW id to container, if not existing already in the container
        // increment GW network address count
        if (gwIds.insert(m_beamHelper->GetGwId(it->first.first, it->first.second)).second)
        {
            gwNetworkAddressCount++; // one network more needed between a GW and Router
        }
    }

    // do final checking of the configured address spaces
    CheckNetwork("Beam",
                 m_beamNetworkAddress,
                 m_beamNetworkMask,
                 networkAddresses,
                 info.size(),
                 beamHostAddressCount);
    CheckNetwork("GW",
                 m_gwNetworkAddress,
                 m_gwNetworkMask,
                 networkAddresses,
                 gwNetworkAddressCount,
                 gwUsers);
    CheckNetwork("UT",
                 m_utNetworkAddress,
                 m_utNetworkMask,
                 networkAddresses,
                 utNetworkAddressCount,
                 utHostAddressCount);

    // set base addresses of the sub-helpers
    m_userHelper->SetBeamBaseAddress(m_beamNetworkAddress, m_beamNetworkMask);
    m_userHelper->SetGwBaseAddress(m_gwNetworkAddress, m_gwNetworkMask);
    m_userHelper->SetUtBaseAddress(m_utNetworkAddress, m_utNetworkMask);
}

void
SatHelper::CheckNetwork(std::string networkName,
                        const Ipv4Address& firstNetwork,
                        const Ipv4Mask& mask,
                        const std::set<uint32_t>& networkAddresses,
                        uint32_t networkCount,
                        uint32_t hostCount) const
{
    NS_LOG_FUNCTION(this);

    uint16_t addressPrefixLength = mask.GetPrefixLength();

    // test that configured mask is valid (address prefix length is in valid range)
    if ((addressPrefixLength < MIN_ADDRESS_PREFIX_LENGTH) ||
        (addressPrefixLength > MAX_ADDRESS_PREFIX_LENGTH))
    {
        NS_FATAL_ERROR(networkName
                       << " network mask value out of range (0xFFFFFF70 to 0x10000000).");
    }

    // test that configured initial network number (prefix) is valid, consistent with mask
    if ((firstNetwork.Get() & mask.GetInverse()) != 0)
    {
        NS_FATAL_ERROR(networkName << " network address and mask inconsistent.");
    }

    std::set<uint32_t>::const_iterator currentAddressIt = networkAddresses.find(firstNetwork.Get());

    // test that network we are checking is in given container
    if (currentAddressIt != networkAddresses.end())
    {
        // calculate network and host count based on configured initial network address and
        // mask for the network space
        uint32_t hostAddressCount = std::pow(2, (32 - addressPrefixLength)) - 2;
        uint32_t firstAddressValue = firstNetwork.Get();
        uint32_t networkAddressCount =
            mask.Get() - firstAddressValue +
            1; // increase subtraction result by one, to include also first address

        currentAddressIt++;

        // test in the case that checked address space is not last ('highest') in the
        // given address container that the address space doesn't overlap with other configured
        // address spaces
        if ((currentAddressIt != networkAddresses.end()) &&
            (firstAddressValue + (networkCount << (32 - addressPrefixLength))) >= *currentAddressIt)
        {
            NS_FATAL_ERROR(networkName << " network's addresses overlaps with some other network)");
        }

        // test that enough network addresses are available in address space
        if (networkCount > networkAddressCount)
        {
            NS_FATAL_ERROR("Not enough network addresses for '" << networkName << "' network");
        }

        // test that enough host addresses are available in address space
        if (hostCount > hostAddressCount)
        {
            NS_FATAL_ERROR("Not enough host addresses for '" << networkName << "' network");
        }
    }
    else
    {
        NS_FATAL_ERROR(networkName
                       << "network's initial address number not among of the given addresses");
    }
}

void
SatHelper::ReadStandard(std::string pathName)
{
    NS_LOG_FUNCTION(this << pathName);

    // READ FROM THE SPECIFIED INPUT FILE
    std::ifstream* ifs = new std::ifstream(pathName.c_str(), std::ifstream::in);

    if (!ifs->is_open())
    {
        // script might be launched by test.py, try a different base path
        delete ifs;
        pathName = "../../" + pathName;
        ifs = new std::ifstream(pathName.c_str(), std::ifstream::in);

        if (!ifs->is_open())
        {
            NS_FATAL_ERROR("The file " << pathName << " is not found.");
        }
    }

    std::string standardString;
    *ifs >> standardString;

    ifs->close();
    delete ifs;

    if (standardString == "DVB")
    {
        m_standard = SatEnums::DVB;
    }
    else if (standardString == "LORA")
    {
        m_standard = SatEnums::LORA;
    }
    else
    {
        NS_FATAL_ERROR("Unknown standard: " << standardString << ". Must be DVB or LORA");
    }
}

} // namespace ns3
