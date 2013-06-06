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
#include "ns3/log.h"
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

  NodeContainer utUsers = helper.GetUtUsers();

  // >>> Start of actual test using Simple scenario >>>

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));

  ApplicationContainer GwApps = cbr.Install (helper.GetGwUsers());
  GwApps.Start (Seconds (1.0));
  GwApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(0)), port)));

  ApplicationContainer UtApps = sink.Install (utUsers);
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
  NS_TEST_ASSERT_MSG_NE (sender->GetSent(), (uint32_t)0, "Nothing sent!");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx(), sender->GetSent(), "Packets were lost!");

  // <<< End of actual test using Simple scenario <<<
}

/**
 * \brief 'Forward Link Unicast, Larger' test case implementation, id: simple-p2p-2 / TN4.
 *
 * This case tests successful transmission of a single UDP packet from GW connected user
 * to UT connected users in larger scenario.
 *  1.  Larger test scenario set with helper
 *  2.  A single packet is transmitted from Node-1 UDP application to Node-2 and Node-6 UDP receivers.
 *
 *  Expected result:
 *    A single UDP packet sent by GW connected node-1 using CBR application is received by
 *    UT connected node-2 and Node-6 .
 *
 *  Notes: Current test case uses very first versions of the sat net devices and channels.
 *
 */
class SimpleP2p2 : public TestCase
{
public:
  SimpleP2p2 ();
  virtual ~SimpleP2p2 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleP2p2::SimpleP2p2 ()
  : TestCase ("'Forward Link Unicast, Larger' case tests successful transmission of a single UDP packet from GW connected user to UT connected users in larger scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleP2p2::~SimpleP2p2 ()
{
}

//
// SimpleP2p2 TestCase implementation
//
void
SimpleP2p2::DoRun (void)
{
  // Create larger scenario
  SatHelper helper(SatHelper::Larger);

  NodeContainer utUsers = helper.GetUtUsers();

  // >>> Start of actual test using Larger scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)

  // app to send receiver 1
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));
  ApplicationContainer GwApps = cbr.Install (helper.GetGwUsers());

  // app to send receiver 2
  cbr.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(4)), port))));
  GwApps.Add(cbr.Install (helper.GetGwUsers()));

  GwApps.Start (Seconds (1.0));
  GwApps.Stop (Seconds (2.1));

  // Create a packet sinks to receive these packets

  // receiver 1
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(0)), port)));
  ApplicationContainer UtApps = sink.Install (utUsers.Get(0));

  // receiver 2
  sink.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(4)), port))));
  UtApps.Add(sink.Install (utUsers.Get(4)));

  UtApps.Start (Seconds (1.0));
  UtApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver1 = DynamicCast<PacketSink> (UtApps.Get (0));
  Ptr<CbrApplication> sender1 = DynamicCast<CbrApplication> (GwApps.Get (0));

  Ptr<PacketSink> receiver2 = DynamicCast<PacketSink> (UtApps.Get (1));
  Ptr<CbrApplication> sender2 = DynamicCast<CbrApplication> (GwApps.Get (1));

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receivers got all all data sent

  NS_TEST_ASSERT_MSG_NE (sender1->GetSent(), (uint32_t)0, "Nothing sent by sender 1!");
  NS_TEST_ASSERT_MSG_EQ (receiver1->GetTotalRx(), sender1->GetSent(), "Packets were lost between sender 1 and receiver 1!");

  NS_TEST_ASSERT_MSG_NE (sender2->GetSent(), (uint32_t)0, "Nothing sent !");
  NS_TEST_ASSERT_MSG_EQ (receiver2->GetTotalRx(), sender2->GetSent(), "Packets were lost between sender 2 and receiver 2");

  // <<< End of actual test using Larger scenario <<<
}

