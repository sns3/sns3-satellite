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
 *
 */

#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * @file sat-generic-launcher.cc
 * @ingroup satellite
 *
 * @brief Simulation script to run example simulation results with
 * a high degree of customization through XML file.
 *
 * execute command -> ./waf --run "sat-generic-launcher --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE("sat-generic-launcher");

static double g_txMaxPower = 5.0;
static bool g_ascending = false;
static Time g_cnoInterval = MilliSeconds(100);
static Time g_simulationTime = Minutes(1);

static void
ChangeCno(const std::vector<Ptr<SatUtPhy>>& utsPhysicalLayers)
{
    g_txMaxPower += g_ascending ? 0.2 : -0.2;
    g_ascending = (g_ascending && g_txMaxPower < 30.0) || (!g_ascending && g_txMaxPower < -30.0);

    for (auto& phy : utsPhysicalLayers)
    {
        phy->SetAttribute("TxMaxPowerDbw", DoubleValue(g_txMaxPower));
        phy->Initialize();
    }

    Simulator::Schedule(g_cnoInterval, &ChangeCno, utsPhysicalLayers);
}

int
main(int argc, char* argv[])
{
    bool varyingCno(false);
    uint32_t maxSubdivisions(0);
    uint32_t frameConfigType(2);
    double initialBandwidth(3.75e6);
    Time superframeDuration(MicroSeconds(26500));
    std::string inputFileNameWithPath =
        SatEnvVariables::GetInstance()->LocateDirectory("contrib/satellite/examples") +
        "/generic-input-attributes.xml";

    Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper>("generic-launcher");
    simulationHelper->SetDefaultValues();

    CommandLine cmd;
    cmd.AddValue("MaxCarrierSubdivision",
                 "The maximum amount of subdivision for a single carrier",
                 maxSubdivisions);
    cmd.AddValue("FrameConfigType",
                 "The frame configuration type used for super frame",
                 frameConfigType);
    cmd.AddValue("UseVaryingCno",
                 "Simulate varying C/N0 for UTs instead of changing their traffic overtime",
                 varyingCno);
    simulationHelper->AddDefaultUiArguments(cmd, inputFileNameWithPath);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::SimulationHelperConf::BeamsIDs", StringValue("12"));
    Config::SetDefault("ns3::SimulationHelperConf::UtCountPerBeam",
                       StringValue("ns3::ConstantRandomVariable[Constant=30]"));
    Config::SetDefault("ns3::SimulationHelperConf::UserCountPerUt",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));

    Config::SetDefault("ns3::SatSuperframeConf0::FrameCount", UintegerValue(1));
    Config::SetDefault("ns3::SatSuperframeConf0::FrameConfigType",
                       StringValue("ConfigType_" + std::to_string(frameConfigType)));
    Config::SetDefault("ns3::SatSuperframeConf0::MaxCarrierSubdivision",
                       UintegerValue(maxSubdivisions));
    Config::SetDefault("ns3::SatSuperframeConf0::Frame0_AllocatedBandwidthHz",
                       DoubleValue(initialBandwidth));
    Config::SetDefault("ns3::SatSuperframeConf0::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(initialBandwidth));
    Config::SetDefault("ns3::SatSuperframeConf0::Frame0_CarrierRollOff", DoubleValue(0.2));
    Config::SetDefault("ns3::SatSuperframeConf0::Frame0_CarrierSpacing", DoubleValue(0.0));
    Config::SetDefault("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue(false));

    Config::SetDefault("ns3::SatSuperframeSeq::TargetDuration", TimeValue(superframeDuration));
    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(superframeDuration));

    Config::SetDefault("ns3::SatFwdLinkScheduler::CnoEstimationWindow",
                       TimeValue(MilliSeconds(500)));
    Config::SetDefault("ns3::SatRequestManager::CnoReportInterval", TimeValue(g_cnoInterval));
    Config::SetDefault("ns3::SatBeamScheduler::CnoEstimationMode",
                       StringValue("MinimumValueInWindow"));
    Config::SetDefault("ns3::SatBeamScheduler::CnoEstimationWindow", TimeValue(g_cnoInterval));

    Config::SetDefault("ns3::SatUtPhy::TxMaxPowerDbw", DoubleValue(g_txMaxPower));
    Config::SetDefault("ns3::SatOrbiterUserPhy::TxMaxPowerDbw", DoubleValue(15.0));
    Config::SetDefault("ns3::SatOrbiterFeederPhy::FixedAmplificationGainDb", DoubleValue(200.0));

    simulationHelper->ReadInputAttributesFromFile(inputFileNameWithPath);

    // Manual configuration of the simulation to avoid creating unnecessary traffic
    Ptr<SimulationHelperConf> simulationConf = CreateObject<SimulationHelperConf>();
    g_simulationTime = simulationConf->m_simTime;
    simulationHelper->SetBeams(simulationConf->m_enabledBeams);
    simulationHelper->SetUtCountPerBeam(simulationConf->m_utCount);
    simulationHelper->SetUserCountPerUt(simulationConf->m_utUserCount);
    simulationHelper->SetUserCountPerMobileUt(simulationConf->m_utMobileUserCount);
    simulationHelper->SetSimulationTime(g_simulationTime);

    simulationHelper->LoadScenario("geo-33E");

    simulationHelper->CreateSatScenario(SatHelper::NONE, simulationConf->m_mobileUtsFolder);
    if (simulationConf->m_activateProgressLogging)
    {
        simulationHelper->EnableProgressLogs();
    }

    simulationHelper->StoreAttributesToFile("parametersUsed.xml");

    if (varyingCno)
    {
        std::vector<Ptr<SatUtPhy>> utsPhysicalLayers;
        NodeContainer utNodes = Singleton<SatTopology>::Get()->GetUtNodes();
        Ptr<Node> node;
        for (NodeContainer::Iterator it = utNodes.Begin(); it != utNodes.End(); it++)
        {
            node = *it;
            for (uint32_t j = 0; j < node->GetNDevices(); ++j)
            {
                Ptr<SatNetDevice> dev = DynamicCast<SatNetDevice>(node->GetDevice(j));
                if (dev != nullptr)
                {
                    Ptr<SatUtPhy> phy = DynamicCast<SatUtPhy>(dev->GetPhy());
                    if (phy != nullptr)
                    {
                        utsPhysicalLayers.push_back(phy);
                    }
                }
            }
        }
        Simulator::Schedule(Seconds(0), &ChangeCno, utsPhysicalLayers);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            512,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(0),
            g_simulationTime,
            MilliSeconds(50));
    }
    else
    {
        // Configure our own kind of traffic
        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            40,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(0),
            g_simulationTime,
            MilliSeconds(50));

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            25600,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(0),
            Seconds(10),
            MilliSeconds(50),
            0.3);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            1000,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(5),
            Seconds(15),
            MilliSeconds(50),
            0.4);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            1000,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(10),
            Seconds(20),
            MilliSeconds(50),
            0.5);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            40000,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(15),
            Seconds(25),
            MilliSeconds(50),
            0.2);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            1,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(20),
            Seconds(30),
            MilliSeconds(50),
            0.7);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            100,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(25),
            Seconds(35),
            MilliSeconds(50),
            0.45);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            3000,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(30),
            Seconds(40),
            MilliSeconds(50),
            0.55);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            40000,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(35),
            Seconds(45),
            MilliSeconds(50),
            0.2);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            30000,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(40),
            Seconds(50),
            MilliSeconds(50),
            0.3);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            1500,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(45),
            Seconds(55),
            MilliSeconds(50),
            0.6);

        simulationHelper->GetTrafficHelper()->AddCbrTraffic(
            SatTrafficHelper::RTN_LINK,
            SatTrafficHelper::UDP,
            superframeDuration,
            800,
            NodeContainer(Singleton<SatTopology>::Get()->GetGwUserNode(0)),
            Singleton<SatTopology>::Get()->GetUtUserNodes(),
            Seconds(50),
            Seconds(60),
            MilliSeconds(50),
            0.9);
    }

    if (simulationConf->m_activateStatistics)
    {
        simulationHelper->CreateDefaultStats();
        Ptr<SatStatsHelperContainer> stats = simulationHelper->GetStatisticsContainer();
        stats->AddGlobalRtnFeederLinkRxPower(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddGlobalRtnFeederLinkRxPower(SatStatsHelper::OUTPUT_SCATTER_FILE);
        stats->AddGlobalRtnUserLinkRxPower(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddGlobalRtnUserLinkRxPower(SatStatsHelper::OUTPUT_SCATTER_FILE);
        stats->AddPerUtRtnAppDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddPerUtRtnAppDelay(SatStatsHelper::OUTPUT_SCATTER_FILE);
        stats->AddPerUtRtnMacDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddPerUtRtnMacDelay(SatStatsHelper::OUTPUT_SCATTER_FILE);
        stats->AddPerUtRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddPerUtRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
        stats->AddPerUtCarrierId(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddPerUtCarrierId(SatStatsHelper::OUTPUT_SCATTER_FILE);
        stats->AddPerUtRtnCompositeSinr(SatStatsHelper::OUTPUT_SCALAR_FILE);
        stats->AddPerUtRtnCompositeSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);
    }

    simulationHelper->RunSimulation();
}
