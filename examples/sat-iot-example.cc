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
 * \brief This file allows to create an IOT scenario
 */

NS_LOG_COMPONENT_DEFINE ("sat-iot-example");

int
main (int argc, char *argv[])
{
  // Variables
  std::string beams = "8";
  uint32_t nbGw = 1;
  uint32_t nbUtsPerBeam = 10;
  uint32_t nbEndUsersPerUt = 1;

  Time appStartTime = Seconds (0.001);
  Time simLength = Seconds (100.0);

  uint32_t queueSize = 10;
  double maxPowerTerminalW = 0.3;
  bool useSic = true;

  std::string modcodsUsed = "QPSK_1_TO_2 QPSK_3_TO_5 QPSK_2_TO_3 QPSK_3_TO_4 QPSK_4_TO_5 QPSK_5_TO_6 QPSK_8_TO_9 QPSK_9_TO_10 "
          "8PSK_3_TO_5 8PSK_2_TO_3 8PSK_3_TO_4 8PSK_5_TO_6 8PSK_8_TO_9 8PSK_9_TO_10 "
          "16APSK_2_TO_3 16APSK_3_TO_4 16APSK_4_TO_5 16APSK_5_TO_6 16APSK_8_TO_9 16APSK_9_TO_10 "
          "32APSK_3_TO_4 32APSK_4_TO_5 32APSK_5_TO_6 32APSK_8_TO_9";

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-iot-example");

  // Read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("NbGw", "Number of GWs", nbGw);
  cmd.AddValue ("NbUtsPerBeam", "Number of UTs per spot-beam", nbUtsPerBeam);
  cmd.AddValue ("NbEndUsersPerUt", "Number of end users per UT", nbEndUsersPerUt);
  cmd.AddValue ("QueueSize", "Satellite queue sizes in packets", queueSize);
  cmd.AddValue ("AppStartTime", "Applications start time (in seconds, or add unit)", appStartTime);
  cmd.AddValue ("SimLength", "Simulation length (in seconds, or add unit)", simLength);
  cmd.AddValue ("MaxPowerTerminalW", "Maximum power of terminals in W", maxPowerTerminalW);
  cmd.AddValue ("UseSic", "Set true to activate the SIC", useSic);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);


  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  /*
   * FWD link
   */
  // Set defaults TODO default plan ?
  Config::SetDefault ("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue (8e+09));
  Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (500e+06));
  Config::SetDefault ("ns3::SatConf::FwdCarrierRollOff", DoubleValue (0.05));

  // ModCods selection
  Config::SetDefault ("ns3::SatBeamHelper::DvbVersion", StringValue ("DVB_S2"));
  Config::SetDefault ("ns3::SatBbFrameConf::ModCodsUsed", StringValue (modcodsUsed));
  Config::SetDefault ("ns3::SatBbFrameConf::DefaultModCod", StringValue ("QPSK_1_TO_2"));

  // Queue size
  Config::SetDefault ("ns3::SatQueue::MaxPackets", UintegerValue (queueSize));

  // Power limitation
  Config::SetDefault ("ns3::SatUtPhy::TxMaxPowerDbw", DoubleValue (SatUtils::LinearToDb (maxPowerTerminalW)));

  /*
   * RTN link
   */
  // Band
  Config::SetDefault ("ns3::SatConf::RtnFeederLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::RtnFeederLinkBaseFrequency", DoubleValue (1.77e+10));
  Config::SetDefault ("ns3::SatConf::RtnUserLinkBandwidth", DoubleValue (5e+08));
  Config::SetDefault ("ns3::SatConf::RtnUserLinkBaseFrequency", DoubleValue (2.95e+10));

  Config::SetDefault ("ns3::SatConf::RtnUserLinkChannels", UintegerValue (4));
  Config::SetDefault ("ns3::SatConf::RtnFeederLinkChannels", UintegerValue (16));

  // SIC
  if (useSic)
    {
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_PERFECT));
    }
  else
    {
      NS_FATAL_ERROR ("Not implemented yet...");
      Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_NONE));
    }

  // Other


  /*
   * Traffics
   */
  simulationHelper->SetSimulationTime (simLength);

  // We set the UT count and UT user count using attributes when configuring a pre-defined scenario
  simulationHelper->SetGwUserCount (nbGw);
  simulationHelper->SetUtCountPerBeam (nbUtsPerBeam);
  simulationHelper->SetUserCountPerUt (nbEndUsersPerUt);
  simulationHelper->SetBeams (beams);

  simulationHelper->CreateSatScenario ();

  Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();
  Ptr<SatTrafficHelper> trafficHelper = simulationHelper->GetTrafficHelper ();
  trafficHelper->AddPoissonTraffic (SatTrafficHelper::FWD_LINK,
                                  Seconds (1),
                                  Seconds (0.1), // TODO I have no idea of these values
                                  "100kb/s",
                                  1500,
                                  satHelper->GetGwUsers (),
                                  satHelper->GetUtUsers (),
                                  appStartTime,
                                  simLength,
                                  Seconds (0.001));
  trafficHelper->AddCbrTraffic (SatTrafficHelper::FWD_LINK,
                                  "10ms",
                                  1500, // TODO I have no idea of these values neither
                                  satHelper->GetGwUsers (),
                                  satHelper->GetUtUsers (),
                                  appStartTime,
                                  simLength,
                                  Seconds (0.001));

  // Link results
  Ptr<SatCnoHelper> satCnoHelper = simulationHelper->GetCnoHelper ();
  satCnoHelper->UseTracesForDefault (false);
  // TODO change path
  for (uint32_t i = 0; i < satHelper->GetBeamHelper ()->GetUtNodes ().GetN (); i++)
    {
      satCnoHelper->SetUtNodeCnoFile (satHelper->GetBeamHelper ()->GetUtNodes ().Get (i), SatEnums::FORWARD_USER_CH, "contrib/satellite/data/rxcnotraces/input/BEAM_8_GW_2_channelType_FORWARD_FEEDER_CH");
    }

  /*
   * Outputs
   */
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  simulationHelper->EnableProgressLogs ();

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("contrib/satellite/data/sims/sat-iot-example/output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  // Link SINR
  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // SINR
  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);

  // Link RX Power
  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  simulationHelper->RunSimulation ();

  return 0;

} // end of `int main (int argc, char *argv[])`
