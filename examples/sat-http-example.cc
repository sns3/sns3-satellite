/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/satellite-module.h>
#include <ns3/traffic-module.h>
#include <iomanip>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SatHttpExample");


/**
 * \ingroup satellite
 *
 * \brief Example of using HTTP traffic model in a satellite network.
 *
 * One HTTP server application is installed in the first GW user. One HTTP
 * client application is installed in each UT user, configured to point to the
 * server.
 *
 * By default, the SIMPLE test scenario is used. Another test scenario can be
 * given from command line as user argument, e.g.:
 *
 *     $ ./waf --run="sat-http-example --scenario=larger"
 *     $ ./waf --run="sat-http-example --scenario=full"
 *
 * Simulation runs for 1000 seconds by default. This can be changed from the
 * command line argument as well, e.g.:
 *
 *     $ ./waf --run="sat-http-example --duration=500"
 *
 * To see help for user arguments:
 *
 *     $ ./waf --run "sat-http-example --PrintHelp"
 *
 */
int
main (int argc, char *argv[])
{
  std::string scenario = "simple";
  double duration = 1000;
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full)",
                scenario);
  cmd.AddValue ("duration", "Simulation duration (in seconds)",
                duration);
  cmd.AddValue ("logFile", "File name for scenario creation log",
                scenarioLogFile);
  cmd.Parse (argc, argv);

  if (scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if (scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  //LogComponentEnableAll (LOG_PREFIX_ALL);
  //LogComponentEnable ("HttpClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("HttpServer", LOG_LEVEL_ALL);
  LogComponentEnable ("SatHttpExample", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  // GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Create reference system, two options:
  // - "Scenario72"
  // - "Scenario98"
  std::string scenarioName = "Scenario72";
  // std::string scenarioName = "Scenario98";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if (scenarioLogFile != "")
    {
      helper->EnableCreationTraces (scenarioLogFile, false);
    }

  helper->CreateScenario (satScenario);

  // get users
  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  HttpHelper httpHelper ("ns3::TcpSocketFactory");
  httpHelper.InstallUsingIpv4 (gwUsers.Get (0), utUsers);
  httpHelper.GetServer ().Start (Seconds (1.0));
  httpHelper.GetClients ().Start (Seconds (3.0));

  // install KPI statistics collector
  HttpKpiHelper kpiHelper (&httpHelper);

  NS_LOG_INFO ("--- sat-http-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO ("  ");

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();

  kpiHelper.Print ();

  Simulator::Destroy ();

  return 0;
}
