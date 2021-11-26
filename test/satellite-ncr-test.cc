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
 * \file satellite-ncr-test.cc
 * \ingroup satellite
 * \brief NCR test case implementations.
 *
 * In this module implements the NCR Test Cases.
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

using namespace ns3;

/**
 * \ingroup satellite
 * \brief 'NCR, test 1' test case implementation.
 *
 * This case tests logon mechanism, and that no data is sent by UT before entering TDMA_SYNC state.
 *
 *  Expected result:
 *    GW receives nothing before logon
 *    GW always receives data after logon
 *    At the end, GW receives all data sent by UT
 */
class SatNcrTest1 : public TestCase
{
public:
  SatNcrTest1 ();
  virtual ~SatNcrTest1 ();

private:
  virtual void DoRun (void);
  void GetData (Ptr<CbrApplication> sender, Ptr<PacketSink> receiver);

  Ptr<SatHelper> m_helper;

  std::vector<int> m_totalSent;
  std::vector<int> m_totalReceived;
  std::vector<SatUtMacState::RcstState_t> m_states;
};

// Add some help text to this case to describe what it is intended to test
SatNcrTest1::SatNcrTest1 ()
  : TestCase ("This case tests logon mechanism, and that no data is sent by UT before entering TDMA_SYNC state.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatNcrTest1::~SatNcrTest1 ()
{
}

//
// SatNcrTest1 TestCase implementation
//
void
SatNcrTest1::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-ncr", "", true);

  // Set 2 RA frames including one for logon
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_LogonFrame", BooleanValue (true));

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

  Simulator::Schedule (Seconds (1), &SatNcrTest1::GetData, this, sender, receiver);

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
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender->GetSent (), "Packets were lost !");
}

void
SatNcrTest1::GetData (Ptr<CbrApplication> sender, Ptr<PacketSink> receiver)
{
  m_totalSent.push_back (sender->GetSent ());
  m_totalReceived.push_back (receiver->GetTotalRx ());
  m_states.push_back (DynamicCast<SatUtMac> (DynamicCast<SatNetDevice> (m_helper->UtNodes ().Get (0)->GetDevice (2))->GetMac ())->GetRcstState ());

  Simulator::Schedule (Seconds (1), &SatNcrTest1::GetData, this, sender, receiver);
}

/**
 * \ingroup satellite
 * \brief 'NCR, test 2' test case implementation.
 *
 * This case tests ncr recovery mechanism.
 *
 *  Expected result:
 *    UT switch to NCR_RECOVERY if NCR reception is stopped
 *    UT switch back to READY_FOR_TDMA_SYNC if NCR reception come back before timeout
 *    GW receives data only when UT in TDMA_SYNC state
 */
class SatNcrTest2 : public TestCase
{
public:
  SatNcrTest2 ();
  virtual ~SatNcrTest2 ();

private:
  virtual void DoRun (void);
  void GetData (Ptr<CbrApplication> sender, Ptr<PacketSink> receiver);
  void ChangeTxStatus (bool enable);

  Ptr<SatHelper> m_helper;

  std::vector<int> m_totalSent;
  std::vector<int> m_totalReceived;
  std::vector<SatUtMacState::RcstState_t> m_states;
  Ptr<SatChannel> m_channel;
};

// Add some help text to this case to describe what it is intended to test
SatNcrTest2::SatNcrTest2 ()
  : TestCase ("This case tests ncr recovery mechanism.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatNcrTest2::~SatNcrTest2 ()
{
}

//
// SatNcrTest2 TestCase implementation
//
void
SatNcrTest2::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-ncr", "", true);

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));

  // Set 2 RA frames including one for logon
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_RandomAccessFrame", BooleanValue (true));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame1_LogonFrame", BooleanValue (true));

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

  Simulator::Schedule (Seconds (1), &SatNcrTest2::GetData, this, sender, receiver);

  // Schedule GW transmitter switch off and switch on
  Simulator::Schedule (Seconds (30), &SatNcrTest2::ChangeTxStatus, this, false);
  Simulator::Schedule (Seconds (38), &SatNcrTest2::ChangeTxStatus, this, true);

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

  // State is never OFF_STANDBY after logon
  for (uint32_t i = indexSwitchTdmaSync; i < m_states.size () - 1; i++)
    {
      NS_TEST_ASSERT_MSG_NE (m_states[i], SatUtMacState::OFF_STANDBY, "UT should not switch to OFF_STANDBY after NCR_RECOVERY");
    }

  // State is NCR RECOVERY between 32s and 38s
  for (uint32_t i = 31; i < 39; i++)
    {
      NS_TEST_ASSERT_MSG_EQ (m_states[i], SatUtMacState::NCR_RECOVERY, "UT should be in NCR_RECOVERY after loss of NCR");
    }

  uint32_t receivedBeforeNcrRecovery = m_totalReceived[32];
  // Nothing received between 30s and 38s
  for (uint32_t i = 31; i < 39; i++)
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t) m_totalReceived[i], receivedBeforeNcrRecovery, "Receiver should not receive anything between 31s and 38s");
    }

  // Receiver has always received data after recovering NCR messages
  for (uint32_t i = 39; i < m_totalReceived.size () - 1; i++)
    {
      NS_TEST_ASSERT_MSG_GT (m_totalReceived[i+1], m_totalReceived[i], "Receiver should always receive data after logon");
    }

  // At the end, receiver got all all data sent
  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), sender->GetSent (), "Packets were lost !");
}

void
SatNcrTest2::GetData (Ptr<CbrApplication> sender, Ptr<PacketSink> receiver)
{
  m_totalSent.push_back (sender->GetSent ());
  m_totalReceived.push_back (receiver->GetTotalRx ());
  m_states.push_back (DynamicCast<SatUtMac> (DynamicCast<SatNetDevice> (m_helper->UtNodes ().Get (0)->GetDevice (2))->GetMac ())->GetRcstState ());

  Simulator::Schedule (Seconds (1), &SatNcrTest2::GetData, this, sender, receiver);
}

void
SatNcrTest2::ChangeTxStatus (bool enable)
{
  if (enable)
    {
      DynamicCast<SatGwMac> (DynamicCast<SatNetDevice> (m_helper->GwNodes ().Get (0)->GetDevice (1))->GetMac ())->SetAttribute ("NcrBroadcastPeriod", TimeValue (MilliSeconds (100)));
    }
  else
    {
      DynamicCast<SatGwMac> (DynamicCast<SatNetDevice> (m_helper->GwNodes ().Get (0)->GetDevice (1))->GetMac ())->SetAttribute ("NcrBroadcastPeriod", TimeValue (Seconds (9)));
    }
}

// The TestSuite class names the TestSuite as sat-ncr-test, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class SatNcrTestSuite : public TestSuite
{
public:
  SatNcrTestSuite ();
};

SatNcrTestSuite::SatNcrTestSuite ()
  : TestSuite ("sat-ncr-test", SYSTEM)
{
  AddTestCase (new SatNcrTest1, TestCase::QUICK);
  AddTestCase (new SatNcrTest2, TestCase::QUICK);
}

// Allocate an instance of this TestSuite
static SatNcrTestSuite satNcrTestSuite;

