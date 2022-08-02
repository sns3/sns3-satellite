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
 * \file sat-regeneration-example.cc
 * \ingroup satellite
 *
 * \brief This file gives an example of satellite regeneration.
 *
 * TODO complete brief
 * TODO add more cmd options
 * TODO Clean
 * TODO add other regeneration modes
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-regeneration-example");

int
main (int argc, char *argv[])
{
  uint32_t beamIdInFullScenario = 10;
  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "simple";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;
  bool useSlottedAloha = false;

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-regeneration");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("beamIdInFullScenario", "Id where Sending/Receiving UT is selected in FULL scenario. (used only when scenario is full) ", beamIdInFullScenario);
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue ("slottedAlohaScenario", "Use Slotted Aloha scenario to test collisions.", useSlottedAloha);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  if (useSlottedAloha)
    {
      packetSize = 512;
      interval = "1ms";

      // Enable Random Access
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));

      // Set Random Access interference model
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));

      // Set Random Access collision model
      Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

      // Disable periodic control slots
      Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));

      // Set dynamic load control parameters
      Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (false));
      Config::SetDefault ("ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize", UintegerValue (10));

      // Set random access parameters
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (250));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (10000));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (30000));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.5));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval", TimeValue (MilliSeconds (100)));
      Config::SetDefault ("ns3::SatRandomAccessConf::CrdsaSignalingOverheadInBytes", UintegerValue (5));
      Config::SetDefault ("ns3::SatRandomAccessConf::SlottedAlohaSignalingOverheadInBytes", UintegerValue (3));

      // Disable CRA and DA
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

      Config::SetDefault ("ns3::SatGeoHelper::FwdLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_NONE));
      Config::SetDefault ("ns3::SatGeoHelper::RtnLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_NONE));
      Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_ALWAYS_DROP_ALL_COLLIDING_PACKETS));
    }

  // Set tag, if output path is not explicitly defined
  simulationHelper->SetOutputTag (scenario);

  simulationHelper->SetSimulationTime (Seconds (11));

  // Set beam ID
  std::stringstream beamsEnabled;
  beamsEnabled << beamIdInFullScenario;
  simulationHelper->SetBeams (beamsEnabled.str ());

  LogComponentEnable ("sat-regeneration-example", LOG_LEVEL_INFO);

  Ptr<SatHelper> helper;
  if (useSlottedAloha)
    {
      simulationHelper->SetUserCountPerUt (1);
      simulationHelper->SetUtCountPerBeam (50);
      simulationHelper->SetBeamSet ({1});
      helper = simulationHelper->CreateSatScenario ();
    }
  else
    {
      helper = simulationHelper->CreateSatScenario (satScenario);
    }

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize) );

  if (!useSlottedAloha)
    {
      simulationHelper->InstallTrafficModel (
        SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
        Seconds (1.0), Seconds (10.0));
    }
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (1.0), Seconds (10.0));

  NS_LOG_INFO ("--- sat-regeneration-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  if ( scenario == "full" )
    {
      NS_LOG_INFO ("  UT used in full scenario from beam: " << beamIdInFullScenario );
    }
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

  s->AddPerGwFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerSatFeederSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatUserSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
