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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"
#include "ns3/config-store-module.h"


using namespace ns3;

/**
 * \file sat-profiling-sim-tn8.cc
 * \ingroup satellite
 *
 *\brief An example used for running the performance profiling simulations
 * presented in TN8. The example supports a selection between a single beam
 * simulation and a full scenario. Additionally, it is possible to select
 * the number of UTs per beam.
 *
 * execute command -> ./waf --run "sat-profiling-sim-tn8 --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-profiling-sim-tn8");

void
TimeTickerCallback ()
{
  std::cout << "Time: " << Simulator::Now ().GetSeconds () << "s" << std::endl;
}

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (1);
  uint32_t profilingConf (0);

  // 256 kbps per end user
  uint32_t packetSize (1280); // in bytes
  double intervalSeconds = 0.04;

  double simLength; // defined later in scenario creation
  Time appStartTime = Seconds (0.1);

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-profiling-sim-tn8");

  // To read attributes from file
//  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/tn8-profiling-input-attributes.xml";
//  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
//  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
//  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
//  ConfigStore inputConfig;
//  inputConfig.ConfigureDefaults ();

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("profilingConf", "Profiling configuration", profilingConf);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (64));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Ptr<SatHelper> helper;
  switch (profilingConf)
    {
    // Single beam
    case 0:
      {
        // Spot-beam over Finland
        uint32_t beamId = 18;
        simLength = 60.0; // in seconds

        // create user defined scenario
        simulationHelper->SetUtCountPerBeam (utsPerBeam);
        simulationHelper->SetUserCountPerUt (endUsersPerUt);
        simulationHelper->SetBeamSet ({beamId});
        simulationHelper->SetSimulationTime (simLength);
        helper = simulationHelper->CreateSatScenario ();
        break;
      }
    // Full
    case 1:
      {
        simLength = 30.0; // in seconds

        simulationHelper->SetSimulationTime (simLength);
        helper = simulationHelper->CreateSatScenario (SatHelper::FULL);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Invalid profiling configuration");
      }
    }

  /**
   * Set-up CBR traffic
   */

	Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));
	Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (Seconds (intervalSeconds)));
	simulationHelper->InstallTrafficModel (
	  		SimulationHelper::CBR,
				SimulationHelper::UDP,
				SimulationHelper::RTN_LINK,
				appStartTime, Seconds (simLength + 1), MilliSeconds (10));

  /**
   * Set-up statistics
   */
  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);

  NS_LOG_INFO ("--- sat-profiling-sim-tn8 ---");
  NS_LOG_INFO ("  Packet size: " << packetSize);
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  /**
   * Store attributes into XML output
   */
  std::stringstream filename;
  filename << "tn8-profiling-output-attributes-conf-" << profilingConf << "-uts-" << utsPerBeam << ".xml";

//  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (filename.str ()));
//  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
//  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
//  ConfigStore outputConfig;
//  outputConfig.ConfigureDefaults ();

  /**
   * Install simple stdout time ticker
   */

  double t = 0.0;
  while (t <= simLength)
    {
      Simulator::Schedule (Seconds (t), &TimeTickerCallback);
      t = t + 1.0;
    }

  /**
   * Run simulation
   */
  simulationHelper->RunSimulation ();

  return 0;
}

