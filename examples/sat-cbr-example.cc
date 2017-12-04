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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
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
 * \file sat-cbr-example.cc
 * \ingroup satellite
 *
 * \brief  Cbr example application to use satellite network.
 *          Interval, packet size and test scenario can be given
 *         in command line as user argument.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-cbr-example --PrintHelp"
 *
 *         Cbr example application sends first packets from GW connected user
 *         to UT connected users and after that from UT connected user to GW connected
 *         user.
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-cbr-example");

int
main (int argc, char *argv[])
{
  uint32_t beamIdInFullScenario = 10;
  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "simple";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

//  To read attributes from file
//  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("input-attributes.xml"));
//  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
//  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
//  ConfigStore inputConfig;
//  inputConfig.ConfigureDefaults ();

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-cbr");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("beamIdInFullScenario", "Id where Sending/Receiving UT is selected in FULL scenario. (used only when scenario is full) ", beamIdInFullScenario);
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full", scenario);
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
  // Set tag, if output path is not explicitly defined
  simulationHelper->SetOutputTag (scenario);

  simulationHelper->SetSimulationTime (Seconds (11));

  // Set beam ID
  std::stringstream beamsEnabled; beamsEnabled  << beamIdInFullScenario;
  simulationHelper->SetBeams (beamsEnabled.str ());

  // enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-cbr-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);

  // in full scenario get given beam UTs and use first UT's users
  // other scenarios get all UT users.
  if ( scenario == "full")
    {
  		// Manual configuration of applications

  		// get users
      NodeContainer uts = helper->GetBeamHelper ()->GetUtNodes (beamIdInFullScenario);
      NodeContainer utUsers = helper->GetUserHelper ()->GetUtUsers (uts.Get (0));

      NodeContainer gwUsers = helper->GetGwUsers ();

      uint16_t port = 9;

      // create application on GW user
      PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));
      CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));
      cbrHelper.SetAttribute ("Interval", StringValue (interval));
      cbrHelper.SetAttribute ("PacketSize", UintegerValue (packetSize) );

      ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
      gwSink.Start (Seconds (1.0));
      gwSink.Stop (Seconds (10.0));

      ApplicationContainer gwCbr = cbrHelper.Install (gwUsers.Get (0));
      gwCbr.Start (Seconds (1.0));
      gwCbr.Stop (Seconds (2.1));

      // create application on UT user
      sinkHelper.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port))));
      cbrHelper.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

      ApplicationContainer utSink = sinkHelper.Install (utUsers.Get (0));
      utSink.Start (Seconds (1.0));
      utSink.Stop (Seconds (10.0));

      ApplicationContainer utCbr = cbrHelper.Install (utUsers.Get (0));
      utCbr.Start (Seconds (7.0));
      utCbr.Stop (Seconds (9.1));
    }
  else
    {
      Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
      Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize) );

		  simulationHelper->InstallTrafficModel (
					       SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
					       Seconds (1.0), Seconds (2.1));
		  simulationHelper->InstallTrafficModel (
		  					 SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
		  					 Seconds (7.0), Seconds (9.1));

    }

  NS_LOG_INFO ("--- sat-cbr-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  if ( scenario == "full" )
    {
      NS_LOG_INFO ("  UT used in full scenario from beam: " << beamIdInFullScenario );
    }
  NS_LOG_INFO ("  PacketSize: " << packetSize);
  NS_LOG_INFO ("  Interval: " << interval);
  NS_LOG_INFO ("  ");

  // To store attributes to file
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.xml"));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig;
  // outputConfig.ConfigureDefaults ();

  simulationHelper->RunSimulation ();

  return 0;
}
