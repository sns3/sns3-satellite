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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
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
 * \file traffic-helper-example.cc
 * \ingroup satellite
 *
 * \brief  Test of satellite-traffic-helper module.
 */

NS_LOG_COMPONENT_DEFINE ("sat-traffic-helper");

int
main (int argc, char *argv[])
{
  std::string beams = "12";
  uint32_t nb_gw = 1;
  uint32_t endUsersPerUt = 1;
  uint32_t utsPerBeam = 10;
  //uint32_t packetSize = 1000;
  std::string interval = "10ms";
  double simLength = 60.0;

  Time appStartTime = Seconds (0.001);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  //Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));
  //Config::SetDefault ("ns3::SatBeamHelper::FadingModel", StringValue ("FadingMarkov"));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  Config::SetDefault ("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue (8e+09));
  Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (500e+06));
  Config::SetDefault ("ns3::SatConf::FwdCarrierRollOff", DoubleValue (0.05));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-traffic-helper");
  simulationHelper->SetSimulationTime (simLength);

  // We set the UT count and UT user count using attributes when configuring a pre-defined scenario
  simulationHelper->SetGwUserCount (nb_gw);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetBeams (beams);

  //TODO cancel some changes made in TrafficHelper...
  //Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  //Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  simulationHelper->CreateSatScenario ();

  /*simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    appStartTime, Seconds (simLength), Seconds (0.001));*/

  Ptr<SatTrafficHelper> trafficHelper = simulationHelper->GetTrafficHelper ();
  Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();
  trafficHelper->AddVoipTraffic (SatTrafficHelper::G_711_1, satHelper->GetGwUsers (), satHelper->GetUtUsers (), appStartTime, Seconds (simLength), Seconds (0.001));

  //simulationHelper->CreateDefaultFwdLinkStats ();
  simulationHelper->EnableProgressLogs ();

  // To store attributes to file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("contrib/satellite/data/sims/sat-traffic-helper/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  // Stats
  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  
  s->AddPerUtFwdPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerUtFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->RunSimulation ();

  return 0;

} // end of `int main (int argc, char *argv[])`
