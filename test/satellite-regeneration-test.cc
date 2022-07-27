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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

/**
 * \file satellite-regeneration-test.cc
 * \ingroup satellite
 * \brief Regeneration test case implementations.
 *
 * In this module implements the Regeneration Test Cases.
 */

#include "ns3/string.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/cbr-application.h"
#include "ns3/cbr-helper.h"
#include "ns3/satellite-helper.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-ut-mac-state.h"
#include "ns3/satellite-gw-mac.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-geo-feeder-phy.h"
#include "ns3/satellite-geo-user-phy.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 1' test case implementation.
 *
 * This case tests that delay of packets takes into account regeneration in satellite
 *
 *  Expected result:
 *    Packet delay = propagation time + 2*transmission time (in SAT and ground station)
 */
class SatRegenerationTest1 : public TestCase
{
public:
  SatRegenerationTest1 ();
  virtual ~SatRegenerationTest1 ();

private:
  virtual void DoRun (void);
  void PhyDelayTraceCb (std::string context, const Time & time, const Address & address);
  void GeoPhyTraceDelayCb (const Time & time, const Address & address);

  Ptr<SatHelper> m_helper;

  Address m_gwAddress;
  Address m_stAddress;

  std::vector<Time> m_forwardDelay;
  std::vector<Time> m_returnDelay;
  std::vector<Time> m_geoForwardDelay;
  std::vector<Time> m_geoReturnDelay;
};

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest1::SatRegenerationTest1 ()
  : TestCase ("This case tests that delay of packets takes into account regeneration in satellite.")
{
}

void
SatRegenerationTest1::PhyDelayTraceCb (std::string context, const Time & time, const Address & address)
{
  if (address == m_gwAddress)
    {
      m_forwardDelay.push_back (time);
    }
  if (address == m_stAddress)
    {
      m_returnDelay.push_back (time);
    }
}

