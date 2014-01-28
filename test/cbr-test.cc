/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simple-channel.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/cbr-helper.h"
#include "ns3/cbr-application.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

// \ brief Simple test case to verify Cbr application functionality.
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
  : TestCase ("Cbr test case to verify all sent data is got by receiver.")
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
  serverApps.Stop (Seconds (10.0));

  CbrHelper client ("ns3::UdpSocketFactory", serverAddress);
  client.SetAttribute ("Interval", StringValue ("1s"));
  ApplicationContainer clientApps = client.Install (n.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (8.0));

  Simulator::Run ();
  Simulator::Destroy ();

  Ptr<PacketSink> sink = DynamicCast<PacketSink> (serverApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (clientApps.Get (0));

  NS_TEST_ASSERT_MSG_NE (sender->GetSent(), (uint32_t)0, "Nothing sent !");
  NS_TEST_ASSERT_MSG_EQ (sink->GetTotalRx(), sender->GetSent(), "Packets were lost !");
}

// The CbrTestSuite class names the TestSuite as cbr-test, identifies what type of TestSuite (UNIT),
// and enables the TestCases to be run CbrTestCase1.
//
class CbrTestSuite : public TestSuite
{
public:
  CbrTestSuite ();
};

CbrTestSuite::CbrTestSuite ()
  : TestSuite ("cbr-test", UNIT)
{
  AddTestCase (new CbrTestCase1, TestCase::QUICK);
}

// Allocate an instance of this TestSuite
static CbrTestSuite cbrTestSuite;