/**
 * \brief 'Forward Link Unicast, Full' test case implementation, id: simple-p2p-3 / TN4.
 *
 * This case tests successful transmission of a single UDP packet from GW connected user
 * to UT connected user in full scenario.
 *  1.  Full test scenario set with helper
 *  2.  A single packet is transmitted from Node-1 UDP application to Node-5 UDP receiver.
 *  3.  A single packet is transmitted from Node-2 UDP application to Node-6 UDP receiver.
 *
 *  Expected result:
 *    A single UDP packet sent by GW connected node-1 using CBR application is received by
 *    UT connected node-5 and Node-6 .
 *
 *  Notes: Current test case uses very first versions of the sat net devices and channels.
 *
 */
class SimpleP2p3 : public TestCase
{
public:
  SimpleP2p3 ();
  virtual ~SimpleP2p3 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleP2p3::SimpleP2p3 ()
  : TestCase ("'Forward Link Unicast, Full' case tests successful transmission of a single UDP packet from GW connected user to UT connected users in full scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleP2p3::~SimpleP2p3 ()
{
}

//
// SimpleP2p3 TestCase implementation
//
void
SimpleP2p3::DoRun (void)
{
  // Create full scenario
  SatHelper helper(SatHelper::Full);

  NodeContainer utUsers = helper.GetUtUsers();
  NodeContainer gwUsers = helper.GetGwUsers();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 0.5s)
  Time cbrInterval = Seconds(0.5);
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(0)), port)));
  cbr.SetAttribute ("Interval", TimeValue(cbrInterval));

  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(0)), port)));

  // initialized time values for simulation
  uint32_t maxReceivers = utUsers.GetN();
  Time cbrStartDelay = Seconds(0.01);
  Time cbrStartBaseValue = Seconds(1.0);
  Time cbrStopBaseValue = cbrStartBaseValue + cbrInterval + cbrStartDelay;
  Time stopTime = Seconds(maxReceivers*cbrStartDelay.GetSeconds()) + cbrStopBaseValue + Seconds(0.5);

  ApplicationContainer GwApps;
  ApplicationContainer UtApps;

  // Cbr and Sink applications creation
  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      cbr.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(i)), port))));
      sink.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper.GetUserAddress (utUsers.Get(i)), port))));

      GwApps.Add(cbr.Install (gwUsers.Get(4)));
      UtApps.Add(sink.Install (utUsers.Get(i)));

      Time startStopDelta = Seconds(i * cbrStartDelay.GetSeconds());

      GwApps.Get(i)->SetStartTime(startStopDelta + cbrStartBaseValue);
      GwApps.Get(i)->SetStopTime(startStopDelta + cbrStopBaseValue);
    }

  UtApps.Start (cbrStartBaseValue);
  UtApps.Stop (stopTime);

  Simulator::Stop (stopTime);
  Simulator::Run ();

  Simulator::Destroy ();

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receivers got all all data sent

  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      Ptr<PacketSink> receiver = DynamicCast<PacketSink> (UtApps.Get (i));
      Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (GwApps.Get (i));

      NS_TEST_ASSERT_MSG_NE (sender->GetSent(), (uint32_t)0, "Nothing sent by sender" << i << "!");
      NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx(), sender->GetSent(), "Packets were lost between sender and receiver" << i << "!");
    }

  // <<< End of actual test using Full scenario <<<
}

/**
 * \brief 'Return Link Unicast, Simple' test case implementation, id: simple-p2p-4 / TN4.
 *
 * This case tests successful transmission of a single UDP packet from UT connected user
 * to GW connected user in simple scenario.
 *  1.  Simple test scenario set with helper
 *  2.  A single packet is transmitted from Node-2 UDP application to Node-1 UDP receiver.
 *
 *  Expected result:
 *    A single UDP packet sent by UT connected node-2 using CBR application is received by
 *    GW connected node-1.
 *
 *  Notes: Current test case uses very first versions of the sat net devices and channels.
 *
 */
