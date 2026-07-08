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

/**
 * @ingroup satellite
 * @file satellite-mobility-test.cc
 * @brief Test cases to unit test Satellite Mobility.
 */

// Include a header file from your module to test.
#include "ns3/boolean.h"
#include "ns3/cbr-application.h"
#include "ns3/cbr-helper.h"
#include "ns3/config.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/lora-periodic-sender.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/mobility-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/satellite-enums.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-helper.h"
#include "ns3/satellite-lora-conf.h"
#include "ns3/satellite-lorawan-net-device.h"
#include "ns3/satellite-mobility-model.h"
#include "ns3/satellite-position-allocator.h"
#include "ns3/satellite-topology.h"
#include "ns3/simulation-helper.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

#include <iostream>
#include <stdint.h>

using namespace ns3;

/**
 * @ingroup satellite
 * @brief Test case to check if Lora ack arrives in first reception window.
 *
 *  Expected result:
 *    Ack is received and with correct date range, corresponding to first window opening and
 * closing.
 *
 */
class SatLoraRegenerativeFirstWindowTestCase : public TestCase
{
  public:
    SatLoraRegenerativeFirstWindowTestCase();
    virtual ~SatLoraRegenerativeFirstWindowTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    Time m_gwReceiveDate;
    Time m_edReceiveDate;

    Address m_edAddress;
    Address m_orbiterUserAddress;
};

SatLoraRegenerativeFirstWindowTestCase::SatLoraRegenerativeFirstWindowTestCase()
    : TestCase("Test regenerative satellite lorawan with acks sent in first window."),
      m_gwReceiveDate(Seconds(0)),
      m_edReceiveDate(Seconds(0))
{
}

SatLoraRegenerativeFirstWindowTestCase::~SatLoraRegenerativeFirstWindowTestCase()
{
}

void
SatLoraRegenerativeFirstWindowTestCase::MacTraceCb(std::string context,
                                                   Ptr<const Packet> packet,
                                                   const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDate = Simulator::Now();
    }

    if (address == m_orbiterUserAddress)
    {
        m_edReceiveDate = Simulator::Now();
    }
}

