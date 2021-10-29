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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

//TODO change mobitility words to lora

/**
 * \ingroup satellite
 * \file satellite-mobility-test.cc
 * \brief Test cases to unit test Satellite Mobility.
 */

// Include a header file from your module to test.
#include <iostream>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/mobility-helper.h"
#include "../model/satellite-mobility-model.h"
#include "../model/satellite-position-allocator.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

#include <ns3/enum.h>
#include <ns3/uinteger.h>

#include <ns3/satellite-helper.h>
#include <ns3/satellite-enums.h>

#include <ns3/satellite-lorawan-net-device.h>
#include <ns3/satellite-lora-conf.h>
#include <ns3/lorawan-mac-header.h>
#include <ns3/lora-periodic-sender.h>


#include <ns3/packet-sink-helper.h>
#include <ns3/packet-sink.h>
#include <ns3/cbr-application.h>
#include <ns3/cbr-helper.h>

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case to check if Lora ack arrives in first reception window.
 *
 *  Expected result:
 *    Ack is received and with correct date range, corresponding to first window opening and closing.
 *
 */
class SatLoraFirstWindowTestCase : public TestCase
{
public:
  SatLoraFirstWindowTestCase ();
  virtual ~SatLoraFirstWindowTestCase ();

private:
  virtual void DoRun (void);
  void MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address);

  Time m_gwReceiveDate;
  Time m_edReceiveDate;

  Address m_gwAddress;
  Address m_edAddress;
};

SatLoraFirstWindowTestCase::SatLoraFirstWindowTestCase ()
  : TestCase ("Test satellite lorawan with acks sent in first window."),
  m_gwReceiveDate (Seconds(0)),
  m_edReceiveDate (Seconds(0))
{
}

SatLoraFirstWindowTestCase::~SatLoraFirstWindowTestCase ()
{
}

void
SatLoraFirstWindowTestCase::MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address)
{
  if (address == m_edAddress)
    {
      m_gwReceiveDate = Simulator::Now ();
    }

  if (address == m_gwAddress)
    {
      m_edReceiveDate = Simulator::Now ();
    }
}

void
SatLoraFirstWindowTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-lora", "first-window", true);

  // Enable Lora
  Config::SetDefault ("ns3::SatHelper::Standard", EnumValue (SatEnums::LORA));
  Config::SetDefault ("ns3::LorawanMacEndDevice::DataRate", UintegerValue (5));
  Config::SetDefault ("ns3::LorawanMacEndDevice::MType", EnumValue (LorawanMacHeader::CONFIRMED_DATA_UP));
  Config::SetDefault ("ns3::SatLorawanNetDevice::ForwardToUtUsers", BooleanValue (false));
  Config::SetDefault ("ns3::SatLoraConf::Standard", EnumValue (SatLoraConf::SATELLITE));

  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay", TimeValue (MilliSeconds (1500)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue (Seconds (2)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration", TimeValue (MilliSeconds (400)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration", TimeValue (MilliSeconds (400)));
  Config::SetDefault ("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue (Seconds (1)));
  Config::SetDefault ("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue (Seconds (2)));

  // Superframe configuration
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (15000));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (15000));

  // CRDSA only
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));

  // Configure RA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_LORA));
  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2));
  Config::SetDefault ("ns3::SatHelper::RtnLinkWaveformConfFileName", StringValue("loraWaveforms.txt"));

  // Configure E-SSA
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("600ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("200ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (5));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));

  Config::SetDefault ("ns3::SatMac::EnableStatisticsTags", BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  // >>> Start of actual test using Simple scenario >>>
  Ptr<Node> utNode = helper->UtNodes ().Get (0);
  Ptr<LoraPeriodicSender> app = Create<LoraPeriodicSender> ();

  app->SetInterval (Seconds (10));

  app->SetStartTime (Seconds (1.0));
  app->SetStopTime (Seconds (10.0));
  app->SetPacketSize (24);

  app->SetNode (utNode);
  utNode->AddApplication (app);

  m_gwAddress = helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();
  m_edAddress = helper->UtNodes ().Get (0)->GetDevice (2)->GetAddress ();

  Config::Connect ("/NodeList/*/DeviceList/*/SatMac/Rx", MakeCallback (&SatLoraFirstWindowTestCase::MacTraceCb, this));

  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  NS_TEST_ASSERT_MSG_NE (m_gwReceiveDate, Seconds (0), "Packet should be received by Gateway.");
  NS_TEST_ASSERT_MSG_NE (m_edReceiveDate, Seconds (0), "Ack should be received by End Device.");
  NS_TEST_ASSERT_MSG_GT (m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

  Time difference = m_edReceiveDate - m_gwReceiveDate;
  Time delay = MilliSeconds (130);

  NS_TEST_ASSERT_MSG_GT (difference, Seconds (1) + delay, "Ack arrived too early.");
  NS_TEST_ASSERT_MSG_LT (difference + delay, MilliSeconds (1900) + delay, "Ack arrived too late. First window should be closed.");
}

/**
 * \ingroup satellite
 * \brief Test case to check if Lora ack arrives in second reception window.
 *
 *  Expected result:
 *    Ack is received and with correct date range, corresponding to second window opening and closing.
 *
 */
class SatLoraSecondWindowTestCase : public TestCase
{
public:
  SatLoraSecondWindowTestCase ();
  virtual ~SatLoraSecondWindowTestCase ();

private:
  virtual void DoRun (void);
  void MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address);

  Time m_gwReceiveDate;
  Time m_edReceiveDate;

  Address m_gwAddress;
  Address m_edAddress;
};

