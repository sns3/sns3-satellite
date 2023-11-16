/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 *
 */

#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-handover-example.cc
 * \ingroup satellite
 *
 * \brief  Test handover for UTs
 *
 */

NS_LOG_COMPONENT_DEFINE("sat-handover-example");

int
main(int argc, char* argv[])
{
    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    Config::SetDefault("ns3::SatUtHandoverModule::NumberClosestSats", UintegerValue(2));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    /// Enable packet trace
    Config::SetDefault("ns3::SatHelper::PacketTraceEnabled", BooleanValue(true));
    Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper>("example-handover");
    Ptr<SimulationHelperConf> simulationConf = CreateObject<SimulationHelperConf>();
    simulationHelper->SetSimulationTime(Seconds(100));
    simulationHelper->SetGwUserCount(1);
    simulationHelper->SetUserCountPerUt(1);
    simulationHelper->SetBeamSet({0,  20, 21, 22, 32, 33, 34, 35, 36, 37, 44, 45, 46,
                                  47, 53, 54, 55, 56, 59, 60, 61, 66, 67, 68, 71, 72});
    simulationHelper->SetUserCountPerMobileUt(simulationConf->m_utMobileUserCount);

    simulationHelper->LoadScenario("constellation-eutelsat-geo-2-sats-isls");

    std::string mobileUtFolder = Singleton<SatEnvVariables>::Get()->LocateDataDirectory() +
                                 "/additional-input/utpositions/mobiles/scenario6";
    Ptr<SatHelper> helper = simulationHelper->CreateSatScenario(SatHelper::NONE, mobileUtFolder);

    helper->PrintTopology(std::cout);

    Config::SetDefault("ns3::CbrApplication::Interval", StringValue("100ms"));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          Seconds(1.0),
                                          Seconds(100.0));

    // To store attributes to file
    Config::SetDefault("ns3::ConfigStore::Filename", StringValue("output-attributes.xml"));
    Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("Xml"));
    Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Save"));
    ConfigStore outputConfig;
    outputConfig.ConfigureDefaults();

    Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer();

    s->AddPerSatFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerSatFwdUserDevThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddPerBeamFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerBeamFwdUserDevThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerBeamBeamServiceTime(SatStatsHelper::OUTPUT_SCALAR_FILE);

    simulationHelper->EnableProgressLogs();
    simulationHelper->RunSimulation();

    return 0;
}
