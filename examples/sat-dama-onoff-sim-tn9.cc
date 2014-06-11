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
* execute command -> ./waf --run "sat-dama-onoff-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-dama-onoff-sim-tn9");

int
main (int argc, char *argv[])
{
  // Spot-beam over Finland
  uint32_t beamId = 18;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (220); // 70% system load according to CRA 1 kbps
  uint32_t packetSize (1280); // in bytes
  double simLength (300.0); // in seconds
  Time appStartTime = Seconds (0.1);

  DataRate dataRate (32000); // in bps
  uint32_t damaConf (0);
  uint32_t crTxConf (0);

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
   *   - 70% system load
   *
   * Frame configuration (configured in tn9-dama-input-attributes.xml):
   *   - 4 frames (13.75 MHz user bandwidth)
   *     - 8 x 0.3125 MHz -> 2.5 MHz
   *     - 8 x 0.625 MHz  -> 5 MHz
   *     - 4 x 1.25 MHz   -> 5 MHz
   *     - 1 x 1.25 MHz   -> 1.25 MHz
   *
   * DAMA configuration mode (selected from command line argument):
   *   - RBDC
   *   - VBDC
   *
   * NCC configuration mode:
   *   - Conf-2 scheduling mode (dynamic time slots)
   *   - FCA disabled
   *
   * CR transmission modes (selected from command line argument):
   *   - RA slotted ALOHA
   *   - CDRSA (strict RC 0)
   *   - Periodical control slots
   *   - CRA 1kbps
   *
   * Fading configuration:
   *   - Rain
   *   - Markov fading disabled
   *
   * RTN link
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
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("damaConf", "DAMA configuration", damaConf);
  cmd.AddValue ("crTxConf", "CR transmission configuration", crTxConf);
  cmd.Parse (argc, argv);

  // NCC configuration
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", StringValue ("Config type 2"));
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));

  // Rain fading
  Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName",
                      StringValue ("Beam1_UT_fading_fwddwn_traces.txt"));
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName",
                      StringValue ("Beam1_UT_fading_rtnup_traces.txt"));

  // Disable Markov fading
  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (SatEnums::FADING_OFF));

  switch (damaConf)
  {
    // RBDC
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate", UintegerValue (16));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
        break;
      }
    // VBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported damaConf: " << damaConf);
        break;
      }
  }

  switch (crTxConf)
  {
    // RA slotted ALOHA
    case 0:
      {
        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    // CRDSA (strict RC 0)
    case 1:
      {
        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_CRDSA));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        Config::SetDefault ("ns3::SatUtMac::UseCrdsaOnlyForControlPackets", BooleanValue (true));
        break;
      }
    // Periodical control slots
    case 2:
      {
        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
        break;
      }
    // CRA 1 kbps
    case 3:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));

        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported crTxConf: " << crTxConf);
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
      appStartTime += MilliSeconds (50);

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
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MaxValue", DoubleValue (30.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::BinLength", DoubleValue (0.3));
  Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (3.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.03));
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

  s->AddPerBeamCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  NS_LOG_INFO("--- sat-dama-onoff-sim-tn9 ---");
  NS_LOG_INFO("  Packet size: " << packetSize);
  NS_LOG_INFO("  Offered data rate: " << dataRate);
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  std::stringstream filename;
  filename << "tn9-dama-onoff-output-attributes-ut" << utsPerBeam
           << "-conf" << crTxConf << ".xml";

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

