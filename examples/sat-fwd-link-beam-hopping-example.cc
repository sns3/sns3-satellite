/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"


using namespace ns3;

/**
 * \file sat-fwd-link-beam-hopping-example.cc
 * \ingroup satellite
 *
 * This simulation script is an example of FWD link beam hopping
 * configuration. All spot-beams of GW-1 are enabled and a proper
 * beam hopping pattern is set at the simulation helper. Each spot-
 * beam has by default even loading.
 *
 *         execute command -> ./waf --run "sat-fwd-link-beam-hopping-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-fwd-link-beam-hopping-example");

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (1);
  bool enableBeamHopping (true);
  Time simLength (Seconds (3.0));
  bool scaleDown (true);

  std::string simulationName ("sat-fwd-link-beam-hopping-example");
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> (simulationName);

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("enableBeamHopping", "Enable FWD link beam hopping", enableBeamHopping);
  cmd.AddValue ("simTime", "Length of simulation", simLength);
  cmd.AddValue ("scaleDown", "Scale down the bandwidth to see differences with less traffic", scaleDown);
  simulationHelper->AddDefaultUiArguments(cmd);
  cmd.Parse (argc, argv);

  simulationHelper->SetDefaultValues ();
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  if (enableBeamHopping) simulationHelper->ConfigureFwdLinkBeamHopping ();
  if (scaleDown)
  {
    Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (enableBeamHopping ?  1e+08 : 2.5e+07));
  }
  simulationHelper->SetSimulationTime (simLength.GetSeconds());

  // All spot-beams of GW-1 (14 in total)
  simulationHelper->SetBeams ("1 2 3 4 11 12 13 14 25 26 27 28 40 41");
  std::map<uint32_t, uint32_t> utsInBeam = {{1, 30}, {2, 9}, {3, 15}, {4, 30},
                                            {11, 15}, {12, 30}, {13, 9}, {14, 18},
                                            {25, 9}, {26, 15}, {27, 18}, {28, 30},
                                            {40, 9}, {41, 15}};

  // Set users unevenly in different beams
  for (const auto it : utsInBeam)
  {
    simulationHelper->SetUtCountPerBeam (it.first, it.second);
  }

  // Create the scenario
  simulationHelper->CreateSatScenario ();

  // Install traffic model
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512) );
  simulationHelper->InstallTrafficModel (SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
                                         Seconds (0.001), simLength, Seconds (0.001));

  auto stats = simulationHelper->GetStatisticsContainer ();
  stats->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  stats->AddPerBeamFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  stats->AddPerBeamBeamServiceTime (SatStatsHelper::OUTPUT_SCALAR_FILE);
  stats->AddGlobalFwdAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  stats->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);

  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
