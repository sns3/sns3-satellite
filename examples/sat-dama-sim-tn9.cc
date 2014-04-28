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
* \brief Simulation script to run example simulation results related to satellite RTN
* link performance. Currently only one beam is simulated with variable amount of users
* and DAMA configuration.
*
* execute command -> ./waf --run "sat-dama-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-dama-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (3);
  uint32_t damaConf (0);
  uint32_t packetSize (1400);
  bool isNoisy (false);

  double simLength (100.0); // in seconds
  Time appStartTime = Seconds(0.1);

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("./src/satellite/examples/tn9-dama-input-attributes.xml"));
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
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("damaConf", "DAMA configuration", damaConf);
  cmd.AddValue ("simLength", "Simulation duration (in seconds)", simLength);
  cmd.AddValue ("packetSize", "Constant packet size (in bytes)", packetSize);
  cmd.AddValue ("isNoisy", "If true, may print some logging messages", isNoisy);
  cmd.Parse (argc, argv);

  if (isNoisy)
    {
      LogComponentEnable ("sat-dama-sim-tn9", LOG_INFO);
    }

  switch (damaConf)
  {
    // CRA only
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=128]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (12.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.02));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MaxValue", DoubleValue (10000.0));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::BinLength", DoubleValue (100.0));
        break;
      }
    // CRA + RBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (3.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.01));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MaxValue", DoubleValue (30000.0));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::BinLength", DoubleValue (300.0));
        break;
      }
    // CRA + VBDC
    case 2:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(true));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (3.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.01));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MaxValue", DoubleValue (30000.0));
        Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::BinLength", DoubleValue (300.0));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported damaConf: " << damaConf);
        break;
      }
  }

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // create user defined scenario
  SatBeamUserInfo beamInfo = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[beamId] = beamInfo;
  helper->SetBeamUserInfo (beamMap);

  helper->CreateScenario (SatHelper::USER_DEFINED);

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)
  const std::string protocol = "ns3::UdpSocketFactory";

  /**
   * Set-up CBR traffic
   */
  Ptr<UniformRandomVariable> randVariable = CreateObject<UniformRandomVariable> ();
  Time minInterval = Seconds (packetSize / (125 * 1024.0)); // 1024 kbps
  Time maxInterval = Seconds (packetSize / (125 * 128.0)); // 128 kbps
  NS_LOG_INFO ("Minimum interval between packets: " << minInterval.GetSeconds ()
                   << "s (equivalent with 1024 kbps data rate)");
  NS_LOG_INFO ("Maximum interval between packets: " << maxInterval.GetSeconds ()
                   << "s (equivalent with 128 kbps data rate)");
  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
  const InetSocketAddress gwAddr = InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port);

  for (NodeContainer::Iterator itUt = utUsers.Begin ();
      itUt != utUsers.End ();
      ++itUt)
    {
      appStartTime += MilliSeconds (10);

      // return link
      Ptr<CbrApplication> rtnApp = CreateObject<CbrApplication> ();
      rtnApp->SetAttribute ("Protocol", StringValue (protocol));
      rtnApp->SetAttribute ("Remote", AddressValue (gwAddr));
      rtnApp->SetAttribute ("PacketSize", UintegerValue (packetSize));
      double intervalSeconds = randVariable->GetValue (minInterval.GetSeconds (),
                                                       maxInterval.GetSeconds ());
      if (isNoisy)
        {
          const Address addr = satIdMapper->GetUtUserMacWithNode (*itUt);
          const int32_t utUserId = satIdMapper->GetUtUserIdWithMac (addr);
          const double kbps = packetSize / intervalSeconds / 125.0;
          std::cout << "UT User " << utUserId
                    << " offers bandwidth of " << kbps << " kbps" << std::endl;
        }
      rtnApp->SetAttribute ("Interval", TimeValue (Seconds (intervalSeconds)));
      rtnApp->SetStartTime (appStartTime);
      (*itUt)->AddApplication (rtnApp);
    }

  // setup packet sinks at all users
  Ptr<PacketSink> ps = CreateObject<PacketSink> ();
  ps->SetAttribute ("Protocol", StringValue (protocol));
  ps->SetAttribute ("Local", AddressValue (gwAddr));
  gwUsers.Get (0)->AddApplication (ps);

  /**
   * Set-up statistics
   */

  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerUtRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);

  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtRtnDevDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtRtnDevDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamFrameLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  NS_LOG_INFO("--- sat-dama-sim-tn9 ---");
  NS_LOG_INFO("  Packet size: " << packetSize);
  NS_LOG_INFO("  Simulation length: " << simLength);
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
  Simulator::Stop (Seconds (simLength));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
