/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
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
 * \file sat-fwd-link-beam-hopping-example.cc
 * \ingroup satellite
 *
 * This simulation script is an example of FWD link beam hopping
 * configuration. All spot-beams of GW-1 are enabled and a proper
 * beam hopping pattern is set at the simulation helper. Each spot-
 * beam has by default even loading.
 *
 *         execute command -> ./waf --run "sat-fwd-link-beam-hopping-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-fwd-link-beam-hopping-example");

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (3);
  bool enableBeamHopping (true);
  Time simLength (Seconds (50.0));

  std::string simulationName ("sat-fwd-link-beam-hopping-example");
  std::string outputPath ("");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("enableBeamHopping", "Enable FWD link beam hopping", enableBeamHopping);
  cmd.AddValue ("OutputPath", "Output path for statistics files.", outputPath);
  cmd.Parse (argc, argv);

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> (simulationName);

  simulationHelper->SetDefaultValues ();
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->ConfigureFwdLinkBeamHopping ();
  simulationHelper->SetSimulationTime (simLength.GetSeconds());

  // All spot-beams of GW-1 (14 in total)
  simulationHelper->SetBeams ("1 2 3 4 11 12 13 14 25 26 27 28 40 41");

  /* Simulation tags *****************************************************************/

  uint32_t bhInt = (enableBeamHopping == true) ? 1 : 0;
  std::stringstream sstag;
  sstag << simulationName << "UTs=" << utsPerBeam << "BH=" << bhInt;
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
  cbrHelper.SetAttribute ("Interval", TimeValue (MilliSeconds (5)));
  cbrHelper.SetAttribute ("PacketSize", UintegerValue (128) );

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  // initialized time values for simulation
  uint32_t maxTransmitters = utUsers.GetN ();

  ApplicationContainer gwApps;
  ApplicationContainer utApps;

  Time cbrStartDelay = Seconds (1);

  // Cbr and Sink applications creation
  for ( uint32_t i = 0; i < maxTransmitters; i++)
    {
      cbrHelper.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (i)), port))));
      sinkHelper.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (i)), port))));

      gwApps.Add (cbrHelper.Install (gwUsers.Get (0)));
      utApps.Add (sinkHelper.Install (utUsers.Get (i)));

      cbrStartDelay += Seconds (0.05);

      utApps.Get (i)->SetStartTime (cbrStartDelay);
      utApps.Get (i)->SetStopTime (simLength);
    }

  utApps.Start (Seconds (1));
  utApps.Stop (simLength);

  simulationHelper->CreateDefaultFwdLinkStats ();
  simulationHelper->EnableProgressLogging ();
  simulationHelper->RunSimulation ();

  return 0;
}