void
SatRegenerationTest1::GeoPhyTraceDelayCb (const Time & time, const Address & address)
{
  if (address == m_gwAddress)
    {
      m_geoForwardDelay.push_back (time);
    }
  if (address == m_stAddress)
    {
      m_geoReturnDelay.push_back (time);
    }
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest1::~SatRegenerationTest1 ()
{
}

//
// SatRegenerationTest1 TestCase implementation
//
void
SatRegenerationTest1::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-regeneration", "", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  // Enable SatMac traces
  Config::SetDefault ("ns3::SatPhy::EnableStatisticsTags", BooleanValue (true));
  Config::SetDefault ("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue (true));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("3s"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512));

  // Creating the reference system.
  m_helper = CreateObject<SatHelper> ();
  m_helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer gwUsers = m_helper->GetGwUsers ();

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 100ms)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer utApps = cbr.Install (m_helper->GetUtUsers ());
  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (5.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer gwApps = sink.Install (gwUsers);
  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (10.0));

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (gwApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (utApps.Get (0));

  m_gwAddress = m_helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();
  m_stAddress = m_helper->UtNodes ().Get (0)->GetDevice (2)->GetAddress ();

  Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetFeederPhy (8));
  Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetUserPhy (8));

  satGeoFeederPhy->TraceConnectWithoutContext ("RxDelay", MakeCallback (&SatRegenerationTest1::GeoPhyTraceDelayCb, this));
  satGeoUserPhy->TraceConnectWithoutContext ("RxDelay", MakeCallback (&SatRegenerationTest1::GeoPhyTraceDelayCb, this));
  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/RxDelay", MakeCallback (&SatRegenerationTest1::PhyDelayTraceCb, this));

  Ptr<SatChannel> feederChannel = satGeoFeederPhy->GetPhyTx ()->GetChannel ();
  Ptr<SatChannel> userChannel = satGeoUserPhy->GetPhyTx ()->GetChannel ();

  Ptr<PropagationDelayModel> feederDelayModel = feederChannel->GetPropagationDelayModel ();
  Ptr<PropagationDelayModel> userDelayModel = userChannel->GetPropagationDelayModel ();

  Time feederDelay = feederDelayModel->GetDelay (DynamicCast<SatNetDevice> (m_helper->GwNodes ().Get (0)->GetDevice (1))->GetPhy ()->GetPhyTx ()->GetMobility (),
                                                  satGeoFeederPhy->GetPhyTx ()->GetMobility ());

  Time userDelay = userDelayModel->GetDelay (DynamicCast<SatNetDevice> (m_helper->UtNodes ().Get (0)->GetDevice (2))->GetPhy ()->GetPhyTx ()->GetMobility (),
                                             satGeoUserPhy->GetPhyTx ()->GetMobility ());

  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();

  // In default configuration, we have:
  // - FWD transmission delay (NORMAL BBFrame = 32400 symbols, QPSK 1/2, BW = 104 MHz):  319 507 ns
  // - RTN transmission delay (frame = 536 symbols, QPSK 1/3, BW = 801 kHz):  668 928 ns
  // We have to remove 1 us default guard time to these delays to get transmission time.

  Time fwdTime = NanoSeconds (319507) - MicroSeconds (1);
  Time rtnTime = NanoSeconds (668928) - MicroSeconds (1);

  for (uint32_t i = 0; i < m_geoForwardDelay.size (); i++)
    {
      NS_TEST_ASSERT_MSG_EQ (m_geoForwardDelay[i] - feederDelay, fwdTime, "Transmission time on FWD FEEDER incorrect.");
    }

  for (uint32_t i = 0; i < m_geoReturnDelay.size (); i++)
    {
      NS_TEST_ASSERT_MSG_EQ (m_geoReturnDelay[i] - userDelay, rtnTime, "Transmission time on RTN USER incorrect.");
    }

  for (uint32_t i = 0; i < m_forwardDelay.size (); i++)
    {
      NS_TEST_ASSERT_MSG_EQ (m_forwardDelay[i] - userDelay, fwdTime, "Transmission time on FWD USER incorrect.");
    }

  for (uint32_t i = 0; i < m_returnDelay.size (); i++)
    {
      NS_TEST_ASSERT_MSG_EQ (m_returnDelay[i] - feederDelay, rtnTime, "Transmission time on RTN FEEDER incorrect.");
    }
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test ' test case implementation.
 *
 * This case tests physical regeneration on satellite. It is based on a LARGER scenario, with losses of uplink.
 *
 *  Expected result:
 *    All packets are received on satellite before error model is applied on uplink
 *    Packets are lost on phy layer after error model is applied on uplink
 */
class SatRegenerationTest2 : public TestCase
{
public:
  SatRegenerationTest2 ();
  virtual ~SatRegenerationTest2 ();

private:
  virtual void DoRun (void);

