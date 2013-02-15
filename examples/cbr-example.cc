/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/satellite-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("cbr-example");

int
main (int argc, char *argv[])
{
  uint32_t packetSize = 512;

  CommandLine cmd;
  cmd.AddValue("packetSize", "Size of constant packet", packetSize);
  cmd.Parse (argc, argv);

  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t port = 9;
  Address serverAddress (InetSocketAddress (interfaces.GetAddress (1), port));
  Address clientAddress (InetSocketAddress (interfaces.GetAddress (0), port));

  PacketSinkHelper sinkServer ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));

  ApplicationContainer serverApps = sinkServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  CbrHelper cbrClient ("ns3::UdpSocketFactory", serverAddress);
  cbrClient.SetAttribute("Interval", StringValue ("1s"));
  cbrClient.SetAttribute("PacketSize", UintegerValue (packetSize) );

  ApplicationContainer clientApps = cbrClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (8.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
