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
 * \file sat-link-budget-example.cc
 * \ingroup satellite
 *
 * \brief  Link budget example application for link budget calculation purposes in satellite network.
 *         Some parameter can be set as command line arguments.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-link-budget-example --PrintHelp"
 *
 *         Application sends one packet from GW connected user to UT connected users and
 *         from UT connected user to GW connected user. User defined scenario is created with given beam ID.
 *
 *         Example reads sat-link-budget-input-attributes.xml, so user is able to change link budget parameter by
 *         modifying this file without compiling example.
 *
 *         Used parameters and link budget calculation result are printed as log info.
 */

NS_LOG_COMPONENT_DEFINE ("sat-link-budget-example");

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
                                      << params->m_carrierFreq_hz << " "
                                      << SatUtils::WToDbW (ifPower) << " "
                                      << SatUtils::WToDbW ( params->m_rxPower_W ) << " "
                                      << SatUtils::LinearToDb (params->m_sinr) << " "
                                      << SatUtils::LinearToDb (cSinr) );
    }
}

int
main (int argc, char *argv[])
{
  uint32_t beamId = 11;
  double latitude = 50.00;
  double longitude = -1.00;
  double altitude = 0.00;

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-link-budget");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/sat-link-budget-input-attributes.xml";

  // read command line parameters can be given by user
  CommandLine cmd;
  cmd.AddValue ("beam", "Beam to use for testing. (1 - 72)", beamId);
  cmd.AddValue ("latitude", "Latitude of UT position (-90 ... 90.0)", latitude);
  cmd.AddValue ("longitude", "Longitude of UT position (-180 ... 180)", longitude);
  cmd.AddValue ("altitude", "Altitude of UT position (meters)", altitude);
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  // To change attributes having affect on link budget,
  // modify attributes available in sat-link-budget-input-attributes.xml found in same directory this source file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  simulationHelper->SetUtCountPerBeam (1);
  simulationHelper->SetUserCountPerUt (1);
  simulationHelper->SetBeamSet ({beamId});
  simulationHelper->SetSimulationTime (Seconds (1.1));

  // enable info logs
  LogComponentEnable ("sat-link-budget-example", LOG_LEVEL_INFO);

  // Create a position allocator for our single UT
  Ptr<SatListPositionAllocator> posAllocator = CreateObject<SatListPositionAllocator> ();
  posAllocator->Add ( GeoCoordinate (latitude, longitude, altitude));
  simulationHelper->SetUtPositionAllocatorForBeam (beamId, posAllocator);

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
  // Set UT position
  NodeContainer ut = helper->UtNodes ();
  Ptr<SatMobilityModel> utMob = ut.Get (0)->GetObject<SatMobilityModel> ();

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

  NodeContainer gw = helper->GwNodes ();
  Ptr<SatMobilityModel> gwMob = gw.Get (0)->GetObject<SatMobilityModel> ();

  Ptr<Node> geo = helper->GeoSatNode ();
  Ptr<SatMobilityModel> geoMob = geo->GetObject<SatMobilityModel> ();

  // print used parameters using log info
  NS_LOG_INFO ("--- satellite-link-budget-example ---");
  NS_LOG_INFO (" Beam ID: " << beamId);
  NS_LOG_INFO (" Geo position: " << geoMob->GetGeoPosition () << " " << geoMob->GetPosition () );
  NS_LOG_INFO (" GW position: " << gwMob->GetGeoPosition () << " " << gwMob->GetPosition () );
  NS_LOG_INFO (" UT position: " << utMob->GetGeoPosition () << " " << utMob->GetPosition ());
  NS_LOG_INFO ("  ");
  NS_LOG_INFO ("Link results (Time, Channel type, Own address, Dest. address, Beam ID, Carrier Center freq, IF Power, RX Power, SINR, Composite SINR) :");
  // results are printed out in callback (LinkBudgetTraceCb)

  simulationHelper->RunSimulation ();

  return 0;
}
