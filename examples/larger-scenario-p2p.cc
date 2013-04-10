/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Oleksandr Puchko <oleksandr.puchko@magister.fi>
 */


#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Larger-scenario");

int 
main (int argc, char *argv[])
{
  LogComponentEnable ("Larger-scenario", LOG_LEVEL_INFO);


  // Set up some default values for the simulation.  Use the 
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  // Allow the user to override any of the defaults and the above

  CommandLine cmd;
  bool enableFlowMonitor = false;
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);

  // Here, we will explicitly create all our nodes.
  
  NS_LOG_INFO ("Create nodes.");
  // Create Nodes
  NodeContainer Nodes;
  Nodes.Create (6);
  // Create UTs
  // If UT is created as NodeContainer, segmentation fault appears in during simulation
  // Maybe it is bug, however it should be investigated in the future

  Ptr<Node> UT1 = CreateObject<Node> ();
  Ptr<Node> UT2 = CreateObject<Node> ();
  Ptr<Node> UT3 = CreateObject<Node> ();
  Ptr<Node> UT4 = CreateObject<Node> ();

  // Create SB or Satellire beams
  Ptr<Node> SB1 = CreateObject<Node> ();
  Ptr<Node> SB2 = CreateObject<Node> ();
  Ptr<Node> SB3 = CreateObject<Node> ();

  // Create GW 
  Ptr<Node> GW1 = CreateObject<Node> ();
  Ptr<Node> GW2 = CreateObject<Node> ();

  // Create IProuter
  Ptr<Node> IPr = CreateObject<Node> ();

  // add all nodes to NodeContainer, for later usage with InternetStackHelper
  // at most 5 nodes can be added to the one NodeContainer
  NodeContainer all1 = NodeContainer (SB1, SB2, SB3, GW1, GW2);
  NodeContainer all2 = NodeContainer (UT1, UT2, UT3, UT4, IPr);

  NodeContainer N1UT1 = NodeContainer (Nodes.Get (1), UT1);
  NodeContainer N2UT1 = NodeContainer (Nodes.Get (2), UT1);
  NodeContainer N3UT2 = NodeContainer (Nodes.Get (3), UT2);
  NodeContainer N4UT3 = NodeContainer (Nodes.Get (4), UT3);
  NodeContainer N5UT4 = NodeContainer (Nodes.Get (5), UT4);
  NodeContainer UT1Sat1 = NodeContainer (UT1, SB1);
  NodeContainer UT2Sat1 = NodeContainer (UT2, SB1);
  NodeContainer UT3Sat2 = NodeContainer (UT3, SB2);
  NodeContainer UT4Sat3 = NodeContainer (UT4, SB3);
  NodeContainer Sat1GW1 = NodeContainer (SB1, GW1);
  NodeContainer Sat2GW1 = NodeContainer (SB2, GW1);
  NodeContainer Sat3GW2 = NodeContainer (SB3, GW2);
  NodeContainer GW1IPr = NodeContainer (GW1, IPr);
  NodeContainer GW2IPr = NodeContainer (GW2, IPr);
  NodeContainer IPrN0 = NodeContainer (IPr, Nodes.Get (0));

// Create position for NetAnim
  AnimationInterface::SetConstantPosition (Nodes.Get (0), 100, 70); 
  AnimationInterface::SetConstantPosition (Nodes.Get (1), 10, 90); 
  AnimationInterface::SetConstantPosition (Nodes.Get (2), 10, 70); 
  AnimationInterface::SetConstantPosition (Nodes.Get (3), 10, 50); 
  AnimationInterface::SetConstantPosition (Nodes.Get (4), 10, 30); 
  AnimationInterface::SetConstantPosition (Nodes.Get (5), 10, 10); 
  AnimationInterface::SetConstantPosition (UT1, 30, 80); 
  AnimationInterface::SetConstantPosition (UT2, 30, 50); 
  AnimationInterface::SetConstantPosition (UT3, 30, 30); 
  AnimationInterface::SetConstantPosition (UT4, 30, 10);
  AnimationInterface::SetConstantPosition (SB1, 50, 55); 
  AnimationInterface::SetConstantPosition (SB2, 50, 45); 
  AnimationInterface::SetConstantPosition (SB3, 50, 20); 
  AnimationInterface::SetConstantPosition (GW1, 70, 75); 
  AnimationInterface::SetConstantPosition (GW2, 70, 10); 
  AnimationInterface::SetConstantPosition (IPr, 100, 40); 



  InternetStackHelper internet;
  internet.Install (all1);
  internet.Install (Nodes);
  internet.Install (all2);

  // Create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");

  // CSMA connection
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // Node to UT
  NetDeviceContainer d1 = csma.Install (N1UT1);
  NetDeviceContainer d2 = csma.Install (N2UT1);
  NetDeviceContainer d3 = csma.Install (N3UT2);
  NetDeviceContainer d4 = csma.Install (N4UT3);
  NetDeviceContainer d5 = csma.Install (N5UT4);

  // GW to IProuter
  NetDeviceContainer d13 = csma.Install (GW1IPr);
  NetDeviceContainer d14 = csma.Install (GW2IPr);

  // IProuter to Node0
  NetDeviceContainer d15 = csma.Install (IPrN0);

  // Point to point connection
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));

  // UT to Satellite beams
  NetDeviceContainer d6 = p2p.Install (UT1Sat1);
  NetDeviceContainer d7 = p2p.Install (UT2Sat1);
  NetDeviceContainer d8 = p2p.Install (UT3Sat2);
  NetDeviceContainer d9 = p2p.Install (UT4Sat3);

  // Satellite beams to GW
  NetDeviceContainer d10 = p2p.Install (Sat1GW1);
  NetDeviceContainer d11 = p2p.Install (Sat2GW1);
  NetDeviceContainer d12 = p2p.Install (Sat3GW2);

  // Add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (d1);
  ipv4.SetBase ("10.2.2.0", "255.255.255.0");
  ipv4.Assign (d2);
  ipv4.SetBase ("10.3.3.0", "255.255.255.0");
  ipv4.Assign (d3);
  ipv4.SetBase ("10.4.4.0", "255.255.255.0");
  ipv4.Assign (d4);
  ipv4.SetBase ("10.5.5.0", "255.255.255.0");
  ipv4.Assign (d5);
  ipv4.SetBase ("10.6.6.0", "255.255.255.0");
  ipv4.Assign (d6);
  ipv4.SetBase ("10.7.7.0", "255.255.255.0");
  ipv4.Assign (d7);
  ipv4.SetBase ("10.8.8.0", "255.255.255.0");
  ipv4.Assign (d8);
  ipv4.SetBase ("10.9.9.0", "255.255.255.0");
  ipv4.Assign (d9);
  ipv4.SetBase ("10.10.10.0", "255.255.255.0");
  ipv4.Assign (d10);
  ipv4.SetBase ("10.11.11.0", "255.255.255.0");
  ipv4.Assign (d11);
  ipv4.SetBase ("10.12.12.0", "255.255.255.0");
  ipv4.Assign (d12);
  ipv4.SetBase ("10.13.13.0", "255.255.255.0");
  ipv4.Assign (d13);
  ipv4.SetBase ("10.14.14.0", "255.255.255.0");
  ipv4.Assign (d14);
  ipv4.SetBase ("10.15.15.0", "255.255.255.0");
  Ipv4InterfaceContainer id15 = ipv4.Assign (d15);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (id15.GetAddress (1), port)));
  onoff.SetConstantRate (DataRate ("448kb/s"));
  ApplicationContainer apps = onoff.Install (Nodes.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (2.0));

