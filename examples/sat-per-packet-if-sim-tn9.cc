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
* \brief Simulation script to run example simulation results related to per-packet
* interference performance.
*
* execute command -> ./waf --run "sat-per-packet-if-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-per-packet-if-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (30);
  DataRate dataRate (32000); // in bps
  uint32_t damaConf (0);
  uint32_t beamConf (0);

  uint32_t packetSize (1280); // in bytes
  double simLength (300.0); // in seconds
  Time appStartTime = Seconds (0.1);

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("./src/satellite/examples/tn9-dama-input-attributes.xml"));
  /// \todo Confirm if we can use the same configuration file.
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  /**
   * Attributes:
   * -----------
   *
   * Scenario (selected from command line argument):
   *   - 1 beam (beam id = 18)
   *   - 18 co-channel beams (user link frequency ID = 1)
   *
   * Frame configuration (configured in tn9-dama-input-attributes.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * NCC configuration modes
   * - Conf-0 (static timeslots with ACM off)
   * - Conf-1 (static timeslots with ACM on)
   * - Conf-2 scheduling mode (dynamic time slots)
   * - FCA disabled
   *
   * RTN link
   *   - Per-packet interference
   *   - AVI error model
   *   - ARQ disabled
   * FWD link
   *   - ACM disabled
   *   - Per-packet interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("beamConf", "Beam configuration", beamConf);
  cmd.AddValue ("damaConf", "DAMA configuration", damaConf);
  cmd.Parse (argc, argv);

  // use 5 seconds store time for control messages
  Config::SetDefault ("ns3::SatBeamHelper::CtrlMsgStoreTimeInRtnLink", TimeValue (Seconds (5)));

  // NCC configuration
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue("Config type 2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue(true));

  switch (damaConf)
  {
    // RBDC
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue(64));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue(false));
        break;
      }
    // VBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue(true));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported damaConf: " << damaConf);
        break;
      }
  }

  Config::SetDefault ("ns3::SatBeamHelper::CtrlMsgStoreTimeInRtnLink", TimeValue (MilliSeconds (350)));

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  switch (beamConf)
  {
    // Single beam
    case 0:
      {
        // Spot-beam over Finland
        uint32_t beamId = 18;

        // create user defined scenario
        std::map<uint32_t, SatBeamUserInfo > beamMap;
        SatBeamUserInfo beamInfo = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[beamId] = beamInfo;
        helper->CreateUserDefinedScenario (beamMap);
        break;
      }
    // All co-channel beams (e.g., color 1 with 72/4 = 18 beams)
    case 1:
      {
        // create user defined scenario
        std::map<uint32_t, SatBeamUserInfo > beamMap;
        // choose beams from user link frequency ID = 1 (TN3 appendix A)
        beamMap[1] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[3] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[5] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[7] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[9] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[22] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[24] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[26] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[28] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[30] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[44] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[46] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[48] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[50] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[59] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[61] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[70] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        beamMap[72] = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
        helper->CreateUserDefinedScenario (beamMap);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported beam configuration: " << beamConf);
      }
  }

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)
  const std::string protocol = "ns3::UdpSocketFactory";

  /**
   * Set-up CBR traffic
   */
  const InetSocketAddress gwAddr = InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port);

  for (NodeContainer::Iterator itUt = utUsers.Begin ();
      itUt != utUsers.End ();
      ++itUt)
    {
      appStartTime += MilliSeconds (10);

      /// \todo Do we need forward link traffic?

      // return link
      Ptr<SatOnOffApplication> rtnApp = CreateObject<SatOnOffApplication> ();
      rtnApp->SetAttribute ("Protocol", StringValue (protocol));
      rtnApp->SetAttribute ("Remote", AddressValue (gwAddr));
      rtnApp->SetAttribute ("PacketSize", UintegerValue (packetSize));
      rtnApp->SetAttribute ("DataRate", DataRateValue (dataRate));
      rtnApp->SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
      rtnApp->SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
      rtnApp->SetStartTime (appStartTime);
      (*itUt)->AddApplication (rtnApp);
      /// \todo Verify if the above configuration is fine.
    }

  // setup packet sinks at all users
  Ptr<PacketSink> ps = CreateObject<PacketSink> ();
  ps->SetAttribute ("Protocol", StringValue (protocol));
  ps->SetAttribute ("Local", AddressValue (gwAddr));
  gwUsers.Get (0)->AddApplication (ps);

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

  s->AddPerUtRtnSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtRtnSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalRtnSinr (SatStatsHelper::OUTPUT_CDF_FILE);

  s->AddPerUtFwdSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtFwdSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamFwdSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamFwdSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalFwdSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalFwdSinr (SatStatsHelper::OUTPUT_CDF_FILE);

  NS_LOG_INFO("--- sat-per-packet-if-sim-tn9 ---");
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