void
SatLoraRegenerativeFirstWindowTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();
    SatEnvVariables::GetInstance()->SetOutputVariables("test-sat-lora-regenerative",
                                                       "first-window",
                                                       true);

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay",
                       TimeValue(MilliSeconds(2500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(600)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue(Seconds(1)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));

    // Creating the reference system.
    Ptr<SatHelper> helper = CreateObject<SatHelper>(
        SatEnvVariables::GetInstance()->LocateDataDirectory() + "/scenarios/geo-33E-lora");
    helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    // >>> Start of actual test using Simple scenario >>>
    Ptr<Node> utNode = Singleton<SatTopology>::Get()->GetUtNode(0);
    Ptr<LoraPeriodicSender> app = CreateObject<LoraPeriodicSender>();

    app->SetInterval(Seconds(10));

    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(10.0));
    app->SetPacketSize(24);

    app->SetNode(utNode);
    utNode->AddApplication(app);

    Ptr<SatOrbiterNetDevice> orbiterNetDevice = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();
    m_orbiterUserAddress = orbiterNetDevice->GetSatelliteUserAddress(8);

    Config::Connect("/NodeList/*/DeviceList/*/SatMac/Rx",
                    MakeCallback(&SatLoraRegenerativeFirstWindowTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederMac/*/Rx",
                    MakeCallback(&SatLoraRegenerativeFirstWindowTestCase::MacTraceCb, this));

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();

    NS_TEST_ASSERT_MSG_NE(m_gwReceiveDate, Seconds(0), "Packet should be received by Gateway.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate, Seconds(0), "Ack should be received by End Device.");
    NS_TEST_ASSERT_MSG_GT(m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

    Time difference = m_edReceiveDate - m_gwReceiveDate;
    Time delay = MilliSeconds(130);

    NS_TEST_ASSERT_MSG_GT(difference, Seconds(1) + delay, "Ack arrived too early.");
    NS_TEST_ASSERT_MSG_LT(difference + delay,
                          MilliSeconds(1900) + delay,
                          "Ack arrived too late. First window should be closed.");
}

/**
 * @ingroup satellite
 * @brief Test case to check if Lora ack arrives in second reception window.
 *
 *  Expected result:
 *    Ack is received and with correct date range, corresponding to second window opening and
 * closing.
 *
 */
class SatLoraRegenerativeSecondWindowTestCase : public TestCase
{
  public:
    SatLoraRegenerativeSecondWindowTestCase();
    virtual ~SatLoraRegenerativeSecondWindowTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    Time m_gwReceiveDate;
    Time m_edReceiveDate;

    Address m_edAddress;
    Address m_orbiterUserAddress;
};

SatLoraRegenerativeSecondWindowTestCase::SatLoraRegenerativeSecondWindowTestCase()
    : TestCase("Test regenerative satellite lorawan with acks sent in second window."),
      m_gwReceiveDate(Seconds(0)),
      m_edReceiveDate(Seconds(0))
{
}

SatLoraRegenerativeSecondWindowTestCase::~SatLoraRegenerativeSecondWindowTestCase()
{
}

void
SatLoraRegenerativeSecondWindowTestCase::MacTraceCb(std::string context,
                                                    Ptr<const Packet> packet,
                                                    const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDate = Simulator::Now();
    }

    if (address == m_orbiterUserAddress)
    {
        m_edReceiveDate = Simulator::Now();
    }
}

void
SatLoraRegenerativeSecondWindowTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();
    SatEnvVariables::GetInstance()->SetOutputVariables("test-sat-lora-regenerative",
                                                       "second-window",
                                                       true);

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue(Seconds(2)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(400)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    // Increase answer delay by 500ms compared to SatLoraRegenerativeSecondWindowTestCase to be in
    // second window on End Device
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay",
                       TimeValue(Seconds(1) + MilliSeconds(500)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));

    // Creating the reference system.
    Ptr<SatHelper> helper = CreateObject<SatHelper>(
        SatEnvVariables::GetInstance()->LocateDataDirectory() + "/scenarios/geo-33E-lora");
    helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    // >>> Start of actual test using Simple scenario >>>
    Ptr<Node> utNode = Singleton<SatTopology>::Get()->GetUtNode(0);
    Ptr<LoraPeriodicSender> app = CreateObject<LoraPeriodicSender>();

    app->SetInterval(Seconds(10));

    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(10.0));
    app->SetPacketSize(24);

    app->SetNode(utNode);
    utNode->AddApplication(app);

    Ptr<SatOrbiterNetDevice> orbiterNetDevice = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();
    m_orbiterUserAddress = orbiterNetDevice->GetSatelliteUserAddress(8);

    Config::Connect("/NodeList/*/DeviceList/*/SatMac/Rx",
                    MakeCallback(&SatLoraRegenerativeSecondWindowTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederMac/*/Rx",
                    MakeCallback(&SatLoraRegenerativeSecondWindowTestCase::MacTraceCb, this));

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();
    NS_TEST_ASSERT_MSG_NE(m_gwReceiveDate, Seconds(0), "Packet should be received by Gateway.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate, Seconds(0), "Ack should be received by End Device.");
    NS_TEST_ASSERT_MSG_GT(m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

    Time difference = m_edReceiveDate - m_gwReceiveDate;
    Time delay = MilliSeconds(130);

    NS_TEST_ASSERT_MSG_GT(difference, Seconds(1.5) + delay, "Ack arrived too early.");
    NS_TEST_ASSERT_MSG_LT(difference + delay,
                          MilliSeconds(2400) + delay,
                          "Ack arrived too late. Second window should be closed.");
}

/**
 * @ingroup satellite
 * @brief Test case to check if packet retransmitted if ack outside of both windows.
 *
 *  Expected result:
 *    Ack is not received and packet is retransmitted.
 *
 */
class SatLoraRegenerativeOutOfWindowWindowTestCase : public TestCase
{
  public:
    SatLoraRegenerativeOutOfWindowWindowTestCase();
    virtual ~SatLoraRegenerativeOutOfWindowWindowTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);
    void PhyTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    std::vector<Time> m_gwReceiveDates;
    Time m_edReceiveDate;

    Address m_edAddress;
    Address m_orbiterUserAddress;

    bool m_phyGwReceive;
    bool m_phyEdReceive;
};

SatLoraRegenerativeOutOfWindowWindowTestCase::SatLoraRegenerativeOutOfWindowWindowTestCase()
    : TestCase("Test regenerative satellite lorawan with acks sent out of reception windows."),
      m_edReceiveDate(Seconds(0)),
      m_phyGwReceive(false),
      m_phyEdReceive(false)
{
}

SatLoraRegenerativeOutOfWindowWindowTestCase::~SatLoraRegenerativeOutOfWindowWindowTestCase()
{
}

void
SatLoraRegenerativeOutOfWindowWindowTestCase::MacTraceCb(std::string context,
                                                         Ptr<const Packet> packet,
                                                         const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDates.push_back(Simulator::Now());
    }

    if (address == m_orbiterUserAddress)
    {
        m_edReceiveDate = Simulator::Now();
    }
}

