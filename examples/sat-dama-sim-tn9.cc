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
* satellite RTN link performance.
*
* execute command -> ./waf --run "sat-dama-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-dama-sim-tn9");

int
main (int argc, char *argv[])
{
  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (1); /// \todo Make it equivalent with 80% system load according to NCC-2
  uint32_t nccConf (0);
  uint32_t fadingConf (0);

  // 16 kbps per end user
  uint32_t packetSize (1280); // in bytes
  double intervalSeconds = 0.64;

  double simLength (300.0); // in seconds
  Time appStartTime = Seconds (0.1);

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
   * Scenario:
   *   - 1 beam (beam id = 18)
   *   - 80% system load
   *
   * Frame configuration (configured in tn9-dama-input-attributes.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * NCC configuration modes (selected from command line argument):
   *   - Conf-0 (static timeslots with ACM off)
   *   - Conf-1 (static timeslots with ACM on)
   *   - Conf-2 scheduling mode (dynamic time slots)
   *   - FCA disabled
   *
   * Fading configuration (selected from command line argument):
   *   - Markov
   *   - Rain
   *
   * RTN link
   *   - Constant interference
   *   - AVI error model
   *   - ARQ disabled
   *   - RBDC with periodical control slots
   * FWD link
   *   - ACM disabled
   *   - Constant interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("nccConf", "NCC configuration", nccConf);
  cmd.AddValue ("fadingConf", "Fading configuration (0: Markov, 1: Rain)", nccConf);
  cmd.Parse (argc, argv);

  // RBDC + periodical control slots
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (64));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));

  switch (nccConf)
  {
    case 0:
      {
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue("Config type 0"));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue(false));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (25.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.1));
        break;
      }
    case 1:
      {
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue("Config type 1"));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue(true));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (6.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.05));
        break;
      }
    case 2:
      {
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue("Config type 2"));
        Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue(true));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (6.0));
        Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.05));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported nccConf: " << nccConf);
        break;
      }
  }

  switch (fadingConf)
  {
    case 0:
      {
        // Markov fading
        Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_MARKOV));
        break;
      }
    case 1:
      {
        // Rain fading
        Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));
        Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
        Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName",
                            StringValue ("Beam1_UT_fading_fwddwn_traces.txt"));
        Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName",
                            StringValue ("Beam1_UT_fading_rtnup_traces.txt"));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported fadingConf: " << fadingConf);
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

  helper->CreateUserDefinedScenario (beamMap);

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

      // return link
      Ptr<CbrApplication> rtnApp = CreateObject<CbrApplication> ();
      rtnApp->SetAttribute ("Protocol", StringValue (protocol));
      rtnApp->SetAttribute ("Remote", AddressValue (gwAddr));
      rtnApp->SetAttribute ("PacketSize", UintegerValue (packetSize));
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
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MaxValue", DoubleValue (400.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::BinLength", DoubleValue (4.0));
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