class SimpleP2p4 : public TestCase
{
public:
  SimpleP2p4 ();
  virtual ~SimpleP2p4 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleP2p4::SimpleP2p4 ()
  : TestCase ("'Return Link Unicast, Simple' case tests successful transmission of a single UDP packet from UT connected user to GW connected user in simple scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleP2p4::~SimpleP2p4 ()
{
}

//
// SimpleP2p4 TestCase implementation
//
void
SimpleP2p4::DoRun (void)
{
  // Create simple scenario
  SatHelper helper(SatHelper::Simple);

  // >>> Start of actual test using Simple scenario >>>

  NodeContainer gwUsers = helper.GetGwUsers();

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (gwUsers.Get(0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));

  ApplicationContainer UtApps = cbr.Install (helper.GetUtUsers());
  UtApps.Start (Seconds (1.0));
  UtApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (gwUsers.Get(0)), port)));

  ApplicationContainer GwApps = sink.Install (gwUsers);
  GwApps.Start (Seconds (1.0));
  GwApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (GwApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (UtApps.Get (0));

  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (sender->GetSent(), (uint32_t)0, "Nothing sent !");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx(), sender->GetSent(), "Packets were lost !");

  // <<< End of actual test using Simple scenario <<<
}

/**
 * \brief 'Return Link Unicast, Larger' test case implementation, id: simple-p2p-5 / TN4.
 *
 * This case tests successful transmission of a single UDP packet from UT connected user
 * to GW connected user in larger scenario.
 *  1.  Larger test scenario set with helper
 *  2.  A single packet is transmitted from Node-2 UDP application to Node-1 UDP receiver.
 *  3.  A single packet is transmitted from Node-6 UDP application to Node-1 UDP receiver.
 *
 *  Expected result:
 *    A single UDP packet sent by UT connected node-2 and node-6 using CBR applications are received by
 *    GW connected node-1.
 *
 *  Notes: Current test case uses very first versions of the sat net devices and channels.
 *
 */
class SimpleP2p5 : public TestCase
{
public:
  SimpleP2p5 ();
  virtual ~SimpleP2p5 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleP2p5::SimpleP2p5 ()
  : TestCase ("'Return Link Unicast, Larger' case tests successful transmission of a single UDP packet from UT connected user to GW connected user in larger scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleP2p5::~SimpleP2p5 ()
{
}

//
// SimpleP2p5 TestCase implementation
//
void
SimpleP2p5::DoRun (void)
{
  // Create larger scenario
  SatHelper helper(SatHelper::Larger);

  // >>> Start of actual test using Larger scenario >>>

  NodeContainer gwUsers = helper.GetGwUsers();
  NodeContainer utUsers = helper.GetUtUsers();

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)

  // sender 1
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (gwUsers.Get(0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));
  ApplicationContainer UtApps = cbr.Install (utUsers.Get(0));

  // sender 2
  UtApps.Add(cbr.Install(utUsers.Get(4)));

  UtApps.Start (Seconds (1.0));
  UtApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (gwUsers.Get(0)), port)));

  ApplicationContainer GwApps = sink.Install (gwUsers);
  GwApps.Start (Seconds (1.0));
  GwApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (GwApps.Get (0));
  Ptr<CbrApplication> sender1 = DynamicCast<CbrApplication> (UtApps.Get (0));
  Ptr<CbrApplication> sender2 = DynamicCast<CbrApplication> (UtApps.Get (1));

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (sender1->GetSent(), (uint32_t)0, "Nothing sent by sender 1!");
  NS_TEST_ASSERT_MSG_NE (sender2->GetSent(), (uint32_t)0, "Nothing sent by sender 2!");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx(), sender1->GetSent() + sender2->GetSent(), "Packets were lost!");

  // <<< End of actual test using Larger scenario <<<
}