SatLoraSecondWindowTestCase::SatLoraSecondWindowTestCase ()
  : TestCase ("Test satellite lorawan with acks sent in second window."),
  m_gwReceiveDate (Seconds(0)),
  m_edReceiveDate (Seconds(0))
{
}

SatLoraSecondWindowTestCase::~SatLoraSecondWindowTestCase ()
{
}

void
SatLoraSecondWindowTestCase::MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address)
{
  if (address == m_edAddress)
    {
      m_gwReceiveDate = Simulator::Now ();
    }

  if (address == m_gwAddress)
    {
      m_edReceiveDate = Simulator::Now ();
    }
}

void
SatLoraSecondWindowTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-lora", "second-window", true);

  // Enable Lora
  Config::SetDefault ("ns3::SatHelper::Standard", EnumValue (SatEnums::LORA));
  Config::SetDefault ("ns3::LorawanMacEndDevice::DataRate", UintegerValue (5));
  Config::SetDefault ("ns3::LorawanMacEndDevice::MType", EnumValue (LorawanMacHeader::CONFIRMED_DATA_UP));
  Config::SetDefault ("ns3::SatLorawanNetDevice::ForwardToUtUsers", BooleanValue (false));
  Config::SetDefault ("ns3::SatLoraConf::Standard", EnumValue (SatLoraConf::SATELLITE));

  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay", TimeValue (MilliSeconds (1500)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue (Seconds (2)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration", TimeValue (MilliSeconds (400)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration", TimeValue (MilliSeconds (400)));
  // Increase answer delay by 500ms compared to SatLoraSecondWindowTestCase to be in second window on End Device
  Config::SetDefault ("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue (Seconds (1) + MilliSeconds (500)));
  Config::SetDefault ("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue (Seconds (2)));

  // Superframe configuration
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (15000));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (15000));

  // CRDSA only
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));

  // Configure RA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_LORA));
  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2));
  Config::SetDefault ("ns3::SatHelper::RtnLinkWaveformConfFileName", StringValue("loraWaveforms.txt"));

  // Configure E-SSA
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("600ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("200ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (5));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));

  Config::SetDefault ("ns3::SatMac::EnableStatisticsTags", BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  // >>> Start of actual test using Simple scenario >>>
  Ptr<Node> utNode = helper->UtNodes ().Get (0);
  Ptr<LoraPeriodicSender> app = Create<LoraPeriodicSender> ();

  app->SetInterval (Seconds (10));

  app->SetStartTime (Seconds (1.0));
  app->SetStopTime (Seconds (10.0));
  app->SetPacketSize (24);

  app->SetNode (utNode);
  utNode->AddApplication (app);

  m_gwAddress = helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();
  m_edAddress = helper->UtNodes ().Get (0)->GetDevice (2)->GetAddress ();

  Config::Connect ("/NodeList/*/DeviceList/*/SatMac/Rx", MakeCallback (&SatLoraSecondWindowTestCase::MacTraceCb, this));

  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
  NS_TEST_ASSERT_MSG_NE (m_gwReceiveDate, Seconds (0), "Packet should be received by Gateway.");
  NS_TEST_ASSERT_MSG_NE (m_edReceiveDate, Seconds (0), "Ack should be received by End Device.");
  NS_TEST_ASSERT_MSG_GT (m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

  Time difference = m_edReceiveDate - m_gwReceiveDate;
  Time delay = MilliSeconds (130);

  NS_TEST_ASSERT_MSG_GT (difference, Seconds (1.5) + delay, "Ack arrived too early.");
  NS_TEST_ASSERT_MSG_LT (difference + delay, MilliSeconds (2400) + delay, "Ack arrived too late. Second window should be closed.");
}

/**
 * \ingroup satellite
 * \brief Test case to check if packet retransmitted if ack outside of both windows.
 *
 *  Expected result:
 *    Ack is not received and packet is retransmitted.
 *
 */
class SatLoraOutOfWindowWindowTestCase : public TestCase
{
public:
  SatLoraOutOfWindowWindowTestCase ();
  virtual ~SatLoraOutOfWindowWindowTestCase ();

private:
  virtual void DoRun (void);
  void MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address);

  std::vector<Time> m_gwReceiveDates;
  Time m_edReceiveDate;

  Address m_gwAddress;
  Address m_edAddress;
};

