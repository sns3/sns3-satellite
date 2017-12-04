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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"


using namespace ns3;

#define CALL_SAT_STATS_BASIC_ELEMENT(id)                                      \
  s->Add ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                          \
  s->Add ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                         \
  s->Add ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                         \

#define CALL_SAT_STATS_BASIC_SET(id)                                          \
  CALL_SAT_STATS_BASIC_ELEMENT (Global ## id)                                  \
  CALL_SAT_STATS_BASIC_ELEMENT (PerGw ## id)                                   \
  CALL_SAT_STATS_BASIC_ELEMENT (PerBeam ## id)                                 \
  CALL_SAT_STATS_BASIC_ELEMENT (PerUt ## id)                                   \


#define CALL_SAT_STATS_DISTRIBUTION_ELEMENT(id)                               \
  s->Add ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                          \
  s->Add ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                         \
  s->Add ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                       \
  s->Add ## id (SatStatsHelper::OUTPUT_PDF_FILE);                             \
  s->Add ## id (SatStatsHelper::OUTPUT_CDF_FILE);                             \
  s->Add ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                         \
  s->Add ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                       \
  s->Add ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                             \
  s->Add ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                             \

#define CALL_SAT_STATS_DISTRIBUTION_SET(id)                                   \
  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (Global ## id)                           \
  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (PerGw ## id)                            \
  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (PerBeam ## id)                          \
  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (PerUt ## id)                            \


#define CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET(id)                          \
  s->AddAverageBeam ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);            \
  s->AddAverageBeam ## id (SatStatsHelper::OUTPUT_PDF_FILE);                  \
  s->AddAverageBeam ## id (SatStatsHelper::OUTPUT_CDF_FILE);                  \
  s->AddAverageBeam ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);            \
  s->AddAverageBeam ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                  \
  s->AddAverageBeam ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                  \
                                                                              \
  s->AddAverageUt ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);              \
  s->AddAverageUt ## id (SatStatsHelper::OUTPUT_PDF_FILE);                    \
  s->AddAverageUt ## id (SatStatsHelper::OUTPUT_CDF_FILE);                    \
  s->AddAverageUt ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);              \
  s->AddAverageUt ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                    \
  s->AddAverageUt ## id (SatStatsHelper::OUTPUT_CDF_PLOT);

/**
 * \file sat-cbr-stats-example.cc
 * \ingroup satellite
 *
 * \brief  Cbr example application to use satellite network and to produce
 *         the full range of statistics. Only some of the statistics are enabled
 *         by default.
 *         Interval, packet size and test scenario can be given
 *         in command line as user argument.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-cbr-stats-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-cbr-stats-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "larger";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::LARGER;
  double duration = 4;

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-cbr-stats");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue ("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue ("duration", "Simulation duration (in seconds)", duration);
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full", scenario);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  // Set tag, if output path is not explicitly defined
  simulationHelper->SetOutputTag (scenario);
  simulationHelper->SetSimulationTime (duration);

  // enable info logs
  //LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-cbr-stats-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);

  // get users
 // NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  //uint16_t port = 9;
  //const std::string protocol = "ns3::UdpSocketFactory";

  // setup CBR traffic
  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  for (uint32_t i = 0; i < gwUsers.GetN (); i++)
		{
  		simulationHelper->SetGwUserId (i);
			simulationHelper->InstallTrafficModel (
					SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
					Seconds (0.1), Seconds (duration), Seconds (0.001));
			simulationHelper->InstallTrafficModel (
								SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
								Seconds (0.1), Seconds (duration), Seconds (0.001));
		}

  /*for (NodeContainer::Iterator itGw = gwUsers.Begin ();
       itGw != gwUsers.End (); ++itGw)
    {
      const InetSocketAddress gwAddr
        = InetSocketAddress (helper->GetUserAddress (*itGw), port);

      for (NodeContainer::Iterator itUt = utUsers.Begin ();
           itUt != utUsers.End (); ++itUt)
        {
          const InetSocketAddress utAddr
            = InetSocketAddress (helper->GetUserAddress (*itUt), port);

          // forward link
          Ptr<CbrApplication> fwdApp = CreateObject<CbrApplication> ();
          fwdApp->SetAttribute ("Protocol", StringValue (protocol));
          fwdApp->SetAttribute ("Remote", AddressValue (utAddr));
          fwdApp->SetAttribute ("Interval", StringValue (interval));
          fwdApp->SetAttribute ("PacketSize", UintegerValue (packetSize));
          (*itGw)->AddApplication (fwdApp);

          // return link
          Ptr<CbrApplication> rtnApp = CreateObject<CbrApplication> ();
          rtnApp->SetAttribute ("Protocol", StringValue (protocol));
          rtnApp->SetAttribute ("Remote", AddressValue (gwAddr));
          rtnApp->SetAttribute ("Interval", StringValue (interval));
          rtnApp->SetAttribute ("PacketSize", UintegerValue (packetSize));
          (*itUt)->AddApplication (rtnApp);
        }
    }

  // setup packet sinks at all users
  NodeContainer allUsers (gwUsers, utUsers);
  for (NodeContainer::Iterator it = allUsers.Begin ();
       it != allUsers.End (); ++it)
    {
      const InetSocketAddress addr = InetSocketAddress (helper->GetUserAddress (*it),
                                                        port);
      Ptr<PacketSink> ps = CreateObject<PacketSink> ();
      ps->SetAttribute ("Protocol", StringValue (protocol));
      ps->SetAttribute ("Local", AddressValue (addr));
      (*it)->AddApplication (ps);
    }*/

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  /*
   * The following is the statements for enabling *all* the satellite
   * statistics.
   */
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdAppDelay)
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (PerUtUserFwdAppDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdAppDelay)
//  s->AddAverageUtUserFwdAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
//  s->AddAverageUtUserFwdAppDelay (SatStatsHelper::OUTPUT_PDF_FILE);
//  s->AddAverageUtUserFwdAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
//  s->AddAverageUtUserFwdAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
//  s->AddAverageUtUserFwdAppDelay (SatStatsHelper::OUTPUT_PDF_PLOT);
//  s->AddAverageUtUserFwdAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdDevDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdDevDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdMacDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdMacDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdPhyDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdPhyDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdQueueBytes)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdQueuePackets)
//  CALL_SAT_STATS_BASIC_SET (FwdSignallingLoad)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdCompositeSinr)
//  CALL_SAT_STATS_BASIC_SET (FwdAppThroughput)
//  CALL_SAT_STATS_BASIC_ELEMENT (PerUtUserFwdAppThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdAppThroughput)
//  s->AddAverageUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
//  s->AddAverageUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_PDF_FILE);
//  s->AddAverageUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
//  s->AddAverageUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
//  s->AddAverageUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_PDF_PLOT);
//  s->AddAverageUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);
//  CALL_SAT_STATS_BASIC_SET (FwdDevThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdDevThroughput)
//  CALL_SAT_STATS_BASIC_SET (FwdMacThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdMacThroughput)
//  CALL_SAT_STATS_BASIC_SET (FwdPhyThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (FwdPhyThroughput)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnAppDelay)
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (PerUtUserRtnAppDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnAppDelay)
//  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
//  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_PDF_FILE);
//  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
//  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
//  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_PDF_PLOT);
//  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnDevDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnDevDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnMacDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnMacDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnPhyDelay)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnPhyDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnQueueBytes)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnQueuePackets)
//  CALL_SAT_STATS_BASIC_SET (RtnSignallingLoad)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnCompositeSinr)
//  CALL_SAT_STATS_BASIC_SET (RtnAppThroughput)
//  CALL_SAT_STATS_BASIC_ELEMENT (PerUtUserRtnAppThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnAppThroughput)
//  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
//  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_PDF_FILE);
//  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
//  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
//  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_PDF_PLOT);
//  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);
//  CALL_SAT_STATS_BASIC_SET (RtnDevThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnDevThroughput)
//  CALL_SAT_STATS_BASIC_SET (RtnMacThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnMacThroughput)
//  CALL_SAT_STATS_BASIC_SET (RtnPhyThroughput)
//  CALL_SAT_STATS_AVERAGED_DISTRIBUTION_SET (RtnPhyThroughput)
//  CALL_SAT_STATS_BASIC_SET (FwdDaPacketError)
//  CALL_SAT_STATS_BASIC_SET (RtnDaPacketError)
//  CALL_SAT_STATS_BASIC_SET (SlottedAlohaPacketError)
//  CALL_SAT_STATS_BASIC_SET (SlottedAlohaPacketCollision)
//  CALL_SAT_STATS_BASIC_SET (CrdsaPacketError)
//  CALL_SAT_STATS_BASIC_SET (CrdsaPacketCollision)
//  s->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerGwCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddGlobalCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  CALL_SAT_STATS_DISTRIBUTION_SET (ResourcesGranted)
//  s->AddPerBeamBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerGwBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddGlobalBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerGwFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddGlobalFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerBeamFrameUserLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerGwFrameUserLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddGlobalFrameUserLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerGwWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddGlobalWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalFwdFeederLinkSinr);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalFwdUserLinkSinr);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalRtnFeederLinkSinr);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalRtnUserLinkSinr);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalFwdFeederLinkRxPower);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalFwdUserLinkRxPower);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalRtnFeederLinkRxPower);
//  CALL_SAT_STATS_DISTRIBUTION_ELEMENT (GlobalRtnUserLinkRxPower);

  /*
   * The following is the statements for enabling some satellite statistics
   * for testing purpose.
   */
  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtRtnAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
  s->AddPerBeamFwdDevDelay (SatStatsHelper::OUTPUT_PDF_PLOT);
  s->AddPerGwRtnDevDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalFwdMacDelay (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerUtRtnMacDelay (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
  s->AddPerBeamFwdPhyDelay (SatStatsHelper::OUTPUT_PDF_FILE);
  s->AddPerGwRtnPhyDelay (SatStatsHelper::OUTPUT_CDF_FILE);

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddAverageUtFwdDevThroughput (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
  s->AddAverageBeamRtnDevThroughput (SatStatsHelper::OUTPUT_PDF_PLOT);
  s->AddPerGwFwdMacThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddAverageBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerGwFwdQueueBytes (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
  s->AddGlobalRtnQueuePackets (SatStatsHelper::OUTPUT_PDF_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_PDF_PLOT);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_PDF_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_CDF_FILE);

  s->AddPerGwFwdSignallingLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnSignallingLoad (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
  s->AddPerGwBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameUserLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerGwWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  NS_LOG_INFO ("--- Cbr-stats-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  PacketSize: " << packetSize);
  NS_LOG_INFO ("  Interval: " << interval);
  NS_LOG_INFO ("  ");

  simulationHelper->RunSimulation ();

  return 0;
}

