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
 * \file sat-constellation-example.cc
 * \ingroup satellite
 *
 * \brief TODO
 */

NS_LOG_COMPONENT_DEFINE ("sat-constellation-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;
  std::string interval = "10ms";
  std::string forwardRegeneration = "regeneration_network";
  std::string returnRegeneration = "regeneration_network";
  std::string startDate = "2022-11-13 12:00:00";

  std::map<std::string, SatEnums::RegenerationMode_t> mapForwardRegeneration { {"transparent", SatEnums::TRANSPARENT},
                                                                               {"regeneration_phy", SatEnums::REGENERATION_PHY},
                                                                               {"regeneration_network", SatEnums::REGENERATION_NETWORK}};
  std::map<std::string, SatEnums::RegenerationMode_t> mapReturnRegeneration { {"transparent", SatEnums::TRANSPARENT},
                                                                              {"regeneration_phy", SatEnums::REGENERATION_PHY},
                                                                              {"regeneration_link", SatEnums::REGENERATION_LINK},
                                                                              {"regeneration_network", SatEnums::REGENERATION_NETWORK}};

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-constellation");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s))", interval);
  cmd.AddValue ("forwardRegeneration", "Regeneration mode on forward link (transparent, regeneration_phy or regeneration_network)", forwardRegeneration);
  cmd.AddValue ("returnRegeneration", "Regeneration mode on forward link (transparent, regeneration_phy, regeneration_link or regeneration_network)", returnRegeneration);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  SatEnums::RegenerationMode_t forwardLinkRegenerationMode = mapForwardRegeneration[forwardRegeneration];
  SatEnums::RegenerationMode_t returnLinkRegenerationMode = mapReturnRegeneration[returnRegeneration];

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatConf::ForwardLinkRegenerationMode", EnumValue (forwardLinkRegenerationMode));
  Config::SetDefault ("ns3::SatConf::ReturnLinkRegenerationMode", EnumValue (returnLinkRegenerationMode));
  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100000));
  Config::SetDefault ("ns3::SatGeoUserPhy::QueueSize", UintegerValue (100000));

  /// Use constellation
  Config::SetDefault ("ns3::SatHelper::SatConstellationEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::SatConstellationFolder", StringValue ("eutelsat-geo-2-sats"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::StartDateStr", StringValue (startDate));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionEachRequest", BooleanValue (false));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionPeriod", TimeValue (Seconds (1)));

  /// Enable ACM
  Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  // Set tag, if output path is not explicitly defined

  simulationHelper->SetSimulationTime (Seconds (30));

  std::set<uint32_t> beamSetAll = {1, 2, 3, 4, 5, 6, 7, 8, 9,
                                   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                   20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                   30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                   40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                                   50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                                   60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
                                   70, 71, 72};

  std::set<uint32_t> beamSet = {11, 18};

  // Set beam ID
  simulationHelper->SetBeamSet (beamSet);
  simulationHelper->SetUserCountPerUt (5);

  LogComponentEnable ("sat-constellation-example", LOG_LEVEL_INFO);

  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario ();

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize) );

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    Seconds (1.0), Seconds (29.0));
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (1.0), Seconds (29.0));

  NS_LOG_INFO ("--- sat-constellation-example ---");
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

  // Throughput statistics
  s->AddPerUtFwdFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Delay statistics
  s->AddPerUtFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdDevDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnDevDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // link delay statistics
  s->AddPerUtFwdFeederPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserPhyLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserMacLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederDevLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserDevLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederDevLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserDevLinkDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Jitter statistics
  s->AddPerUtFwdPhyJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdMacJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdDevJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnPhyJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnMacJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnDevJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Link jitter statistics
  s->AddPerUtFwdFeederPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserPhyLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserMacLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederDevLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserDevLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederDevLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserDevLinkJitter (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Phy RX statistics
  s->AddPerUtFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Other statistics
  s->AddPerUtFwdFeederLinkModcod (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdUserLinkModcod (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnFeederLinkModcod (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnUserLinkModcod (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnFeederQueueBytes (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederQueueBytes (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederQueuePackets (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwFwdUserQueueBytes (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserQueueBytes (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserQueuePackets (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