SatLoraOutOfWindowWindowTestCase::SatLoraOutOfWindowWindowTestCase ()
  : TestCase ("Test satellite lorawan with acks sent in second window."),
  m_edReceiveDate (Seconds(0))
{
}

SatLoraOutOfWindowWindowTestCase::~SatLoraOutOfWindowWindowTestCase ()
{
}

void
SatLoraOutOfWindowWindowTestCase::MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address)
{
  if (address == m_edAddress)
    {
      m_gwReceiveDates.push_back (Simulator::Now ());
    }

  if (address == m_gwAddress)
    {
      m_edReceiveDate = Simulator::Now ();
    }
}

void
SatLoraOutOfWindowWindowTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-lora", "out-of-window", true);

  // Enable Lora
  Config::SetDefault ("ns3::SatHelper::Standard", EnumValue (SatEnums::LORA));
  Config::SetDefault ("ns3::LorawanMacEndDevice::DataRate", UintegerValue (5));
  Config::SetDefault ("ns3::LorawanMacEndDevice::MType", EnumValue (LorawanMacHeader::CONFIRMED_DATA_UP));
  Config::SetDefault ("ns3::SatLorawanNetDevice::ForwardToUtUsers", BooleanValue (false));
  Config::SetDefault ("ns3::SatLoraConf::Standard", EnumValue (SatLoraConf::SATELLITE));

  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay", TimeValue (MilliSeconds (1500)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue (Seconds (2)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration", TimeValue (MilliSeconds (400)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration", TimeValue (MilliSeconds (400)));
  // Send answer too early
  Config::SetDefault ("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue (Seconds (0.1)));
  Config::SetDefault ("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue (Seconds (2)));

  // Superframe configuration
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (15000));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (15000));

  // CRDSA only
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));

  // Configure RA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_LORA));
  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2));
  Config::SetDefault ("ns3::SatHelper::RtnLinkWaveformConfFileName", StringValue("loraWaveforms.txt"));

  // Configure E-SSA
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("600ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("200ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (5));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));

  Config::SetDefault ("ns3::SatMac::EnableStatisticsTags", BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  // >>> Start of actual test using Simple scenario >>>
  Ptr<Node> utNode = helper->UtNodes ().Get (0);
  Ptr<LoraPeriodicSender> app = Create<LoraPeriodicSender> ();

  app->SetInterval (Seconds (10));

  app->SetStartTime (Seconds (1.0));
  app->SetStopTime (Seconds (10.0));
  app->SetPacketSize (24);

  app->SetNode (utNode);
  utNode->AddApplication (app);

  m_gwAddress = helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();
  m_edAddress = helper->UtNodes ().Get (0)->GetDevice (2)->GetAddress ();

  Config::Connect ("/NodeList/*/DeviceList/*/SatMac/Rx", MakeCallback (&SatLoraOutOfWindowWindowTestCase::MacTraceCb, this));

  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  NS_TEST_ASSERT_MSG_EQ (m_gwReceiveDates.size (), 2, "GW should receive a packet and the first retransmission.");
  NS_TEST_ASSERT_MSG_EQ (m_edReceiveDate, Seconds (0), "No ack should be received by End Device.");
}

/**
 * \ingroup satellite
 * \brief Test case to check if ppacket is received on App layer.
 *
 *  Expected result:
 *    Rx and Sink callbacks have data.
 *
 */
class SatLoraCbrTestCase : public TestCase
{
public:
  SatLoraCbrTestCase ();
  virtual ~SatLoraCbrTestCase ();

private:
  virtual void DoRun (void);
  void MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address);

  Time m_gwReceiveDate;
  Time m_edReceiveDate;

  Address m_gwAddress;
  Address m_edAddress;
};

SatLoraCbrTestCase::SatLoraCbrTestCase ()
  : TestCase ("Test satellite lorawan with acks sent in second window."),
  m_edReceiveDate (Seconds(0))
{
}

SatLoraCbrTestCase::~SatLoraCbrTestCase ()
{
}

void
SatLoraCbrTestCase::MacTraceCb ( std::string context, Ptr<const Packet> packet, const Address & address)
{
  if (address == m_edAddress)
    {
      m_gwReceiveDate = Simulator::Now ();
    }

  if (address == m_gwAddress)
    {
      m_edReceiveDate = Simulator::Now ();
    }
}

