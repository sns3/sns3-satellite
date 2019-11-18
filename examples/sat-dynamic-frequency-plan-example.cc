/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 *
 */


#include "ns3/core-module.h"
#include "ns3/traffic-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/config-store-module.h"


using namespace ns3;


/**
 * \file sat-generic-launcher.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results with
 * a high degree of customization through XML file.
 *
 * execute command -> ./waf --run "sat-generic-launcher --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-generic-launcher");


int
main (int argc, char *argv[])
{
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/generic-input-attributes.xml";

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("generic-launcher");
  simulationHelper->DisableAllCapacityAssignmentCategories ();
  simulationHelper->EnableCrdsa ();

  CommandLine cmd;
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  // Disable traffic from XML before configuring the simulation
  Config::SetDefault ("ns3::SimulationHelperConf::TrafficCbrPercentage", DoubleValue (0.0));
  Config::SetDefault ("ns3::SimulationHelperConf::TrafficHttpPercentage", DoubleValue (0.0));
  Config::SetDefault ("ns3::SimulationHelperConf::TrafficOnOffPercentage", DoubleValue (0.0));
  Config::SetDefault ("ns3::SimulationHelperConf::TrafficNrtvPercentage", DoubleValue (0.0));
  simulationHelper->ConfigureAttributesFromFile (inputFileNameWithPath);
  simulationHelper->StoreAttributesToFile ("parametersUsed.xml");

  // Configure our own kind of traffic
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (40));
  simulationHelper->InstallTrafficModel (
      SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
      Seconds (0), Seconds (60), MilliSeconds (50), 1.0);

  simulationHelper->RunSimulation ();
}
