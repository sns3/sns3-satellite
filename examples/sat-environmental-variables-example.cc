/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/core-module.h"
#include "ns3/satellite-module.h"

using namespace ns3;

/**
 * \file sat-environmental-variables-example.cc
 * \ingroup satellite
 *
 * \brief Example for environmental variables class.
 * Demonstrates various functions for locating directories
 * used by the satellite module.
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-environmental-variables-example");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("sat-environmental-variables-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatEnvVariables", LOG_LEVEL_INFO);

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("exampleCampaign");
  simulationHelper->SetOutputTag ("exampleTag");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  CommandLine cmd;
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  /// Create default environmental variables
  Ptr<SatEnvVariables> envVariables = CreateObject<SatEnvVariables> ();

  /// Run simulation
  Simulator::Schedule (MilliSeconds (0), &SatEnvVariables::GetCurrentWorkingDirectory, envVariables);
  Simulator::Schedule (MilliSeconds (1), &SatEnvVariables::GetPathToExecutable, envVariables);
  Simulator::Schedule (MilliSeconds (2), &SatEnvVariables::GetDataPath, envVariables);
  Simulator::Schedule (MilliSeconds (3), &SatEnvVariables::GetOutputPath, envVariables);
  Simulator::Schedule (MilliSeconds (4), &SatEnvVariables::LocateDataDirectory, envVariables);
  Simulator::Schedule (MilliSeconds (5), &SatEnvVariables::LocateDirectory, envVariables, "contrib/satellite/data");
  Simulator::Schedule (MilliSeconds (6), &SatEnvVariables::IsValidDirectory, envVariables, "contrib/satellite/data/notfound");
  Simulator::Schedule (MilliSeconds (7), &SatEnvVariables::GetCurrentDateAndTime, envVariables);

  simulationHelper->SetSimulationTime (Seconds (1));
  simulationHelper->RunSimulation ();

  return 0;
}
