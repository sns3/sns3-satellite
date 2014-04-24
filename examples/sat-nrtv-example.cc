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

using namespace ns3;


/**
 * \ingroup satellite
 * \brief Example of using NRTV traffic model in a satellite network.
 *
 * One NRTV server application is installed in the first GW user. Then one NRTV
 * client application is installed in each UT user, configured to point to the
 * server. TCP protocol is used between the applications.
 *
 * By default, the SIMPLE test scenario is used. Another test scenario can be
 * given from command line as user argument, e.g.:
 *
 *     $ ./waf --run="sat-nrtv-example --scenario=larger"
 *     $ ./waf --run="sat-nrtv-example --scenario=full"
 *
 * Simulation runs for 100 seconds by default. This can be changed from the
 * command line argument as well, e.g.:
 *
 *     $ ./waf --run="sat-nrtv-example --duration=500"
 *
 * To see help for user arguments:
 *
 *     $ ./waf --run "sat-nrtv-example --PrintHelp"
 *
 */
NS_LOG_COMPONENT_DEFINE ("sat-nrtv-example");


int
main (int argc, char *argv[])
{
  // a workaround to partially resolve weird splitting in lower layer
  // Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
  //                     StringValue ("ns3::TcpRfc793"));

  std::string scenario = "simple";
  double duration = 100;
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
  //LogComponentEnable ("NrtvClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("NrtvServer", LOG_LEVEL_ALL);
  LogComponentEnable ("sat-nrtv-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  // GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if (scenarioLogFile != "")
    {
      helper->EnableCreationTraces (scenarioLogFile, false);
    }

  helper->CreateScenario (satScenario);

  // get users
  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  NrtvHelper nrtvHelper ("ns3::TcpSocketFactory");
  nrtvHelper.InstallUsingIpv4 (gwUsers.Get (0), utUsers);
  nrtvHelper.GetServer ().Start (Seconds (1.0));
  nrtvHelper.GetClients ().Start (Seconds (3.0));

  // install KPI statistics collector
  NrtvKpiHelper kpiHelper (&nrtvHelper);

  NS_LOG_INFO ("--- sat-nrtv-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO ("  ");

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();

  kpiHelper.Print ();

  Simulator::Destroy ();

  return 0;

} // end of `int main (int argc, char *argv[])`
