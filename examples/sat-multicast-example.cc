/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/cbr-helper.h"

using namespace ns3;

/**
* \ingroup satellite
*
* \brief  Multicast example application based on Cbr example to use satellite network.
*         Interval, packet size and test scenario can be given
*         in command line as user argument.
*         To see help for user arguments:
*         execute command -> ./waf --run "sat-multicast-example --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-multicast-example");

int
main (int argc, char *argv[])
{
  /// GWs 1,3
  /// UTs 1,2,3,4
  /// UT users 1,2,3,4,5
  bool enableMulticastGroup_1 = true;

  /// GWs 1,3
  /// UTs 1,2,4
  /// UT users 2,3,5
  bool enableMulticastGroup_2 = true;

  /// Enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-multicast-example", LOG_LEVEL_INFO);

  NS_LOG_INFO ("--- Starting sat-multicast-example ---");

  uint32_t packetSize = 512;
  std::string interval = "1s";
  std::string scenario = "larger";
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::LARGER;

  /// Read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue("scenario", "Test scenario to use. (simple, larger or full", scenario);
  cmd.AddValue("logFile", "File name for scenario creation log", scenarioLogFile);
  cmd.Parse (argc, argv);

  /// Set network types which support multicast
  Config::SetDefault ("ns3::SatUserHelper::BackboneNetworkType",EnumValue (SatUserHelper::NETWORK_TYPE_CSMA));
  Config::SetDefault ("ns3::SatUserHelper::SubscriberNetworkType",EnumValue (SatUserHelper::NETWORK_TYPE_CSMA));

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  /// Remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /// Create satellite helper with given scenario default=simple

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  NS_LOG_INFO ("--- Creating scenario ---");
  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if ( scenarioLogFile != "" )
    {
      helper->EnableCreationTraces (scenarioLogFile, false);
    }

  helper->CreatePredefinedScenario (satScenario);
  helper->EnablePacketTrace ();

  /// Device IDs for different nodes
  uint32_t ipRouterInputDeviceId = 3;
  uint32_t ipRouterOutputDeviceId_1 = 1;
  uint32_t ipRouterOutputDeviceId_2 = 2;

  uint32_t gw_1_InputDeviceId = 3;
  uint32_t gw_1_OutputDeviceId_1 = 1;
  uint32_t gw_1_OutputDeviceId_2 = 2;

  uint32_t gw_3_InputDeviceId = 2;
  uint32_t gw_3_OutputDeviceId_1 = 1;

  uint32_t ut_1_InputDeviceId = 2;
  uint32_t ut_1_OutputDeviceId_1 = 1;

  uint32_t ut_2_InputDeviceId = 2;
  uint32_t ut_2_OutputDeviceId_1 = 1;

  uint32_t ut_3_InputDeviceId = 2;
  uint32_t ut_3_OutputDeviceId_1 = 1;

  uint32_t ut_4_InputDeviceId = 2;
  uint32_t ut_4_OutputDeviceId_1 = 1;

  uint16_t multicastPort = 9;   // Discard port (RFC 863)

  /// Get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  NS_LOG_INFO ("UT users: " << utUsers.GetN () << ", GW users: " << gwUsers.GetN ());

  NS_LOG_INFO ("--- Creating multicast groups ---");

  /// Create multicast groups
  Ipv4Address multicastSource_1 (helper->GetUserAddress (gwUsers.Get (0)));
  Ipv4Address multicastGroup_1 ("225.1.1.1");
  Mac48Address multicastMacGroup_1;

  Ipv4Address multicastSource_2 (helper->GetUserAddress (gwUsers.Get (0)));
  Ipv4Address multicastGroup_2 ("225.1.2.1");
  Mac48Address multicastMacGroup_2;

  /// Check the address sanity
  if (multicastGroup_1.IsMulticast ())
    {
      multicastMacGroup_1 = Mac48Address::GetMulticast (multicastGroup_1);
      NS_LOG_INFO ("IP address for multicast group 1: " << multicastGroup_1 << ", MAC address for multicast group 1: " << multicastMacGroup_1);
    }
  else
    {
      NS_FATAL_ERROR ("Invalid address for multicast group 1");
    }

  if (multicastGroup_2.IsMulticast ())
    {
      multicastMacGroup_2 = Mac48Address::GetMulticast (multicastGroup_2);
      NS_LOG_INFO ("IP address for multicast group 1: " << multicastGroup_2 << ", MAC address for multicast group 1: " << multicastMacGroup_2);
    }
  else
    {
      NS_FATAL_ERROR ("Invalid address for multicast group 2");
    }

  NS_LOG_INFO ("--- Creating multicast routes ---");

  Ipv4StaticRoutingHelper multicast;                               // Routing helper
  Ptr<Node> multicastRouter;                                       // The node in question
  Ptr<NetDevice> inputDevice;                                      // The input NetDevice
  NodeContainer utNodes = helper->GetBeamHelper ()->GetUtNodes (); // UT nodes
  Ptr<SatLlc> llc;                                                 // LLC
  Ptr<SatQueue> queue;                                             // Queue

  if (enableMulticastGroup_1)
    {
      NS_LOG_INFO ("--- Routes for multicast group 1 ---");

      /// Configure a (static) multicast route for group 1 on GW 1
      /// Behind this Router are GWs 1, 2 and UTs 1, 2, 3, 4
      /// Get IP router node
      multicastRouter = helper->GetUserHelper ()->GetRouter ();

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ipRouterInputDeviceId);

      /// A container of IP router output NetDevices
      NetDeviceContainer outputDevicesRouter;

      /// Add output NetDevices
      outputDevicesRouter.Add (multicastRouter->GetDevice (ipRouterOutputDeviceId_1));
      outputDevicesRouter.Add (multicastRouter->GetDevice (ipRouterOutputDeviceId_2));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesRouter);

      NS_LOG_INFO ("--- IP Router ---");
      NS_LOG_INFO ("IP Router num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("IP Router input device: " << ipRouterInputDeviceId);
      NS_LOG_INFO ("IP Router output devices: " << ipRouterOutputDeviceId_1 << ", " << ipRouterOutputDeviceId_2);

      /// Configure a (static) multicast route for group 1 on GW 1
      /// Behind this GW are UTs 1, 2, 3
      /// Get GW 1 node
      multicastRouter = helper->GetBeamHelper ()->GetGwNode (1);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (gw_1_InputDeviceId);

      /// A container of GW 1 output NetDevices
      NetDeviceContainer outputDevicesGW1;

      /// Add output NetDevices
      outputDevicesGW1.Add (multicastRouter->GetDevice (gw_1_OutputDeviceId_1));
      outputDevicesGW1.Add (multicastRouter->GetDevice (gw_1_OutputDeviceId_2));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesGW1);

      NS_LOG_INFO ("--- GW 1 ---");
      NS_LOG_INFO ("GW 1 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("GW 1 input device: " << gw_1_InputDeviceId);
      NS_LOG_INFO ("GW 1 output devices: " << gw_1_OutputDeviceId_1 << ", " << gw_1_OutputDeviceId_2);

      /// Configure a (static) multicast route for group 1 on GW 3
      /// Behind this GW is UT 4
      /// Get GW 3 node
      multicastRouter = helper->GetBeamHelper ()->GetGwNode (3);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (gw_3_InputDeviceId);

      /// A container of GW 3 output NetDevices
      NetDeviceContainer outputDevicesGW3;

      /// Add output NetDevices
      outputDevicesGW3.Add (multicastRouter->GetDevice (gw_3_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesGW3);

      NS_LOG_INFO ("--- GW 3 ---");
      NS_LOG_INFO ("GW 3 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("GW 3 input device: " << gw_3_InputDeviceId);
      NS_LOG_INFO ("GW 3 output devices: " << gw_3_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 1 on UT 1
      /// Get UT 1 node
      multicastRouter = utNodes.Get (0);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_1_InputDeviceId);

      /// A container of UT 1 output NetDevices
      NetDeviceContainer outputDevicesUT1;

      /// Add output NetDevices
      outputDevicesUT1.Add (multicastRouter->GetDevice (ut_1_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesUT1);

      NS_LOG_INFO ("--- UT 1 ---");
      NS_LOG_INFO ("UT 1 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 1 input device: " << ut_1_InputDeviceId);
      NS_LOG_INFO ("UT 1 output devices: " << ut_1_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 1 on UT 2
      /// Get UT 2 node
      multicastRouter = utNodes.Get (1);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_2_InputDeviceId);

      /// A container of UT 2 output NetDevices
      NetDeviceContainer outputDevicesUT2;

      /// Add output NetDevices
      outputDevicesUT2.Add (multicastRouter->GetDevice (ut_2_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesUT2);

      NS_LOG_INFO ("--- UT 2 ---");
      NS_LOG_INFO ("UT 2 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 2 input device: " << ut_2_InputDeviceId);
      NS_LOG_INFO ("UT 2 output devices: " << ut_2_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 1 on UT 3
      /// Get UT 3 node
      multicastRouter = utNodes.Get (2);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_3_InputDeviceId);

      /// A container of UT 3 output NetDevices
      NetDeviceContainer outputDevicesUT3;

      /// Add output NetDevices
      outputDevicesUT3.Add (multicastRouter->GetDevice (ut_3_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesUT3);

      NS_LOG_INFO ("--- UT 3 ---");
      NS_LOG_INFO ("UT 3 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 3 input device: " << ut_3_InputDeviceId);
      NS_LOG_INFO ("UT 3 output devices: " << ut_3_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 1 on UT 4
      /// Get UT 4 node
      multicastRouter = utNodes.Get (3);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_4_InputDeviceId);

      /// A container of UT 4 output NetDevices
      NetDeviceContainer outputDevicesUT4;

      /// Add output NetDevices
      outputDevicesUT4.Add (multicastRouter->GetDevice (ut_4_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_1,
                                   multicastGroup_1,
                                   inputDevice,
                                   outputDevicesUT4);

      NS_LOG_INFO ("--- UT 4 ---");
      NS_LOG_INFO ("UT 4 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 4 input device: " << ut_4_InputDeviceId);
      NS_LOG_INFO ("UT 4 output devices: " << ut_4_OutputDeviceId_1);

      NS_LOG_INFO ("--- Creating traffic generator for multicast group 1 ---");

      /// Traffic generators for multicast group 1
      CbrHelper gwCbrHelper_1 ("ns3::UdpSocketFactory",
                               InetSocketAddress (multicastGroup_1,
                                                  multicastPort));
      gwCbrHelper_1.SetAttribute ("Interval",
                                  StringValue (interval));
      gwCbrHelper_1.SetAttribute ("PacketSize",
                                  UintegerValue (packetSize));

      ApplicationContainer gwCbr_1 = gwCbrHelper_1.Install (gwUsers.Get (0));
      gwCbr_1.Start (Seconds (2.0));
      gwCbr_1.Stop (Seconds (3.1));
    }

  if (enableMulticastGroup_2)
    {
      NS_LOG_INFO ("--- Routes for multicast group 2 ---");

      /// Configure a (static) multicast route for group 2 on GW 1
      /// Behind this Router are GWs 1, 2 and UTs 1, 2, 4
      /// Get IP router node
      multicastRouter = helper->GetUserHelper ()->GetRouter ();

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ipRouterInputDeviceId);

      /// A container of IP router output NetDevices
      NetDeviceContainer outputDevicesRouter;

      /// Add output NetDevices
      outputDevicesRouter.Add (multicastRouter->GetDevice (ipRouterOutputDeviceId_1));
      outputDevicesRouter.Add (multicastRouter->GetDevice (ipRouterOutputDeviceId_2));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_2,
                                   multicastGroup_2,
                                   inputDevice,
                                   outputDevicesRouter);

      NS_LOG_INFO ("--- IP Router ---");
      NS_LOG_INFO ("IP Router num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("IP Router input device: " << ipRouterInputDeviceId);
      NS_LOG_INFO ("IP Router output devices: " << ipRouterOutputDeviceId_1 << ", " << ipRouterOutputDeviceId_2);

      /// Configure a (static) multicast route for group 2 on GW 1
      /// Behind this GW are UTs 1, 2
      /// Get GW 1 node
      multicastRouter = helper->GetBeamHelper ()->GetGwNode (1);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (gw_1_InputDeviceId);

      /// A container of GW 1 output NetDevices
      NetDeviceContainer outputDevicesGW1;

      /// Add output NetDevices
      outputDevicesGW1.Add (multicastRouter->GetDevice (gw_1_OutputDeviceId_1));
      outputDevicesGW1.Add (multicastRouter->GetDevice (gw_1_OutputDeviceId_2));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_2,
                                   multicastGroup_2,
                                   inputDevice,
                                   outputDevicesGW1);

      NS_LOG_INFO ("--- GW 1 ---");
      NS_LOG_INFO ("GW 1 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("GW 1 input device: " << gw_1_InputDeviceId);
      NS_LOG_INFO ("GW 1 output devices: " << gw_1_OutputDeviceId_1 << ", " << gw_1_OutputDeviceId_2);

      /// Configure a (static) multicast route for group 1 on GW 3
      /// Behind this GW is UT 4
      /// Get GW 3 node
      multicastRouter = helper->GetBeamHelper ()->GetGwNode (3);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (gw_3_InputDeviceId);

      /// A container of GW 3 output NetDevices
      NetDeviceContainer outputDevicesGW3;

      /// Add output NetDevices
      outputDevicesGW3.Add (multicastRouter->GetDevice (gw_3_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_2,
                                   multicastGroup_2,
                                   inputDevice,
                                   outputDevicesGW3);

      NS_LOG_INFO ("--- GW 3 ---");
      NS_LOG_INFO ("GW 3 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("GW 3 input device: " << gw_3_InputDeviceId);
      NS_LOG_INFO ("GW 3 output devices: " << gw_3_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 2 on UT 1
      /// Get UT 1 node
      multicastRouter = utNodes.Get (0);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_1_InputDeviceId);

      /// A container of UT 1 output NetDevices
      NetDeviceContainer outputDevicesUT1;

      /// Add output NetDevices
      outputDevicesUT1.Add (multicastRouter->GetDevice (ut_1_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_2,
                                   multicastGroup_2,
                                   inputDevice,
                                   outputDevicesUT1);

      NS_LOG_INFO ("--- UT 1 ---");
      NS_LOG_INFO ("UT 1 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 1 input device: " << ut_1_InputDeviceId);
      NS_LOG_INFO ("UT 1 output devices: " << ut_1_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 2 on UT 2
      /// Get UT 2 node
      multicastRouter = utNodes.Get (1);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_2_InputDeviceId);

      /// A container of UT 2 output NetDevices
      NetDeviceContainer outputDevicesUT2;

      /// Add output NetDevices
      outputDevicesUT2.Add (multicastRouter->GetDevice (ut_2_OutputDeviceId_1));

      /// Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_2,
                                   multicastGroup_2,
                                   inputDevice,
                                   outputDevicesUT2);

      NS_LOG_INFO ("--- UT 2 ---");
      NS_LOG_INFO ("UT 2 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 2 input device: " << ut_2_InputDeviceId);
      NS_LOG_INFO ("UT 2 output devices: " << ut_2_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 1 on UT 3
      /// Get UT 3 node
      multicastRouter = utNodes.Get (2);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_3_InputDeviceId);

      //NS_LOG_INFO ("--- UT 3 ---");
      //NS_LOG_INFO ("UT 3 num of devices: " << multicastRouter->GetNDevices ());
      //NS_LOG_INFO ("UT 3 input device: " << ut_3_InputDeviceId);
      //NS_LOG_INFO ("UT 3 output devices: " << ut_3_OutputDeviceId_1);

      /// Configure a (static) multicast route for group 2 on UT 4
      /// Get UT 4 node
      multicastRouter = utNodes.Get (3);

      /// Get input NetDevice
      inputDevice = multicastRouter->GetDevice (ut_4_InputDeviceId);

      /// A container of UT 4 output NetDevices
      NetDeviceContainer outputDevicesUT4;

      /// Add output NetDevices
      outputDevicesUT4.Add (multicastRouter->GetDevice (ut_4_OutputDeviceId_1));

      // Add multicast route
      multicast.AddMulticastRoute (multicastRouter,
                                   multicastSource_2,
                                   multicastGroup_2,
                                   inputDevice,
                                   outputDevicesUT4);

      NS_LOG_INFO ("--- UT 4 ---");
      NS_LOG_INFO ("UT 4 num of devices: " << multicastRouter->GetNDevices ());
      NS_LOG_INFO ("UT 4 input device: " << ut_4_InputDeviceId);
      NS_LOG_INFO ("UT 4 output devices: " << ut_4_OutputDeviceId_1);

      NS_LOG_INFO ("--- Create traffic generator for multicast group 2 ---");

      /// Traffic generators for multicast group 1
      CbrHelper gwCbrHelper_2 ("ns3::UdpSocketFactory",
                               InetSocketAddress (multicastGroup_2,
                                                  multicastPort));
      gwCbrHelper_2.SetAttribute ("Interval",
                                  StringValue (interval));
      gwCbrHelper_2.SetAttribute ("PacketSize",
                                  UintegerValue (packetSize));

      ApplicationContainer gwCbr_2 = gwCbrHelper_2.Install (gwUsers.Get (0));
      gwCbr_2.Start (Seconds (5.0));
      gwCbr_2.Stop (Seconds (6.1));
    }

  NS_LOG_INFO ("--- Creating UT sinks ---");

  /// Create sink on UT user for receiving the traffic
  ApplicationContainer utSink;

  /// Sinks for group 1, all UT users will receive the traffic
  for (uint32_t i = 0; i < utUsers.GetN (); i++)
    {
      PacketSinkHelper utSinkHelperGroup_1 ("ns3::UdpSocketFactory", InetSocketAddress (multicastGroup_1, multicastPort));
      utSink = utSinkHelperGroup_1.Install (utUsers.Get (i));
    }

  /// Sinks for group 2, one non-multicast user will not have a sink.
  /// Excluding the user needs to be done on sink level (no application
  /// to receive the transmission) as it is sharing the same link from the UT with a multicast user.
  for (uint32_t i = 1; i < utUsers.GetN (); i++)
    {
      PacketSinkHelper utSinkHelperGroup_2 ("ns3::UdpSocketFactory", InetSocketAddress (multicastGroup_2, multicastPort));
      utSink = utSinkHelperGroup_2.Install (utUsers.Get (i));
    }

  utSink.Start (Seconds (1.0));
  utSink.Stop (Seconds (10.0));

  NS_LOG_INFO ("--- Traffic information ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  PacketSize: " << packetSize);
  NS_LOG_INFO ("  Interval: " << interval);
  NS_LOG_INFO ("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO ("  ");

  NS_LOG_INFO ("--- Running simulation ---");

  Simulator::Stop (Seconds(11));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_INFO ("--- Finished ---");
  return 0;
}