void
SatLoraCbrTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-lora", "cbr", true);

  // Enable Lora
  Config::SetDefault ("ns3::SatHelper::Standard", EnumValue (SatEnums::LORA));
  Config::SetDefault ("ns3::LorawanMacEndDevice::DataRate", UintegerValue (5));
  Config::SetDefault ("ns3::LorawanMacEndDevice::MType", EnumValue (LorawanMacHeader::CONFIRMED_DATA_UP));
  Config::SetDefault ("ns3::SatLorawanNetDevice::ForwardToUtUsers", BooleanValue (true));
  Config::SetDefault ("ns3::SatLoraConf::Standard", EnumValue (SatLoraConf::SATELLITE));

  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay", TimeValue (MilliSeconds (1500)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue (Seconds (2)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration", TimeValue (MilliSeconds (400)));
  Config::SetDefault ("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration", TimeValue (MilliSeconds (400)));
  Config::SetDefault ("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue (Seconds (1)));
  Config::SetDefault ("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue (Seconds (2)));

  // Superframe configuration
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_4));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue (15000));
  Config::SetDefault ("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (15000));

  // CRDSA only
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));

  // Configure RA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_ESSA));
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceEliminationModel", EnumValue (SatPhyRxCarrierConf::SIC_RESIDUAL));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue (SatEnums::LR_LORA));
  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (2));
  Config::SetDefault ("ns3::SatHelper::RtnLinkWaveformConfFileName", StringValue("loraWaveforms.txt"));

  // Configure E-SSA
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue ("600ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue ("200ms"));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue (5));
  Config::SetDefault ("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue (false));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("10s"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (24));

  Config::SetDefault ("ns3::SatMac::EnableStatisticsTags", BooleanValue (true));
  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();
  InetSocketAddress gwUserAddr = InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), 9);

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", Address ());
  CbrHelper cbrHelper ("ns3::UdpSocketFactory", Address ());
  ApplicationContainer sinkContainer;
  ApplicationContainer cbrContainer;

  sinkHelper.SetAttribute ("Local", AddressValue (Address (gwUserAddr)));
  sinkContainer.Add (sinkHelper.Install (gwUsers.Get (0)));

  cbrHelper.SetAttribute ("Remote", AddressValue (Address (gwUserAddr)));

  auto app = cbrHelper.Install (utUsers.Get (0)).Get (0);
  app->SetStartTime (Seconds (1));
  cbrContainer.Add (app);

  sinkContainer.Start (Seconds (1));
  sinkContainer.Stop (Seconds (20));

  m_gwAddress = helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();
  m_edAddress = helper->UtNodes ().Get (0)->GetDevice (2)->GetAddress ();

  Ptr<PacketSink> receiver = DynamicCast<PacketSink> (sinkContainer.Get (0));

  Config::Connect ("/NodeList/*/DeviceList/*/SatMac/Rx", MakeCallback (&SatLoraCbrTestCase::MacTraceCb, this));

  Simulator::Stop (Seconds (20));
  Simulator::Run ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  NS_TEST_ASSERT_MSG_NE (m_gwReceiveDate, Seconds (0), "Packet should be received by Gateway.");
  NS_TEST_ASSERT_MSG_NE (m_edReceiveDate, Seconds (0), "Ack should be received by End Device.");
  NS_TEST_ASSERT_MSG_GT (m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

  Time difference = m_edReceiveDate - m_gwReceiveDate;
  Time delay = MilliSeconds (130);

  NS_TEST_ASSERT_MSG_GT (difference, Seconds (1) + delay, "Ack arrived too early.");
  NS_TEST_ASSERT_MSG_LT (difference + delay, MilliSeconds (1900) + delay, "Ack arrived too late. First window should be closed.");

  NS_TEST_ASSERT_MSG_EQ (receiver->GetTotalRx (), 24, "Sink should receive one packet of 24 bytes");
}

/**
 * \ingroup satellite
 * \brief Test suite for Satellite mobility unit test cases.
 */
class SatLoraTestSuite : public TestSuite
{
public:
  SatLoraTestSuite ();
};

SatLoraTestSuite::SatLoraTestSuite ()
  : TestSuite ("sat-lora-test", UNIT)
{
  AddTestCase (new SatLoraFirstWindowTestCase, TestCase::QUICK);
  AddTestCase (new SatLoraSecondWindowTestCase, TestCase::QUICK);
  AddTestCase (new SatLoraOutOfWindowWindowTestCase, TestCase::QUICK);
  AddTestCase (new SatLoraCbrTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatLoraTestSuite satLoraTestSuite;

