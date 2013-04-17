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

/**
 * \file simple-p2p.cc
 * \brief 'Simple Point-to-Point Test Cases' implementation of TN4.
 *
 * In this module are implemented all 'Simple Point-to-Point' Test Cases
 * defined in document TN4.
 *
 */

#include "ns3/string.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/cbr-helper.h"
#include "ns3/cbr-application.h"
#include "ns3/satellite-helper.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

/**
 * \brief 'Forward Link Unicast, Simple' test case implementation, id: simple-p2p-1 / TN4.
 *
 * This case tests successful transmission of a single UDP packet from GW connected user
 * to UT connected user in simple scenario.
 *  1.  Simple test scenario set with helper
 *  2.  A single packet is transmitted from Node-1 UDP application to Node-2 UDP receiver.
 *
 *  Expected result:
 *    A single UDP packet sent by GW connected node-1 using CBR application is received by
 *    UT connected node-2.
 *
 *  Notes: Current test case uses very first versions of the sat net devices and channels.
 *         Sat helper implementation is missing and not used by this test case yet.
 *
 */
class SimpleP2p1 : public TestCase
{
public:
  SimpleP2p1 ();
  virtual ~SimpleP2p1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleP2p1::SimpleP2p1 ()
  : TestCase ("'Forward Link Unicast, Simple' case tests successful transmission of a single UDP packet from GW connected user to UT connected user in simple scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleP2p1::~SimpleP2p1 ()
{
}

//
// SimpleP2p1 TestCase implementation
//
void
SimpleP2p1::DoRun (void)
{
  // Create simple scenario
  SatHelper helper(SatHelper::Simple);

  // >>> Start of actual test using Simple scenario >>>

  // Create the Cbr application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s, one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (1), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));

  ApplicationContainer GwApps = cbr.Install (helper.GetUser(1));
  GwApps.Start (Seconds (1.0));
  GwApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (1), port)));

  ApplicationContainer UtApps = sink.Install (helper.GetUser(0));
  UtApps.Start (Seconds (1.0));
  UtApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (UtApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (GwApps.Get (0));

  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (sender->GetSent(), (uint32_t)0, "Nothing sent !");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx(), sender->GetSent(), "Packets were lost !");

  // <<< End of actual test using Simple scenario <<<
}

// The TestSuite class names the TestSuite as sat-simple-p2p, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run (SimpleP2p1).  Typically, only the constructor for
// this class must be defined
//
class SimpleP2pTestSuite : public TestSuite
{
public:
  SimpleP2pTestSuite ();
};

SimpleP2pTestSuite::SimpleP2pTestSuite ()
  : TestSuite ("sat-simple-p2p", SYSTEM)
{
  // add simple-p2p-1 case to suite sat-simple-p2p
  AddTestCase (new SimpleP2p1);
}

// Allocate an instance of this TestSuite
static SimpleP2pTestSuite simplePspTestSuite;

