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
 * \file satellite-simple-unicast.cc
 * \ingroup satellite
 * \brief 'Simple Point-to-Point Test Cases' implementation of TN4.
 *
 * In this module are implemented all 'Simple Point-to-Point' Test Cases
 * defined in document TN4.
 *
 */

#include "ns3/string.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/enum.h"
#include "ns3/cbr-application.h"
#include "ns3/cbr-helper.h"
#include "../helper/satellite-helper.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief 'Forward Link Unicast, Simple' test case implementation, id: simple_unicast-1 / TN4.
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
class SimpleUnicast1 : public TestCase
{
public:
  SimpleUnicast1 ();
  virtual ~SimpleUnicast1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast1::SimpleUnicast1 ()
  : TestCase ("'Forward Link Unicast, Simple' case tests successful transmission of a single UDP packet from GW connected user to UT connected user in simple scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast1::~SimpleUnicast1 ()
{
}

//
// SimpleUnicast1 TestCase implementation
//
void
SimpleUnicast1::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast1", true);

  // Create simple scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer utUsers = helper->GetUtUsers ();

  // >>> Start of actual test using Simple scenario >>>

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));

  ApplicationContainer gwApps = cbr.Install (helper->GetGwUsers ());
  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  ApplicationContainer utApps = sink.Install (utUsers);
  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (utApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (gwApps.Get (0));

  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (sender->GetSent (), (uint32_t)0, "Nothing sent!");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender->GetSent (), "Packets were lost!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Simple scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Forward Link Unicast, Larger' test case implementation, id: simple_unicast-2 / TN4.
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
class SimpleUnicast2 : public TestCase
{
public:
  SimpleUnicast2 ();
  virtual ~SimpleUnicast2 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast2::SimpleUnicast2 ()
  : TestCase ("'Forward Link Unicast, Larger' case tests successful transmission of a single UDP packet from GW connected user to UT connected users in larger scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast2::~SimpleUnicast2 ()
{
}

//
// SimpleUnicast2 TestCase implementation
//
void
SimpleUnicast2::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast2", true);

  // Create larger scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::LARGER);

  NodeContainer utUsers = helper->GetUtUsers ();

  // >>> Start of actual test using Larger scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)

  // app to send receiver 1
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));
  ApplicationContainer gwApps = cbr.Install (helper->GetGwUsers ());

  // app to send receiver 2
  cbr.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (4)), port))));
  gwApps.Add (cbr.Install (helper->GetGwUsers ()));

  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (2.1));

  // Create a packet sinks to receive these packets

  // receiver 1
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  ApplicationContainer utApps = sink.Install (utUsers.Get (0));

  // receiver 2
  sink.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (4)), port))));
  utApps.Add (sink.Install (utUsers.Get (4)));

  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver1 = DynamicCast<PacketSink> (utApps.Get (0));
  Ptr<CbrApplication> sender1 = DynamicCast<CbrApplication> (gwApps.Get (0));

  Ptr<PacketSink> receiver2 = DynamicCast<PacketSink> (utApps.Get (1));
  Ptr<CbrApplication> sender2 = DynamicCast<CbrApplication> (gwApps.Get (1));

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receivers got all all data sent

  NS_TEST_ASSERT_MSG_NE (sender1->GetSent (), (uint32_t)0, "Nothing sent by sender 1!");
  NS_TEST_ASSERT_MSG_EQ (receiver1->GetTotalRx (), sender1->GetSent (), "Packets were lost between sender 1 and receiver 1!");

  NS_TEST_ASSERT_MSG_NE (sender2->GetSent (), (uint32_t)0, "Nothing sent !");
  NS_TEST_ASSERT_MSG_EQ (receiver2->GetTotalRx (), sender2->GetSent (), "Packets were lost between sender 2 and receiver 2!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Larger scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Forward Link Unicast, Full' test case implementation, id: simple_unicast-3 / TN4.
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
class SimpleUnicast3 : public TestCase
{
public:
  SimpleUnicast3 ();
  virtual ~SimpleUnicast3 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast3::SimpleUnicast3 ()
  : TestCase ("'Forward Link Unicast, Full' case tests successful transmission of a single UDP packet from GW connected user to UT connected users in full scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast3::~SimpleUnicast3 ()
{
}

//
// SimpleUnicast3 TestCase implementation
//
void
SimpleUnicast3::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast3", true);

  // Create full scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::FULL);

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 0.5s)
  Time cbrInterval = Seconds (0.5);
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", TimeValue (cbrInterval));

  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  // initialized time values for simulation
  uint32_t maxReceivers = utUsers.GetN ();
  Time cbrStartDelay = Seconds (0.01);
  Time cbrStopDelay = Seconds (0.1);
  Time stopTime = Seconds (maxReceivers * cbrStartDelay.GetSeconds ()) + cbrInterval + cbrInterval + Seconds (5);

  ApplicationContainer gwApps;
  ApplicationContainer utApps;

  // Cbr and Sink applications creation
  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      cbr.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (i)), port))));
      sink.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (i)), port))));

      gwApps.Add (cbr.Install (gwUsers.Get (4)));
      utApps.Add (sink.Install (utUsers.Get (i)));

      cbrStartDelay += Seconds (0.01);

      gwApps.Get (i)->SetStartTime (cbrStartDelay);
      gwApps.Get (i)->SetStopTime (cbrStartDelay + cbrInterval + cbrStopDelay);
    }

  utApps.Start (Seconds (0.00001));
  utApps.Stop (stopTime);

  Simulator::Stop (stopTime);
  Simulator::Run ();

  Simulator::Destroy ();

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receivers got all all data sent

  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      Ptr<PacketSink> receiver = DynamicCast<PacketSink> (utApps.Get (i));
      Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (gwApps.Get (i));

      NS_TEST_ASSERT_MSG_NE (sender->GetSent (), (uint32_t)0, "Nothing sent by sender" << i << "!");
      NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender->GetSent (), "Packets were lost between sender and receiver" << i << "!");
    }

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Full scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Return Link Unicast, Simple' test case implementation, id: simple_unicast-4 / TN4.
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
class SimpleUnicast4 : public TestCase
{
public:
  SimpleUnicast4 ();
  virtual ~SimpleUnicast4 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast4::SimpleUnicast4 ()
  : TestCase ("'Return Link Unicast, Simple' case tests successful transmission of a single UDP packet from UT connected user to GW connected user in simple scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast4::~SimpleUnicast4 ()
{
}

//
// SimpleUnicast4 TestCase implementation
//
void
SimpleUnicast4::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast4", true);

  // Create simple scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  // >>> Start of actual test using Simple scenario >>>

  NodeContainer gwUsers = helper->GetGwUsers ();

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));

  ApplicationContainer utApps = cbr.Install (helper->GetUtUsers ());
  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port)));

  ApplicationContainer gwApps = sink.Install (gwUsers);
  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (gwApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (utApps.Get (0));

  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (sender->GetSent (), (uint32_t)0, "Nothing sent !");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender->GetSent (), "Packets were lost !");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Simple scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Return Link Unicast, Larger' test case implementation, id: simple_unicast-5 / TN4.
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
class SimpleUnicast5 : public TestCase
{
public:
  SimpleUnicast5 ();
  virtual ~SimpleUnicast5 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast5::SimpleUnicast5 ()
  : TestCase ("'Return Link Unicast, Larger' case tests successful transmission of a single UDP packet from UT connected user to GW connected user in larger scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast5::~SimpleUnicast5 ()
{
}

//
// SimpleUnicast5 TestCase implementation
//
void
SimpleUnicast5::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast5", true);

  // Create larger scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::LARGER);

  // >>> Start of actual test using Larger scenario >>>

  NodeContainer gwUsers = helper->GetGwUsers ();
  NodeContainer utUsers = helper->GetUtUsers ();

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)

