/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/on-off-helper.h"


using namespace ns3;

/**
* \ingroup satellite
*
* \brief  OnOff example application to use satellite network.
*         Data rate, packet size, sender(s), on/off times, test scenario and
*         creation log file name can be given in command line as user argument.
*         To see help for user arguments:
*         execute command -> ./waf --run "on-off-example --PrintHelp"
*
*         On-Off application send packets from GW connected user
*         to UT connected user and after that from UT connected user to GW connected
*         user according to given parameters.
*/

NS_LOG_COMPONENT_DEFINE ("OnOff-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;
  std::string dataRate = "500kb/s";
  std::string onTime = "1.0";
  std::string offTime = "0.5";
  std::string scenario = "simple";
  std::string sender = "both";
  std::string scenarioLogFile = "";
  std::string simDuration = "11s";

  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("packetSize", "Size of constant packet (bytes e.g 512)", packetSize);
  cmd.AddValue("dataRate", "Data rate (e.g. 500kb/s)", dataRate);
  cmd.AddValue("onTime", "Time for packet sending is on in seconds, (e.g. (1.0)", onTime);
  cmd.AddValue("offTime", "Time for packet sending is off in seconds, (e.g. (0.5)", offTime);
  cmd.AddValue("sender", "Packet sender (ut, gw, or both).", sender);
  cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue("simDuration", "Duration of the simulation (Time)", simDuration);
  cmd.AddValue("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.Parse (argc, argv);

  // select scenario, if correct one given, by default simple scenarion is used.
  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  // Set up user given parameters for on/off functionality.
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (dataRate));
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + onTime + "]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + offTime + "]"));

  // enable info logs
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("OnOff-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // create scenarion creation log if file name is given
  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces(scenarioLogFile, false);
    }

  // create scenario
  helper->CreatePredefinedScenario(satScenario);

  // enable packet traces on satellite modules
  helper->EnablePacketTrace ();

  // --- Create applications according to given user parameters

  // get users (first GW side user and first UT connected users)
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // select port
  uint16_t port = 9;

  // create helpers for application creation
  // set address of the first UT connected user
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress (utUsers.Get (0)), port));
  SatOnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress (utUsers.Get (0)), port));

  // assert if sender is not valid
  NS_ASSERT_MSG ( ( (sender == "gw") || ( sender == "ut") || ( sender == "both") ) , "Sender argument invalid.");

  // in case of sender is GW or Both, create OnOff application to GW connected user
  // and Sink application to UT connected user
  if ( (sender == "gw" ) || ( sender == "both") )
    {
      ApplicationContainer gwOnOff = onOffHelper.Install (gwUsers.Get (0));
      gwOnOff.Start (Seconds (1.0));

      ApplicationContainer utSink = sinkHelper.Install (utUsers.Get (0));
      utSink.Start (Seconds (0.1));
    }

  // in case of sender is UT or Both, create OnOff application to UT connected user
  // and Sink application to GW connected user
  if (sender == "ut" || sender == "both" )
    {
      // set address of the first GW connected user
      sinkHelper.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
      onOffHelper.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

      ApplicationContainer utOnOff = onOffHelper.Install (utUsers.Get (0));
      utOnOff.Start (Seconds (2.0));

      ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
      gwSink.Start (Seconds (0.1));
    }

  // prompt info of the used parameters
  NS_LOG_INFO("--- sat-onoff-example ---");
  NS_LOG_INFO("  Scenario used: " << scenario);
  NS_LOG_INFO("  Sender: " << sender);
  NS_LOG_INFO("  PacketSize: " << packetSize);
  NS_LOG_INFO("  DataRate: " << dataRate);
  NS_LOG_INFO("  OnTime: " << onTime);
  NS_LOG_INFO("  OffTime: " << offTime);
  NS_LOG_INFO("  Duration: " << simDuration);
  NS_LOG_INFO("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO("  ");

  // run simulation and finally destroy it
  Simulator::Stop (Time (simDuration));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
