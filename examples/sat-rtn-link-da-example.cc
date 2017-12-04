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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"


using namespace ns3;

/**
 * \file sat-rtn-link-da-example.cc
 * \ingroup satellite
 *
 *         execute command -> ./waf --run "sat-rtn-link-da-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-rtn-link-da-example");

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (3);
  uint32_t utsPerBeam (3);
  Time simLength (Seconds (50.0));

  std::string simulationName = "sat-rtn-link-da-example";
  auto simulationHelper = CreateObject<SimulationHelper> (simulationName);

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  simulationHelper->SetDefaultValues ();
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetSimulationTime (simLength.GetSeconds());
  simulationHelper->EnableOnlyRbdc (3);

  simulationHelper->SetBeams ("1 3 5 7 9 22 24 26 28 30 44 46 48 50 59 61 70 72");

  /* Simulation tags *****************************************************************/

  std::stringstream sstag;
  sstag << simulationName << "UTs=" << utsPerBeam;
  simulationHelper->SetOutputTag (sstag.str ());

  // Create satellite scenario
  simulationHelper->CreateSatScenario ();

  // >>> Start of actual test using Full scenario >>>
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (MilliSeconds (5)));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (128) );
  simulationHelper->InstallTrafficModel (
  		SimulationHelper::CBR,
			SimulationHelper::UDP,
			SimulationHelper::RTN_LINK,
			Seconds (1), Seconds (simLength), Seconds (0.05));

  // Create RTN link statistics
  simulationHelper->CreateDefaultRtnLinkStats ();

  // Enable logs
  simulationHelper->EnableProgressLogs ();

  // Run
  simulationHelper->RunSimulation ();

  return 0;
}
