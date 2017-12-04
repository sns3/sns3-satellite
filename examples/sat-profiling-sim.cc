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
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-profiling-sim.cc
 * \ingroup satellite
 *
 * \brief
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-profiling-sim --PrintHelp"
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-profiling-sim");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

  uint32_t utsPerBeam (1);
  uint32_t endUsersPerUt (1);
  double simulationTime (10.0);

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-profiling-sim");

  simulationHelper->SetDefaultValues ();
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetSimulationTime (simulationTime);

  simulationHelper->SetBeams ("48");
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario ();

  // ----------------------------------
  // ----- CREATE CBR APPLICATION -----
  // ----------------------------------
  NodeContainer utUsers = helper->GetUtUsers ();

  NS_ASSERT (utUsers.GetN () == 1);

  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (64));
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (Seconds (2)));
  simulationHelper->InstallTrafficModel (
  		SimulationHelper::CBR,
			SimulationHelper::UDP,
			SimulationHelper::RTN_LINK,
			Seconds (1));
  simulationHelper->InstallTrafficModel (
			SimulationHelper::CBR,
			SimulationHelper::UDP,
			SimulationHelper::FWD_LINK,
			Seconds (1));
  // ----------------------------------
  // ----- CREATE CBR APPLICATION -----
  // ----------------------------------

  simulationHelper->CreateDefaultRtnLinkStats ();
  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