  // sender 1
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("1s"));
  ApplicationContainer utApps = cbr.Install (utUsers.Get (0));

  // sender 2
  utApps.Add (cbr.Install (utUsers.Get (4)));

  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (2.1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port)));

  ApplicationContainer gwApps = sink.Install (gwUsers);
  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (3.0));

  Simulator::Stop (Seconds (11));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (gwApps.Get (0));
  Ptr<CbrApplication> sender1 = DynamicCast<CbrApplication> (utApps.Get (0));
  Ptr<CbrApplication> sender2 = DynamicCast<CbrApplication> (utApps.Get (1));

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (sender1->GetSent (), (uint32_t)0, "Nothing sent by sender 1!");
  NS_TEST_ASSERT_MSG_NE (sender2->GetSent (), (uint32_t)0, "Nothing sent by sender 2!");
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender1->GetSent () + sender2->GetSent (), "Packets were lost!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Larger scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Return Link Unicast, Full' test case implementation, id: simple_unicast-6 / TN4.
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
class SimpleUnicast6 : public TestCase
{
public:
  SimpleUnicast6 ();
  virtual ~SimpleUnicast6 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast6::SimpleUnicast6 ()
  : TestCase ("'Return Link Unicast, Full' case tests successful transmission of a single UDP packet from UT connected user to GW connected user in full scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast6::~SimpleUnicast6 ()
{
}

//
// SimpleUnicast6 TestCase implementation
//
void
SimpleUnicast6::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast6", true);

  // Create full scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::FULL);

  // >>> Start of actual test using Full scenario >>>

  NodeContainer gwUsers = helper->GetGwUsers ();
  NodeContainer utUsers = helper->GetUtUsers ();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (3)), port)));

  // Create the Cbr applications to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 0.5s)
  Time cbrInterval = Seconds (0.01);
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (3)), port)));
  cbr.SetAttribute ("Interval", TimeValue (cbrInterval));

  // initialized time values for simulation
  uint32_t maxReceivers = utUsers.GetN ();
  Time cbrStartDelay = Seconds (1.0);
  Time cbrStopDelay = Seconds (0.005);
  Time stopTime = Seconds (maxReceivers * 0.04) + cbrStartDelay + cbrInterval + cbrStopDelay;

  ApplicationContainer utApps;

  // Cbr applications creation
  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      utApps.Add (cbr.Install (utUsers.Get (i)));

      cbrStartDelay += Seconds (0.03);

      utApps.Get (i)->SetStartTime (cbrStartDelay);
      utApps.Get (i)->SetStopTime (cbrStartDelay + cbrInterval + cbrStopDelay);
    }

  ApplicationContainer gwApps = sink.Install (gwUsers.Get (3));
  gwApps.Start (Seconds (0.001));
  gwApps.Stop (stopTime);

  Simulator::Stop (stopTime);
  Simulator::Run ();

  Simulator::Destroy ();

  // here we check that results are as expected.
  // * Senders have sent something
  // * Receiver got all all data sent

  uint32_t totalTxBytes = 0;
  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (gwApps.Get (0));

  for (uint32_t i = 0; i < maxReceivers; i++)
    {
      Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (utApps.Get (i));

      NS_TEST_ASSERT_MSG_NE (sender->GetSent (), (uint32_t)0, "Nothing sent by sender " << i + 1);
      totalTxBytes += sender->GetSent ();
    }

  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), totalTxBytes, "Packets were lost!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Full scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Forward Link Unicast, Simple' test case implementation, id: simple_unicast-1 / TN4.
 *
 * This case tests successful transmission of a single TCP packet from GW and UT connected
 * users to each other’s in simple scenario.
 *  1.  Simple test scenario set with helper
 *  2.  Both Node-1 and Node-2 transmit single packet with an application utilizing TCP as transport layer.
 *
 *  Expected result:
 *    A packet successfully received by both nodes (Node-1 and Node-2).
 *    A packet acknowledged by both nodes.
 *
 *  Notes: Node-1 is attached to GW node and Node-2 attached to UT node.
 *
 */
