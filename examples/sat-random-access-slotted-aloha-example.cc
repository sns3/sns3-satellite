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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-random-access-slotted-aloha-example.cc
 * \ingroup satellite
 *
 * \brief  This is an example of Random Access usage in satellite network. This
 *         example uses only the Slotted ALOHA model regardless of the number of instances
 *         parameter. CRA is disabled in this example. The example allows to
 *         set various RA parameters such as the maximum rate limitation related
 *         parameters, back off and load control related parameters.
 *
 *         By default, the RA dynamic load control is disabled in this
 *         example and only Slotted ALOHA is used. This example is aimed for Slotted ALOHA
 *         debugging and modified to produce full Slotted ALOHA log output with only one UT.
 *
 *         The script is using CBR application in user defined scenario,
 *         which means that user can change the scenario size quite to be
 *         whatever between 1 and full scenario (72 beams). Currently it
 *         is configured to using only one beam. CBR application is sending
 *         packets in RTN link, i.e. from UT side to GW side. Packet trace
 *         is enabled by default. End user may change the number of UTs and
 *         end users from the command line.
 *
 *         execute command -> ./waf --run "sat-random-access-slotted-aloha-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-random-access-slotted-aloha-example");

int
main (int argc, char *argv[])
{
  uint32_t beamId (1);
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (1);
  uint32_t packetSize (20);
  Time interval (Seconds (0.01));
  Time simLength (Seconds (3.00));
  Time appStartTime = Seconds (0.01);

  // Enable info logs
  LogComponentEnable ("sat-random-access-slotted-aloha-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatRandomAccess", LOG_LEVEL_INFO);
  LogComponentEnable ("SatUtMac", LOG_LEVEL_INFO);
  LogComponentEnable ("SatPhyRxCarrier", LOG_LEVEL_INFO);
  LogComponentEnable ("SatInterference", LOG_LEVEL_INFO);
  //LogComponentEnable ("SatBeamScheduler", LOG_LEVEL_INFO);

  auto sh = CreateObject<SimulationHelper> ("example-random-access-slotted-aloha");
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  // Read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  sh->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);


  // Configure error model
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_AVI);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));
  //Config::SetDefault ("ns3::SatUtMac::CrUpdatePeriod", TimeValue(Seconds(10.0)));

  // Enable Random Access with Slotted ALOHA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel",EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));

  // Set Random Access interference model
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));

  // Set Random Access collision model
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel",EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

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
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (1));
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

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  sh->SetSimulationTime (simLength);
  sh->SetUserCountPerUt (endUsersPerUt);
  sh->SetUtCountPerBeam (utsPerBeam);
  sh->SetBeamSet ({beamId});
  sh->CreateSatScenario ();

  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize) );
  sh->InstallTrafficModel (
  		SimulationHelper::CBR,
			SimulationHelper::UDP,
			SimulationHelper::RTN_LINK,
			appStartTime, Seconds (simLength + 1), Seconds (0.05));

  NS_LOG_INFO ("--- Cbr-user-defined-example ---");
  NS_LOG_INFO ("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO ("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO ("  Simulation length: " << simLength.GetSeconds ());
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  sh->RunSimulation ();
  return 0;
}