void
SatLoraRegenerativeOutOfWindowWindowTestCase::PhyTraceCb(std::string context,
                                                         Ptr<const Packet> packet,
                                                         const Address& address)
{
    if (address == m_edAddress)
    {
        m_phyGwReceive = true;
    }

    if (address == m_orbiterUserAddress)
    {
        m_phyEdReceive = true;
    }
}

void
SatLoraRegenerativeOutOfWindowWindowTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();
    SatEnvVariables::GetInstance()->SetOutputVariables("test-sat-lora-regenerative",
                                                       "out-of-window",
                                                       true);

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue(Seconds(2)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(400)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    // Send answer too early
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay",
                       TimeValue(Seconds(0.1)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));

    // Creating the reference system.
    Ptr<SatHelper> helper = CreateObject<SatHelper>(
        SatEnvVariables::GetInstance()->LocateDataDirectory() + "/scenarios/geo-33E-lora");
    helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    // >>> Start of actual test using Simple scenario >>>
    Ptr<Node> utNode = Singleton<SatTopology>::Get()->GetUtNode(0);
    Ptr<LoraPeriodicSender> app = CreateObject<LoraPeriodicSender>();

    app->SetInterval(Seconds(10));

    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(10.0));
    app->SetPacketSize(24);

    app->SetNode(utNode);
    utNode->AddApplication(app);

    Ptr<SatOrbiterNetDevice> orbiterNetDevice = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();
    m_orbiterUserAddress = orbiterNetDevice->GetSatelliteUserAddress(8);

    Config::Connect("/NodeList/*/DeviceList/*/SatMac/Rx",
                    MakeCallback(&SatLoraRegenerativeOutOfWindowWindowTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederMac/*/Rx",
                    MakeCallback(&SatLoraRegenerativeOutOfWindowWindowTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/SatPhy/Rx",
                    MakeCallback(&SatLoraRegenerativeOutOfWindowWindowTestCase::PhyTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederPhy/*/Rx",
                    MakeCallback(&SatLoraRegenerativeOutOfWindowWindowTestCase::PhyTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/UserPhy/*/Rx",
                    MakeCallback(&SatLoraRegenerativeOutOfWindowWindowTestCase::PhyTraceCb, this));

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();

    NS_TEST_ASSERT_MSG_EQ(m_gwReceiveDates.size(),
                          2,
                          "GW should receive a packet and the first retransmission.");
    NS_TEST_ASSERT_MSG_EQ(m_edReceiveDate, Seconds(0), "No ack should be received by End Device.");

    NS_TEST_ASSERT_MSG_EQ(m_phyGwReceive,
                          true,
                          "Phy layer should trace traffic from End Device to Gateway.");
    NS_TEST_ASSERT_MSG_EQ(m_phyEdReceive,
                          false,
                          "Phy layer should not trace traffic from Gateway to End Device, as phy "
                          "layer is in SLEEP state.");
}

/**
 * @ingroup satellite
 * @brief Test case to check that packet is not retransmitted if ack outside of both windows but no
 * retransmission asked.
 *
 *  Expected result:
 *    Ack is not received and packet is not retransmitted.
 *
 */
class SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase : public TestCase
{
  public:
    SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase();
    virtual ~SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);
    void PhyTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    std::vector<Time> m_gwReceiveDates;
    Time m_edReceiveDate;

    Address m_edAddress;
    Address m_orbiterUserAddress;
};

SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase::
    SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase()
    : TestCase("Test regenerative satellite lorawan with acks sent out of reception windows and no "
               "retransmission needed."),
      m_edReceiveDate(Seconds(0))
{
}

SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase::
    ~SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase()
{
}

void
SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase::MacTraceCb(std::string context,
                                                                         Ptr<const Packet> packet,
                                                                         const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDates.push_back(Simulator::Now());
    }

    if (address == m_orbiterUserAddress)
    {
        m_edReceiveDate = Simulator::Now();
    }
}

void
SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();
    SatEnvVariables::GetInstance()->SetOutputVariables("test-sat-lora-regenerative",
                                                       "out-of-window",
                                                       true);

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::UNCONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue(Seconds(2)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(400)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    // Send answer too early
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay",
                       TimeValue(Seconds(0.1)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));

    // Creating the reference system.
    Ptr<SatHelper> helper = CreateObject<SatHelper>(
        SatEnvVariables::GetInstance()->LocateDataDirectory() + "/scenarios/geo-33E-lora");
    helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    // >>> Start of actual test using Simple scenario >>>
    Ptr<Node> utNode = Singleton<SatTopology>::Get()->GetUtNode(0);
    Ptr<LoraPeriodicSender> app = CreateObject<LoraPeriodicSender>();

    app->SetInterval(Seconds(10));

    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(10.0));
    app->SetPacketSize(24);

    app->SetNode(utNode);
    utNode->AddApplication(app);

    Ptr<SatOrbiterNetDevice> orbiterNetDevice = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();
    m_orbiterUserAddress = orbiterNetDevice->GetSatelliteUserAddress(8);

    Config::Connect(
        "/NodeList/*/DeviceList/*/SatMac/Rx",
        MakeCallback(&SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase::MacTraceCb,
                     this));
    Config::Connect(
        "/NodeList/*/DeviceList/*/FeederMac/*/Rx",
        MakeCallback(&SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase::MacTraceCb,
                     this));

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();

    NS_TEST_ASSERT_MSG_EQ(m_gwReceiveDates.size(),
                          1,
                          "GW should receive a packet but no retransmission.");
    NS_TEST_ASSERT_MSG_EQ(m_edReceiveDate, Seconds(0), "No ack should be received by End Device.");
}

