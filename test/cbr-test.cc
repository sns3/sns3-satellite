/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include a header file from your module to test.
#include "ns3/cbr-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include <fstream>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simple-channel.h"
#include "ns3/simulator.h"

// An essential include is test.h
#include "ns3/test.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

// This is an example TestCase.
class CbrTestCase1 : public TestCase
{
public:
  CbrTestCase1 ();
  virtual ~CbrTestCase1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
CbrTestCase1::CbrTestCase1 ()
  : TestCase ("Cbr test case (does nothing)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
CbrTestCase1::~CbrTestCase1 ()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
CbrTestCase1::DoRun (void)
{
  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);

  // link the two nodes
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();
  n.Get (0)->AddDevice (txDev);
  n.Get (1)->AddDevice (rxDev);
  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
  rxDev->SetChannel (channel1);
  txDev->SetChannel (channel1);
  NetDeviceContainer d;
  d.Add (txDev);
  d.Add (rxDev);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (d);

  uint16_t port = 4000;
  Address serverAddress (InetSocketAddress (i.GetAddress (1), port));

  PacketSinkHelper server ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer serverApps = server.Install (n.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (11.0));

  CbrHelper client ("ns3::UdpSocketFactory", serverAddress);
  client.SetAttribute ("Interval", StringValue ("6s"));
  ApplicationContainer clientApps = client.Install (n.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //Simulator::Stop (Seconds (12.0));
  Simulator::Run ();
  Simulator::Destroy ();

  Ptr<PacketSink> sink = DynamicCast<PacketSink> (serverApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (clientApps.Get (0));


  NS_TEST_ASSERT_MSG_EQ (sink->GetTotalRx(), sender->GetSent(), "Packets were lost !");
}

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class CbrTestSuite : public TestSuite
{
public:
  CbrTestSuite ();
};

CbrTestSuite::CbrTestSuite ()
  : TestSuite ("satellite-cbr", UNIT)
{
  AddTestCase (new CbrTestCase1);
}

// Do not forget to allocate an instance of this TestSuite
static CbrTestSuite cbrTestSuite;

