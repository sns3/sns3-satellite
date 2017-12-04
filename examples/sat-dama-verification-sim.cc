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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
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
 * \file sat-dama-verification-sim.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results related to
 * satellite RTN link performance.
 *
 * execute command -> ./waf --run "sat-dama-verification-sim --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-dama-verification-sim");

int
main (int argc, char *argv[])
{
  // LogComponentEnable ("sat-dama-verification-sim", LOG_LEVEL_INFO);

  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (220);

  // CBR
  uint32_t packetSize (1280); // in bytes
  double intervalSeconds = 0.005;

  double simLength (50.0); // in seconds
  Time appStartTime = Seconds (0.1);

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-dama-verification-sim");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));


  // Find the input xml file in case example is run from other than ns-3 root directory
  Singleton<SatEnvVariables> satEnvVariables;
  std::string pathToFile = satEnvVariables.Get ()->LocateFile ("contrib/satellite/examples/tn9-dama-input-attributes.xml");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("packetSize", "Packet size in bytes", packetSize);
  simulationHelper->AddDefaultUiArguments (cmd, pathToFile);
  cmd.Parse (argc, argv);

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (pathToFile));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetBeamSet ({beamId});

  // 5 ms -> 200 packets per second
  // 250 B -> 400 kbps
  // 500 B -> 800 kbps
  // 750 B -> 1200 kbps
  // 1000 B -> 1600 kbps
  // 1250 B -> 2000 kbps

  // RBDC + periodical control slots
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (10));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue (Seconds (1)));

  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));

  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));

  /**
   * Carriers:
   * 80 x 312.5 kHz
   * 80 x 625 kHz
   * 40 x 1.25 MHz
   */
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameCount", UintegerValue (3));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_AllocatedBandwidthHz", DoubleValue (2.5e+07));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_AllocatedBandwidthHz", DoubleValue (5e+07));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame2_AllocatedBandwidthHz", DoubleValue (5e+07));

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  simulationHelper->CreateSatScenario ();

  /**
   * Set-up CBR traffic
   */
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (Seconds (intervalSeconds)));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  simulationHelper->InstallTrafficModel (
  					SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
  					appStartTime, Seconds (simLength), MilliSeconds (50));
  /**
   * Set-up statistics
   */
  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddGlobalRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);


  NS_LOG_INFO ("--- sat-dama-verification-sim ---");
  NS_LOG_INFO ("  Packet size: " << packetSize);
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  /**
   * Store attributes into XML output
   */
  // std::stringstream filename;
  // filename << "tn9-dama-output-attributes-ut" << utsPerBeam
  //          << "-packetSize" << packetSize
  //          << ".xml";
  //
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (filename.str ()));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig;
  // outputConfig.ConfigureDefaults ();

  /**
   * Run simulation
   */
  simulationHelper->RunSimulation ();

  return 0;
}

