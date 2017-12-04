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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"

#include <sys/stat.h>
#include <unistd.h>

using namespace ns3;

/**
 * \file sat-per-packet-if-sim-tn9.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results related to per-packet
 * interference performance.
 *
 * execute command -> ./waf --run "sat-per-packet-if-sim-tn9 --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-per-packet-if-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (220); // expected to be equivalent with 70% system load
  DataRate dataRate (32000); // in bps
  uint32_t beamConf (0);
  uint32_t fadingConf (0);

  uint32_t packetSize (1280); // in bytes
  double simLength (50.0); // in seconds
  Time appStartTime = Seconds (0.1);

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-per-packet-if-sim-tn9");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  // To read attributes from file
  Singleton<SatEnvVariables> satEnvVariables;
  std::string pathToFile = satEnvVariables.Get ()->LocateFile ("contrib/satellite/examples/tn9-dama-input-attributes.xml");

  /**
   * Attributes:
   * -----------
   *
   * Scenario (selected from command line argument):
   *   - 1 beam (beam id = 18)
   *   - 18 co-channel beams (user link frequency ID = 1)
   *
   * Frame configuration (configured in tn9-dama-input-attributes.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * NCC configuration modes
   *   - Conf-2 scheduling mode (dynamic time slots)
   *   - FCA disabled
   *
   * Fading configuration (selected from command line argument):
   *   - Markov
   *   - Rain
   *
   * RTN link
   *   - Per-packet interference
   *   - AVI error model
   *   - ARQ disabled
   * FWD link
   *   - ACM disabled
   *   - Per-packet interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("beamConf", "Beam configuration", beamConf);
  cmd.AddValue ("fadingConf", "Fading configuration (0: Markov, 1: Rain)", fadingConf);
  simulationHelper->AddDefaultUiArguments (cmd, pathToFile);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (pathToFile));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // NCC configuration
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));

  // RBDC
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (16));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));

  // Per-packet interference
  Config::SetDefault ("ns3::SatUtHelper::DaFwdLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::DaFwdLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::DaRtnLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGwHelper::DaRtnLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));

  switch (fadingConf)
    {
    case 0:
      {
        // Markov fading
        Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_MARKOV));
        break;
      }
    case 1:
      {
        Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));

        // Rain fading
        // Note, that the positions of the fading files do not necessarily match with the
        // beam location, since this example is not using list position allocator!
        Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
        Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName", StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));
        Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName", StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));

        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported fadingConf: " << fadingConf);
        break;
      }
    }

  simulationHelper->SetSimulationTime (simLength);
	simulationHelper->SetUtCountPerBeam (utsPerBeam);
	simulationHelper->SetUserCountPerUt (endUsersPerUt);
  switch (beamConf)
    {
    // Single beam
    case 0:
      {
        // Spot-beam over Finland
      	simulationHelper->SetBeamSet ({18});
        break;
      }
    // All co-channel beams (e.g., color 1 with 72/4 = 18 beams)
    case 1:
      {
      	// choose beams from user link frequency ID = 1 (TN3 appendix A)
      	simulationHelper->SetBeamSet ({1,3,5,7,9,22,24,26,28,30,44,46,48,50,59,61,70,72});
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported beam configuration: " << beamConf);
      }
    }

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario ();

  /**
   * Set-up CBR traffic
   */
  simulationHelper->InstallTrafficModel (
  		SimulationHelper::ONOFF,
			SimulationHelper::UDP,
			SimulationHelper::RTN_LINK,
			appStartTime,
			Seconds (simLength+1),
			MilliSeconds (50)
			);

  /**
   * Set-up statistics
   */
  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);

  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);

  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_CDF_FILE);

  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_CDF_FILE);

  NS_LOG_INFO ("--- sat-per-packet-if-sim-tn9 ---");
  NS_LOG_INFO ("  Packet size: " << packetSize);
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  /**
   * Store attributes into XML output
   */
  // std::stringstream filename;
  // filename << "tn9-per-packet-if-output-attributes-ut" << utsPerBeam
  //          << "-conf" << beamConf << ".xml";
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

