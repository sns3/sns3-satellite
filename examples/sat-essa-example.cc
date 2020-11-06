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
  uint32_t nbUtsPerBeam = 1;
  //uint32_t nbUtsPerBeam = 5000;
  uint32_t nbEndUsersPerUt = 1;

  Time appStartTime = Seconds (0.001);
  Time simLength = Seconds (60.0);

  uint32_t packetSize = 37; // OK

  double frame0_AllocatedBandwidthHz = 15000; // OK
  double frame0_CarrierAllocatedBandwidthHz = 15000; // OK
  double frame0_CarrierRollOff = 0.22; // OK
  double frame0_CarrierSpacing = 0; // OK
  uint32_t frame0_SpreadingFactor = 256; // OK

  bool interferenceModePerPacket = true;

  // Defaults
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (false));

  Config::SetDefault ("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue (8e+09));
  Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (500e+06));
  Config::SetDefault ("ns3::SatConf::FwdCarrierRollOff", DoubleValue (0.05));

  Config::SetDefault ("ns3::SatConf::RtnUserLinkBandwidth", DoubleValue (500e+06));
  Config::SetDefault ("ns3::SatConf::RtnFeederLinkBandwidth", DoubleValue (2e+09));
  // Config::SetDefault ("ns3::SatConf::RtnFeederLinkBaseFrequency", DoubleValue (1.77e+10));
  // Config::SetDefault ("ns3::SatConf::RtnUserLinkBaseFrequency", DoubleValue (2.95e+10));

  // Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaServiceCount", UintegerValue (4));

  // Config::SetDefault ("ns3::SatSuperframeConf0::FrameCount", UintegerValue (10));
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));

  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (frame0_AllocatedBandwidthHz));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (frame0_CarrierAllocatedBandwidthHz));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierRollOff", DoubleValue (frame0_CarrierRollOff));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierSpacing", DoubleValue (frame0_CarrierSpacing));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_SpreadingFactor", UintegerValue (frame0_SpreadingFactor));

  Config::SetDefault ("ns3::SatBbFrameConf::DefaultModCod", StringValue ("QPSK_1_TO_2"));
  //Config::SetDefault ("ns3::SatBbFrameConf::DefaultModCod", StringValue ("BPSK_1_TO_3"));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("1s"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-essa-example");

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

  // TODO check
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaServiceCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
  // Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));

  if(interferenceModePerPacket)
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
    }
  else
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_FRAGMENT));
    }

  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  //Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_PERFECT));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatBeamHelper::RaConstantErrorRate", DoubleValue (0.0));

  //Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_RCS2)); // TODO raise error if used with E-SSA
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_FSIM));

  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (false));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize", UintegerValue (10));

  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (50));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffTimeInMilliSeconds", UintegerValue (500));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.99));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed", BooleanValue (false)); // def = true
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed", BooleanValue (false)); // def = true
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue (true));

  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2)); // 2 for 37B packets, 1 for 150B packets ?
  Config::SetDefault ("ns3::SatHelper::WaveformConfigFileName", StringValue("fSimWaveforms.txt"));

  // Target duration time
  Config::SetDefault ("ns3::SatSuperframeSeq::TargetDuration", StringValue("100ms")); // Def = 100ms

  // The duration of the sliding window
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("60ms"));
  // The length of the step between two window iterations
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("20ms"));
  // The delay before processing a sliding window, waiting for incomplete packets
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDelay", StringValue ("0s"));
  // The time at which the first window is processed
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::FirstWindow", StringValue ("0s"));
  // The number of SIC iterations performed on each window
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (10));
  // TODO: this shouldn't be here!! find a way to retrieve SF from SuperFrameConf,
  // The spreading factor of the packets
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::SpreadingFactor", UintegerValue (1));
  // The SNIR Detection Threshold (in magnitude) for a packet
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::DetectionThreshold", DoubleValue (0));
  // Use SIC when decoding a packet
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (true));

/*
Es/N0
13.7 dB
*/

  // Traffics
  simulationHelper->SetSimulationTime (simLength);

  simulationHelper->SetGwUserCount (nbGw);
  simulationHelper->SetUtCountPerBeam (nbUtsPerBeam);
  simulationHelper->SetUserCountPerUt (nbEndUsersPerUt);
  simulationHelper->SetBeams (beams);

  simulationHelper->CreateSatScenario ();

  //Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR,
    SimulationHelper::UDP,
    SimulationHelper::FWD_LINK,
    Seconds (appStartTime), Seconds (simLength));

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR,
    SimulationHelper::UDP,
    SimulationHelper::RTN_LINK,
    Seconds (appStartTime), Seconds (simLength));


  // Outputs
  simulationHelper->EnableProgressLogs ();

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("contrib/satellite/data/sims/sat-essa-example/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();


  simulationHelper->RunSimulation ();
  return 0;



  /*Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  // Capacity request
  s->AddGlobalCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Granted resources
  s->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Link SINR
  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // SINR
  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);

  // Link RX Power
  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // Frame type usage
  s->AddGlobalFrameTypeUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  if (raModel == "MARSALA")
    {
      s->AddPerBeamMarsalaCorrelation (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamMarsalaCorrelation (SatStatsHelper::OUTPUT_SCATTER_FILE);
    }*/

} // end of `int main (int argc, char *argv[])`
