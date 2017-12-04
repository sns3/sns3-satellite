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


using namespace ns3;

/**
 * \file sat-dama-onoff-sim-tn9.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results related to
 * satellite RTN link performance.
 *
 * execute command -> ./waf --run "sat-dama-onoff-sim-tn9 --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-dama-onoff-sim-tn9");

int
main (int argc, char *argv[])
{
  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (220); // 70% system load according to CRA 1 kbps
  uint32_t packetSize (1280); // in bytes
  double simLength (300.0); // in seconds
  Time appStartTime = Seconds (0.1);

  DataRate dataRate (32000); // in bps
  uint32_t damaConf (0);
  uint32_t crTxConf (0);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-dama-onoff-sim-tn9");

  // To read attributes from file
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/tn9-dama-input-attributes.xml";

  /**
   * Attributes:
   * -----------
   *
   * Scenario:
   *   - 1 beam (beam id = 18)
   *   - 70% system load
   *
   * Frame configuration (configured in tn9-dama-input-attributes.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * DAMA configuration mode (selected from command line argument):
   *   - RBDC
   *   - VBDC
   *
   * NCC configuration mode:
   *   - Conf-2 scheduling mode (dynamic time slots)
   *   - FCA disabled
   *
   * CR transmission modes (selected from command line argument):
   *   - RA slotted ALOHA
   *   - CDRSA (strict RC 0)
   *   - Periodical control slots
   *   - CRA 1kbps
   *
   * Fading configuration:
   *   - Rain
   *   - Markov fading disabled
   *
   * RTN link
   *   - Constant interference
   *   - AVI error model
   *   - ARQ disabled
   * FWD link
   *   - ACM disabled
   *   - Constant interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("damaConf", "DAMA configuration", damaConf);
  cmd.AddValue ("crTxConf", "CR transmission configuration", crTxConf);
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  simulationHelper->SetDefaultValues ();

  // Set default attribute values from XML file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  // Set beam ID
  simulationHelper->SetBeamSet ({beamId});
  simulationHelper->SetSimulationTime (simLength);

  // NCC configuration
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("ConfigType_2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));

  // Rain fading
  // Note, that the positions of the fading files do not necessarily match with the
  // beam location, since this example is not using list position allocator!
  Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName", StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName", StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));

  // Disable Markov fading
  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));

  switch (damaConf)
    {
    // RBDC
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (16));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
        break;
      }
    // VBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported damaConf: " << damaConf);
        break;
      }
    }

  switch (crTxConf)
    {
    // RA slotted ALOHA
    case 0:
      {
        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    // CRDSA (strict RC 0)
    case 1:
      {
        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_CRDSA));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        Config::SetDefault ("ns3::SatUtHelper::UseCrdsaOnlyForControlPackets", BooleanValue (true));
        break;
      }
    // Periodical control slots
    case 2:
      {
        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
        break;
      }
    // CRA 1 kbps
    case 3:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));

        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported crTxConf: " << crTxConf);
        break;
      }
    }

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  simulationHelper->CreateSatScenario ();

  /**
   * Set-up on-off traffic
   */
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));

  simulationHelper->InstallTrafficModel (
  		SimulationHelper::ONOFF, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
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

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_PLOT);

  s->AddPerBeamCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  NS_LOG_INFO ("--- sat-dama-onoff-sim-tn9 ---");
  NS_LOG_INFO ("  Packet size: " << packetSize);
  NS_LOG_INFO ("  Offered data rate: " << dataRate);
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  /**
   * Store attributes into XML output
   */

  // std::stringstream filename;
  // filename << "tn9-dama-onoff-output-attributes-ut" << utsPerBeam
  //          << "-conf" << crTxConf << ".xml";
  //
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (filename.str ()));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig;
  // outputConfig.ConfigureDefaults ();

  /**
   * Run simulation
   */
  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}

