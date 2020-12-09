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
  simulationHelper->EnableProgressLogs ();

  // Set beam ID
  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetBeamSet ({beamId});


  // Set 2 RA frames including one for logon
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_LogonFrame", BooleanValue (true));

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

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdMacDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnMacDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdAppDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->RunSimulation ();

  return 0;
}