  Ptr<SatHelper> m_helper;
};

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest2::SatRegenerationTest2 ()
  : TestCase ("This case tests physical regeneration on satellite. It is based on a LARGER scenario, with losses of uplink.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest2::~SatRegenerationTest2 ()
{
}

//
// SatRegenerationTest2 TestCase implementation
//
void
SatRegenerationTest2::DoRun (void)
{
  // Set simulation output details
  /*Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-regeneration", "", true);

  // Set 2 RA frames including one for logon
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_LogonFrame", BooleanValue (true));

  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame2_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame3_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame4_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame5_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame6_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame7_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame8_GuardTimeSymbols", UintegerValue (4));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame9_GuardTimeSymbols", UintegerValue (4));

  Config::SetDefault ("ns3::SatUtMac::WindowInitLogon", TimeValue (Seconds (20)));
  Config::SetDefault ("ns3::SatUtMac::MaxWaitingTimeLogonResponse", TimeValue (Seconds (1)));

  // Set default values for NCR
  Config::SetDefault ("ns3::SatMac::NcrVersion2", BooleanValue (false));
  Config::SetDefault ("ns3::SatGwMac::NcrBroadcastPeriod", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::SatGwMac::UseCmt", BooleanValue (true));
  Config::SetDefault ("ns3::SatUtMacState::NcrSyncTimeout", TimeValue (Seconds (1)));
  Config::SetDefault ("ns3::SatUtMacState::NcrRecoveryTimeout", TimeValue (Seconds (10)));
  Config::SetDefault ("ns3::SatNcc::UtTimeout", TimeValue (Seconds (10)));

  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue (MilliSeconds (500)));

  Config::SetDefault ("ns3::SatUtMac::ClockDrift", IntegerValue (100));
  Config::SetDefault ("ns3::SatGwMac::CmtPeriodMin", TimeValue (MilliSeconds (550)));

  // Creating the reference system.
  m_helper = CreateObject<SatHelper> ();
  m_helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer gwUsers = m_helper->GetGwUsers ();

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 100ms)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("100ms"));
  ApplicationContainer utApps = cbr.Install (m_helper->GetUtUsers ());
  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (59.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer gwApps = sink.Install (gwUsers);
  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (60.0));

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (gwApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (utApps.Get (0));

  Simulator::Stop (Seconds (60));
  Simulator::Run ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  uint32_t indexSwitchTdmaSync = 0;
  for (uint32_t i = 0; i < m_states.size (); i++)
    {
      if (m_states[i] == SatUtMacState::TDMA_SYNC)
        {
          indexSwitchTdmaSync = i;
          break;
        }
    }

  // Check if switch to TDMA_SYNC state
  NS_TEST_ASSERT_MSG_NE (indexSwitchTdmaSync, 0, "UT should switch to TDMA_SYNC before the end of simulation");

  // Check that nothing has been received before logon
  NS_TEST_ASSERT_MSG_NE (m_totalSent[indexSwitchTdmaSync - 1], 0, "Data sent before logon");
  NS_TEST_ASSERT_MSG_EQ (m_totalReceived[indexSwitchTdmaSync - 1], 0, "Nothing received before logon");

  // Receiver has always received data after logon
  for (uint32_t i = indexSwitchTdmaSync + 1; i < m_totalReceived.size () - 1; i++)
    {
      NS_TEST_ASSERT_MSG_GT (m_totalReceived[i+1], m_totalReceived[i], "Receiver should always receive data after logon");
    }

  // At the end, receiver got all all data sent
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender->GetSent (), "Packets were lost !");*/








  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("1s"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512));

  // Creating the reference system.
  m_helper = CreateObject<SatHelper> ();
  m_helper->CreatePredefinedScenario (SatHelper::LARGER);

  NodeContainer gwUsers = m_helper->GetGwUsers ();

  // Create the Cbr application to send UDP datagrams of size
  // 512 bytes at a rate of 500 Kb/s (defaults), one packet send (interval 100ms)
  uint16_t port = 9; // Discard port (RFC 863)
  CbrHelper cbr ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  cbr.SetAttribute ("Interval", StringValue ("40s"));
  ApplicationContainer utApps = cbr.Install (m_helper->GetUtUsers ());
  utApps.Start (Seconds (1.0));
  utApps.Stop (Seconds (59.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer gwApps = sink.Install (gwUsers);
  gwApps.Start (Seconds (1.0));
  gwApps.Stop (Seconds (60.0));

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (gwApps.Get (0));
  Ptr<CbrApplication> sender = DynamicCast<CbrApplication> (utApps.Get (0));

  Simulator::Stop (Seconds (60));
  Simulator::Run ();

  Simulator::Destroy ();

  Ptr<Node> satelliteNode = m_helper->GeoSatNode ();
  std::cout << satelliteNode << std::endl;
}

// The TestSuite class names the TestSuite as sat-regeneration-test, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class SatRegenerationTestSuite : public TestSuite
{
public:
  SatRegenerationTestSuite ();
};

SatRegenerationTestSuite::SatRegenerationTestSuite ()
  : TestSuite ("sat-regeneration-test", SYSTEM)
{
  AddTestCase (new SatRegenerationTest1, TestCase::QUICK); // Test delay
  // AddTestCase (new SatRegenerationTest2, TestCase::QUICK); // Test losses regeneration phy
  // TODO Test collisions regeneration phy
}

// Allocate an instance of this TestSuite
static SatRegenerationTestSuite satRegenerationTestSuite;

