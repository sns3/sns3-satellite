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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-list-position-ext-fading-example.cc
 * \ingroup satellite
 *
 * \brief  List position external fading example demonstrates how to set user defined (list)
 *         positions with external fading. The example is useful when new list positions and
 *         external fading trace sources are taken into use.
 *
 *         The default values for list position (UT positions) and external fading trace are
 *         set before command line parsing, in order to replace them without re-compiling the example.
 *
 *         Some parameter can be set as command line arguments.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-ext-fading-example --PrintHelp"
 *
 *         Example sends packets from GW connected user to every UT users and
 *         from every UT users to GW connected user.
 *
 *         Information of the created UTs and link budget info for every received packet
 *         are printed as log info.
 *
 */

NS_LOG_COMPONENT_DEFINE ("sat-list-position-ext-fading-example");

// callback called when packet is received by phy RX carrier
static void LinkBudgetTraceCb ( std::string context, Ptr<SatSignalParameters> params, Mac48Address ownAdd, Mac48Address destAdd,
                                double ifPower, double cSinr)
{
  // print only unicast message to prevent printing control messages like TBTP messages
  if ( !destAdd.IsBroadcast () )
    {
      NS_LOG_INFO ( Simulator::Now () << " "
                                      << params->m_channelType << " "
                                      << ownAdd << " "
                                      << destAdd << " "
                                      << params->m_beamId << " "
                                      << SatUtils::LinearToDb (params->m_sinr) << " "
                                      << SatUtils::LinearToDb (cSinr) );
    }
}

int
main (int argc, char *argv[])
{
  uint32_t utCount = 4;
  uint32_t usersPerUt = 1;
  uint32_t beamId = 1;
  bool checkBeam = false;
  std::string extUtPositions = "utpositions/BeamId-1_256_UT_Positions.txt";

  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));


  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-list-position-external-fading");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  // read command line parameters can be given by user
  CommandLine cmd;
  cmd.AddValue ("beamId", "The beam ID to be used.", beamId);
  cmd.AddValue ("checkBeam", "Check that given beam is the best according in the configured positions.", checkBeam);
  cmd.AddValue ("utCount", "Number of the UTs.", utCount);
  cmd.AddValue ("usersPerUt", "Users per UT.", usersPerUt);
  cmd.AddValue ("externalUtPositionFile", "UT position input file (in data/)", extUtPositions);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  // Set default values for some attributes for position setting and external fading trace
  // This done before command line parsing in order to override them if needed

  simulationHelper->SetUserCountPerUt (usersPerUt);
  simulationHelper->SetUtCountPerBeam (utCount);
  simulationHelper->SetBeamSet ({beamId});
  simulationHelper->SetSimulationTime (Seconds (1.1));

  // Enable default fading traces
  simulationHelper->EnableExternalFadingInputTrace ();

  // Enable UT positions from input file
  simulationHelper->EnableUtListPositionsFromInputFile (extUtPositions, checkBeam);

  // enable info logs
  LogComponentEnable ("sat-list-position-ext-fading-example", LOG_LEVEL_INFO);

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario ();

  // set callback traces where we want results out
  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  // Install CBR traffic model
  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("0.1s"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512));
  simulationHelper->InstallTrafficModel (
  		SimulationHelper::CBR,
  		SimulationHelper::UDP,
			SimulationHelper::FWD_LINK,
			Seconds (0.1), Seconds (0.25));

  simulationHelper->InstallTrafficModel (
  		SimulationHelper::CBR,
  		SimulationHelper::UDP,
			SimulationHelper::RTN_LINK,
			Seconds (0.1), Seconds (0.25));


  NS_LOG_INFO ("--- List Position External Fading Example ---");
  NS_LOG_INFO ("UT info (Beam ID, UT ID, Latitude, Longitude, Altitude + addresses");

  // print UT info
  NS_LOG_INFO (helper->GetBeamHelper ()->GetUtInfo ());
  NS_LOG_INFO ("Link results (Time, Channel type, Own address, Dest. address, Beam ID, SINR, Composite SINR) :");
  // results are printed out in callback (PacketTraceCb)

  simulationHelper->RunSimulation ();

  return 0;
}
