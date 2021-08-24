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
 * \file sat-group-example.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation with a several groups of UTs
 *
 * Stats using groups:
 * - [Fwd, Rtn] AppDelay
 * - [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - [Fwd, Rtn] Average AppDelay
 * - [Fwd, Rtn] [Dev, Mac, Phy] Average Delay
 * - [Fwd, Rtn] AppJitter
 * - [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - [Fwd, Rtn] Average AppJitter
 * - [Fwd, Rtn] [Dev, Mac, Phy] Average Jitter
 * - [Fwd, Rtn] AppPlt
 * - [Fwd, Rtn] Average AppPlt
 * - [Fwd, Rtn] Queue [Bytes, Packets]
 * - [Fwd, Rtn] SignallingLoad
 * - [Fwd, Rtn] CompositeSinr
 * - [Fwd, Rtn] AppThroughput
 * - [Fwd, Rtn] [Dev, Mac, Phy] Throughput
 * - [Fwd, Rtn] Average AppThroughput
 * - [Fwd, Rtn] [Dev, Mac, Phy] Average Throughput
 * - [FwdDa, RtnDa, SlottedAloha, Crdsa, Essa] PacketError
 * - [SlottedAloha, Crdsa, Essa] PacketCollision
 * - CapacityRequest
 * - ResourcesGranted
 *
 * execute command -> ./waf --run "sat-group-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-group-example");

void
ConfigureAllStats (Ptr<SatStatsHelperContainer> s)
{
  SatStatsHelper::OutputType_t outputType = SatStatsHelper::OUTPUT_SCATTER_FILE; //Only format compatible with all per group statistics

  /*s->AddPerGroupFwdAppDelay (outputType);
  s->AddPerGroupRtnAppDelay (outputType);

  s->AddPerGroupFwdDevDelay (outputType);
  s->AddPerGroupRtnDevDelay (outputType);
  s->AddPerGroupFwdMacDelay (outputType);
  s->AddPerGroupRtnMacDelay (outputType);
  s->AddPerGroupFwdPhyDelay (outputType);
  s->AddPerGroupRtnPhyDelay (outputType);

  s->AddAverageGroupFwdAppDelay (outputType);
  s->AddAverageGroupRtnAppDelay (outputType);

  s->AddAverageGroupFwdDevDelay (outputType);
  s->AddAverageGroupRtnDevDelay (outputType);
  s->AddAverageGroupFwdMacDelay (outputType);
  s->AddAverageGroupRtnMacDelay (outputType);
  s->AddAverageGroupFwdPhyDelay (outputType);
  s->AddAverageGroupRtnPhyDelay (outputType);

  s->AddPerGroupFwdAppJitter (outputType);
  s->AddPerGroupRtnAppJitter (outputType);

  s->AddPerGroupFwdDevJitter (outputType);
  s->AddPerGroupRtnDevJitter (outputType);
  s->AddPerGroupFwdMacJitter (outputType);
  s->AddPerGroupRtnMacJitter (outputType);
  s->AddPerGroupFwdPhyJitter (outputType);
  s->AddPerGroupRtnPhyJitter (outputType);

  s->AddAverageGroupFwdAppJitter (outputType);
  s->AddAverageGroupRtnAppJitter (outputType);

  s->AddAverageGroupFwdDevJitter (outputType);
  s->AddAverageGroupRtnDevJitter (outputType);
  s->AddAverageGroupFwdMacJitter (outputType);
  s->AddAverageGroupRtnMacJitter (outputType);
  s->AddAverageGroupFwdPhyJitter (outputType);
  s->AddAverageGroupRtnPhyJitter (outputType);

  s->AddPerGroupFwdAppPlt (outputType);
  s->AddPerGroupRtnAppPlt (outputType);

  s->AddAverageGroupFwdAppPlt (outputType);
  s->AddAverageGroupRtnAppPlt (outputType);

  s->AddPerGroupFwdQueueBytes (outputType);
  s->AddPerGroupRtnQueueBytes (outputType);

  s->AddPerGroupFwdQueuePackets (outputType);
  s->AddPerGroupRtnQueuePackets (outputType);

  s->AddPerGroupFwdSignallingLoad (outputType);
  s->AddPerGroupRtnSignallingLoad (outputType);

  s->AddPerGroupFwdCompositeSinr (outputType);
  s->AddPerGroupRtnCompositeSinr (outputType);

  s->AddPerGroupFwdAppThroughput (outputType);
  s->AddPerGroupRtnAppThroughput (outputType);*/

  //s->AddPerGroupFwdDevThroughput (outputType);
  //s->AddPerGroupRtnDevThroughput (outputType);
  s->AddPerGroupFwdMacThroughput (outputType);
  s->AddPerGroupRtnMacThroughput (outputType);
  //s->AddPerGroupFwdPhyThroughput (outputType);
  //s->AddPerGroupRtnPhyThroughput (outputType);

  /*s->AddAverageGroupFwdAppThroughput (outputType);
  s->AddAverageGroupRtnAppThroughput (outputType);

  s->AddAverageGroupFwdDevThroughput (outputType);
  s->AddAverageGroupRtnDevThroughput (outputType);
  s->AddAverageGroupFwdMacThroughput (outputType);
  s->AddAverageGroupRtnMacThroughput (outputType);
  s->AddAverageGroupFwdPhyThroughput (outputType);
  s->AddAverageGroupRtnPhyThroughput (outputType);

  s->AddPerGroupFwdDaPacketError (outputType);
  s->AddPerGroupRtnDaPacketError (outputType);
  s->AddPerGroupSlottedAlohaPacketError (outputType);
  s->AddPerGroupCrdsaPacketError (outputType);
  s->AddPerGroupEssaPacketError (outputType);

  s->AddPerGroupSlottedAlohaPacketCollision (outputType);
  s->AddPerGroupCrdsaPacketCollision (outputType);
  s->AddPerGroupEssaPacketCollision (outputType);

  s->AddPerGroupCapacityRequest (outputType);

  s->AddPerGroupResourcesGranted (outputType);*/
}

int
main (int argc, char *argv[])
{
  // Enable info logs
  LogComponentEnable ("sat-group-example", LOG_LEVEL_INFO);

  // Variables
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (20);

  uint32_t packetSize (1500);
  Time interval (Seconds (1.0));

  Time appStartTime = Seconds (0.1);
  Time simLength (Seconds (60.0));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-group-example");

  // Parse command-line
  CommandLine cmd;
  cmd.AddValue ("UtsPerBeam", "Number of UTs per beam", utsPerBeam);
  cmd.AddValue ("PacketSize", "UDP packet size (in bytes)", packetSize);
  cmd.AddValue ("Interval", "CBR interval (in seconds, or add unit)", interval);
  cmd.AddValue ("SimLength", "Simulation length (in seconds, or add unit)", simLength);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  /// Set default values
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);

  // Set beam ID
  std::stringstream beamsEnabled;
  beamsEnabled << beamId;
  simulationHelper->SetBeams (beamsEnabled.str ());

  // Create reference system
  simulationHelper->CreateSatScenario ();
  Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();

  // Create groups
  Ptr<SatGroupHelper> groupHelper = satHelper->GetGroupHelper ();
  NodeContainer utNodes = satHelper->UtNodes ();

  groupHelper->AddUtNodeToGroup (1, utNodes.Get (0));

  NodeContainer nodes2To10;
  for (uint32_t i = 2; i < 11; i++)
    {
      nodes2To10.Add (utNodes.Get (i));
    }
  groupHelper->AddUtNodesToGroup (2, nodes2To10);

  // setup CBR traffic
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  // Setup custom traffics
  Ptr<SatTrafficHelper> trafficHelper = simulationHelper->GetTrafficHelper ();
  trafficHelper-> AddCbrTraffic (SatTrafficHelper::FWD_LINK,
                      "100ms",
                      packetSize,
                      satHelper->GetGwUsers ().Get (0),
                      satHelper->GetUtUsers (groupHelper->GetUtNodes (2)),
                      appStartTime,
                      appStartTime + simLength,
                      Seconds (0.05));

  trafficHelper-> AddCbrTraffic (SatTrafficHelper::RTN_LINK,
                      "1000ms",
                      packetSize,
                      satHelper->GetGwUsers ().Get (0),
                      satHelper->GetUtUsers (groupHelper->GetUtNodes (0)),
                      appStartTime,
                      appStartTime + simLength,
                      Seconds (0.05));

  NS_LOG_INFO ("--- sat-group-example ---");
  NS_LOG_INFO ("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO ("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO ("  Simulation length: " << simLength.GetSeconds ());
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of groups: " << groupHelper->GetN ());
  NS_LOG_INFO ("  Nodes in default group: " << groupHelper->GetUtNodes (0).GetN ());
  NS_LOG_INFO ("  Nodes in group 1: " << groupHelper->GetUtNodes (1).GetN ());
  NS_LOG_INFO ("  Nodes in group 2: " << groupHelper->GetUtNodes (2).GetN ());
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  // Set statistics
  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();
  simulationHelper->EnableProgressLogs ();

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("contrib/satellite/data/sims/sat-group-example/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  ConfigureAllStats (s);

  simulationHelper->RunSimulation ();

  return 0;
}

