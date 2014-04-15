/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/cbr-helper.h"


using namespace ns3;

#define CALL_SAT_STATS_BASIC_SET(id)                                          \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                    \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                   \
                                                                              \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \
                                                                              \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                   \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                  \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                  \
                                                                              \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \

#define CALL_SAT_STATS_DISTRIBUTION_SET(id)                                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                    \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                 \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_PDF_FILE);                       \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_CDF_FILE);                       \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                 \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                       \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                       \
                                                                              \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                  \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_PDF_FILE);                        \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_CDF_FILE);                        \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                  \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                        \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                        \
                                                                              \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                   \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                  \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_PDF_FILE);                      \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_CDF_FILE);                      \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                  \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                      \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                      \
                                                                              \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                  \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_PDF_FILE);                        \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_CDF_FILE);                        \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                  \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                        \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_CDF_PLOT);

/**
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
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::LARGER;
  double duration = 4;

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue("duration", "Simulation duration (in seconds)", duration);
  cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.Parse (argc, argv);

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  // enable info logs
  //LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-cbr-stats-example", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Create reference system, two options:
  // - "Scenario72"
  // - "Scenario98"
  std::string scenarioName = "Scenario72";
  //std::string scenarioName = "Scenario98";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces(scenarioLogFile, false);
    }

  helper->CreateScenario(satScenario);

  helper->EnablePacketTrace ();

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  uint16_t port = 9;
  const std::string protocol = "ns3::UdpSocketFactory";

  // setup CBR traffic
  for (NodeContainer::Iterator itGw = gwUsers.Begin ();
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
    }

  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);
  s->SetName ("cbr");

  /*
   * The following is the statements for enabling *all* the satellite
   * statistics.
   */
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdAppDelay)
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_PDF_FILE);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_SCATTER_PLOT);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_PDF_PLOT);
//  s->AddPerUtUserFwdAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdDevDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdMacDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdPhyDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdQueueBytes)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdQueuePackets)
//  CALL_SAT_STATS_BASIC_SET (FwdSignallingLoad)
//  CALL_SAT_STATS_DISTRIBUTION_SET (FwdSinr)
//  CALL_SAT_STATS_BASIC_SET (FwdAppThroughput)
//  s->AddPerUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerUtUserFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
//  CALL_SAT_STATS_BASIC_SET (FwdDevThroughput)
//  CALL_SAT_STATS_BASIC_SET (FwdMacThroughput)
//  CALL_SAT_STATS_BASIC_SET (FwdPhyThroughput)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnAppDelay)
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_PDF_FILE);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_SCATTER_PLOT);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_PDF_PLOT);
//  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnDevDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnMacDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnPhyDelay)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnQueueBytes)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnQueuePackets)
//  CALL_SAT_STATS_BASIC_SET (RtnSignallingLoad)
//  CALL_SAT_STATS_DISTRIBUTION_SET (RtnSinr)
//  CALL_SAT_STATS_BASIC_SET (RtnAppThroughput)
//  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
//  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
//  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
//  CALL_SAT_STATS_BASIC_SET (RtnDevThroughput)
//  CALL_SAT_STATS_BASIC_SET (RtnMacThroughput)
//  CALL_SAT_STATS_BASIC_SET (RtnPhyThroughput)
//  CALL_SAT_STATS_BASIC_SET (FwdDaPacketError)
//  CALL_SAT_STATS_BASIC_SET (RtnDaPacketError)
//  CALL_SAT_STATS_BASIC_SET (SlottedAlohaPacketError)
//  CALL_SAT_STATS_BASIC_SET (SlottedAlohaPacketCollision)
//  CALL_SAT_STATS_BASIC_SET (CrdsaPacketError)
//  CALL_SAT_STATS_BASIC_SET (CrdsaPacketCollision)
//  CALL_SAT_STATS_DISTRIBUTION_SET (ResourcesGranted)

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
  s->AddPerUtFwdDevThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerUtFwdPhyThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerGwFwdQueueBytes (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
  s->AddGlobalRtnQueuePackets (SatStatsHelper::OUTPUT_PDF_FILE);
  s->AddPerGwFwdSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdSignallingLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnSignallingLoad (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerUtResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);

  NS_LOG_INFO("--- Cbr-example ---");
  NS_LOG_INFO("  Scenario used: " << scenario);
  NS_LOG_INFO("  PacketSize: " << packetSize);
  NS_LOG_INFO("  Interval: " << interval);
  NS_LOG_INFO("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO("  ");

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}