/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

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
  std::string scenarioLogFile = "tutorial-creation.log";

  /*****************************************************************************
   'To Select super frame configuration, Option 2'
   -- Start --                                                                */

//  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("src/satellite/examples") + "/sat-tutorial-input.xml";
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
  cmd.AddValue ("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.AddValue ("scenario", "Scenario to be created", scenario);
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

  // enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-tutorial-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces (scenarioLogFile, false);
    }

  // create satellite helper with given scenario default=simple
  helper->CreatePredefinedScenario (satScenario);

  // Enable packet traces (to file PacketTrace.log).
  helper->EnablePacketTrace ();
                                                                              /**
   -- End --
   Create helper and simulation scenario
  ******************************************************************************/

  /*****************************************************************************
   'To Select super frame configuration, Option 1'
   -- Start --                                                                */

  //Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::CONFIG_TYPE_2));
  //Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", StringValue ("Configuration_2"));
                                                                               /**
   -- End --
   'To Select super frame configuration, Option 1'
  ******************************************************************************/


  /*****************************************************************************
    Creating an installing application (users) to satellite network
    -- Start --
                                                                    */
  // for getting UT users
  NodeContainer utUsers;

  // in full scenario get given beam UTs and use first UT's users
  // other scenarios get all UT users.
  if ( scenario == "full")
    {
      NodeContainer uts = helper->GetBeamHelper ()->GetUtNodes (1);
      utUsers = helper->GetUserHelper()->GetUtUsers (uts.Get (0));
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
  sinkHelper.SetAttribute ("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port))));
  cbrHelper.SetAttribute ("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

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
  NS_LOG_INFO ("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO ("  ");

  /*****************************************************************************
   Store set attribute values to XML output file
   -- Start --                                                                */

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-tutorial-output.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();
                                                                             /**
   -- End --
   Store set attribute values to XML output file
  ******************************************************************************/

  /*****************************************************************************
   Run, stop and destroy simulation
   -- Start --                                                                */

  Simulator::Stop (Seconds(11));
  Simulator::Run ();
  Simulator::Destroy ();
                                                                             /**
   -- End --
   Run, stop and destroy simulation
  ******************************************************************************/

  return 0;
}
