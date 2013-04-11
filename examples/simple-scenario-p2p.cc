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

//       5 Mb/s, 2ms         50 Mb/s, 10ms   
//   Node1 ---- UT ---- Satelite ---- GW ---- Node2
//                50 Mb/s, 10ms          5 Mb/s, 2ms
   

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/sat-net-dev-helper.h"
//#include "ns3/point-to-point-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleGlobalRoutingExample");

int 
main (int argc, char *argv[])
{

  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);

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
  NodeContainer Nodes;
  Nodes.Create (2);
  Ptr<Node> UT = CreateObject<Node> ();
  Ptr<Node> SB = CreateObject<Node> ();
  Ptr<Node> GW = CreateObject<Node> ();
  NodeContainer all = NodeContainer (UT, SB, GW);

  NodeContainer N0UT = NodeContainer (Nodes.Get (0), UT);
  NodeContainer UTSat = NodeContainer (UT, SB);
  NodeContainer SatGW = NodeContainer (SB, GW);
  NodeContainer GWN1 = NodeContainer (GW, Nodes.Get (1));

  AnimationInterface::SetConstantPosition (Nodes.Get (0), 10, 30); 
  AnimationInterface::SetConstantPosition (Nodes.Get (1), 90, 70); 
  AnimationInterface::SetConstantPosition (UT, 30, 50); 
  AnimationInterface::SetConstantPosition (SB, 50, 30); 
  AnimationInterface::SetConstantPosition (GW, 70, 50); 

  InternetStackHelper internet;
  internet.Install (all);
  internet.Install (Nodes);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer d1 = csma.Install (N0UT);
  NetDeviceContainer d4 = csma.Install (GWN1);


  SatNetDevHelper p2p;
  //PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer d2 = p2p.Install (UTSat);

  NetDeviceContainer d3 = p2p.Install (SatGW);

  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign (d1);


  ipv4.SetBase ("10.2.2.0", "255.255.255.0");
  ipv4.Assign (d2);

  ipv4.SetBase ("10.3.3.0", "255.255.255.0");
  ipv4.Assign (d3);

  ipv4.SetBase ("10.4.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign (d4);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (i3i4.GetAddress (1), port)));
  onoff.SetConstantRate (DataRate ("448kb/s"));
  ApplicationContainer apps = onoff.Install (Nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (i0i1.GetAddress (0), port)));
  apps = sink.Install (Nodes.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-scenario-p2p.tr"));
  p2p.EnablePcapAll ("simple-scenario-p2p");

  // Flow Monitor
  Ptr<FlowMonitor> flowmon;

  if (enableFlowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (11));

  AnimationInterface::SetNodeDescription (Nodes, "Nodes"); // Optional
  AnimationInterface::SetNodeDescription (SB, "SB"); // Optional
  AnimationInterface::SetNodeDescription (UT, "UT"); // Optional
  AnimationInterface::SetNodeDescription (GW, "GW"); // Optional
  AnimationInterface::SetNodeColor (Nodes, 0, 255, 0); // Optional
  AnimationInterface::SetNodeColor (SB, 255, 0, 0); // Optional
  AnimationInterface::SetNodeColor (UT, 0, 0, 255); // Optional
  AnimationInterface::SetNodeColor (GW, 0, 0, 255); // Optional
  AnimationInterface anim ("simple-scenario-p2p.xml"); // Mandatory
  anim.EnablePacketMetadata (true); // Optional

  Simulator::Run ();
  NS_LOG_INFO ("Done.");

  if (enableFlowMonitor)
    {
      flowmon->SerializeToXmlFile ("simple-scenario-p2p.flowmon", false, false);
    }

  Simulator::Destroy ();
  return 0;
}
