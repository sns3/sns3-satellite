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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 *
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-trace-output-example.cc
 * \ingroup satellite
 *
 * \brief  Trace output example application based on CBR example for satellite network.
 *         Interval, packet size and test scenario can be given in command line as user argument.
 *         To see help for user arguments, execute the command
 *
 *         ./waf --run "trace-output-example --PrintHelp"
 *
 *         This example application sends first packets from GW connected user
 *         to UT connected users and after that from UT connected user to GW connected
 *         user.
 *
 *         This example produces the following traces:
 *         - interference density trace
 *         - rx power density trace
 *         - fading trace
 *         - composite SINR
 *
 *         The output folder is:
 *         {NS-3-root-folder}/contrib/satellite/data/sims/example-trace-output/{scenario}
 */

NS_LOG_COMPONENT_DEFINE("sat-trace-output-example");

int
main(int argc, char* argv[])
{
    uint32_t packetSize = 512;
    std::string interval = "1s";
    std::string scenario = "simple";
    SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

    Config::SetDefault("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue(true));
    std::string simulationName = "example-trace-output";
    auto simulationHelper = CreateObject<SimulationHelper>(simulationName);

    /// Read command line parameters given by user
    CommandLine cmd;
    cmd.AddValue("packetSize", "Size of constant packet (bytes)", packetSize);
    cmd.AddValue("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
    cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
    simulationHelper->AddDefaultUiArguments(cmd);
    cmd.Parse(argc, argv);

    /// Enable Rx power calculation & Rx power density output trace
    Config::SetDefault("ns3::SatChannel::RxPowerCalculationMode",
                       EnumValue(SatEnums::RX_PWR_CALCULATION));
    Config::SetDefault("ns3::SatChannel::EnableRxPowerOutputTrace", BooleanValue(true));

    /// Enable Markov fading calculation & fading output trace
    Config::SetDefault("ns3::SatBeamHelper::FadingModel", EnumValue(SatEnums::FADING_MARKOV));
    Config::SetDefault("ns3::SatChannel::EnableFadingOutputTrace", BooleanValue(true));

    /// Enable per packet interference & interference density output trace
    Config::SetDefault("ns3::SatGwHelper::DaRtnLinkInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));
    Config::SetDefault("ns3::SatGeoHelper::DaRtnLinkInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));
    Config::SetDefault("ns3::SatGeoHelper::DaFwdLinkInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));
    Config::SetDefault("ns3::SatUtHelper::DaFwdLinkInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));
    Config::SetDefault("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace", BooleanValue(true));

    /// Enable composite SINR output trace
    Config::SetDefault("ns3::SatPhyRxCarrier::EnableCompositeSinrOutputTrace", BooleanValue(true));

    /// Set simulation output details
    simulationHelper->SetOutputTag(scenario);

    /// Disable figure output
    // Singleton<SatFadingOutputTraceContainer>::Get ()->EnableFigureOutput (false);
    // Singleton<SatInterferenceOutputTraceContainer>::Get ()->EnableFigureOutput (false);
    // Singleton<SatRxPowerOutputTraceContainer>::Get ()->EnableFigureOutput (false);
    // Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->EnableFigureOutput (false);

    /// Enable the printing of ID mapper trace IDs
    Singleton<SatIdMapper>::Get()->EnableMapPrint(true);

    if (scenario == "larger")
    {
        satScenario = SatHelper::LARGER;
    }
    else if (scenario == "full")
    {
        satScenario = SatHelper::FULL;
    }

    /// Enable info logs
    LogComponentEnable("CbrApplication", LOG_LEVEL_INFO);
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    LogComponentEnable("sat-trace-output-example", LOG_LEVEL_INFO);

    // Set simulation time
    simulationHelper->SetSimulationTime(Seconds(11));

    /// Remove next line from comments to run real time simulation
    // GlobalValue::Bind ("SimulatorImplementationType", StringValue
    // ("ns3::RealtimeSimulatorImpl"));

    // Creating the reference system. Note, currently the satellite module supports
    // only one reference system, which is named as "Scenario72". The string is utilized
    // in mapping the scenario to the needed reference system configuration files. Arbitrary
    // scenario name results in fatal error.
    simulationHelper->CreateSatScenario(satScenario);

    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(Time(interval)));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(packetSize));

    /// Create application on GW user
    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          Seconds(3.0),
                                          Seconds(5.1));

    /// Create application on UT user
    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::RTN_LINK,
                                          Seconds(7.0),
                                          Seconds(9.1));

    NS_LOG_INFO("--- Trace-output-example ---");
    NS_LOG_INFO("  Scenario used: " << scenario);
    NS_LOG_INFO("  PacketSize: " << packetSize);
    NS_LOG_INFO("  Interval: " << interval);
    NS_LOG_INFO("  ");

    simulationHelper->RunSimulation();

    return 0;
}
