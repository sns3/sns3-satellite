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
* and RA-DAMA configuration. The script supports three different setups: SA + VBDC,
* CRDSA + VBDC and CRDSA only. As output, the example provides statistics about RA
* collision and error rate, throughput, packet delay, SINR, resources granted, frame
* load and waveform usage. The random access results for TN9 were obtained by using
* this script.
*
* execute command -> ./waf --run "sat-ra-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-ra-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t raMode (3);
  uint32_t utsPerBeam (1);
  uint32_t packetSize (64);
  std::string dataRate = "5kb/s";
  std::string onTime = "0.2";
  std::string offTime = "0.8";

  double simLength (300.0); // in seconds

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("./src/satellite/examples/tn9-ra-input-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  Config::SetDefault ("ns3::SatBeamHelper::CtrlMsgStoreTimeInRtnLink", TimeValue (MilliSeconds (350)));

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("raMode", "RA mode", raMode);
  cmd.AddValue ("simLength", "Simulation duration in seconds", simLength);
  cmd.AddValue ("packetSize", "Constant packet size in bytes", packetSize);
  cmd.AddValue ("dataRate", "Data rate (e.g. 500kb/s)", dataRate);
  cmd.AddValue ("onTime", "Time for packet sending is on in seconds", onTime);
  cmd.AddValue ("offTime", "Time for packet sending is off in seconds", offTime);
  cmd.Parse (argc, argv);

  // use 5 seconds store time for control messages
  Config::SetDefault ("ns3::SatBeamHelper::CtrlMsgStoreTimeInRtnLink", TimeValue (Seconds (5)));

  // Enable Random Access with all available modules
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel",EnumValue (SatEnums::RA_MODEL_RCS2_SPECIFICATION));

  // Set Random Access interference model
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));

  // Set Random Access collision model
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel",EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

  // Set dynamic load control parameters
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (false));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize", UintegerValue (10));

  // Set random access parameters
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (50));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (1));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.99));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));

  // Set up user given parameters for on/off functionality.
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (dataRate));
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + onTime + "]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=" + offTime + "]"));

  // Set statistical parameters
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::MaxValue", DoubleValue (5.0));
  Config::SetDefault ("ns3::SatStatsThroughputHelper::BinLength", DoubleValue (0.05));

  switch (raMode)
  {
    // CRDSA + VBDC
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));

        // Disable periodic control slots
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    // SA + VBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));

        Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (1));

        // Disable periodic control slots
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    // Periodic control slots + VBDC
    case 2:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));

        Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel",EnumValue (SatEnums::RA_MODEL_OFF));

        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
        break;
      }
    // CRDSA only
    case 3:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

        // Disable periodic control slots
        Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported raMode: " << raMode);
        break;
      }
  }
  // Statistics settings
  Config::SetDefault ("ns3::SatStatsDelayHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::MaxValue", DoubleValue (30.0));
  Config::SetDefault ("ns3::SatStatsDelayHelper::BinLength", DoubleValue (0.05));

  Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MinValue", DoubleValue (0.0));
  Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::MaxValue", DoubleValue (10000.0));
  Config::SetDefault ("ns3::SatStatsResourcesGrantedHelper::BinLength", DoubleValue (100.0));

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
   * Set-up On-Off traffic
   */
  for (uint32_t i = 0; i < utUsers.GetN (); i++)
    {
      SatOnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress (utUsers.Get (i)), port));
      onOffHelper.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

      ApplicationContainer utOnOff = onOffHelper.Install (utUsers.Get (i));
      utOnOff.Start (Seconds (0.0));
      utOnOff.Stop (Seconds (simLength - 2.0));

      PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress (utUsers.Get (i)), port));
      sinkHelper.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

      ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
      gwSink.Start (Seconds (0.0));
      gwSink.Stop (Seconds (simLength));
    }

  /**
   * Set-up statistics
   */
  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacDelay (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnDevDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnPhyDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnMacDelay (SatStatsHelper::OUTPUT_CDF_FILE);

  //s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  //s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  //s->AddPerUtRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  //s->AddPerUtRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);

  //s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  //s->AddPerUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  //s->AddPerUtRtnDevDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  //s->AddPerUtRtnDevDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  //s->AddPerUtCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  //s->AddPerUtCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  //s->AddPerUtSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  //s->AddPerUtSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  NS_LOG_INFO("--- sat-ra-sim-tn9 ---");
  NS_LOG_INFO("  Packet size: " << packetSize);
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  std::stringstream filename;
  filename << "tn9-ra-output-attributes-ut" << utsPerBeam
           << "-mode" << raMode << ".xml";

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
