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
 * \file sat-regeneration-collisions-example.cc
 * \ingroup satellite
 *
 * \brief This file gives an example of satellite regeneration with collisions.
 *
 * TODO complete brief
 * TODO add MARSALA
 * TODO add more cmd options
 * TODO Clean
 * TODO add other regeneration modes
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-regeneration-collisions-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;
  std::string interval = "1ms";
  std::string randomAccess = "SlottedAloha";

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-regeneration-collisions");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s))", interval);
  cmd.AddValue ("randomAccess", "Choose RA (SlottedAloha, Crdsa, Essa) or DA.", randomAccess);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue (false));

  // Enable Random Access
  if (randomAccess == "SlottedAloha")
    {
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed", BooleanValue (true));
    }
  else if (randomAccess == "Crdsa")
    {
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_CRDSA));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (true));
    }
  else if (randomAccess == "Marsala")
    {
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_MARSALA));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (true));
    }
  else if (randomAccess == "Essa")
    {
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue (true));
    }
  else if (randomAccess == "DA")
    {
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
    }
  else
    {
      NS_FATAL_ERROR ("Unknown random access: " << randomAccess);
    }

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
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.5));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::SatRandomAccessConf::CrdsaSignalingOverheadInBytes", UintegerValue (5));
  Config::SetDefault ("ns3::SatRandomAccessConf::SlottedAlohaSignalingOverheadInBytes", UintegerValue (3));

  Config::SetDefault ("ns3::SatGeoHelper::FwdLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_AVI));
  Config::SetDefault ("ns3::SatGeoHelper::RtnLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_AVI));

  if (randomAccess == "Essa")
    {
      // Superframe configuration
      Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
      Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
      Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (15000));
      Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (15000));
      Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierRollOff", DoubleValue (0.22));
      Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierSpacing", DoubleValue (0));
      Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_SpreadingFactor", UintegerValue (256));

      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_FRAGMENT));
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
      Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_FSIM));
      Config::SetDefault ("ns3::SatGeoHelper::RtnLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_AVI));
      Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2));
      Config::SetDefault ("ns3::SatHelper::RtnLinkWaveformConfFileName", StringValue("fSimWaveforms.txt"));

      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("600ms"));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("200ms"));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDelay", StringValue ("0s"));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::FirstWindow", StringValue ("0s"));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (5));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::SpreadingFactor", UintegerValue (1));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::DetectionThreshold", DoubleValue (0));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));
    }

  // Disable CRA and DA if RA
  if (randomAccess != "DA")
    {
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaServiceCount", UintegerValue (4));
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
    }

  simulationHelper->SetSimulationTime (Seconds (11));

  LogComponentEnable ("sat-regeneration-collisions-example", LOG_LEVEL_INFO);

  simulationHelper->SetUserCountPerUt (1);
  simulationHelper->SetUtCountPerBeam (50);
  simulationHelper->SetBeamSet ({1});
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario ();

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize) );

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (1.0), Seconds (10.0));

  NS_LOG_INFO ("--- sat-regeneration-collisions-example ---");
  NS_LOG_INFO ("  Random Access (or DA): " << randomAccess);
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

  s->AddGlobalFwdUserPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerSatFeederSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatUserSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFeederCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatUserCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFeederEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatUserEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Error stats if no regeneration or DA regeneration
  s->AddPerSatFeederSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFeederCrdsaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFeederEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdUserDaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnFeederDaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Error stats if RA regeneration
  s->AddPerSatUserSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatUserCrdsaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatUserEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdFeederDaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnUserDaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
