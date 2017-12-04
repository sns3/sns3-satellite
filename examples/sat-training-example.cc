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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *
 */


#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"
#include "ns3/config-store-module.h"

using namespace ns3;

/**
 * \file sat-training-example.cc
 * \ingroup satellite
 *
 * \brief Simulation script to be utilized in SNS3 training. The script illustrates
 * the simulation script process starting from command line arguments to running the
 * actual simulation.
 *
 * int main (int argc, char *argv[])
 * {
 *   // Set default attribute values
 *   // Parse command-line arguments
 *   // Configure the topology; nodes, channels, devices, mobility
 *   // Add (Internet) stack to nodes
 *   // Configure IP addressing and routing
 *   // Add and configure applications
 *   // Configure tracing
 *   // Run simulation
 * }
 *
 * execute command -> ./waf --run "sat-training-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-training-example");

int
main (int argc, char *argv[])
{
  // Enabling logging in this program
  LogComponentEnable ("sat-training-example", LOG_LEVEL_INFO);

  NS_LOG_INFO ("--- sat-training-example ---");

  /**
   * --------------------------------------
   * Initialize simulation script variables
   * --------------------------------------
   */
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (1);
  double simDuration (10.0); // in seconds

  // All the co-channel beams enabled for user link
  // frequency color 1
  std::set<uint32_t> coChannelBeams = {1, 3, 5, 7, 9, 22, 24, 26, 28, 30, 44, 46, 48, 50,  59, 61};

  /**
   * Create simulation helper
   */
  std::string simulationName = "example-training";
  auto simulationHelper = CreateObject<SimulationHelper> (simulationName);

  // Find the input xml file in case example is run from other than ns-3 root directory
  Singleton<SatEnvVariables> satEnvVariables;
  std::string pathToFile = satEnvVariables.Get ()->LocateFile ("contrib/satellite/examples/training-input-attributes.xml");


  /**
   * ----------------------------------------------------------------
   * Read the command line arguments. Note, that this allows the user
   * to change also the ns3 attributes from command line.
   * ----------------------------------------------------------------
   */
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("simDurationInSeconds", "Simulation duration in seconds", simDuration);
  simulationHelper->AddDefaultUiArguments (cmd, pathToFile);
  cmd.Parse (argc, argv);


  /**
   * ---------------------------------------------------
   * Read the default attributes from XML attribute file
   * ---------------------------------------------------
   */
  NS_LOG_INFO ("Reading the XML input: training-input-attributes.xml");

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (pathToFile));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  /**
   * -----------------------------------------------
   * Overwrite some attribute values for this script
   * -----------------------------------------------
   */

  // Enable RBDC for BE
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (40));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue (Seconds (1)));

  // Tune the superframe configuration
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameCount", UintegerValue (3));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_AllocatedBandwidthHz", DoubleValue (5e+06));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_AllocatedBandwidthHz", DoubleValue (10e+06));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame2_AllocatedBandwidthHz", DoubleValue (10e+06));

  // Enable traces
  Config::SetDefault ("ns3::SatChannel::EnableRxPowerOutputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatChannel::EnableFadingOutputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatPhyRxCarrier::EnableCompositeSinrOutputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace", BooleanValue (true));

  Singleton<SatFadingOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  Singleton<SatInterferenceOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  Singleton<SatRxPowerOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->EnableFigureOutput (false);

  // Enable creation traces
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  // Enable packet traces
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));


  /**
   * --------------------------------------------------------------------
   * Create satellite system by the usage of satellite helper structures:
   * - simulationHelper->CreateSatScenario (SatHelper::SIMPLE)
   * --------------------------------------------------------------------
   */
  NS_LOG_INFO ("Creating the satellite scenario");
  // Each beam will have 'utsPerBeam' user terminals and 'endUsersPerUt'
	// end users per UT. Note, that this allows also different configurations
	// per spot-beam.
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetBeamSet (coChannelBeams);
  simulationHelper->SetSimulationTime (simDuration);

  simulationHelper->CreateSatScenario ();

  /**
   * --------------------------------------------------------
   * Configure end user applications.
   * In the training example, the users are configured on-off
   * application in return link. In addition, e.g. HTTP, NRTV
   * and CBR traffic models are supported.
   */

  NS_LOG_INFO ("Configuring the on-off application!");

  uint32_t packetSize (1280); // in bytes
  DataRate dataRate (128000); // in bps

  // The application start time is varied to avoid the situation
  // in the beginning that all applications start at the same time.
  Time appStartTime (MilliSeconds (100));

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", DataRateValue (dataRate));
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));

  simulationHelper->InstallTrafficModel (
      		SimulationHelper::ONOFF,
    			SimulationHelper::UDP,
    			SimulationHelper::RTN_LINK,
					appStartTime, Seconds (simDuration + 1), MilliSeconds (25));

  /**
   * -----------------
   * Set-up statistics
   * -----------------
   */
  NS_LOG_INFO ("Setting up statistics");

  // SatStatsHelperContainer is the interface for satellite related
  // statistics configuration.
  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  // Delay
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddAverageBeamRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // Composite SINR
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);

  // Throughput
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerGwRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  /**
   * --------------------------------
   * Store attributes into XML output
   * --------------------------------
   */

  NS_LOG_INFO ("Storing the used attributes to XML file: training-output-attributes-ut-" << utsPerBeam << ".xml");

  // std::stringstream filename;
  // filename << "training-output-attributes-ut" << utsPerBeam << ".xml";
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (filename.str ()));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig;
  // outputConfig.ConfigureDefaults ();

  /**
   * --------------
   * Run simulation
   * --------------
   */
  NS_LOG_INFO ("Running network simulator 3");

  simulationHelper->RunSimulation ();

  return 0;
}

