/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

/**
 * \file satellite-periodic-control-message-test.cc
 * \ingroup satellite
 * \brief Periodic control message test case implementations.
 *
 * In this module implements the Periodic control message test cases
 * defined in TN6.
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
 * \brief 'Periodic control message, test 1' test case implementation.
 *
 * This case tests successful transmission of UDP packets from UT connected user
 * to GW connected user in simple scenario and using periodic control slots and VBDC only.
 *  1.  Simple test scenario set with helper
 *  2.  A single data packet is generated
 *  3.  Capacity request is generated for the packet and transmitted from Node-2 UDP application to Node-1 UDP receiver using periodic control message slots
 *  4.  Data packet is transmitted from Node-2 UDP application to Node-1 UDP receiver using the requested VDBC allocation
 *
 *  Expected result:
 *    One UDP packet sent by UT connected node-2 using CBR application is received by
 *    GW connected node-1.
 */
class SatPeriodicControlMessageTest1 : public TestCase
{
public:
  SatPeriodicControlMessageTest1 ();
  virtual ~SatPeriodicControlMessageTest1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
SatPeriodicControlMessageTest1::SatPeriodicControlMessageTest1 ()
  : TestCase ("'Periodic control message, test 1' case tests successful transmission of UDP packets from UT connected user to GW connected user in simple scenario with periodic control messages.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatPeriodicControlMessageTest1::~SatPeriodicControlMessageTest1 ()
{
}

//
// SatPeriodicControlMessageTest1 TestCase implementation
//
void
SatPeriodicControlMessageTest1::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-periodic-control-message", "", true);

  // Create simple scenario

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Disable Random Access
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel",EnumValue (SatEnums::RA_MODEL_OFF));

  // Enable periodic control slots
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));

  // Enable CRA and disable DA
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));

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
  gwApps.Stop (Seconds (10.0));

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

// The TestSuite class names the TestSuite as sat-periodic-control-message-test, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class SatPeriodicControlMessageTestSuite : public TestSuite
{
public:
  SatPeriodicControlMessageTestSuite ();
};

SatPeriodicControlMessageTestSuite::SatPeriodicControlMessageTestSuite ()
  : TestSuite ("sat-periodic-control-message-test", SYSTEM)
{
  AddTestCase (new SatPeriodicControlMessageTest1, TestCase::QUICK);
}

// Allocate an instance of this TestSuite
static SatPeriodicControlMessageTestSuite satPeriodicControlMessageTestSuite;

