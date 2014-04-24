/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/cbr-helper.h"

using namespace ns3;

/**
 * \ingroup satellite
 *
 * \brief  External input fading example application based on CBR example for satellite network.
 *         Interval, packet size and test scenario can be given in command line as user argument.
 *         To see help for user arguments, execute the command
 *
 *         ./waf --run "trace-input-external-fading-example --PrintHelp"
 *
 *         This example application sends first packets from GW connected user
 *         to UT connected users and after that from UT connected user to GW connected
 *         user.
 *
 *         This example uses additional fading input with normal fading calculations. The results
 *         can be directly compared with the output from "trace-output-example" to see the effects
 *         of additional fading.
 *
 *         This example uses the following trace for input:
 *         - external fading trace
 *
 *         The input folder is:
 *         {NS-3-root-folder}/src/satellite/data/fadingtraces
 *
 *         The input data files must be available in the folder stated above for the example
 *         program to read, otherwise the program will fail.
 *
 *         This example produces the following traces:
 *         - interference density trace
 *         - rx power density trace
 *         - fading trace
 *         - composite SINR
 *
 *         The output folders are:
 *         {NS-3-root-folder}/src/satellite/data/interferencetraces/output
 *         {NS-3-root-folder}/src/satellite/data/rxpowertraces/output
 *         {NS-3-root-folder}/src/satellite/data/fadingtraces/output
 *         {NS-3-root-folder}/src/satellite/data/compositesinrtraces/output
 */

NS_LOG_COMPONENT_DEFINE ("sat-trace-input-external-fading-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "simple";
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  /// Read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.Parse (argc, argv);

  /// Enable external fading trace input
  Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace",BooleanValue (true));

  /// Enable Rx power calculation & Rx power density output trace
  Config::SetDefault ("ns3::SatChannel::RxPowerCalculationMode",EnumValue (SatEnums::RX_PWR_CALCULATION));
  Config::SetDefault ("ns3::SatChannel::EnableRxPowerOutputTrace",BooleanValue (true));

  /// Enable Markov fading calculation & fading output trace
  Config::SetDefault ("ns3::SatBeamHelper::FadingModel",EnumValue (SatEnums::FADING_MARKOV));
  Config::SetDefault ("ns3::SatChannel::EnableFadingOutputTrace",BooleanValue (true));

  /// Enable per packet interference & interference density output trace
  Config::SetDefault ("ns3::SatGwHelper::DaRtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::DaRtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::DaFwdLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatUtHelper::DaFwdLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace",BooleanValue (true));

  /// Enable composite SINR output trace
  Config::SetDefault ("ns3::SatPhyRxCarrier::EnableCompositeSinrOutputTrace",BooleanValue (true));

  //Singleton<SatFadingOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  //Singleton<SatInterferenceOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  //Singleton<SatRxPowerOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  //Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->EnableFigureOutput (false);

  //Singleton<SatFadingOutputTraceContainer>::Get ()->InsertTag ("_fadingExampleTag");
  //Singleton<SatInterferenceOutputTraceContainer>::Get ()->InsertTag ("_interferenceExampleTag");
  //Singleton<SatRxPowerOutputTraceContainer>::Get ()->InsertTag ("_rxPowerExampleTag");
  //Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->InsertTag ("_rxPowerExampleTag");

  /// Enable the printing of ID mapper trace IDs
  Singleton<SatIdMapper>::Get ()->EnableMapPrint (true);

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  /// Enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-trace-input-external-fading-example", LOG_LEVEL_INFO);

  /// Remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /// Create satellite helper with given scenario default=simple

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces(scenarioLogFile, false);
    }

  helper->CreateScenario(satScenario);

  /// Get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  uint16_t port = 9;

  /// Create application on GW user
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress(gwUsers.Get(0)), port));
  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress(utUsers.Get(0)), port));
  cbrHelper.SetAttribute("Interval", StringValue (interval));
  cbrHelper.SetAttribute("PacketSize", UintegerValue (packetSize) );

  ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
  gwSink.Start (Seconds (1.0));
  gwSink.Stop (Seconds (10.0));

  ApplicationContainer gwCbr = cbrHelper.Install (gwUsers.Get (0));
  gwCbr.Start (Seconds (3.0));
  gwCbr.Stop (Seconds (5.1));

  /// Create application on UT user
  sinkHelper.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get(0)), port))));
  cbrHelper.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get(0)), port))));

  ApplicationContainer utSink = sinkHelper.Install (utUsers.Get (0));
  utSink.Start (Seconds (1.0));
  utSink.Stop (Seconds (10.0));

  ApplicationContainer utCbr = cbrHelper.Install (utUsers.Get (0));
  utCbr.Start (Seconds (7.0));
  utCbr.Stop (Seconds (9.1));

  NS_LOG_INFO("--- Trace-output-example ---");
  NS_LOG_INFO("  Scenario used: " << scenario);
  NS_LOG_INFO("  PacketSize: " << packetSize);
  NS_LOG_INFO("  Interval: " << interval);
  NS_LOG_INFO("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO("  ");

  Simulator::Stop (Seconds(11));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
