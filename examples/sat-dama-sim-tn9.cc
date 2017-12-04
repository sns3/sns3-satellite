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
 * \file sat-dama-sim-tn9.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results related to
 * satellite RTN link performance.
 *
 * execute command -> ./waf --run "sat-dama-sim-tn9 --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-dama-sim-tn9");

int
main (int argc, char *argv[])
{
  // LogComponentEnable ("sat-dama-sim-tn9", LOG_LEVEL_INFO);

  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (220); // 80% system load according to NCC-2
  uint32_t nccConf (0);
  uint32_t fadingConf (0);

  // 16 kbps per end user
  uint32_t packetSize (1280); // in bytes
  double intervalSeconds = 0.64;

  double simLength (300.0); // in seconds
  Time appStartTime = Seconds (0.1);

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-dama-sim-tn9");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/tn9-dama-input-attributes.xml";

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("nccConf", "NCC configuration", nccConf);
  cmd.AddValue ("fadingConf", "Fading configuration (0: Markov, 1: Rain)", fadingConf);
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  /**
   * Attributes:
   * -----------
   *
   * Scenario:
   *   - 1 beam (beam id = 18)
   *   - 80% system load
   *
   * Frame configuration (configured in tn9-dama-input-attributes.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * NCC configuration modes (selected from command line argument):
   *   - Conf-0 (static timeslots with ACM off)
   *   - Conf-1 (static timeslots with ACM on)
   *   - Conf-2 scheduling mode (dynamic time slots)
   *   - FCA disabled
   *
   * Fading configuration (selected from command line argument):
   *   - Markov
   *   - Rain
   *
   * RTN link
   *   - Constant interference
   *   - AVI error model
   *   - ARQ disabled
   *   - RBDC with periodical control slots
   * FWD link
   *   - ACM disabled
   *   - Constant interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetBeamSet ({beamId});

  // RBDC + periodical control slots
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (16));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));

  switch (nccConf)
    {
    case 0:
      {
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_0"));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (false));
        break;
      }
    case 1:
      {
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_1"));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));
        break;
      }
    case 2:
      {
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_2"));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported nccConf: " << nccConf);
        break;
      }
    }

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
        // Rain fading
        Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));

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

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  simulationHelper->CreateSatScenario ();


  // setup CBR traffic
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (Seconds (intervalSeconds)));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  simulationHelper->InstallTrafficModel (
  					SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
  					appStartTime, Seconds (simLength), MilliSeconds (50));

  /**
   * Set-up statistics
   */
  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_PLOT);

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_PLOT);

  NS_LOG_INFO ("--- sat-dama-sim-tn9 ---");
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
  //          << "-ncc" << nccConf
  //          << "-fading" << fadingConf << ".xml";
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

