/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store.h"


using namespace ns3;

/**
* \ingroup satellite
*
* \brief Simulation script to run example simulation results related to
* satellite system tests.
*
* To get help of the command line arguments for the example,
* execute command -> ./waf --run "sat-dama-onoff-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-rtn-sys-test");

int
main (int argc, char *argv[])
{
  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt = 10;
  uint32_t utsPerBeam = 10;
  Time utAppStartTime = Seconds (0.1);

  UintegerValue packetSize (1280); // in bytes
  TimeValue interval (MilliSeconds (50));
  DataRateValue dataRate (16000);

  double simLength (30.0); // in seconds
  std::string preDefinedFrameConfig = "Configuration_0";
  uint32_t testCase = 0;
  uint32_t trafficModel = 0;

  // set default values for traffic model apps here
  // attributes can be overridden by command line arguments or input xml when needed
  Config::SetDefault ("ns3::CbrApplication::PacketSize", packetSize);
  Config::SetDefault ("ns3::CbrApplication::Interval", interval);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", packetSize);
  Config::SetDefault ("ns3::OnOffApplication::DataRate", dataRate);
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));

  // To read input attributes from input xml-file
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("src/satellite/examples") + "/sys-rtn-test.xml";

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputFrameConfig;
  inputFrameConfig.ConfigureDefaults ();

  /**
   * Attributes:
   * -----------
   *
   * Scenario:
   *   - 1 beam (beam id = 18)
   *
   * Frame configurations (configured in sys-test-frame-confs.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * FWD link
   *   - ACM disabled / enabled
   *   - Markov enabled
   *   - External fading input trace enabled
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("testCase", "Test case to execute", testCase);
  cmd.AddValue ("frameConf", "Pre-defined super frame configuration", preDefinedFrameConfig);
  cmd.AddValue ("trafficModel", "Traffic model to use (0 = CBR, 1 = OnOff)", trafficModel);
  cmd.AddValue ("simLength", "Simulation length", simLength);
  cmd.Parse (argc, argv);

  // select pre-defined super frame configuration wanted to use.
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", StringValue (preDefinedFrameConfig));

  switch (testCase)
  {
    case 0: // scheduler, CRA
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=5]"));
      Config::SetDefault ("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue (false));
      break;

    case 1: // scheduler, FCA (CRA + VBDC)
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (true));
      Config::SetDefault ("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue (true));
      break;

    case 2: // ACM, one UT with one user, SA enabled, CRDSA disabled, MARKOV fading on
      break;

    case 3: // RM, one UT with one user, CRA only
      break;

    case 4: // RM, one UT with one user, RBDC only
      break;

    case 5: // RM, one UT with one user, VBDC only
      break;

    default:
      break;
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

  helper->CreateUserDefinedScenario (beamMap);

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  /**
   * Set-up CBR or OnOff traffic
   */

  // Create and install sink application to first GW user to received packets from UTs
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));
  sink.Install (gwUsers.Get (0));

  /**
   * Create traffic model to sent packets
   */

  CbrHelper cbr ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));
  SatOnOffHelper onOff ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));

  for ( NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); it++ )
    {
      ApplicationContainer appAdded;

      switch (trafficModel)
        {
          case 0: // CBR
            appAdded = cbr.Install (*it);
            DynamicCast<CbrApplication> (appAdded.Get (0))->GetAttribute ("PacketSize", packetSize );
            DynamicCast<CbrApplication> (appAdded.Get (0))->GetAttribute ("Interval", interval );
            break;

          case 1: // On-Off
            appAdded = onOff.Install (*it);
            DynamicCast<OnOffApplication> (appAdded.Get (0))->GetAttribute ("PacketSize", packetSize );
            DynamicCast<OnOffApplication> (appAdded.Get (0))->GetAttribute ("DataRate", dataRate );
            break;

          default:
            NS_FATAL_ERROR ("Not Supported Traffic Model!");
            break;
        }

      appAdded.Get (0)->SetStartTime (utAppStartTime);
      utAppStartTime += MilliSeconds (500);
    }

  /**
   * Set-up statistics
   */
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MaxValue", DoubleValue (400.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::BinLength", DoubleValue (4.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (6.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.05));
  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // Enable some logs.
  LogComponentEnable ("sat-rtn-sys-test", LOG_INFO);

  // enable application info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);


  NS_LOG_INFO("--- sat-rtn-sys-test ---");
  NS_LOG_INFO("  Test case: " << testCase);
  NS_LOG_INFO("  Traffic model: " << trafficModel);
  NS_LOG_INFO("  Packet size: " << packetSize.Get ());
  NS_LOG_INFO("  Data rate (on-off): " << dataRate.Get ());
  NS_LOG_INFO("  Interval (CBR): " << interval.Get ().GetSeconds ());
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-rtn-sys-test.xml"));
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