ApplicationContainer apps2 = onoff.Install (Nodes.Get (2));
  apps2.Start (Seconds (2.0));
  apps2.Stop (Seconds (3.0));

ApplicationContainer apps3 = onoff.Install (Nodes.Get (3));
  apps3.Start (Seconds (3.0));
  apps3.Stop (Seconds (4.0));

ApplicationContainer apps4 = onoff.Install (Nodes.Get (4));
  apps4.Start (Seconds (4.0));
  apps4.Stop (Seconds (5.0));

ApplicationContainer apps5 = onoff.Install (Nodes.Get (5));
  apps5.Start (Seconds (5.0));
  apps5.Stop (Seconds (6.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));

  apps = sink.Install (Nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (2.0));

  apps2 = sink.Install (Nodes.Get (0));
  apps2.Start (Seconds (2.0));
  apps2.Stop (Seconds (3.0));

  apps3 = sink.Install (Nodes.Get (0));
  apps3.Start (Seconds (3.0));
  apps3.Stop (Seconds (4.0));

  apps4 = sink.Install (Nodes.Get (0));
  apps4.Start (Seconds (4.0));
  apps4.Stop (Seconds (5.0));

  apps5 = sink.Install (Nodes.Get (0));
  apps5.Start (Seconds (5.0));
  apps5.Stop (Seconds (6.0));

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("larger-scenario.tr");
  p2p.EnableAsciiAll (stream);
  csma.EnableAsciiAll (stream);

  // Flow Monitor
  Ptr<FlowMonitor> flowmon;
  if (enableFlowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (7));

  AnimationInterface::SetNodeDescription (Nodes, "Nodes"); // Optional
  AnimationInterface::SetNodeDescription (SB1, "SB1"); // Optional
  AnimationInterface::SetNodeDescription (SB2, "SB2"); // Optional
  AnimationInterface::SetNodeDescription (SB3, "SB3"); // Optional
  AnimationInterface::SetNodeDescription (UT1,"UT1"); // Optional
  AnimationInterface::SetNodeDescription (UT2,"UT2"); // Optional
  AnimationInterface::SetNodeDescription (UT3,"UT3"); // Optional
  AnimationInterface::SetNodeDescription (UT4,"UT4"); // Optional
  AnimationInterface::SetNodeDescription (GW1, "GW1"); // Optional
  AnimationInterface::SetNodeDescription (GW2, "GW2"); // Optional
  AnimationInterface::SetNodeDescription (IPr, "IP Router"); // Optional
  AnimationInterface::SetNodeColor (Nodes, 0, 255, 0); // Optional
  AnimationInterface::SetNodeColor (SB1, 255, 0, 0); // Optional
  AnimationInterface::SetNodeColor (UT1, 0, 0, 255); // Optional
  AnimationInterface::SetNodeColor (GW1, 0, 0, 255); // Optional
  AnimationInterface anim ("larger-scenario.xml"); // Mandatory
  anim.EnablePacketMetadata (true); // Optional

  Simulator::Run ();
  NS_LOG_INFO ("Done.");

  if (enableFlowMonitor)
    {
      flowmon->SerializeToXmlFile ("larger-scenario.flowmon", false, false);
    }

  Simulator::Destroy ();
  return 0;
}
