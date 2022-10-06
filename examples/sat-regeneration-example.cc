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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-regeneration-example.cc
 * \ingroup satellite
 *
 * \brief This file gives an example of satellite regeneration.
 *
 * TODO complete brief
 * TODO add more cmd options
 * TODO Clean
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-regeneration-example");

int
main (int argc, char *argv[])
{
  uint32_t beamIdInFullScenario = 10;
  uint32_t packetSize = 512;
  std::string interval = "10ms";
  std::string scenario = "simple";
  std::string forwardRegeneration = "regeneration_phy";
  std::string returnRegeneration = "regeneration_link";

  std::map<std::string, SatHelper::PreDefinedScenario_t> mapScenario { {"simple", SatHelper::SIMPLE},
                                                                       {"larger", SatHelper::LARGER},
                                                                       {"full", SatHelper::FULL}};
  std::map<std::string, SatEnums::RegenerationMode_t> mapForwardRegeneration { {"transparent", SatEnums::TRANSPARENT},
                                                                               {"regeneration_phy", SatEnums::REGENERATION_PHY},
                                                                               {"regeneration_network", SatEnums::REGENERATION_NETWORK}};
  std::map<std::string, SatEnums::RegenerationMode_t> mapReturnRegeneration { {"transparent", SatEnums::TRANSPARENT},
                                                                              {"regeneration_phy", SatEnums::REGENERATION_PHY},
                                                                              {"regeneration_link", SatEnums::REGENERATION_LINK},
                                                                              {"regeneration_network", SatEnums::REGENERATION_NETWORK}};

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-regeneration");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("beamIdInFullScenario", "Id where Sending/Receiving UT is selected in FULL scenario. (used only when scenario is full) ", beamIdInFullScenario);
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s))", interval);
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full)", scenario);
  cmd.AddValue ("forwardRegeneration", "Regeneration mode on forward link (transparent, regeneration_phy or regeneration_network)", forwardRegeneration);
  cmd.AddValue ("returnRegeneration", "Regeneration mode on forward link (transparent, regeneration_phy, regeneration_link or regeneration_network)", returnRegeneration);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  SatHelper::PreDefinedScenario_t satScenario = mapScenario[scenario];
  SatEnums::RegenerationMode_t forwardLinkRegenerationMode = mapForwardRegeneration[forwardRegeneration];
  SatEnums::RegenerationMode_t returnLinkRegenerationMode = mapReturnRegeneration[returnRegeneration];

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatConf::ForwardLinkRegenerationMode", EnumValue (forwardLinkRegenerationMode));
  Config::SetDefault ("ns3::SatConf::ReturnLinkRegenerationMode", EnumValue (returnLinkRegenerationMode));
  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100000));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  // Set tag, if output path is not explicitly defined
  simulationHelper->SetOutputTag (scenario);

  simulationHelper->SetSimulationTime (Seconds (30));

  // Set beam ID
  std::stringstream beamsEnabled;
  beamsEnabled << beamIdInFullScenario;
  simulationHelper->SetBeams (beamsEnabled.str ());

  LogComponentEnable ("sat-regeneration-example", LOG_LEVEL_INFO);

  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize) );

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    Seconds (1.0), Seconds (29.0));
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (1.0), Seconds (29.0));

  NS_LOG_INFO ("--- sat-regeneration-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  if ( scenario == "full" )
    {
      NS_LOG_INFO ("  UT used in full scenario from beam: " << beamIdInFullScenario );
    }
  NS_LOG_INFO ("  PacketSize: " << packetSize);
  NS_LOG_INFO ("  Interval: " << interval);
  NS_LOG_INFO ("  ");

  // To store attributes to file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddPerGwFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerSatFwdFeederPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerSatFwdFeederPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtRtnFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerBeamRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnFeederQueueBytes (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederQueueBytes (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederQueuePackets (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerSatFwdFeederMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnMacJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnAppJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
