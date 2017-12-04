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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;


/**
 * \file sat-http-example.cc
 * \ingroup satellite
 * \brief Example of using HTTP traffic model in a satellite network.
 *
 * One HTTP server application is installed in the first GW user. Then one HTTP
 * client application is installed in each UT user, configured to point to the
 * server. TCP protocol is used between the applications.
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
NS_LOG_COMPONENT_DEFINE ("sat-http-example");


int
main (int argc, char *argv[])
{
  std::string scenario = "simple";
  double duration = 1000;
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  auto simulationHelper = CreateObject<SimulationHelper> ("example-http");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full)",
                scenario);
  cmd.AddValue ("duration", "Simulation duration (in seconds)",
                duration);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  if (scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if (scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  /// Set simulation output details
  simulationHelper->SetSimulationTime (duration);
  simulationHelper->SetOutputTag (scenario);

  //LogComponentEnableAll (LOG_PREFIX_ALL);
  //LogComponentEnable ("HttpClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("HttpServer", LOG_LEVEL_ALL);
  LogComponentEnable ("sat-http-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  // GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);

  // get users
  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  ThreeGppHttpHelper httpHelper;
  httpHelper.InstallUsingIpv4 (gwUsers.Get (0), utUsers);
  httpHelper.GetServer ().Start (Seconds (1.0));

  auto apps = httpHelper.GetClients ();
  apps.Start (Seconds (3.0));

  uint32_t i = 0;
  std::vector<Ptr<ClientRxTracePlot> > plots;
  for (auto app = apps.Begin (); app != apps.End (); app++, i++)
  {
    std::stringstream plotName;
    plotName << "3GPP-HTTP-client-" << i << "-trace";
  	plots.push_back (CreateObject<ClientRxTracePlot> (*app, plotName.str ()));
  }

  NS_LOG_INFO ("--- sat-http-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  ");

  simulationHelper->RunSimulation();

  plots.clear();

  return 0;

} // end of `int main (int argc, char *argv[])`