/**
 * \brief 'Return Link Unicast, Full' test case implementation, id: simple-p2p-6 / TN4.
 *
 * This case tests successful transmission of a single UDP packet from UT connected user
 * to GW connected user in full scenario.
 *  1.  Full test scenario set with helper
 *  2.  A single packet is transmitted from every UT connected application to GW connected receiver.
 *
 *  Expected result:
 *    A single UDP packet sent by UT connected node using CBR application is received by
 *    GW connected node application.
 *
 *  Notes: Current test case uses very first versions of the sat net devices and channels.
 *
 */
class SimpleP2p6 : public TestCase
{
public:
  SimpleP2p6 ();
  virtual ~SimpleP2p6 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleP2p6::SimpleP2p6 ()
  : TestCase ("'Return Link Unicast, Full' case tests successful transmission of a single UDP packet from UT connected user to GW connected user in full scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleP2p6::~SimpleP2p6 ()
{
}

//
// SimpleP2p6 TestCase implementation
//
void
SimpleP2p6::DoRun (void)
{
  // Create full scenario
  SatHelper helper(SatHelper::Full);

  // >>> Start of actual test using Full scenario >>>

  NodeContainer gwUsers = helper.GetGwUsers();
  NodeContainer utUsers = helper.GetUtUsers();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (gwUsers.Get(3)), port)));

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 0.5s)
  Time cbrInterval = Seconds(0.5);
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper.GetUserAddress (gwUsers.Get(3)), port)));
  cbr.SetAttribute ("Interval", TimeValue(cbrInterval));

  // initialized time values for simulation
  uint32_t maxReceivers = utUsers.GetN();
  Time cbrStartDelay = Seconds(0.01);
  Time cbrStartBaseValue = Seconds(1.0);
  Time cbrStopBaseValue = cbrStartBaseValue + cbrInterval + cbrStartDelay;
  Time stopTime = Seconds(maxReceivers*cbrStartDelay.GetSeconds()) + cbrStopBaseValue + Seconds(0.5);

  ApplicationContainer UtApps;

  // Cbr applications creation
  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      UtApps.Add(cbr.Install (utUsers.Get(i)));

      Time startStopDelta = Seconds(i * cbrStartDelay.GetSeconds());

      UtApps.Get(i)->SetStartTime(startStopDelta + cbrStartBaseValue);
      UtApps.Get(i)->SetStopTime(startStopDelta + cbrStopBaseValue);
    }

  ApplicationContainer GwApps = sink.Install (gwUsers.Get(3));
  GwApps.Start (cbrStartBaseValue);
  GwApps.Stop (stopTime);

  Simulator::Stop (stopTime);
  Simulator::Run ();

  Simulator::Destroy ();

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receiver got all all data sent

  uint32_t totalTxBytes = 0;
  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (GwApps.Get (0));

  for (uint32_t i = 0; i < maxReceivers; i++)
    {
      Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (UtApps.Get (i));

      NS_TEST_ASSERT_MSG_NE (sender->GetSent(), (uint32_t)0, "Nothing sent by sender " << i+1);
      totalTxBytes += sender->GetSent();
    }

  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx(), totalTxBytes, "Packets were lost!");

  // <<< End of actual test using Full scenario <<<
}

// The TestSuite class names the TestSuite as sat-simple-p2p, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run.  Typically, only the constructor for
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

  // add simple-p2p-2 case to suite sat-simple-p2p
  AddTestCase (new SimpleP2p2);

  // add simple-p2p-3 case to suite sat-simple-p2p
  AddTestCase (new SimpleP2p3);

  // add simple-p2p-4 case to suite sat-simple-p2p
  AddTestCase (new SimpleP2p4);

  // add simple-p2p-5 case to suite sat-simple-p2p
  AddTestCase (new SimpleP2p5);

  // add simple-p2p-6 case to suite sat-simple-p2p
  AddTestCase (new SimpleP2p6);
}

// Allocate an instance of this TestSuite
static SimpleP2pTestSuite simplePspTestSuite;

