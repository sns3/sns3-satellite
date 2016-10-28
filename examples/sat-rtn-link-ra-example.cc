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
 * \file sat-rtn-link-ra-example.cc
 * \ingroup satellite
 *
 *         execute command -> ./waf --run "sat-rtn-link-ra-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-rtn-link-ra-example");

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (3);
  uint32_t utsPerBeam (3);
  Time simLength (Seconds (50.0));

  std::string simulationName ("sat-rtn-link-ra-example");
  std::string outputPath ("");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("OutputPath", "Output path for statistics files.", outputPath);
  cmd.Parse (argc, argv);

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> (simulationName);

  simulationHelper->SetDefaultValues ();
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetSimulationTime (simLength.GetSeconds ());
  simulationHelper->DisableAllCapacityAssignmentCategories ();
  simulationHelper->EnableCrdsa ();

  simulationHelper->SetBeams ("1 3 5 7 9 22 24 26 28 30 44 46 48 50 59 61 70 72");

  /* Simulation tags *****************************************************************/

  std::stringstream sstag;
  sstag << simulationName  << "UTs=" << utsPerBeam;
  simulationHelper->SetOutputTag (sstag.str ());

  if (outputPath != "")
  {
      simulationHelper->SetOutputPath (outputPath);
  }

  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario ();

  // get users
  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  cbrHelper.SetAttribute ("Interval", TimeValue (MilliSeconds (33)));
  cbrHelper.SetAttribute ("PacketSize", UintegerValue (20) );

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  // initialized time values for simulation
  uint32_t maxTransmitters = utUsers.GetN ();

  ApplicationContainer gwApps;
  ApplicationContainer utApps;

  Time cbrStartDelay = Seconds (1);

  // Cbr and Sink applications creation
  for ( uint32_t i = 0; i < maxTransmitters; i++)
    {
      cbrHelper.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
      sinkHelper.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

      utApps.Add (cbrHelper.Install (utUsers.Get (i)));
      gwApps.Add (sinkHelper.Install (gwUsers.Get (0)));

      cbrStartDelay += Seconds (0.05);

      utApps.Get (i)->SetStartTime (cbrStartDelay);
      utApps.Get (i)->SetStopTime (simLength);
    }

  utApps.Start (Seconds (1));
  utApps.Stop (simLength);

  simulationHelper->CreateDefaultRtnLinkStats ();
  simulationHelper->EnableProgressLogging ();
  simulationHelper->RunSimulation ();

  return 0;
}
