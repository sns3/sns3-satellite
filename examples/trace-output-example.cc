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
 * \brief  Trace output example application based on CBR example for satellite network.
 *         Interval, packet size and test scenario can be given in command line as user argument.
 *         To see help for user arguments, execute the command
 *
 *         ./waf --run "trace-output-example --PrintHelp"
 *
 *         This example application sends first packets from GW connected user
 *         to UT connected users and after that from UT connected user to GW connected
 *         user.
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

NS_LOG_COMPONENT_DEFINE ("trace-output-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "simple";
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  /// read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.Parse (argc, argv);

  /// enable Rx power calculation & Rx power density output trace
  Config::SetDefault ("ns3::SatChannel::RxPowerCalculationMode",EnumValue (SatEnums::RX_PWR_CALCULATION));
  Config::SetDefault ("ns3::SatChannel::EnableRxPowerOutputTrace",BooleanValue (true));

  /// enable Markov fading calculation & fading output trace
  Config::SetDefault ("ns3::SatBeamHelper::FadingModel",EnumValue (SatEnums::FADING_MARKOV));
  Config::SetDefault ("ns3::SatChannel::EnableFadingOutputTrace",BooleanValue (true));

  /// enable per packet interference & interference density output trace
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::RtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::FwdLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace",BooleanValue (true));

  /// enable composite SINR output trace
  Config::SetDefault ("ns3::SatPhyRxCarrier::EnableCompositeSinrOutputTrace",BooleanValue (true));

  //Singleton<SatFadingOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  //Singleton<SatInterferenceOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  //Singleton<SatRxPowerOutputTraceContainer>::Get ()->EnableFigureOutput (false);
  //Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->EnableFigureOutput (false);

  //Singleton<SatFadingOutputTraceContainer>::Get ()->InsertTag ("_fadingExampleTag");
  //Singleton<SatInterferenceOutputTraceContainer>::Get ()->InsertTag ("_interferenceExampleTag");
  //Singleton<SatRxPowerOutputTraceContainer>::Get ()->InsertTag ("_rxPowerExampleTag");
  //Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->InsertTag ("_rxPowerExampleTag");

  /// enable the printing of ID mapper trace IDs
  Singleton<SatIdMapper>::Get ()->EnableMapPrint (true);

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  /// enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("trace-output-example", LOG_LEVEL_INFO);

  /// remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /// create satellite helper with given scenario default=simple

  /// Create reference system, two options:
  /// - "Scenario72"
  /// - "Scenario98"
  std::string scenarioName = "Scenario72";
  //std::string scenarioName = "Scenario98";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces(scenarioLogFile, false);
    }

  helper->CreateScenario(satScenario);

  /// get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  uint16_t port = 9;

  /// create application on GW user
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

  /// create application on UT user
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
