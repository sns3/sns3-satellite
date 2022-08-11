/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
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
 * \file sat-handover-example.cc
 * \ingroup satellite
 *
 * \brief  Cbr example application to use satellite network.
 *          Interval, packet size and test scenario can be given
 *         in command line as user argument.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-handover-example --PrintHelp"
 *
 *         Cbr example application sends first packets from GW connected user
 *         to UT connected users and after that from UT connected user to GW connected
 *         user.
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-handover-example");

void
PerformHandover (Ptr<SatMobilityModel> model, GeoCoordinate pos)
{
  std::cout << "Handover" << std::endl;
  model->SetGeoPosition (pos);
}

int
main (int argc, char *argv[])
{
  /// Set regeneration mode
  //Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  //Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100000));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-handover");

  simulationHelper->SetSimulationTime (Seconds (10));

  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (SatHelper::LARGER);

  Ptr<Node> ut13 = helper->UtNodes ().Get (3);
  Ptr<SatMobilityModel> mobilityModel13 = ut13->GetObject<SatMobilityModel> ();
  Ptr<Node> ut11 = helper->UtNodes ().Get (3);
  Ptr<SatMobilityModel> mobilityModel11 = ut11->GetObject<SatMobilityModel> ();

  Simulator::Schedule (Seconds (5), &PerformHandover, mobilityModel13, mobilityModel11->GetGeoPosition ());

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("100ms"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512) );

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    Seconds (1.0), Seconds (10.0));

  // To store attributes to file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddPerBeamFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->EnableProgressLogs ();
  simulationHelper->RunSimulation ();

  return 0;
}
