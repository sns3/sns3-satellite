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
 * \file sat-ncr-example.cc
 * \ingroup satellite *
 */

NS_LOG_COMPONENT_DEFINE ("sat-ncr-example");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt = 1;
  uint32_t utsPerBeam = 10;

  uint32_t packetSize = 512;
  std::string interval = "100ms";

  double simLength = 60.0;

  uint32_t guardTime = 4;
  int32_t clockDrift = 50;

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-ncr");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("beamId", "ID of beam used", beamId);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("endUsersPerUt", "Number end users per UT", endUsersPerUt);
  cmd.AddValue ("packetSize", "Constant packet size in bytes", packetSize);
  cmd.AddValue ("interval", "Interval between two UDP packets per UT", interval);
  cmd.AddValue ("guardTime", "Guard time in time slots in symbols", guardTime);
  cmd.AddValue ("clockDrift", "Drift value of UT clocks in ticks per second", clockDrift);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  // Set beam ID
  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetBeamSet ({beamId});
  simulationHelper->SetSimulationTime (Seconds (simLength));
  simulationHelper->EnableProgressLogs ();


  // Set 2 RA frames including one for logon
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_LogonFrame", BooleanValue (true));

  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame2_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame3_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame4_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame5_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame6_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame7_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame8_GuardTimeSymbols", UintegerValue (guardTime));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame9_GuardTimeSymbols", UintegerValue (guardTime));

  Config::SetDefault ("ns3::SatUtMac::WindowInitLogon", TimeValue (Seconds (20)));
  Config::SetDefault ("ns3::SatUtMac::MaxWaitingTimeLogonResponse", TimeValue (Seconds (1)));


  Config::SetDefault ("ns3::SatMac::NcrVersion2", BooleanValue (false));
  Config::SetDefault ("ns3::SatGwMac::NcrBroadcastPeriod", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::SatGwMac::UseCmt", BooleanValue (true));
  Config::SetDefault ("ns3::SatUtMacState::NcrSyncTimeout", TimeValue (Seconds (1)));
  Config::SetDefault ("ns3::SatUtMacState::NcrRecoveryTimeout", TimeValue (Seconds (10)));
  Config::SetDefault ("ns3::SatNcc::UtTimeout", TimeValue (Seconds (10)));

  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue (MilliSeconds (500)));

  Config::SetDefault ("ns3::SatUtMac::ClockDrift", IntegerValue (clockDrift));
  Config::SetDefault ("ns3::SatGwMac::CmtPeriodMin", TimeValue (MilliSeconds (550)));

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

  s->AddGlobalFwdUserMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

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