/**
 * @ingroup satellite
 * @brief Test case to check if packet is received on App layer.
 *
 *  Expected result:
 *    Rx and Sink callbacks have data.
 *
 */
class SatLoraRegenerativeCbrTestCase : public TestCase
{
  public:
    SatLoraRegenerativeCbrTestCase();
    virtual ~SatLoraRegenerativeCbrTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    Time m_gwReceiveDate;
    Time m_edReceiveDate;

    Address m_edAddress;
    Address m_orbiterUserAddress;
};

SatLoraRegenerativeCbrTestCase::SatLoraRegenerativeCbrTestCase()
    : TestCase("Test regenerativesatellite lorawan with CBR traffic."),
      m_edReceiveDate(Seconds(0))
{
}

SatLoraRegenerativeCbrTestCase::~SatLoraRegenerativeCbrTestCase()
{
}

void
SatLoraRegenerativeCbrTestCase::MacTraceCb(std::string context,
                                           Ptr<const Packet> packet,
                                           const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDate = Simulator::Now();
    }

    if (address == m_orbiterUserAddress)
    {
        m_edReceiveDate = Simulator::Now();
    }
}

void
SatLoraRegenerativeCbrTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();
    SatEnvVariables::GetInstance()->SetOutputVariables("test-sat-lora-regenerative", "cbr", true);

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue(Seconds(2)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(450)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue(Seconds(1)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::CbrApplication::Interval", StringValue("10s"));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(24));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));

    // Creating the reference system.
    Ptr<SatHelper> helper = CreateObject<SatHelper>(
        SatEnvVariables::GetInstance()->LocateDataDirectory() + "/scenarios/geo-33E-lora");
    helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    NodeContainer utUsers = Singleton<SatTopology>::Get()->GetUtUserNodes();
    NodeContainer gwUsers = Singleton<SatTopology>::Get()->GetGwUserNodes();
    InetSocketAddress gwUserAddr = InetSocketAddress(helper->GetUserAddress(gwUsers.Get(0)), 9);

    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", Address());
    CbrHelper cbrHelper("ns3::UdpSocketFactory", Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer cbrContainer;

    sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
    sinkContainer.Add(sinkHelper.Install(gwUsers.Get(0)));

    cbrHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));

    auto app = cbrHelper.Install(utUsers.Get(0)).Get(0);
    app->SetStartTime(Seconds(1));
    cbrContainer.Add(app);

    sinkContainer.Start(Seconds(1));
    sinkContainer.Stop(Seconds(20));

    Ptr<SatOrbiterNetDevice> orbiterNetDevice = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();
    m_orbiterUserAddress = orbiterNetDevice->GetSatelliteUserAddress(8);

    Ptr<PacketSink> receiver = DynamicCast<PacketSink>(sinkContainer.Get(0));

    Config::Connect("/NodeList/*/DeviceList/*/SatMac/Rx",
                    MakeCallback(&SatLoraRegenerativeCbrTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederMac/*/Rx",
                    MakeCallback(&SatLoraRegenerativeCbrTestCase::MacTraceCb, this));

    Simulator::Stop(Seconds(20));
    Simulator::Run();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();

    NS_TEST_ASSERT_MSG_NE(m_gwReceiveDate, Seconds(0), "Packet should be received by Gateway.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate, Seconds(0), "Ack should be received by End Device.");
    NS_TEST_ASSERT_MSG_GT(m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

    Time difference = m_edReceiveDate - m_gwReceiveDate;
    Time delay = MilliSeconds(130);

    NS_TEST_ASSERT_MSG_GT(difference, Seconds(1) + delay, "Ack arrived too early.");
    NS_TEST_ASSERT_MSG_LT(difference + delay,
                          MilliSeconds(1900) + delay,
                          "Ack arrived too late. First window should be closed.");

    NS_TEST_ASSERT_MSG_EQ(receiver->GetTotalRx(), 24, "Sink should receive one packet of 24 bytes");
}

/**
 * @ingroup satellite
 * @brief Test case to check if Lora ack arrives in first reception window when using a
 * constellation
 *
 *  Expected result:
 *    Ack is received and with correct date range, corresponding to first window opening and
 * closing.
 *
 */
class SatLoraConstellationFirstWindowTestCase : public TestCase
{
  public:
    SatLoraConstellationFirstWindowTestCase();
    virtual ~SatLoraConstellationFirstWindowTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    Time m_gwReceiveDate;
    Time m_edReceiveDate;

    Address m_edAddress;
    Address m_orbiterUserAddress;
};

SatLoraConstellationFirstWindowTestCase::SatLoraConstellationFirstWindowTestCase()
    : TestCase("Test constellation satellite lorawan with acks sent in first window."),
      m_gwReceiveDate(Seconds(0)),
      m_edReceiveDate(Seconds(0))
{
}

SatLoraConstellationFirstWindowTestCase::~SatLoraConstellationFirstWindowTestCase()
{
}

void
SatLoraConstellationFirstWindowTestCase::MacTraceCb(std::string context,
                                                    Ptr<const Packet> packet,
                                                    const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDate = Simulator::Now();
    }

    if (address == m_orbiterUserAddress)
    {
        m_edReceiveDate = Simulator::Now();
    }
}

void
SatLoraConstellationFirstWindowTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-lora-regenerative/constellation-first-window");

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1500)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue(Seconds(2)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(400)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue(Seconds(1)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));

    // Traffics
    simulationHelper->SetSimulationTime(Seconds(15.0));

    simulationHelper->SetGwUserCount(1);
    simulationHelper->SetUtCountPerBeam(1);
    simulationHelper->SetUserCountPerUt(1);
    simulationHelper->SetBeams("30 43");

    simulationHelper->LoadScenario("constellation-eutelsat-geo-2-sats-isls-lora");

    simulationHelper->CreateSatScenario();

    // >>> Start of actual test using Simple scenario >>>
    Ptr<Node> utNode = Singleton<SatTopology>::Get()->GetUtNode(0);
    Ptr<LoraPeriodicSender> app = CreateObject<LoraPeriodicSender>();

    app->SetInterval(Seconds(10));

    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(10.0));
    app->SetPacketSize(24);

    app->SetNode(utNode);
    utNode->AddApplication(app);

    Ptr<SatOrbiterNetDevice> orbiterNetDevice0 = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    Ptr<SatOrbiterNetDevice> orbiterNetDevice1 = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(1)->GetDevice(0));
    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();
    m_orbiterUserAddress = orbiterNetDevice0->GetSatelliteUserAddress(43);

    Config::Connect("/NodeList/*/DeviceList/*/SatMac/Rx",
                    MakeCallback(&SatLoraConstellationFirstWindowTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederMac/*/Rx",
                    MakeCallback(&SatLoraConstellationFirstWindowTestCase::MacTraceCb, this));

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();

    NS_TEST_ASSERT_MSG_NE(m_gwReceiveDate, Seconds(0), "Packet should be received by Gateway.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate, Seconds(0), "Ack should be received by End Device.");
    NS_TEST_ASSERT_MSG_GT(m_edReceiveDate, m_gwReceiveDate, "Ack should be received after packet.");

    Time difference = m_edReceiveDate - m_gwReceiveDate;
    Time delay = MilliSeconds(130);

    NS_TEST_ASSERT_MSG_GT(difference, Seconds(1) + delay, "Ack arrived too early.");
    NS_TEST_ASSERT_MSG_LT(difference + delay,
                          MilliSeconds(1900) + delay,
                          "Ack arrived too late. First window should be closed.");
}

/**
 * @ingroup satellite
 * @brief Test case to check if Lora still works after hadovers
 *
 *  Expected result:
 *    Acks are received during all simulation
 *    Satellite ID and beam change over the simulation: 1/41 -> 1/40 -> 0/31
 *
 */
class SatLoraConstellationHandoverTestCase : public TestCase
{
  public:
    SatLoraConstellationHandoverTestCase();
    virtual ~SatLoraConstellationHandoverTestCase();

  private:
    virtual void DoRun(void);
    void MacTraceCb(std::string context, Ptr<const Packet> packet, const Address& address);

    Time m_gwReceiveDate;
    Time m_edReceiveDate31;
    Time m_edReceiveDate40;
    Time m_edReceiveDate41;

    Address m_edAddress;
    Address m_orbiterUserAddress31;
    Address m_orbiterUserAddress40;
    Address m_orbiterUserAddress41;
};

SatLoraConstellationHandoverTestCase::SatLoraConstellationHandoverTestCase()
    : TestCase("Test constellation satellite lorawan with handovers."),
      m_gwReceiveDate(Seconds(0)),
      m_edReceiveDate31(Seconds(0)),
      m_edReceiveDate40(Seconds(0)),
      m_edReceiveDate41(Seconds(0))
{
}

SatLoraConstellationHandoverTestCase::~SatLoraConstellationHandoverTestCase()
{
}

void
SatLoraConstellationHandoverTestCase::MacTraceCb(std::string context,
                                                 Ptr<const Packet> packet,
                                                 const Address& address)
{
    if (address == m_edAddress)
    {
        m_gwReceiveDate = Simulator::Now();
    }

    if (address == m_orbiterUserAddress31)
    {
        m_edReceiveDate31 = Simulator::Now();
    }

    if (address == m_orbiterUserAddress40)
    {
        m_edReceiveDate40 = Simulator::Now();
    }

    if (address == m_orbiterUserAddress41)
    {
        m_edReceiveDate41 = Simulator::Now();
    }
}

void
SatLoraConstellationHandoverTestCase::DoRun(void)
{
    // Set simulation output details
    SatEnvVariables::GetInstance()->DoInitialize();

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-lora-regenerative/constellation-handover");

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(MilliSeconds(1000)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay", TimeValue(Seconds(2)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(MilliSeconds(900)));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(MilliSeconds(500)));
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay", TimeValue(Seconds(1)));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay", TimeValue(Seconds(2)));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz", DoubleValue(15000));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(15000));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    // Configure E-SSA
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    Config::SetDefault("ns3::SatMac::EnableStatisticsTags", BooleanValue(true));

    Config::SetDefault("ns3::SatHelper::HandoversEnabled", BooleanValue(true));
    Config::SetDefault("ns3::SatHandoverModule::NumberClosestSats", UintegerValue(2));

    // Traffics
    Ptr<SimulationHelperConf> simulationConf = CreateObject<SimulationHelperConf>();
    simulationHelper->SetSimulationTime(Seconds(100));
    simulationHelper->SetGwUserCount(1);
    simulationHelper->SetUserCountPerUt(1);
    simulationHelper->SetBeamSet({31, 40, 41, 43});
    simulationHelper->LoadScenario("constellation-leo-2-satellites-lora");
    simulationHelper->SetUserCountPerMobileUt(simulationConf->m_utMobileUserCount);
    simulationHelper->CreateSatScenario(SatHelper::NONE);

    Ptr<Node> utNode = Singleton<SatTopology>::Get()->GetUtNode(0);
    simulationHelper->GetTrafficHelper()->AddLoraPeriodicTraffic(Seconds(10),
                                                                 24,
                                                                 NodeContainer(utNode),
                                                                 Seconds(0.001),
                                                                 Seconds(100),
                                                                 Seconds(3));

    Ptr<SatOrbiterNetDevice> orbiterNetDevice0 = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(0)->GetDevice(0));
    Ptr<SatOrbiterNetDevice> orbiterNetDevice1 = DynamicCast<SatOrbiterNetDevice>(
        Singleton<SatTopology>::Get()->GetOrbiterNode(1)->GetDevice(0));

    m_edAddress = Singleton<SatTopology>::Get()->GetUtNode(0)->GetDevice(2)->GetAddress();

    m_orbiterUserAddress31 = orbiterNetDevice0->GetSatelliteUserAddress(31);
    m_orbiterUserAddress40 = orbiterNetDevice1->GetSatelliteUserAddress(40);
    m_orbiterUserAddress41 = orbiterNetDevice1->GetSatelliteUserAddress(41);

    Config::Connect("/NodeList/*/DeviceList/*/SatMac/Rx",
                    MakeCallback(&SatLoraConstellationHandoverTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/FeederMac/*/Rx",
                    MakeCallback(&SatLoraConstellationHandoverTestCase::MacTraceCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/UserMac/*/Rx",
                    MakeCallback(&SatLoraConstellationHandoverTestCase::MacTraceCb, this));

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    SatEnvVariables::GetInstance()->DoDispose();

    NS_TEST_ASSERT_MSG_NE(m_gwReceiveDate, Seconds(0), "Packet should be received by Gateway.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate31,
                          Seconds(0),
                          "Ack should be received by End Device from satellite 0 and beam 31.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate40,
                          Seconds(0),
                          "Ack should be received by End Device from satellite 1 and beam 40.");
    NS_TEST_ASSERT_MSG_NE(m_edReceiveDate41,
                          Seconds(0),
                          "Ack should be received by End Device from satellite 1 and beam 41.");
}

/**
 * @ingroup satellite
 * @brief Test suite for Satellite mobility unit test cases.
 */
class SatLoraRegenerativeTestSuite : public TestSuite
{
  public:
    SatLoraRegenerativeTestSuite();
};

SatLoraRegenerativeTestSuite::SatLoraRegenerativeTestSuite()
    : TestSuite("sat-lora-regenerative-test", Type::SYSTEM)
{
    AddTestCase(new SatLoraRegenerativeFirstWindowTestCase, TestCase::Duration::QUICK);
    AddTestCase(new SatLoraRegenerativeSecondWindowTestCase, TestCase::Duration::QUICK);
    AddTestCase(new SatLoraRegenerativeOutOfWindowWindowTestCase, TestCase::Duration::QUICK);
    AddTestCase(new SatLoraRegenerativeOutOfWindowWindowNoRetransmissionTestCase,
                TestCase::Duration::QUICK);
    AddTestCase(new SatLoraRegenerativeCbrTestCase, TestCase::Duration::QUICK);
    AddTestCase(new SatLoraConstellationFirstWindowTestCase, TestCase::Duration::QUICK);
    AddTestCase(new SatLoraConstellationHandoverTestCase, TestCase::Duration::QUICK);
}

// Do allocate an instance of this TestSuite
static SatLoraRegenerativeTestSuite satLoraRegenerativeTestSuite;
