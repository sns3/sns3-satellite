/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/cbr-helper.h"
#include "ns3/config-store.h"


using namespace ns3;

/**
* \ingroup satellite
*
* \brief  Another example of CBR application usage in satellite network.
*         The scripts is using user defined scenario, which means that user
*         can change the scenario size quite to be whatever between 1 and
*         full scenario (72 beams). Currently it is configured to using only
*         one beam. CBR application is sending packets in RTN link, i.e. from UT
*         side to GW side. Packet trace and KpiHelper are enabled by default.
*         End user may change the number of UTs and end users from
*         the command line.
*
*         execute command -> ./waf --run "sat-cbr-example --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-dama-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (3);
  uint32_t damaConf (0);
  Time simLength (Seconds(5.0));
  Time appStartTime = Seconds(0.5);

  // CBR parameters
  uint32_t packetSize (128);
  Time interval (Seconds(1.0));

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("tn9-dama-input-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  /**
   * Attributes:
   * -----------
   *
   * Scenario: 1 beam (beam id = 1)
   *
   * Frame configuration:
   * - 3 frames
   * - 125 MHz user bandwidth
   *    - 40 x 0.625 MHz -> 25 MHz
   *    - 40 x 1.25 MHz -> 50 MHz
   *    - 20 x 2.5 MHz -> 50 MHz
   *
   * Conf-2 scheduling mode (dynamic time slots)
   * - FCA disabled
   *
   * RTN link
   *   - ACM enabled
   *   - Constant interference
   *   - AVI error model
   *   - ARQ disabled
   * FWD link
   *   - ACM disabled
   *   - Constant interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue("damaConf", "DAMA configuration", damaConf);
  cmd.Parse (argc, argv);

  switch (damaConf)
  {
    // CRA only
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=50]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=128]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(false));
        break;
      }
    // CRA + RBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=50]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(false));
        break;
      }
    // CRA + VBDC
    case 2:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=50]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(true));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported damaConf: " << damaConf);
        break;
      }
  }

  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // create user defined scenario
  SatBeamUserInfo beamInfo = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[beamId] = beamInfo;
  helper->SetBeamUserInfo (beamMap);
  helper->EnablePacketTrace ();

  helper->CreateScenario (SatHelper::USER_DEFINED);

  // enable info logs
  //LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("sat-cbr-user-defined-example", LOG_LEVEL_INFO);

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
   cbrHelper.SetAttribute("Interval", TimeValue (interval));
   cbrHelper.SetAttribute("PacketSize", UintegerValue (packetSize) );

   PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

   // initialized time values for simulation
   uint32_t maxTransmitters = utUsers.GetN ();

   ApplicationContainer gwApps;
   ApplicationContainer utApps;

   Time cbrStartDelay = appStartTime;

   // Cbr and Sink applications creation
   for ( uint32_t i = 0; i < maxTransmitters; i++)
     {
       cbrHelper.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
       sinkHelper.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

       utApps.Add(cbrHelper.Install (utUsers.Get (i)));
       gwApps.Add(sinkHelper.Install (gwUsers.Get (0)));

       cbrStartDelay += Seconds (0.05);

       utApps.Get(i)->SetStartTime (cbrStartDelay);
       utApps.Get(i)->SetStopTime (simLength);
     }

   // Add the created applications to CbrKpiHelper
   CbrKpiHelper kpiHelper (KpiHelper::KPI_RTN);
   kpiHelper.AddSink (gwApps);
   kpiHelper.AddSender (utApps);

   utApps.Start (appStartTime);
   utApps.Stop (simLength);

  //const std::string protocol = "ns3::UdpSocketFactory";

  /**
   * Set-up CBR traffic
   */
   /*
  const InetSocketAddress gwAddr = InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port);

  for (NodeContainer::Iterator itUt = utUsers.Begin ();
      itUt != utUsers.End ();
      ++itUt)
    {
      appStartTime += Seconds (0.05);

      // return link
      Ptr<CbrApplication> rtnApp = CreateObject<CbrApplication> ();
      rtnApp->SetAttribute ("Protocol", StringValue (protocol));
      rtnApp->SetAttribute ("Remote", AddressValue (gwAddr));
      rtnApp->SetAttribute ("Interval", TimeValue (interval));
      rtnApp->SetAttribute ("PacketSize", UintegerValue (packetSize));
      rtnApp->SetStartTime (appStartTime);
      rtnApp->SetStopTime (simLength);
      (*itUt)->AddApplication (rtnApp);
    }

  // setup packet sinks at all users
  Ptr<PacketSink> ps = CreateObject<PacketSink> ();
  ps->SetAttribute ("Protocol", StringValue (protocol));
  ps->SetAttribute ("Local", AddressValue (gwAddr));
  gwUsers.Get (0)->AddApplication (ps);
*/
  /**
   * Set-up statistics
   */

  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);
  s->SetName ("cbr");

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
  //  s->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //  s->AddPerGwCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //  s->AddGlobalCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //  CALL_SAT_STATS_DISTRIBUTION_SET (ResourcesGranted)
  //  s->AddPerBeamBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //  s->AddPerGwBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //  s->AddGlobalBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);

  NS_LOG_INFO("--- Cbr-user-defined-example ---");
  NS_LOG_INFO("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO("  Simulation length: " << simLength.GetSeconds ());
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("tn9-dama-output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();

  /**
   * Run simulation
   */
  Simulator::Stop (simLength);
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
