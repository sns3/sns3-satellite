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
 * \file iot-example.cc
 * \ingroup satellite
 *
 * \brief TODO
 */

NS_LOG_COMPONENT_DEFINE ("sat-iot-example");

int
main (int argc, char *argv[])
{
  // TODO system
  // TODO FWD
  // TODO RTN
  // TODO outputs

  // Variables
  std::string beams = "12";
  uint32_t nb_gw = 1;
  uint32_t endUsersPerUt = 1;
  uint32_t utsPerBeam = 10;

  Time appStartTime = Seconds (0.001);
  double simLength = 60.0;

  std::string modcodsUsed = "QPSK_1_TO_3 QPSK_1_TO_2 QPSK_3_TO_5 QPSK_2_TO_3 QPSK_3_TO_4 QPSK_4_TO_5 QPSK_5_TO_6 QPSK_8_TO_9 QPSK_9_TO_10 "
          "8PSK_3_TO_5 8PSK_2_TO_3 8PSK_3_TO_4 8PSK_5_TO_6 8PSK_8_TO_9 8PSK_9_TO_10 "
          "16APSK_2_TO_3 16APSK_3_TO_4 16APSK_4_TO_5 16APSK_5_TO_6 16APSK_8_TO_9 16APSK_9_TO_10 "
          "32APSK_3_TO_4 32APSK_4_TO_5 32APSK_5_TO_6 32APSK_8_TO_9";

  /*
   * FWD link
   */
  // Frequency plan

  // Link results


  /*
   * RTN link
   */

  // Porteuse

  // Link results

  // Other


  /*
   * Traffics
   */
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-iot-example");
  simulationHelper->SetSimulationTime (simLength);

  // We set the UT count and UT user count using attributes when configuring a pre-defined scenario
  simulationHelper->SetGwUserCount (nb_gw);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetBeams (beams);

  simulationHelper->CreateSatScenario ();

  Ptr<SatTrafficHelper> trafficHelper = simulationHelper->GetTrafficHelper ();
  Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();
  trafficHelper->AddPoissonTraffic (SatTrafficHelper::FWD_LINK,
                                    Seconds (0.1),
                                    Seconds (1),
                                    "10Mbps",
                                    1000,
                                    satHelper->GetGwUsers (),
                                    satHelper->GetUtUsers (),
                                    appStartTime,
                                    Seconds (simLength),
                                    Seconds (0.001));

  /*
   * Outputs
   */
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  simulationHelper->EnableProgressLogs ();

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("contrib/satellite/data/sims/sat-traffic-helper/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

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

  s->AddPerUtRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerUtRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwFwdPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerGwFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerGwFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerGwFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddPerGwRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerGwRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerGwRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerGwRtnPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->RunSimulation ();

  return 0;

} // end of `int main (int argc, char *argv[])`
