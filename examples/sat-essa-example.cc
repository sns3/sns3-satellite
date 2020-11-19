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

#include <chrono>

using namespace ns3;

/**
 * \file sat-essa-example.cc
 * \ingroup satellite
 *
 * \brief This file allows to create a E-SSA scenario
 */

NS_LOG_COMPONENT_DEFINE ("sat-essa-example");

int
main (int argc, char *argv[])
{
  // Variables
  std::string beams = "8";
  uint32_t nbGw = 1;
  uint32_t nbUtsPerBeam = 100;
  uint32_t nbEndUsersPerUt = 1;

  Time appStartTime = Seconds (0.001);
  Time simLength = Seconds (60.0);

  double txMaxPowerDbw = 4.0;

  uint32_t packetSize = 64;
  std::string dataRate = "5kbps";
  std::string onTime = "0.2";
  std::string offTime = "0.8";

  double frame0_AllocatedBandwidthHz = 30000;
  double frame0_CarrierAllocatedBandwidthHz = 30000;
  double frame0_CarrierRollOff = 0.22;
  double frame0_CarrierSpacing = 0;
  uint32_t frame0_SpreadingFactor = 256;

  bool interferenceModePerPacket = true;
  bool displayTraces = true;

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-essa-example");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("model", "interferenceModePerPacket", interferenceModePerPacket);
  cmd.AddValue ("traces", "displayTraces", displayTraces);
  cmd.AddValue ("power", "txMaxPowerDbw", txMaxPowerDbw);
  cmd.AddValue ("ut", "nbUtsPerBeam", nbUtsPerBeam);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  std::cout << interferenceModePerPacket << " " << displayTraces << " " << txMaxPowerDbw << " " << nbUtsPerBeam << std::endl;

  // Defaults
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (frame0_AllocatedBandwidthHz));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (frame0_CarrierAllocatedBandwidthHz));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierRollOff", DoubleValue (frame0_CarrierRollOff));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierSpacing", DoubleValue (frame0_CarrierSpacing));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_SpreadingFactor", UintegerValue (frame0_SpreadingFactor));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaServiceCount", UintegerValue (4));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));

  if(interferenceModePerPacket)
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));
    }
  else
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_FRAGMENT));
      Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (true));
    }

  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_FSIM));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (50));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.99));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed", BooleanValue (true)); // def = true
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (false)); // def = true
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue (true));

  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2));
  Config::SetDefault ("ns3::SatHelper::WaveformConfigFileName", StringValue("fSimWaveforms.txt"));

  // The duration of the sliding window
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("600ms")); // Default = 60ms
  // The length of the step between two window iterations
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("200ms")); // Default = 20ms
  // The delay before processing a sliding window, waiting for incomplete packets
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDelay", StringValue ("0s"));
  // The time at which the first window is processed
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::FirstWindow", StringValue ("0s"));
  // The number of SIC iterations performed on each window
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (5));
  // The spreading factor of the packets
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::SpreadingFactor", UintegerValue (1));
  // The SNIR Detection Threshold (in magnitude) for a packet
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::DetectionThreshold", DoubleValue (0));
  // Use SIC when decoding a packet
  //Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));

  Config::SetDefault ("ns3::SatUtPhy::TxMaxPowerDbw", DoubleValue (txMaxPowerDbw)); // Default = 4.00

  // Traffics
  simulationHelper->SetSimulationTime (simLength);

  simulationHelper->SetGwUserCount (nbGw);
  simulationHelper->SetUtCountPerBeam (nbUtsPerBeam);
  simulationHelper->SetUserCountPerUt (nbEndUsersPerUt);
  simulationHelper->SetBeams (beams);

  simulationHelper->CreateSatScenario ();

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("200ms"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (dataRate));
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + onTime + "]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + offTime + "]"));

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR,
    SimulationHelper::UDP,
    SimulationHelper::RTN_LINK,
    Seconds (appStartTime), Seconds (simLength), Seconds (0.001));

  /*simulationHelper->InstallTrafficModel (
    SimulationHelper::ONOFF,
    SimulationHelper::UDP,
    SimulationHelper::RTN_LINK,
    Seconds (appStartTime), Seconds (simLength));*/

  /*simulationHelper->InstallTrafficModel (
    SimulationHelper::ONOFF,
    SimulationHelper::UDP,
    SimulationHelper::FWD_LINK,
    Seconds (appStartTime), Seconds (simLength));*/

  /*simulationHelper->InstallTrafficModel (
    SimulationHelper::HTTP,
    SimulationHelper::TCP,
    SimulationHelper::FWD_LINK,
    Seconds (appStartTime), Seconds (simLength));*/

  // Outputs
  simulationHelper->EnableProgressLogs ();

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("contrib/satellite/data/sims/sat-essa-example/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  if(displayTraces)
    {
      Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

      s->AddGlobalEssaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerGwEssaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerGwEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerBeamEssaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtEssaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalEssaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerGwEssaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerGwEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerBeamEssaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtEssaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerGwRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerGwRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerBeamRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddGlobalRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerGwRtnCompositeSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerGwRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
    }

  simulationHelper->RunSimulation ();

  return 0;

} // end of `int main (int argc, char *argv[])`
