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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
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
 * \file sat-cbr-full-example.cc
 * \ingroup satellite
 *
 * \brief  An example to run CBR application in full satellite scenario and FWD link.
 *
 *         execute command -> ./waf --run "sat-cbr-full-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-cbr-full-example");

int
main (int argc, char *argv[])
{
  // LogComponentEnable ("CbrApplication", LOG_LEVEL_ALL);
  // LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
  // LogComponentEnable ("sat-arq-rtn-example", LOG_LEVEL_INFO);

  uint32_t endUsersPerUt (3);
  uint32_t utsPerBeam (3);
  uint32_t packetSize (128);
  std::string interval ("1s");
  double simLength (10.0);

  Time appStartTime = Seconds (0.001);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-cbr-full");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue ("simLength", "Simulation length in seconds", simLength);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  simulationHelper->SetSimulationTime (simLength);

  // We set the UT count and UT user count using attributes when configuring a pre-defined scenario
  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue (utsPerBeam));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue (endUsersPerUt));

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_CONSTANT);
  double errorRate (0.2);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkConstantErrorRate", DoubleValue (errorRate));
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkConstantErrorRate", DoubleValue (errorRate));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Create full scenario

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  simulationHelper->CreateSatScenario (SatHelper::FULL);

  // >>> Start of actual test using Full scenario >>>
  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  simulationHelper->InstallTrafficModel (
  		SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
			appStartTime, Seconds (simLength), Seconds (0.001));

  NS_LOG_INFO ("--- sat-cbr-full-example ---");
  NS_LOG_INFO ("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO ("  Packet sending interval: " << interval);
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  simulationHelper->RunSimulation ();

  return 0;

} // end of `int main (int argc, char *argv[])`
