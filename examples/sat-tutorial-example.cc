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
 * \file sat-tutorial-example.cc
 * \ingroup satellite
 *
 * \brief  Tutorial example application guides how to build up different scenarios of satellite network.
 *         It also guides how to configure the satellite network e.g. modifying attributes.
 *         Interval, packet size and test scenario can be given
 *         in command line as user argument.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-tutorial-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-tutorial-example");

int
main (int argc, char *argv[])
{
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;
  std::string scenario = "Simple";

  // Create simulation helper
  auto simulationHelper = CreateObject<SimulationHelper> ("example-tutorial");


  // Enable creation traces
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  // Enable packet traces (to file PacketTrace.log).
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  /*****************************************************************************
   'To Select super frame configuration, Option 2'
   -- Start --                                                                */

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::SimulationTag", StringValue (scenario));
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

//  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/sat-tutorial-input.xml";
//
//  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
//  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
//  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
//  ConfigStore inputConfig;
//  inputConfig.ConfigureDefaults ();
/**
-- End --
'To Select super frame configuration, Option 2'
******************************************************************************/

  /******************************************************************************
    Read command line arguments
    -- Start --                                                                */

  CommandLine cmd;
  cmd.AddValue ("scenario", "Scenario to be created", scenario);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);
  /**
-- End --
Read command line arguments
******************************************************************************/

  /*****************************************************************************
    Create helper and simulation scenario
    -- Start --                                                                */

  // select simulation scenario to use
  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  simulationHelper->SetOutputTag (scenario);
  simulationHelper->SetSimulationTime (Seconds (11));


  // enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-tutorial-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /*****************************************************************************
     'To Select super frame configuration, Option 1'
     -- Start --                                                                */

//    Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::CONFIG_TYPE_2));
//    Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", StringValue ("Configuration_2"));

  /**
-- End --
'To Select super frame configuration, Option 1'
******************************************************************************/

  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);
  /**
-- End --
Create helper and simulation scenario
******************************************************************************/

  /*****************************************************************************
    Manually creating an installing application (users) to satellite network.
    Note that you may simply call SimulationHelper::Install TrafficModel when
    using all nodes.
    -- Start --
                                                                    */
  // for getting UT users
  NodeContainer utUsers;

  // in full scenario get given beam UTs and use first UT's users
  // other scenarios get all UT users.
  if ( scenario == "full")
    {
      NodeContainer uts = helper->GetBeamHelper ()->GetUtNodes (1);
      utUsers = helper->GetUserHelper ()->GetUtUsers (uts.Get (0));
    }
  else
    {
      utUsers = helper->GetUtUsers ();
    }

  // get GW users
  NodeContainer gwUsers = helper->GetGwUsers ();

  uint16_t port = 9;

  // create and install applications on GW user
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));
  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));

  // install sink to receive packets from UT
  ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
  gwSink.Start (Seconds (1.0));
  gwSink.Stop (Seconds (10.0));

  // install CBR to send packets to UT
  ApplicationContainer gwCbr = cbrHelper.Install (gwUsers.Get (0));
  gwCbr.Start (Seconds (1.0));
  gwCbr.Stop (Seconds (2.1));

  // create applications on UT user
  sinkHelper.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port))));
  cbrHelper.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

  // install sink to receive packets from GW
  ApplicationContainer utSink = sinkHelper.Install (utUsers.Get (0));
  utSink.Start (Seconds (1.0));
  utSink.Stop (Seconds (10.0));

  // install CBR to send packets to GW
  ApplicationContainer utCbr = cbrHelper.Install (utUsers.Get (0));
  utCbr.Start (Seconds (7.0));
  utCbr.Stop (Seconds (9.1));
  /**
-- End --
Creating and installing application (users) to satellite network
******************************************************************************/

  NS_LOG_INFO ("--- Tutorial-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  ");

  /*****************************************************************************
   Store set attribute values to XML output file
   -- Start --                                                                */

  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-tutorial-output.xml"));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig;
  // outputConfig.ConfigureDefaults ();

  /**
  -- End --
  Store set attribute values to XML output file
  *****************************************************************************/

  /*****************************************************************************
   Run, stop and destroy simulation
   -- Start --                                                                */

  simulationHelper->RunSimulation ();
  /**
-- End --
Run, stop and destroy simulation
******************************************************************************/

  return 0;
}
