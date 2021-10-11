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
 * \file sat-lora-example.cc
 * \ingroup satellite
 *
 * \brief This file allows to create a E-SSA scenario
 */

NS_LOG_COMPONENT_DEFINE ("sat-lora-example");

int
main (int argc, char *argv[])
{
  // Variables
  std::string beams = "8";
  uint32_t nbGw = 1;
  uint32_t nbUtsPerBeam = 100;
  uint32_t nbEndUsersPerUt = 1;

  Time appStartTime = Seconds (0.001);
  Time simLength = Seconds (10.0);

  uint32_t packetSize = 20;
  std::string dataRate = "5kbps";
  std::string onTime = "0.2";
  std::string offTime = "0.8";

  double frameAllocatedBandwidthHz = 15000;
  double frameCarrierAllocatedBandwidthHz = 15000;
  double frameCarrierRollOff = 0.22;
  double frameCarrierSpacing = 0;
  uint32_t frameSpreadingFactor = 256;

  bool interferenceModePerPacket = true;
  bool displayTraces = true;

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-lora");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("modelPP", "interferenceModePerPacket", interferenceModePerPacket);
  cmd.AddValue ("traces", "displayTraces", displayTraces);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", nbUtsPerBeam);
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("packetSize", "Constant packet size in bytes", packetSize);
  cmd.AddValue ("dataRate", "Data rate (e.g. 500kb/s)", dataRate);
  cmd.AddValue ("onTime", "Time for packet sending is on in seconds", onTime);
  cmd.AddValue ("offTime", "Time for packet sending is off in seconds", offTime);
  cmd.AddValue ("frameAllocatedBandwidthHz", "Allocated bandwidth in Hz", frameAllocatedBandwidthHz);
  cmd.AddValue ("frameCarrierAllocatedBandwidthHz", "Allocated carrier bandwidth in Hz", frameCarrierAllocatedBandwidthHz);
  cmd.AddValue ("frameCarrierRollOff", "Roll-off factor", frameCarrierRollOff);
  cmd.AddValue ("frameCarrierSpacing", "Carrier spacing factor", frameCarrierSpacing);
  cmd.AddValue ("frameSpreadingFactor", "Carrier spreading factor", frameSpreadingFactor);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  // Enable Lora
  Config::SetDefault ("ns3::SatBeamHelper::Standard", EnumValue (SatEnums::LORA));
  Config::SetDefault ("ns3::LorawanMacEndDevice::DataRate", UintegerValue (5));
  Config::SetDefault ("ns3::LorawanMacEndDevice::MType", EnumValue (LorawanMacHeader::CONFIRMED_DATA_UP));

  // Defaults
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  // Superframe configuration
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (frameAllocatedBandwidthHz));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (frameCarrierAllocatedBandwidthHz));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierRollOff", DoubleValue (frameCarrierRollOff));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierSpacing", DoubleValue (frameCarrierSpacing));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_SpreadingFactor", UintegerValue (frameSpreadingFactor));

  // CRDSA only
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

  // Configure RA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
  if(interferenceModePerPacket)
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
    }
  else
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_FRAGMENT));
    }
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_FSIM));
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

  // Set random access parameters
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (50));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.99));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue (true));

  // Traffics
  simulationHelper->SetSimulationTime (simLength);

  simulationHelper->SetGwUserCount (nbGw);
  simulationHelper->SetUtCountPerBeam (nbUtsPerBeam);
  simulationHelper->SetUserCountPerUt (nbEndUsersPerUt);
  simulationHelper->SetBeams (beams);

  simulationHelper->CreateSatScenario ();

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (dataRate));
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + onTime + "]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + offTime + "]"));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("1s"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  /*simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR,
    SimulationHelper::UDP,
    SimulationHelper::RTN_LINK,
    appStartTime, simLength, MilliSeconds (10));*/

  /*simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR,
    SimulationHelper::UDP,
    SimulationHelper::FWD_LINK,
    appStartTime, simLength);*/

  simulationHelper->InstallLoraTrafficModel (
    SimulationHelper::PERIODIC,
    Seconds (2),
    24,
    appStartTime, simLength, MilliSeconds (10));

  // Outputs
  simulationHelper->EnableProgressLogs ();

  std::string outputPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/data/sims/example-lora");
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (outputPath + "/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  if(displayTraces)
    {
      Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

      s->AddGlobalEssaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtEssaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtEssaPacketError (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalEssaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtEssaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtEssaPacketCollision (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerBeamRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamRtnFeederWindowLoad (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

      s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
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


      s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
      s->AddPerUtFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
    }

  simulationHelper->RunSimulation ();

  return 0;

} // end of `int main (int argc, char *argv[])`
