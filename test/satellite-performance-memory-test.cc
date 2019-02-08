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
 * \file satellite-performance-memory-test.cc
 * \ingroup satellite
 * \brief 'Performance and Memory Consumption Test Cases' implementation of TN4.
 *
 * In this module are implemented all 'Performance and Memory Consumption' Test Cases
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
 * \brief 'Performance and memory tracking' test case implementation, id: pm-1 / TN4.
 *
 * Full scenario created with helper
 * 1.  GW connected user sends single packet to UT connected user.
 * 2.  UT connected user sends single packet to GW connected user.
 * 3.  After simulation trace results, memory consumption reports and execution time saved for further analysing.
 *
 * Expected results: Execution time of simulation is expected to be reasonable level.
 *
 * This test case is expected to be run regular basis and results saved
 * for tracking and analysing purposes. Valgrind program is utilized in test to validate memory usage.
 * Test result is referred to previous results and analysed that execution time and
 * memory consumption doesn’t increase unacceptable level. The case helps recognizing of possible
 * performance and memory usage problems in implementation or design early enough.
 *
 *
 */
class Pm1 : public TestCase
{
public:
  Pm1 ();
  virtual ~Pm1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
Pm1::Pm1 ()
  : TestCase ("'Performance and memory tracking' tests servers as follow-up test for performance and memory usage.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
Pm1::~Pm1 ()
{
}

//
// Pm1 TestCase implementation
//
void
Pm1::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-perf-mem", "", true);

  // Create simple scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::FULL);

  NodeContainer gwUsers = helper->GetGwUsers ();
  NodeContainer utUsers = helper->GetUtUsers ();

  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("0.8s"));

  // Create a packet sink to receive packet and a Cbr to sent packet in UT
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  ApplicationContainer utApps = sink.Install (utUsers.Get (0));
  utApps.Add (cbr.Install (utUsers.Get (0)));
  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (2.5));

  // Create a packet sink to receive packet and a Cbr to sent packet in GW
  sink.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
  cbr.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port))));

  ApplicationContainer gwApps = sink.Install (gwUsers.Get (0));
  gwApps.Add (cbr.Install (gwUsers.Get (0)));

  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (2.5));

  Simulator::Stop (Seconds (2.5));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<PacketSink> utReceiver = DynamicCast<PacketSink> (utApps.Get (0));
  Ptr<CbrApplication> utSender = DynamicCast<CbrApplication> (utApps.Get (1));

  Ptr<PacketSink> gwReceiver = DynamicCast<PacketSink> (gwApps.Get (0));
  Ptr<CbrApplication> gwSender = DynamicCast<CbrApplication> (gwApps.Get (1));


  // here we check that results are as expected.
  // * Sender has sent something
  // * Receiver got all all data sent
  NS_TEST_ASSERT_MSG_NE (utSender->GetSent (), (uint32_t)0, "Nothing sent by UT user!");
  NS_TEST_ASSERT_MSG_EQ (gwReceiver->GetTotalRx (), utSender->GetSent (), "Packets were lost between UT and GW!");

  NS_TEST_ASSERT_MSG_NE (gwSender->GetSent (), (uint32_t)0, "Nothing sent by GW user!");
  NS_TEST_ASSERT_MSG_EQ (utReceiver->GetTotalRx (), gwSender->GetSent (), "Packets were lost between GW and UT!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  // <<< End of actual test using Simple scenario <<<
}


// The TestSuite class names the TestSuite as sat-perf-mem, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class PerfMemTestSuite : public TestSuite
{
public:
  PerfMemTestSuite ();
};

PerfMemTestSuite::PerfMemTestSuite ()
  : TestSuite ("sat-perf-mem", SYSTEM)
{
  // add pm-1 case to suite sat-perf-mem
  AddTestCase (new Pm1, TestCase::QUICK);
}

// Allocate an instance of this TestSuite
static PerfMemTestSuite perfMemSuite;

