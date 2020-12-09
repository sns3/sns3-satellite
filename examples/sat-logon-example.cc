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
 * \file sat-logon-example.cc
 * \ingroup satellite *
 */

NS_LOG_COMPONENT_DEFINE ("sat-logon-example");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt = 1;
  uint32_t utsPerBeam = 10;

  uint32_t packetSize = 512;
  std::string interval = "10ms";

  double simLength = 30.0;

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-logon");

  simulationHelper->SetSimulationTime (Seconds (simLength));

  // Set beam ID
  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetBeamSet ({beamId});

  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));

  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_LogonFrame", BooleanValue (true));

  // Enable Random Access with all available modules
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_RCS2_SPECIFICATION));

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
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaServiceCount", UintegerValue (3));
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
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue (false));

  // Disable DA
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

  simulationHelper->CreateSatScenario ();

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    Seconds (0.1), Seconds (simLength));
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (0.1), Seconds (simLength));

  // To store attributes to file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  simulationHelper->StoreAttributesToFile ("output-attributes.xml");
  simulationHelper->EnableProgressLogs ();

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddGlobalFwdPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdPhyDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnPhyDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnPhyDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdMacDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnMacDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdDevDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdDevDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnDevDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnDevDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdAppDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->RunSimulation ();

  return 0;
}