class SimpleUnicast7 : public TestCase
{
public:
  SimpleUnicast7 ();
  virtual ~SimpleUnicast7 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast7::SimpleUnicast7 ()
  : TestCase ("'Two-way Unicast, Simple' case tests successful transmission of a single TCP packet from GW connected and UT connected users to each other's in simple scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast7::~SimpleUnicast7 ()
{
}

//
// SimpleUnicast7 TestCase implementation
//
void
SimpleUnicast7::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast7", true);

  // Create simple scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  // >>> Start of actual test using Simple scenario >>>

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::TcpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  cbr.SetAttribute ("Interval", StringValue ("1s"));

  // create CBR application in GW
  ApplicationContainer gwCbrApp = cbr.Install (gwUsers);
  gwCbrApp.Start (Seconds (1.0));
  gwCbrApp.Stop (Seconds (2.9));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::TcpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  ApplicationContainer utSinkApp = sink.Install (utUsers);
  utSinkApp.Start (Seconds (1.0));
  utSinkApp.Stop (Seconds (9.0));

  // set sink and cbr addresses from GW user
  sink.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
  cbr.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

  ApplicationContainer gwSinkApp = sink.Install (gwUsers);
  gwSinkApp.Start (Seconds (1.0));
  gwSinkApp.Stop (Seconds (9.0));

  ApplicationContainer utCbrApp = cbr.Install (utUsers);
  utCbrApp.Start (Seconds (1.0));
  utCbrApp.Stop (Seconds (2.9));

  Simulator::Stop (Seconds (15));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> utReceiver = DynamicCast<PacketSink> (utSinkApp.Get (0));
  Ptr<CbrApplication> gwSender = DynamicCast<CbrApplication> (gwCbrApp.Get (0));

  Ptr<PacketSink> gwReceiver = DynamicCast<PacketSink> (gwSinkApp.Get (0));
  Ptr<CbrApplication> utSender = DynamicCast<CbrApplication> (utCbrApp.Get (0));

  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (gwSender->GetSent (), (uint32_t)0, "Nothing sent by GW app!");
  NS_TEST_ASSERT_MSG_EQ (utReceiver->GetTotalRx (), gwSender->GetSent (), "Packets were lost to UT!");

  NS_TEST_ASSERT_MSG_NE (utSender->GetSent (), (uint32_t)0, "Nothing sent by UT app!");
  NS_TEST_ASSERT_MSG_EQ (gwReceiver->GetTotalRx (), utSender->GetSent (), "Packets were lost to GW!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Simple scenario <<<
}

/**
 * \ingroup satellite
 * \brief 'Forward Link Unicast, Simple' test case implementation, id: simple_unicast-1 / TN4.
 *
 * This case tests successful transmission of a single TCP packet from GW and UT connected
 * users to each other’s in simple scenario.
 *  1.  Simple test scenario set with helper
 *  2.  Both Node-1 and Node-2 transmit single packet with an application utilizing TCP as transport layer.
 *
 *  Expected result:
 *    A packet successfully received by both nodes (Node-1 and Node-2).
 *    A packet acknowledged by both nodes.
 *
 *  Notes: Node-1 is attached to GW node and Node-2 attached to UT node.
 *
 */
class SimpleUnicast8 : public TestCase
{
public:
  SimpleUnicast8 ();
  virtual ~SimpleUnicast8 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SimpleUnicast8::SimpleUnicast8 ()
  : TestCase ("'Two-way Unicast, Larger' case tests successful transmission of a single TCP packet from GW connected and UT connected users to each other's in larger scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SimpleUnicast8::~SimpleUnicast8 ()
{
}

//
// SimpleUnicast8 TestCase implementation
//
void
SimpleUnicast8::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-simple-unicast", "unicast8", true);

  // Create Larger scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::LARGER);

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  // >>> Start of actual test using Simple scenario >>>

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 1s)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::TcpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  cbr.SetAttribute ("Interval", StringValue ("1s"));

  // create CBR applications in GW
  ApplicationContainer gwCbrApps = cbr.Install (gwUsers);

  cbr.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (4)), port))));
  gwCbrApps.Add (cbr.Install (gwUsers));

  gwCbrApps.Start (Seconds (1.0));
  gwCbrApps.Stop (Seconds (2.9));

  // Create a packet sinks to receive these packets
  PacketSinkHelper sink ("ns3::TcpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  ApplicationContainer utSinkApps = sink.Install (utUsers.Get (0));

  sink.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (4)), port))));
  utSinkApps.Add (sink.Install (utUsers.Get (4)));

  utSinkApps.Start (Seconds (1.0));
  utSinkApps.Stop (Seconds (9.0));

  // set sink and cbr addresses from GW user
  sink.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
  cbr.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

  // create GW sink to receive packets from UTs
  ApplicationContainer gwSinkApp = sink.Install (gwUsers);
  gwSinkApp.Start (Seconds (1.0));
  gwSinkApp.Stop (Seconds (9.0));

  // create UT cbr apps to send packets to GW
  ApplicationContainer utCbrApps = cbr.Install (utUsers.Get (4));
  utCbrApps.Add (cbr.Install (utUsers.Get (0)));
  utCbrApps.Start (Seconds (1.0));
  utCbrApps.Stop (Seconds (2.9));

  Simulator::Stop (Seconds (15));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> utReceiver1 = DynamicCast<PacketSink> (utSinkApps.Get (0));
  Ptr<PacketSink> utReceiver2 = DynamicCast<PacketSink> (utSinkApps.Get (1));
  Ptr<CbrApplication> gwSender1 = DynamicCast<CbrApplication> (gwCbrApps.Get (0));
  Ptr<CbrApplication> gwSender2 = DynamicCast<CbrApplication> (gwCbrApps.Get (1));

  Ptr<PacketSink> gwReceiver = DynamicCast<PacketSink> (gwSinkApp.Get (0));
  Ptr<CbrApplication> utSender1 = DynamicCast<CbrApplication> (utCbrApps.Get (0));
  Ptr<CbrApplication> utSender2 = DynamicCast<CbrApplication> (utCbrApps.Get (1));

  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (gwSender1->GetSent (), (uint32_t)0, "Nothing sent by GW app 1!");
  NS_TEST_ASSERT_MSG_EQ (utReceiver1->GetTotalRx (), gwSender1->GetSent (), "Packets were lost to UT1!");

  NS_TEST_ASSERT_MSG_NE (gwSender2->GetSent (), (uint32_t)0, "Nothing sent by GW app 2!");
  NS_TEST_ASSERT_MSG_EQ (utReceiver2->GetTotalRx (), gwSender2->GetSent (), "Packets were lost to UT2!");

  NS_TEST_ASSERT_MSG_NE (utSender1->GetSent (), (uint32_t)0, "Nothing sent by UT app 1!");
  NS_TEST_ASSERT_MSG_NE (utSender2->GetSent (), (uint32_t)0, "Nothing sent by UT app 2!");
  NS_TEST_ASSERT_MSG_EQ (gwReceiver->GetTotalRx (), utSender1->GetSent () + utSender2->GetSent (), "Packets were lost to GW!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Larger scenario <<<
}


// The TestSuite class names the TestSuite as sat-simple-unicast, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class SimpleUnicastTestSuite : public TestSuite
{
public:
  SimpleUnicastTestSuite ();
};

SimpleUnicastTestSuite::SimpleUnicastTestSuite ()
  : TestSuite ("sat-simple-unicast", SYSTEM)
{
  // add simple-unicast-1 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast1, TestCase::QUICK);

  // add simple-unicast-2 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast2, TestCase::QUICK);

  // add simple_unicast-3 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast3, TestCase::QUICK);

  // add simple-unicast-4 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast4, TestCase::QUICK);

  // add simple-unicast-5 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast5, TestCase::QUICK);

  // add simple-unicast-6 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast6, TestCase::QUICK);

  // add simple-unicast-7 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast7, TestCase::QUICK);

  // add simple-unicast-8 case to suite sat-simple-unicast
  AddTestCase (new SimpleUnicast8, TestCase::QUICK);
}

// Allocate an instance of this TestSuite
static SimpleUnicastTestSuite simpleUnicastTestSuite;

