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
* execute command -> ./waf --run "sat-dama-verification-sim --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-dama-verification-sim");

int
main (int argc, char *argv[])
{
  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (220);

  // CBR
  uint32_t packetSize (1280); // in bytes
  double intervalSeconds = 0.01;

  double simLength (50.0); // in seconds
  Time appStartTime = Seconds (0.1);

  // To read attributes from file
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("src/satellite/examples") + "/tn9-dama-input-attributes.xml";
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("packetSize", "Packet size in bytes", packetSize);
  cmd.Parse (argc, argv);

  // 10 ms -> 100 packets per second
  // 125 B -> 100 kbps
  // 250 B -> 200 kbps
  // 500 B -> 400 kbps
  // 750 B -> 600 kbps
  // 1000 B -> 800 kbps
  // 1250 B -> 1000 kbps

  // RBDC + periodical control slots
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (10));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue (Seconds (1)));

  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue("Config type 2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue(true));

  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_MARKOV));

  /**
   * Carriers:
   * 80 x 312.5 kHz
   * 80 x 625 kHz
   * 40 x 1.25 MHz
   */
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameCount", UintegerValue (3));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_AllocatedBandwidthHz", DoubleValue (2.5e+07));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_AllocatedBandwidthHz", DoubleValue (5e+07));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame2_AllocatedBandwidthHz", DoubleValue (5e+07));

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
      appStartTime += MilliSeconds (50);

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
  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);

  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddGlobalRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);


  NS_LOG_INFO("--- sat-dama-sim-tn9 ---");
  NS_LOG_INFO("  Packet size: " << packetSize);
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  std::stringstream filename;
  filename << "tn9-dama-output-attributes-ut" << utsPerBeam
           << "-packetSize" << packetSize
           << ".xml";

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (filename.str ()));
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

