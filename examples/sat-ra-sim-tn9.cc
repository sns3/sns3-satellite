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
* and RA-DAMA configuration.
*
* execute command -> ./waf --run "sat-ra-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-ra-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t raMode (0);
  uint32_t utsPerBeam (3);

  double simLength (30.0); // in seconds
  Time appStartTime = Seconds(0.1);

  // CBR parameters
  uint32_t minPacketSizeBytes (20); // -> 3.2 kbps
  uint32_t maxPacketSizeBytes (800); // -> 128 kbps
  Time interval (MilliSeconds(50));

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("./src/satellite/examples/tn9-ra-input-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  Ptr<UniformRandomVariable> randVariable = CreateObject<UniformRandomVariable> ();

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue ("raMode", "RA mode", raMode);
  cmd.AddValue ("simLength", "Simulation duration (in seconds)", simLength);
  cmd.Parse (argc, argv);

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
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
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
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

        Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (1));
        break;
      }
    // CRDSA only
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
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported raMode: " << raMode);
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
      rtnApp->SetAttribute ("Interval", TimeValue (interval));

      // Random static packet size
      uint32_t size = randVariable->GetInteger (minPacketSizeBytes, maxPacketSizeBytes);
      rtnApp->SetAttribute ("PacketSize", UintegerValue (size));

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

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerUtCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerUtSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  LogComponentEnable ("sat-ra-sim-tn9", LOG_INFO);
  NS_LOG_INFO("--- sat-ra-sim-tn9 ---");
  NS_LOG_INFO("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("tn9-ra-output-attributes.xml"));
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
