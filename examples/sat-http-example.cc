/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/satellite-module.h>
#include <ns3/traffic-module.h>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("sat-http-example");


/**
 * \ingroup satellite
 *
 * \brief  Example of using HTTP traffic model in a satellite network.
 *
 * By default, the SIMPLE test scenario is used. Another test scenario can be
 * given from command line as user argument, e.g.:
 *
 *     $ ./waf --run="sat-http-example --scenario=larger"
 *     $ ./waf --run="sat-http-example --scenario=full"
 *
 * To see help for user arguments:
 *
 *     $ ./waf --run "cbr-example --PrintHelp"
 *
 */
int
main (int argc, char *argv[])
{
  std::string scenario = "simple";
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.Parse (argc, argv);

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  LogComponentEnableAll (LOG_PREFIX_ALL);
  LogComponentEnable ("HttpClient", LOG_WARN);
  LogComponentEnable ("HttpServer", LOG_WARN);
  LogComponentEnable ("sat-http-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Create reference system, two options:
  // - "Scenario72"
  // - "Scenario98"
  std::string scenarioName = "Scenario72";
  //std::string scenarioName = "Scenario98";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces(scenarioLogFile, false);
    }

  helper->CreateScenario(satScenario);

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  Ipv4Address serverAddress = helper->GetUserAddress (gwUsers.Get (0));

  // create application on GW user
  HttpServerHelper httpServerHelper ("ns3::TcpSocketFactory", serverAddress);
  ApplicationContainer serverApps = httpServerHelper.Install (gwUsers.Get (0));
  serverApps.Start (Seconds (1.0));

  // create application on UT user
  HttpClientHelper httpClientHelper ("ns3::TcpSocketFactory", serverAddress);
  ApplicationContainer clientApps = httpClientHelper.Install (utUsers);
  clientApps.Start (Seconds (3.0));

  NS_LOG_INFO("--- sat-http-example ---");
  NS_LOG_INFO("  Scenario used: " << scenario);
  NS_LOG_INFO("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO("  ");

  /**
   * \todo Simulation still produces error if run more than 120 seconds.
   *       Related to the ns3::ArpCache::AliveTimeout attribute.
   */
  Simulator::Stop (Seconds (1000));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
