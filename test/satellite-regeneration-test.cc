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

#include "ns3/cbr-application.h"
#include "ns3/cbr-helper.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-geo-feeder-phy.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-geo-user-phy.h"
#include "ns3/satellite-gw-mac.h"
#include "ns3/satellite-helper.h"
#include "ns3/satellite-phy-rx-carrier.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-ut-mac-state.h"
#include "ns3/simulation-helper.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/string.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 1' test case implementation.
 *
 * This case tests that delay of packets takes into account regeneration in satellite
 *
 *  Expected result:
 *    Packet delay = propagation time + transmission time (in SAT and ground entities)
 */
class SatRegenerationTest1 : public TestCase
{
  public:
    SatRegenerationTest1();
    virtual ~SatRegenerationTest1();

  private:
    virtual void DoRun(void);
    void PhyDelayTraceCb(std::string context, const Time& time, const Address& address);
    void GeoFeederPhyTraceDelayCb(const Time& time, const Address& address);
    void GeoUserPhyTraceDelayCb(const Time& time, const Address& address);

    Ptr<SatHelper> m_helper;

    Address m_gwAddress;
    Address m_stAddress;

    std::vector<Time> m_forwardDelay;
    std::vector<Time> m_returnDelay;
    std::vector<Time> m_geoForwardDelay;
    std::vector<Time> m_geoReturnDelay;
};

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest1::SatRegenerationTest1()
    : TestCase(
          "This case tests that delay of packets takes into account regeneration in satellite.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest1::~SatRegenerationTest1()
{
}

void
SatRegenerationTest1::PhyDelayTraceCb(std::string context, const Time& time, const Address& address)
{
    if (address == m_gwAddress)
    {
        m_forwardDelay.push_back(time);
    }
    if (address == m_stAddress)
    {
        m_returnDelay.push_back(time);
    }
}

void
SatRegenerationTest1::GeoFeederPhyTraceDelayCb(const Time& time, const Address& address)
{
    m_geoForwardDelay.push_back(time);
}

void
SatRegenerationTest1::GeoUserPhyTraceDelayCb(const Time& time, const Address& address)
{
    m_geoReturnDelay.push_back(time);
}

//
// SatRegenerationTest1 TestCase implementation
//
void
SatRegenerationTest1::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test1", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    // Enable traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Config::SetDefault("ns3::CbrApplication::Interval", StringValue("500ms"));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));

    // Creating the reference system.
    m_helper = CreateObject<SatHelper>();
    m_helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    NodeContainer utUsers = m_helper->GetUtUsers();
    NodeContainer gwUsers = m_helper->GetGwUsers();
    uint16_t port = 9;

    // Install forward traffic
    CbrHelper cbrForward(
        "ns3::UdpSocketFactory",
        Address(InetSocketAddress(m_helper->GetUserAddress(utUsers.Get(0)), port)));
    ApplicationContainer gwAppsForward = cbrForward.Install(gwUsers);
    gwAppsForward.Start(Seconds(1.0));
    gwAppsForward.Stop(Seconds(5.0));

    PacketSinkHelper sinkForward(
        "ns3::UdpSocketFactory",
        Address(InetSocketAddress(m_helper->GetUserAddress(utUsers.Get(0)), port)));
    ApplicationContainer utAppsForward = sinkForward.Install(utUsers);
    utAppsForward.Start(Seconds(1.0));
    utAppsForward.Stop(Seconds(10.0));

    // Install return traffic
    CbrHelper cbrReturn("ns3::UdpSocketFactory",
                        Address(InetSocketAddress(m_helper->GetUserAddress(gwUsers.Get(0)), port)));
    ApplicationContainer utAppsReturn = cbrReturn.Install(utUsers);
    utAppsReturn.Start(Seconds(1.0));
    utAppsReturn.Stop(Seconds(5.0));

    PacketSinkHelper sinkReturn(
        "ns3::UdpSocketFactory",
        Address(InetSocketAddress(m_helper->GetUserAddress(gwUsers.Get(0)), port)));
    ApplicationContainer gwAppsReturn = sinkReturn.Install(gwUsers);
    gwAppsReturn.Start(Seconds(1.0));
    gwAppsReturn.Stop(Seconds(10.0));

    m_gwAddress = m_helper->GwNodes().Get(0)->GetDevice(1)->GetAddress();
    m_stAddress = m_helper->UtNodes().Get(0)->GetDevice(2)->GetAddress();

    Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy>(
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))
            ->GetFeederPhy(8));
    Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy>(
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetUserPhy(8));

    satGeoFeederPhy->TraceConnectWithoutContext(
        "RxLinkDelay",
        MakeCallback(&SatRegenerationTest1::GeoFeederPhyTraceDelayCb, this));
    satGeoUserPhy->TraceConnectWithoutContext(
        "RxLinkDelay",
        MakeCallback(&SatRegenerationTest1::GeoUserPhyTraceDelayCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/SatPhy/RxLinkDelay",
                    MakeCallback(&SatRegenerationTest1::PhyDelayTraceCb, this));

    Ptr<SatChannel> feederChannel = satGeoFeederPhy->GetPhyTx()->GetChannel();
    Ptr<SatChannel> userChannel = satGeoUserPhy->GetPhyTx()->GetChannel();

    Ptr<PropagationDelayModel> feederDelayModel = feederChannel->GetPropagationDelayModel();
    Ptr<PropagationDelayModel> userDelayModel = userChannel->GetPropagationDelayModel();

    Time feederDelay = feederDelayModel->GetDelay(
        DynamicCast<SatNetDevice>(m_helper->GwNodes().Get(0)->GetDevice(1))
            ->GetPhy()
            ->GetPhyTx()
            ->GetMobility(),
        satGeoFeederPhy->GetPhyTx()->GetMobility());

    Time userDelay =
        userDelayModel->GetDelay(DynamicCast<SatNetDevice>(m_helper->UtNodes().Get(0)->GetDevice(2))
                                     ->GetPhy()
                                     ->GetPhyTx()
                                     ->GetMobility(),
                                 satGeoUserPhy->GetPhyTx()->GetMobility());

    Simulator::Stop(Seconds(10));
    Simulator::Run();

    Simulator::Destroy();

    // In default configuration, we have:
    // - FWD transmission delay (NORMAL BBFrame = 32400 symbols, QPSK 1/2, BW = 104 MHz):  319 507
    // ns
    // - RTN transmission delay (frame = 536 symbols, QPSK 1/3, BW = 801 kHz):  668 928 ns
    // We have to remove 1 us default guard time to these delays to get transmission time.

    Time fwdTime = NanoSeconds(319507) - MicroSeconds(1);
    Time rtnTime = NanoSeconds(668928) - MicroSeconds(1);

    for (uint32_t i = 0; i < m_geoForwardDelay.size(); i++)
    {
        NS_TEST_ASSERT_MSG_EQ(m_geoForwardDelay[i] - feederDelay,
                              fwdTime,
                              "Transmission time on FWD FEEDER incorrect.");
    }

    for (uint32_t i = 0; i < m_geoReturnDelay.size(); i++)
    {
        NS_TEST_ASSERT_MSG_EQ(m_geoReturnDelay[i] - userDelay,
                              rtnTime,
                              "Transmission time on RTN USER incorrect.");
    }

    for (uint32_t i = 0; i < m_forwardDelay.size(); i++)
    {
        NS_TEST_ASSERT_MSG_EQ(m_forwardDelay[i] - userDelay,
                              fwdTime,
                              "Transmission time on FWD USER incorrect.");
    }

    for (uint32_t i = 0; i < m_returnDelay.size(); i++)
    {
        NS_TEST_ASSERT_MSG_EQ(m_returnDelay[i] - feederDelay,
                              rtnTime,
                              "Transmission time on RTN FEEDER incorrect.");
    }
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 2' test case implementation.
 *
 * This case tests physical regeneration on satellite. It is based on a SIMPLE scenario, with losses
 * on uplink, forward and return.
 *
 *  Expected result:
 *    Packets are lost on uplink with expect drop rate
 *    No packets are lost on downlink
 *    All packets received on uplink are received on GW/UT
 */
class SatRegenerationTest2 : public TestCase
{
  public:
    SatRegenerationTest2();
    virtual ~SatRegenerationTest2();

  private:
    virtual void DoRun(void);
    void GeoPhyTraceCb(Time time,
                       SatEnums::SatPacketEvent_t event,
                       SatEnums::SatNodeType_t type,
                       uint32_t nodeId,
                       Mac48Address address,
                       SatEnums::SatLogLevel_t level,
                       SatEnums::SatLinkDir_t dir,
                       std::string packetInfo);
    void PhyTraceCb(Time time,
                    SatEnums::SatPacketEvent_t event,
                    SatEnums::SatNodeType_t type,
                    uint32_t nodeId,
                    Mac48Address address,
                    SatEnums::SatLogLevel_t level,
                    SatEnums::SatLinkDir_t dir,
                    std::string packetInfo);

    Ptr<SatHelper> m_helper;

    uint32_t m_packetsReceivedFeeder;
    uint32_t m_packetsDroppedFeeder;
    uint32_t m_packetsReceivedUser;
    uint32_t m_packetsDroppedUser;

    uint32_t m_packetsReceivedUt;
    uint32_t m_packetsDroppedUt;
    uint32_t m_packetsReceivedGw;
    uint32_t m_packetsDroppedGw;
};

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest2::SatRegenerationTest2()
    : TestCase("This case tests physical regeneration on satellite. It is based on a SIMPLE "
               "scenario, with losses on uplink, forward and return."),
      m_packetsReceivedFeeder(0),
      m_packetsDroppedFeeder(0),
      m_packetsReceivedUser(0),
      m_packetsDroppedUser(0),
      m_packetsReceivedUt(0),
      m_packetsDroppedUt(0),
      m_packetsReceivedGw(0),
      m_packetsDroppedGw(0)
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest2::~SatRegenerationTest2()
{
}

void
SatRegenerationTest2::GeoPhyTraceCb(Time time,
                                    SatEnums::SatPacketEvent_t event,
                                    SatEnums::SatNodeType_t type,
                                    uint32_t nodeId,
                                    Mac48Address address,
                                    SatEnums::SatLogLevel_t level,
                                    SatEnums::SatLinkDir_t dir,
                                    std::string packetInfo)
{
    switch (dir)
    {
    case SatEnums::LD_FORWARD:
        if (event == SatEnums::PACKET_RECV)
        {
            m_packetsReceivedFeeder++;
        }
        else if (event == SatEnums::PACKET_DROP)
        {
            m_packetsDroppedFeeder++;
        }
        break;
    case SatEnums::LD_RETURN:
        if (event == SatEnums::PACKET_RECV)
        {
            m_packetsReceivedUser++;
        }
        else if (event == SatEnums::PACKET_DROP)
        {
            m_packetsDroppedUser++;
        }
        break;
    default:
        break;
    }
}

void
SatRegenerationTest2::PhyTraceCb(Time time,
                                 SatEnums::SatPacketEvent_t event,
                                 SatEnums::SatNodeType_t type,
                                 uint32_t nodeId,
                                 Mac48Address address,
                                 SatEnums::SatLogLevel_t level,
                                 SatEnums::SatLinkDir_t dir,
                                 std::string packetInfo)
{
    switch (dir)
    {
    case SatEnums::LD_FORWARD:
        if (event == SatEnums::PACKET_RECV)
        {
            m_packetsReceivedUt++;
        }
        else if (event == SatEnums::PACKET_DROP)
        {
            m_packetsDroppedUt++;
        }
        break;
    case SatEnums::LD_RETURN:
        if (event == SatEnums::PACKET_RECV)
        {
            m_packetsReceivedGw++;
        }
        else if (event == SatEnums::PACKET_DROP)
        {
            m_packetsDroppedGw++;
        }
        break;
    default:
        break;
    }
}

//
// SatRegenerationTest2 TestCase implementation
//
void
SatRegenerationTest2::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test2", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    /// Set constant 10% losses on Uplink
    Config::SetDefault("ns3::SatGeoHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_CONSTANT));
    Config::SetDefault("ns3::SatGeoHelper::FwdLinkConstantErrorRate", DoubleValue(0.1));
    Config::SetDefault("ns3::SatGeoHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_CONSTANT));
    Config::SetDefault("ns3::SatGeoHelper::RtnLinkConstantErrorRate", DoubleValue(0.1));

    // Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Config::SetDefault("ns3::CbrApplication::Interval", StringValue("10ms"));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));

    // Creating the reference system.
    m_helper = CreateObject<SatHelper>();
    m_helper->CreatePredefinedScenario(SatHelper::SIMPLE);

    NodeContainer utUsers = m_helper->GetUtUsers();
    NodeContainer gwUsers = m_helper->GetGwUsers();
    uint16_t port = 9;

    // Install forward traffic
    CbrHelper cbrForward(
        "ns3::UdpSocketFactory",
        Address(InetSocketAddress(m_helper->GetUserAddress(utUsers.Get(0)), port)));
    ApplicationContainer gwAppsForward = cbrForward.Install(gwUsers);
    gwAppsForward.Start(Seconds(1.0));
    gwAppsForward.Stop(Seconds(59.0));

    PacketSinkHelper sinkForward(
        "ns3::UdpSocketFactory",
        Address(InetSocketAddress(m_helper->GetUserAddress(utUsers.Get(0)), port)));
    ApplicationContainer utAppsForward = sinkForward.Install(utUsers);
    utAppsForward.Start(Seconds(1.0));
    utAppsForward.Stop(Seconds(60.0));

    // Install return traffic
    CbrHelper cbrReturn("ns3::UdpSocketFactory",
                        Address(InetSocketAddress(m_helper->GetUserAddress(gwUsers.Get(0)), port)));
    ApplicationContainer utAppsReturn = cbrReturn.Install(utUsers);
    utAppsReturn.Start(Seconds(1.0));
    utAppsReturn.Stop(Seconds(59.0));

    PacketSinkHelper sinkReturn(
        "ns3::UdpSocketFactory",
        Address(InetSocketAddress(m_helper->GetUserAddress(gwUsers.Get(0)), port)));
    ApplicationContainer gwAppsReturn = sinkReturn.Install(gwUsers);
    gwAppsReturn.Start(Seconds(1.0));
    gwAppsReturn.Stop(Seconds(60.0));

    Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy>(
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))
            ->GetFeederPhy(8));
    Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy>(
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetUserPhy(8));

    satGeoFeederPhy->TraceConnectWithoutContext(
        "PacketTrace",
        MakeCallback(&SatRegenerationTest2::GeoPhyTraceCb, this));
    satGeoUserPhy->TraceConnectWithoutContext(
        "PacketTrace",
        MakeCallback(&SatRegenerationTest2::GeoPhyTraceCb, this));

    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/SatPhy/PacketTrace",
                                  MakeCallback(&SatRegenerationTest2::PhyTraceCb, this));

    Simulator::Stop(Seconds(60));
    Simulator::Run();

    Simulator::Destroy();

    double dropRateForwardFeeder =
        (m_packetsDroppedFeeder * 100.0 / (m_packetsReceivedFeeder + m_packetsDroppedFeeder));
    double dropRateReturnUser =
        (m_packetsDroppedUser * 100.0 / (m_packetsReceivedUser + m_packetsDroppedUser));

    double forwardDifference =
        std::abs((1.0 * m_packetsReceivedUt - m_packetsReceivedFeeder) / m_packetsReceivedUt);
    double returnDifference =
        std::abs((1.0 * m_packetsReceivedGw - m_packetsReceivedUser) / m_packetsReceivedGw);

    NS_TEST_ASSERT_MSG_GT(dropRateForwardFeeder, 8.0, "Not enough losses on FWD feeder");
    NS_TEST_ASSERT_MSG_LT(dropRateForwardFeeder, 12.0, "Too much losses on FWD feeder");
    NS_TEST_ASSERT_MSG_GT(dropRateReturnUser, 8.0, "Not enough losses on RTN user");
    NS_TEST_ASSERT_MSG_LT(dropRateReturnUser, 12.0, "Too much losses on RTN user");

    NS_TEST_ASSERT_MSG_NE(m_packetsReceivedGw, 0, "Packets must be received by GW");
    NS_TEST_ASSERT_MSG_NE(m_packetsReceivedUt, 0, "Packets must be received by UT");
    NS_TEST_ASSERT_MSG_EQ(m_packetsDroppedGw, 0, "Packets must not be dropped by GW");
    NS_TEST_ASSERT_MSG_EQ(m_packetsDroppedUt, 0, "Packets must not be dropped by UT");

    NS_TEST_ASSERT_MSG_LT(
        forwardDifference,
        0.01,
        "Number of packets received on FWD should be almost the same between SAT and UT");
    NS_TEST_ASSERT_MSG_LT(
        returnDifference,
        0.01,
        "Number of packets received on RTN should be almost the same between SAT and GW");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 3' test case implementation.
 *
 * This case tests physical regeneration on satellite. It is based on a LARGER scenario, with
 * collisions on return uplink.
 *
 *  Expected result:
 *    Packets received on feeder receiver
 *    No packets lost on RTN feeder
 *    Packets lost on RTN user due to collisions
 */
class SatRegenerationTest3 : public TestCase
{
  public:
    SatRegenerationTest3();
    virtual ~SatRegenerationTest3();

  private:
    virtual void DoRun(void);
    void GeoPhyTraceErrorCb(std::string, uint32_t nPackets, const Address& address, bool hasError);
    void GeoPhyTraceCollisionCb(std::string,
                                uint32_t nPackets,
                                const Address& address,
                                bool hasCollision);
    void GeoPhyTraceCb(Time time,
                       SatEnums::SatPacketEvent_t event,
                       SatEnums::SatNodeType_t type,
                       uint32_t nodeId,
                       Mac48Address address,
                       SatEnums::SatLogLevel_t level,
                       SatEnums::SatLinkDir_t dir,
                       std::string packetInfo);

    Ptr<SatHelper> m_helper;

    Address m_gwAddress;

    uint32_t m_nbErrorpacketsFwd;
    uint32_t m_nbErrorpacketsRtn;
    uint32_t m_nbCollisionPacketsFwd;
    uint32_t m_nbCollisionPacketsRtn;

    uint32_t m_packetsReceivedFeeder;
    uint32_t m_packetsDroppedFeeder;
    uint32_t m_packetsReceivedUser;
    uint32_t m_packetsDroppedUser;
};

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest3::SatRegenerationTest3()
    : TestCase("This case tests physical regeneration on satellite. It is based on a LARGER "
               "scenario, with collisions on return uplink."),
      m_nbErrorpacketsFwd(0),
      m_nbErrorpacketsRtn(0),
      m_nbCollisionPacketsFwd(0),
      m_nbCollisionPacketsRtn(0),
      m_packetsReceivedFeeder(0),
      m_packetsDroppedFeeder(0),
      m_packetsReceivedUser(0),
      m_packetsDroppedUser(0)
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest3::~SatRegenerationTest3()
{
}

void
SatRegenerationTest3::GeoPhyTraceErrorCb(std::string,
                                         uint32_t nPackets,
                                         const Address& address,
                                         bool hasError)
{
    if (!hasError)
    {
        return;
    }
    if (address == m_gwAddress)
    {
        m_nbErrorpacketsFwd += nPackets;
    }
    else
    {
        m_nbErrorpacketsRtn += nPackets;
    }
}

void
SatRegenerationTest3::GeoPhyTraceCollisionCb(std::string,
                                             uint32_t nPackets,
                                             const Address& address,
                                             bool hasCollision)
{
    if (!hasCollision)
    {
        return;
    }
    if (address == m_gwAddress)
    {
        m_nbCollisionPacketsFwd += nPackets;
    }
    else
    {
        m_nbCollisionPacketsRtn += nPackets;
    }
}

void
SatRegenerationTest3::GeoPhyTraceCb(Time time,
                                    SatEnums::SatPacketEvent_t event,
                                    SatEnums::SatNodeType_t type,
                                    uint32_t nodeId,
                                    Mac48Address address,
                                    SatEnums::SatLogLevel_t level,
                                    SatEnums::SatLinkDir_t dir,
                                    std::string packetInfo)
{
    switch (dir)
    {
    case SatEnums::LD_FORWARD:
        if (event == SatEnums::PACKET_RECV)
        {
            m_packetsReceivedFeeder++;
        }
        else if (event == SatEnums::PACKET_DROP)
        {
            m_packetsDroppedFeeder++;
        }
        break;
    case SatEnums::LD_RETURN:
        if (event == SatEnums::PACKET_RECV)
        {
            m_packetsReceivedUser++;
        }
        else if (event == SatEnums::PACKET_DROP)
        {
            m_packetsDroppedUser++;
        }
        break;
    default:
        break;
    }
}

//
// SatRegenerationTest3 TestCase implementation
//
void
SatRegenerationTest3::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test3", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    /// Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    // Enable Random Access
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel",
                       EnumValue(SatEnums::RA_MODEL_SLOTTED_ALOHA));

    // Set Random Access interference model
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));

    // Disable periodic control slots
    Config::SetDefault("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue(false));

    // Set dynamic load control parameters
    Config::SetDefault("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl",
                       BooleanValue(false));
    Config::SetDefault(
        "ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize",
        UintegerValue(10));

    // Set random access parameters
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock",
                       UintegerValue(3));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed",
                       UintegerValue(6));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock",
                       UintegerValue(2));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds",
                       UintegerValue(250));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability",
                       UintegerValue(10000));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability",
                       UintegerValue(30000));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances",
                       UintegerValue(3));
    Config::SetDefault(
        "ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold",
        DoubleValue(0.5));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval",
                       TimeValue(MilliSeconds(100)));
    Config::SetDefault("ns3::SatRandomAccessConf::CrdsaSignalingOverheadInBytes", UintegerValue(5));
    Config::SetDefault("ns3::SatRandomAccessConf::SlottedAlohaSignalingOverheadInBytes",
                       UintegerValue(3));

    // Disable CRA and DA
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed",
                       BooleanValue(false));

    Config::SetDefault("ns3::SatGeoHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_NONE));
    Config::SetDefault("ns3::SatGeoHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-regeneration");
    simulationHelper->SetSimulationTime(Seconds(5));
    simulationHelper->SetUserCountPerUt(1);
    simulationHelper->SetUtCountPerBeam(50);
    simulationHelper->SetBeamSet({1});
    simulationHelper->CreateSatScenario();

    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(MilliSeconds(1)));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));
    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::RTN_LINK,
                                          Seconds(0.01),
                                          Seconds(5),
                                          Seconds(0.01));

    m_helper = simulationHelper->GetSatelliteHelper();

    Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy>(
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))
            ->GetFeederPhy(1));
    Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy>(
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetUserPhy(1));

    m_gwAddress = m_helper->GwNodes().Get(0)->GetDevice(1)->GetAddress();

    satGeoFeederPhy->TraceConnectWithoutContext(
        "PacketTrace",
        MakeCallback(&SatRegenerationTest3::GeoPhyTraceCb, this));
    satGeoUserPhy->TraceConnectWithoutContext(
        "PacketTrace",
        MakeCallback(&SatRegenerationTest3::GeoPhyTraceCb, this));

    Config::Connect(
        "/NodeList/*/DeviceList/*/FeederPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxError",
        MakeCallback(&SatRegenerationTest3::GeoPhyTraceErrorCb, this));
    Config::Connect(
        "/NodeList/*/DeviceList/*/FeederPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxCollision",
        MakeCallback(&SatRegenerationTest3::GeoPhyTraceCollisionCb, this));
    Config::Connect("/NodeList/*/DeviceList/*/UserPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxError",
                    MakeCallback(&SatRegenerationTest3::GeoPhyTraceErrorCb, this));
    Config::Connect(
        "/NodeList/*/DeviceList/*/UserPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxCollision",
        MakeCallback(&SatRegenerationTest3::GeoPhyTraceCollisionCb, this));

    Simulator::Stop(Seconds(5));
    Simulator::Run();

    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_NE(m_packetsReceivedFeeder, 0, "Packets received on FWD feeder link");
    NS_TEST_ASSERT_MSG_NE(m_packetsReceivedUser,
                          0,
                          "Packets received on RTN user link due to collisions");
    NS_TEST_ASSERT_MSG_EQ(m_packetsDroppedFeeder, 0, "No packets dropped on FWD feeder link");
    NS_TEST_ASSERT_MSG_NE(m_packetsDroppedUser,
                          0,
                          "Packets dropped on RTN user link due to collisions");

    NS_TEST_ASSERT_MSG_EQ(m_nbErrorpacketsFwd, 0, "No errors on FWD feeder link");
    NS_TEST_ASSERT_MSG_EQ(m_nbCollisionPacketsFwd, 0, "No collisions on FWD feeder link");
    NS_TEST_ASSERT_MSG_NE(m_nbErrorpacketsRtn, 0, "Need errors on RTN feeder link");
    NS_TEST_ASSERT_MSG_NE(m_nbCollisionPacketsRtn, 0, "Need collisions on RTN feeder link");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 4' test case implementation.
 *
 * This case tests link regeneration on satellite. It is based on a LARGER scenario.
 *
 *  Expected result:
 *    No bytes sent or received on FWD
 *    Same number of bytes sent and received on RTN
 */
class SatRegenerationTest4 : public TestCase
{
  public:
    SatRegenerationTest4();
    virtual ~SatRegenerationTest4();

  private:
    virtual void DoRun(void);
    void GeoDevGwTxTraceCb(Ptr<const Packet> packet);
    void GeoDevUtTxTraceCb(Ptr<const Packet> packet);
    void GeoDevGwRxTraceCb(Ptr<const Packet> packet, const Address&);
    void GeoDevUtRxTraceCb(Ptr<const Packet> packet, const Address&);
    bool HasSinkInstalled(Ptr<Node> node, uint16_t port);

    Ptr<SatHelper> m_helper;

    uint32_t m_totalSentGw;
    uint32_t m_totalSentUt;
    uint32_t m_totalReceivedGw;
    uint32_t m_totalReceivedUt;
};

void
SatRegenerationTest4::GeoDevGwTxTraceCb(Ptr<const Packet> packet)
{
    m_totalSentGw += packet->GetSize();
}

void
SatRegenerationTest4::GeoDevUtTxTraceCb(Ptr<const Packet> packet)
{
    m_totalSentUt += packet->GetSize();
}

void
SatRegenerationTest4::GeoDevGwRxTraceCb(Ptr<const Packet> packet, const Address&)
{
    m_totalReceivedGw += packet->GetSize();
}

void
SatRegenerationTest4::GeoDevUtRxTraceCb(Ptr<const Packet> packet, const Address& address)
{
    m_totalReceivedUt += packet->GetSize();
}

bool
SatRegenerationTest4::HasSinkInstalled(Ptr<Node> node, uint16_t port)
{
    for (uint32_t i = 0; i < node->GetNApplications(); i++)
    {
        Ptr<PacketSink> sink = DynamicCast<PacketSink>(node->GetApplication(i));
        if (sink != NULL)
        {
            AddressValue av;
            sink->GetAttribute("Local", av);
            if (InetSocketAddress::ConvertFrom(av.Get()).GetPort() == port)
            {
                return true;
            }
        }
    }
    return false;
}

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest4::SatRegenerationTest4()
    : TestCase("This case tests link regeneration on satellite. It is based on a LARGER scenario.")
{
    m_totalSentGw = 0;
    m_totalSentUt = 0;
    m_totalReceivedGw = 0;
    m_totalReceivedUt = 0;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest4::~SatRegenerationTest4()
{
}

//
// SatRegenerationTest4 TestCase implementation
//
void
SatRegenerationTest4::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test4", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_LINK));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    // Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-regeneration");
    simulationHelper->SetSimulationTime(Seconds(20));
    simulationHelper->CreateSatScenario(SatHelper::LARGER);

    m_helper = simulationHelper->GetSatelliteHelper();

    NodeContainer gws = m_helper->GwNodes();
    NodeContainer uts = m_helper->UtNodes();

    uint32_t i;
    for (i = 0; i < gws.GetN(); i++)
    {
        gws.Get(i)->GetDevice(1)->TraceConnectWithoutContext(
            "Tx",
            MakeCallback(&SatRegenerationTest4::GeoDevGwTxTraceCb, this));
        gws.Get(i)->GetDevice(1)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&SatRegenerationTest4::GeoDevGwRxTraceCb, this));
    }
    for (i = 0; i < uts.GetN(); i++)
    {
        uts.Get(i)->GetDevice(2)->TraceConnectWithoutContext(
            "Tx",
            MakeCallback(&SatRegenerationTest4::GeoDevUtTxTraceCb, this));
        uts.Get(i)->GetDevice(2)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&SatRegenerationTest4::GeoDevUtRxTraceCb, this));
    }

    std::string socketFactory = "ns3::UdpSocketFactory";
    uint16_t port = 9;

    PacketSinkHelper sinkHelper(socketFactory, Address());
    CbrHelper cbrHelper(socketFactory, Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer cbrContainer;

    Time startTime = Seconds(1);
    Time stopTime = Seconds(15);
    Time startDelay = MilliSeconds(10);
    Time interval = MilliSeconds(100);
    uint32_t packetSize = 512;

    for (uint32_t j = 0; j < gws.GetN(); j++)
    {
        for (uint32_t i = 0; i < uts.GetN(); i++)
        {
            InetSocketAddress gwUserAddr =
                InetSocketAddress(m_helper->GetUserAddress(gws.Get(j)), port);
            if (!HasSinkInstalled(gws.Get(j), port))
            {
                sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                sinkContainer.Add(sinkHelper.Install(gws.Get(j)));
            }

            cbrHelper.SetConstantTraffic(Time(interval), packetSize);
            cbrHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));
            auto app = cbrHelper.Install(uts.Get(i)).Get(0);
            app->SetStartTime(startTime + (i + j * gws.GetN() + 1) * startDelay);
            app->SetStopTime(stopTime);
            cbrContainer.Add(app);
        }
    }

    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime + Seconds(1));

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_EQ(m_totalSentGw, 0, "No packets sent on FWD link");
    NS_TEST_ASSERT_MSG_EQ(m_totalReceivedUt, 0, "No packets received on FWD link");
    NS_TEST_ASSERT_MSG_EQ(m_totalSentUt,
                          m_totalReceivedGw,
                          "Same number of packets sent and received on RTN link");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 5' test case implementation.
 *
 * This case tests ACM on all links.
 * It is based on a SIMPLE scenario, with PHY regeneration on FWD and LINK regeneration on RTN.
 *
 *  Expected result:
 *    Most robust MODCOD used a start of simulation
 *    Efficient MODCOD used on all links after a few seconds
 *    Same MODCOD on FWD feeder and FWD user
 *    Not same MODCOD on RTN feeder and RTN user
 */
class SatRegenerationTest5 : public TestCase
{
  public:
    SatRegenerationTest5();
    virtual ~SatRegenerationTest5();

  private:
    virtual void DoRun(void);
    void GeoPhyGwModcodTraceCb(uint32_t modcod, const Address& address);
    void GeoPhyUtModcodTraceCb(uint32_t modcod, const Address& address);
    void GeoPhyFeederModcodTraceCb(uint32_t modcod, const Address& address);
    void GeoPhyUserModcodTraceCb(uint32_t modcod, const Address& address);

    double GetAverage(std::vector<uint32_t> list, uint32_t beg, uint32_t end);
    double GetMostFrequent(std::vector<uint32_t> list, uint32_t beg, uint32_t end);

    Ptr<SatHelper> m_helper;

    std::vector<uint32_t> m_gwModcods;
    std::vector<uint32_t> m_utModcods;
    std::vector<uint32_t> m_feederModcods;
    std::vector<uint32_t> m_userModcods;
};

void
SatRegenerationTest5::GeoPhyGwModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_gwModcods.push_back(modcod);
}

void
SatRegenerationTest5::GeoPhyUtModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_utModcods.push_back(modcod);
}

void
SatRegenerationTest5::GeoPhyFeederModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_feederModcods.push_back(modcod);
}

void
SatRegenerationTest5::GeoPhyUserModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_userModcods.push_back(modcod);
}

double
SatRegenerationTest5::GetAverage(std::vector<uint32_t> list, uint32_t beg, uint32_t end)
{
    uint32_t sum = 0;
    for (uint32_t i = beg; i < end; i++)
    {
        sum += list[i];
    }
    return 1.0 * sum / (end - beg);
}

double
SatRegenerationTest5::GetMostFrequent(std::vector<uint32_t> list, uint32_t beg, uint32_t end)
{
    std::map<uint32_t, uint32_t> frequencies;
    for (uint32_t i = beg; i < end; i++)
    {
        frequencies[list[i]]++;
    }

    uint32_t max_count = 0;
    uint32_t index = -1;
    for (std::pair<const uint32_t, uint32_t>& i : frequencies)
    {
        if (max_count < i.second)
        {
            index = i.first;
            max_count = i.second;
        }
    }

    return index;
}

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest5::SatRegenerationTest5()
    : TestCase("This case tests ACM on all links. It is based on a SIMPLE scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest5::~SatRegenerationTest5()
{
}

//
// SatRegenerationTest5 TestCase implementation
//
void
SatRegenerationTest5::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test5", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_PHY));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_LINK));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    /// Enable ACM
    Config::SetDefault("ns3::SatBbFrameConf::AcmEnabled", BooleanValue(true));

    // Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-regeneration");
    simulationHelper->SetSimulationTime(Seconds(20));
    simulationHelper->CreateSatScenario(SatHelper::SIMPLE);

    m_helper = simulationHelper->GetSatelliteHelper();

    Ptr<Node> gwNode = m_helper->GwNodes().Get(0);
    Ptr<Node> utNode = m_helper->UtNodes().Get(0);
    Ptr<Node> geoNode = m_helper->GeoSatNodes().Get(0);
    Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy>(
        DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0))->GetFeederPhy(8));
    Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy>(
        DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0))->GetUserPhy(8));
    Ptr<SatPhy> satGwPhy =
        DynamicCast<SatPhy>(DynamicCast<SatNetDevice>(gwNode->GetDevice(1))->GetPhy());
    Ptr<SatPhy> satUtPhy =
        DynamicCast<SatPhy>(DynamicCast<SatNetDevice>(utNode->GetDevice(2))->GetPhy());

    satGwPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest5::GeoPhyGwModcodTraceCb, this));
    satUtPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest5::GeoPhyUtModcodTraceCb, this));
    satGeoFeederPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest5::GeoPhyFeederModcodTraceCb, this));
    satGeoUserPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest5::GeoPhyUserModcodTraceCb, this));

    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(MilliSeconds(20)));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::RTN_LINK,
                                          Seconds(1),
                                          Seconds(10),
                                          Seconds(0.01));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          Seconds(1),
                                          Seconds(10),
                                          Seconds(0.01));

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    double averageGwModcodsBeg = GetAverage(m_gwModcods, 0, 5);
    double averageUtModcodsBeg = GetAverage(m_utModcods, 0, 5);
    double averageFeederModcodsBeg = GetAverage(m_feederModcods, 0, 5);
    double averageUserModcodsBeg = GetAverage(m_userModcods, 0, 5);
    double averageGwModcodsEnd =
        GetMostFrequent(m_gwModcods, m_gwModcods.size() - 200, m_gwModcods.size());
    double averageUtModcodsEnd =
        GetMostFrequent(m_utModcods, m_utModcods.size() - 200, m_utModcods.size());
    double averageFeederModcodsEnd =
        GetMostFrequent(m_feederModcods, m_feederModcods.size() - 200, m_feederModcods.size());
    double averageUserModcodsEnd =
        GetMostFrequent(m_userModcods, m_userModcods.size() - 200, m_userModcods.size());

    NS_TEST_ASSERT_MSG_EQ(averageFeederModcodsBeg,
                          2,
                          "Most robust MODCOD on FWD feeder link at startup");
    NS_TEST_ASSERT_MSG_EQ(averageUtModcodsBeg, 2, "Most robust MODCOD on FWD user link at startup");
    NS_TEST_ASSERT_MSG_EQ(averageGwModcodsBeg,
                          2,
                          "Most robust MODCOD on RTN feeder link at startup");
    NS_TEST_ASSERT_MSG_EQ(averageUserModcodsBeg,
                          1,
                          "Most robust MODCOD on RTN user link at startup");

    NS_TEST_ASSERT_MSG_LT(averageFeederModcodsEnd, 28, "Max MODCOD on FWD feeder link is 27");
    NS_TEST_ASSERT_MSG_LT(averageUtModcodsEnd, 28, "Max MODCOD on FWD user link is 27");
    NS_TEST_ASSERT_MSG_LT(averageGwModcodsEnd, 28, "Max MODCOD on RTN feeder link is 27");
    NS_TEST_ASSERT_MSG_LT(averageUserModcodsEnd, 24, "Max MODCOD on RTN user link is 23");

    NS_TEST_ASSERT_MSG_GT(averageFeederModcodsEnd,
                          2,
                          "Most robust MODCOD on FWD feeder link not used after a few seconds");
    NS_TEST_ASSERT_MSG_GT(averageUtModcodsEnd,
                          2,
                          "Most robust MODCOD on FWD user link not used after a few seconds");
    NS_TEST_ASSERT_MSG_GT(averageGwModcodsEnd,
                          2,
                          "Most robust MODCOD on RTN feeder link not used after a few seconds");
    NS_TEST_ASSERT_MSG_GT(averageUserModcodsEnd,
                          1,
                          "Most robust MODCOD on RTN user link not used after a few seconds");

    NS_TEST_ASSERT_MSG_EQ(averageFeederModcodsEnd,
                          averageUtModcodsEnd,
                          "Same MODCOD on both FWD links");
    NS_TEST_ASSERT_MSG_NE(averageUserModcodsEnd,
                          averageGwModcodsEnd,
                          "Not same MODCOD on both RTN links");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 6' test case implementation.
 *
 * This case tests network regeneration on satellite. It is based on a LARGER scenario.
 *
 *  Expected result:
 *    Same number of bytes sent and received on FWD
 *    Same number of bytes sent and received on RTN
 */
class SatRegenerationTest6 : public TestCase
{
  public:
    SatRegenerationTest6();
    virtual ~SatRegenerationTest6();

  private:
    virtual void DoRun(void);
    void GeoDevGwTxTraceCb(Ptr<const Packet> packet);
    void GeoDevUtTxTraceCb(Ptr<const Packet> packet);
    void GeoDevGwRxTraceCb(Ptr<const Packet> packet, const Address&);
    void GeoDevUtRxTraceCb(Ptr<const Packet> packet, const Address&);
    bool HasSinkInstalled(Ptr<Node> node, uint16_t port);

    Ptr<SatHelper> m_helper;

    uint32_t m_totalSentGw;
    uint32_t m_totalSentUt;
    uint32_t m_totalReceivedGw;
    uint32_t m_totalReceivedUt;
};

void
SatRegenerationTest6::GeoDevGwTxTraceCb(Ptr<const Packet> packet)
{
    m_totalSentGw += packet->GetSize();
}

void
SatRegenerationTest6::GeoDevUtTxTraceCb(Ptr<const Packet> packet)
{
    m_totalSentUt += packet->GetSize();
}

void
SatRegenerationTest6::GeoDevGwRxTraceCb(Ptr<const Packet> packet, const Address&)
{
    m_totalReceivedGw += packet->GetSize();
}

void
SatRegenerationTest6::GeoDevUtRxTraceCb(Ptr<const Packet> packet, const Address& address)
{
    m_totalReceivedUt += packet->GetSize();
}

bool
SatRegenerationTest6::HasSinkInstalled(Ptr<Node> node, uint16_t port)
{
    for (uint32_t i = 0; i < node->GetNApplications(); i++)
    {
        Ptr<PacketSink> sink = DynamicCast<PacketSink>(node->GetApplication(i));
        if (sink != NULL)
        {
            AddressValue av;
            sink->GetAttribute("Local", av);
            if (InetSocketAddress::ConvertFrom(av.Get()).GetPort() == port)
            {
                return true;
            }
        }
    }
    return false;
}

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest6::SatRegenerationTest6()
    : TestCase(
          "This case tests network regeneration on satellite. It is based on a LARGER scenario.")
{
    m_totalSentGw = 0;
    m_totalSentUt = 0;
    m_totalReceivedGw = 0;
    m_totalReceivedUt = 0;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest6::~SatRegenerationTest6()
{
}

//
// SatRegenerationTest6 TestCase implementation
//
void
SatRegenerationTest6::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test6", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    // Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-regeneration");
    simulationHelper->SetSimulationTime(Seconds(20));
    simulationHelper->CreateSatScenario(SatHelper::LARGER);

    m_helper = simulationHelper->GetSatelliteHelper();

    NodeContainer gws = m_helper->GwNodes();
    NodeContainer uts = m_helper->UtNodes();

    uint32_t i;
    for (i = 0; i < gws.GetN(); i++)
    {
        gws.Get(i)->GetDevice(1)->TraceConnectWithoutContext(
            "Tx",
            MakeCallback(&SatRegenerationTest6::GeoDevGwTxTraceCb, this));
        gws.Get(i)->GetDevice(1)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&SatRegenerationTest6::GeoDevGwRxTraceCb, this));
    }
    for (i = 0; i < uts.GetN(); i++)
    {
        uts.Get(i)->GetDevice(2)->TraceConnectWithoutContext(
            "Tx",
            MakeCallback(&SatRegenerationTest6::GeoDevUtTxTraceCb, this));
        uts.Get(i)->GetDevice(2)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&SatRegenerationTest6::GeoDevUtRxTraceCb, this));
    }

    Time startTime = Seconds(1);
    Time stopTime = Seconds(15);
    Time startDelay = MilliSeconds(10);
    Time interval = MilliSeconds(1000);
    uint32_t packetSize = 512;

    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(interval));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(packetSize));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::RTN_LINK,
                                          Seconds(1),
                                          Seconds(10),
                                          Seconds(0.01));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          Seconds(1),
                                          Seconds(10),
                                          Seconds(0.01));

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    NS_TEST_ASSERT_MSG_NE(m_totalReceivedGw, 0, "Packets are received on GW");
    NS_TEST_ASSERT_MSG_NE(m_totalReceivedUt, 0, "Packets are received on UT");
    NS_TEST_ASSERT_MSG_EQ(m_totalSentGw * 5,
                          m_totalReceivedUt * 4,
                          "Same number of packets sent and received on FWD link (taking into "
                          "account 2 UTs on beam 3)");
    NS_TEST_ASSERT_MSG_EQ(m_totalSentUt,
                          m_totalReceivedGw,
                          "Same number of packets sent and received on RTN link");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 7' test case implementation.
 *
 * This case tests ACM on all links.
 * It is based on a SIMPLE scenario, with network regeneration on FWD and RTN.
 *
 *  Expected result:
 *    Most robust MODCOD used a start of simulation
 *    Efficient MODCOD used on all links after a few seconds
 *    Same MODCOD on FWD feeder and FWD user
 *    Not same MODCOD on RTN feeder and RTN user
 */
class SatRegenerationTest7 : public TestCase
{
  public:
    SatRegenerationTest7();
    virtual ~SatRegenerationTest7();

  private:
    virtual void DoRun(void);
    void GeoPhyGwModcodTraceCb(uint32_t modcod, const Address& address);
    void GeoPhyUtModcodTraceCb(uint32_t modcod, const Address& address);
    void GeoPhyFeederModcodTraceCb(uint32_t modcod, const Address& address);
    void GeoPhyUserModcodTraceCb(uint32_t modcod, const Address& address);

    double GetAverage(std::vector<uint32_t> list, uint32_t beg, uint32_t end);
    double GetMostFrequent(std::vector<uint32_t> list, uint32_t beg, uint32_t end);

    Ptr<SatHelper> m_helper;

    std::vector<uint32_t> m_gwModcods;
    std::vector<uint32_t> m_utModcods;
    std::vector<uint32_t> m_feederModcods;
    std::vector<uint32_t> m_userModcods;
};

void
SatRegenerationTest7::GeoPhyGwModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_gwModcods.push_back(modcod);
}

void
SatRegenerationTest7::GeoPhyUtModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_utModcods.push_back(modcod);
}

void
SatRegenerationTest7::GeoPhyFeederModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_feederModcods.push_back(modcod);
}

void
SatRegenerationTest7::GeoPhyUserModcodTraceCb(uint32_t modcod, const Address& address)
{
    m_userModcods.push_back(modcod);
}

double
SatRegenerationTest7::GetAverage(std::vector<uint32_t> list, uint32_t beg, uint32_t end)
{
    uint32_t sum = 0;
    for (uint32_t i = beg; i < end; i++)
    {
        sum += list[i];
    }
    return 1.0 * sum / (end - beg);
}

double
SatRegenerationTest7::GetMostFrequent(std::vector<uint32_t> list, uint32_t beg, uint32_t end)
{
    std::map<uint32_t, uint32_t> frequencies;
    for (uint32_t i = beg; i < end; i++)
    {
        frequencies[list[i]]++;
    }

    uint32_t max_count = 0;
    uint32_t index = -1;
    for (std::pair<const uint32_t, uint32_t>& i : frequencies)
    {
        if (max_count < i.second)
        {
            index = i.first;
            max_count = i.second;
        }
    }

    return index;
}

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest7::SatRegenerationTest7()
    : TestCase("This case tests ACM on all links. It is based on a SIMPLE scenario.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest7::~SatRegenerationTest7()
{
}

//
// SatRegenerationTest7 TestCase implementation
//
void
SatRegenerationTest7::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test7", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    /// Enable ACM
    Config::SetDefault("ns3::SatBbFrameConf::AcmEnabled", BooleanValue(true));

    // Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-regeneration");
    simulationHelper->SetSimulationTime(Seconds(20));
    simulationHelper->CreateSatScenario(SatHelper::SIMPLE);

    m_helper = simulationHelper->GetSatelliteHelper();

    Ptr<Node> gwNode = m_helper->GwNodes().Get(0);
    Ptr<Node> utNode = m_helper->UtNodes().Get(0);
    Ptr<Node> geoNode = m_helper->GeoSatNodes().Get(0);
    Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy>(
        DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0))->GetFeederPhy(8));
    Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy>(
        DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0))->GetUserPhy(8));
    Ptr<SatPhy> satGwPhy =
        DynamicCast<SatPhy>(DynamicCast<SatNetDevice>(gwNode->GetDevice(1))->GetPhy());
    Ptr<SatPhy> satUtPhy =
        DynamicCast<SatPhy>(DynamicCast<SatNetDevice>(utNode->GetDevice(2))->GetPhy());

    satGwPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest7::GeoPhyGwModcodTraceCb, this));
    satUtPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest7::GeoPhyUtModcodTraceCb, this));
    satGeoFeederPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest7::GeoPhyFeederModcodTraceCb, this));
    satGeoUserPhy->TraceConnectWithoutContext(
        "RxLinkModcod",
        MakeCallback(&SatRegenerationTest7::GeoPhyUserModcodTraceCb, this));

    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(MilliSeconds(20)));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::RTN_LINK,
                                          Seconds(1),
                                          Seconds(10),
                                          Seconds(0.01));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          Seconds(1),
                                          Seconds(10),
                                          Seconds(0.01));

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    double averageGwModcodsBeg = GetAverage(m_gwModcods, 0, 3);
    double averageUtModcodsBeg = GetAverage(m_utModcods, 0, 3);
    double averageFeederModcodsBeg = GetAverage(m_feederModcods, 0, 3);
    double averageUserModcodsBeg = GetAverage(m_userModcods, 0, 3);
    double averageGwModcodsEnd =
        GetMostFrequent(m_gwModcods, m_gwModcods.size() - 200, m_gwModcods.size());
    double averageUtModcodsEnd =
        GetMostFrequent(m_utModcods, m_utModcods.size() - 200, m_utModcods.size());
    double averageFeederModcodsEnd =
        GetMostFrequent(m_feederModcods, m_feederModcods.size() - 200, m_feederModcods.size());
    double averageUserModcodsEnd =
        GetMostFrequent(m_userModcods, m_userModcods.size() - 200, m_userModcods.size());

    NS_TEST_ASSERT_MSG_EQ(averageFeederModcodsBeg,
                          2,
                          "Most robust MODCOD on FWD feeder link at startup");
    NS_TEST_ASSERT_MSG_EQ(averageUtModcodsBeg, 2, "Most robust MODCOD on FWD user link at startup");
    NS_TEST_ASSERT_MSG_EQ(averageGwModcodsBeg,
                          2,
                          "Most robust MODCOD on RTN feeder link at startup");
    NS_TEST_ASSERT_MSG_EQ(averageUserModcodsBeg,
                          1,
                          "Most robust MODCOD on RTN user link at startup");

    NS_TEST_ASSERT_MSG_LT(averageFeederModcodsEnd, 28, "Max MODCOD on FWD feeder link is 27");
    NS_TEST_ASSERT_MSG_LT(averageUtModcodsEnd, 28, "Max MODCOD on FWD user link is 27");
    NS_TEST_ASSERT_MSG_LT(averageGwModcodsEnd, 28, "Max MODCOD on RTN feeder link is 27");
    NS_TEST_ASSERT_MSG_LT(averageUserModcodsEnd, 24, "Max MODCOD on RTN user link is 23");

    NS_TEST_ASSERT_MSG_GT(averageFeederModcodsEnd,
                          2,
                          "Most robust MODCOD on FWD feeder link not used after a few seconds");
    NS_TEST_ASSERT_MSG_GT(averageUtModcodsEnd,
                          2,
                          "Most robust MODCOD on FWD user link not used after a few seconds");
    NS_TEST_ASSERT_MSG_GT(averageGwModcodsEnd,
                          2,
                          "Most robust MODCOD on RTN feeder link not used after a few seconds");
    NS_TEST_ASSERT_MSG_GT(averageUserModcodsEnd,
                          1,
                          "Most robust MODCOD on RTN user link not used after a few seconds");

    NS_TEST_ASSERT_MSG_EQ(averageFeederModcodsEnd,
                          averageUtModcodsEnd,
                          "Same MODCOD on both FWD links");
    NS_TEST_ASSERT_MSG_NE(averageUserModcodsEnd,
                          averageGwModcodsEnd,
                          "Not same MODCOD on both RTN links");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 8' test case implementation.
 *
 * This test is launched several time to test every regeneration combination.
 * We measure if packets are seen on sat traces, and should have (for RX):
 *  - For transparent: only phy traces
 *  - For regeneration phy: only phy traces
 *  - For regeneration link: phy and MAC
 *  - For regeneration network: phy, MAC and network
 *
 * 10 packets are sent from/to each UT per second, during 14s.
 * This means at least 700 packets received for each probe (not counting control packets).
 * When including control packets, we should have at least 1800 packets
 */
class SatRegenerationTest8 : public TestCase
{
  public:
    SatRegenerationTest8(SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                         SatEnums::RegenerationMode_t returnLinkRegenerationMode);
    virtual ~SatRegenerationTest8();

  private:
    virtual void DoRun(void);

    void AddTraceEntry(Time now,
                       SatEnums::SatPacketEvent_t packetEvent,
                       SatEnums::SatNodeType_t nodeType,
                       uint32_t nodeId,
                       Mac48Address macAddress,
                       SatEnums::SatLogLevel_t logLevel,
                       SatEnums::SatLinkDir_t linkDir,
                       std::string packetInfo);

    Ptr<SatHelper> m_helper;

    SatEnums::RegenerationMode_t m_forwardLinkRegenerationMode;
    SatEnums::RegenerationMode_t m_returnLinkRegenerationMode;

    uint32_t m_rxFeederPhy;
    uint32_t m_rxFeederMac;
    uint32_t m_rxFeederNet;
    uint32_t m_rxUserPhy;
    uint32_t m_rxUserMac;
    uint32_t m_rxUserNet;
};

void
SatRegenerationTest8::AddTraceEntry(Time now,
                                    SatEnums::SatPacketEvent_t packetEvent,
                                    SatEnums::SatNodeType_t nodeType,
                                    uint32_t nodeId,
                                    Mac48Address macAddress,
                                    SatEnums::SatLogLevel_t logLevel,
                                    SatEnums::SatLinkDir_t linkDir,
                                    std::string packetInfo)
{
    if (packetEvent != SatEnums::PACKET_RECV)
    {
        return;
    }
    switch (logLevel)
    {
    case SatEnums::LL_PHY: {
        if (linkDir == SatEnums::LD_FORWARD)
        {
            m_rxFeederPhy++;
        }
        else if (linkDir == SatEnums::LD_RETURN)
        {
            m_rxUserPhy++;
        }
        break;
    }
    case SatEnums::LL_MAC: {
        if (linkDir == SatEnums::LD_FORWARD)
        {
            m_rxFeederMac++;
        }
        else if (linkDir == SatEnums::LD_RETURN)
        {
            m_rxUserMac++;
        }
        break;
    }
    case SatEnums::LL_ND: {
        if (linkDir == SatEnums::LD_FORWARD)
        {
            m_rxFeederNet++;
        }
        else if (linkDir == SatEnums::LD_RETURN)
        {
            m_rxUserNet++;
        }
        break;
    }
    default:
        NS_FATAL_ERROR("Wrong log level received");
    }
}

// Add some help text to this case to describe what it is intended to test
SatRegenerationTest8::SatRegenerationTest8(SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                           SatEnums::RegenerationMode_t returnLinkRegenerationMode)
    : TestCase("This test is launched several time to test every regeneration combination.")
{
    m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
    m_returnLinkRegenerationMode = returnLinkRegenerationMode;

    m_rxFeederPhy = 0;
    m_rxFeederMac = 0;
    m_rxFeederNet = 0;
    m_rxUserPhy = 0;
    m_rxUserMac = 0;
    m_rxUserNet = 0;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest8::~SatRegenerationTest8()
{
}

//
// SatRegenerationTest8 TestCase implementation
//
void
SatRegenerationTest8::DoRun(void)
{
    Config::Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-regeneration", "test8", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(m_forwardLinkRegenerationMode));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(m_returnLinkRegenerationMode));

    SatEnums::RegenerationMode_t maxRegeneration =
        std::max(m_forwardLinkRegenerationMode, m_returnLinkRegenerationMode);

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    // Enable SatMac traces
    Config::SetDefault("ns3::SatPhy::EnableStatisticsTags", BooleanValue(true));
    Config::SetDefault("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-regeneration");
    simulationHelper->SetSimulationTime(Seconds(20));
    simulationHelper->CreateSatScenario(SatHelper::LARGER);

    m_helper = simulationHelper->GetSatelliteHelper();

    std::map<uint32_t, Ptr<SatPhy>> satGeoFeederPhy =
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetFeederPhy();
    std::map<uint32_t, Ptr<SatPhy>> satGeoUserPhy =
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetUserPhy();
    std::map<uint32_t, Ptr<SatMac>> satGeoFeederMac =
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetFeederMac();
    std::map<uint32_t, Ptr<SatMac>> satGeoUserMac =
        DynamicCast<SatGeoNetDevice>(m_helper->GeoSatNodes().Get(0)->GetDevice(0))->GetUserMac();

    Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/UserPhy/*/PacketTrace",
                                  MakeCallback(&SatRegenerationTest8::AddTraceEntry, this));
    Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/FeederPhy/*/PacketTrace",
                                  MakeCallback(&SatRegenerationTest8::AddTraceEntry, this));
    if (maxRegeneration == SatEnums::REGENERATION_LINK ||
        maxRegeneration == SatEnums::REGENERATION_NETWORK)
    {
        Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/UserMac/*/PacketTrace",
                                      MakeCallback(&SatRegenerationTest8::AddTraceEntry, this));
        Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/FeederMac/*/PacketTrace",
                                      MakeCallback(&SatRegenerationTest8::AddTraceEntry, this));
    }
    if (maxRegeneration == SatEnums::REGENERATION_NETWORK)
    {
        Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/PacketTrace",
                                      MakeCallback(&SatRegenerationTest8::AddTraceEntry, this));
    }

    Time startTime = Seconds(1);
    Time stopTime = Seconds(15);
    Time startDelay = MilliSeconds(10);
    Time interval = MilliSeconds(100);
    uint32_t packetSize = 512;

    Config::SetDefault("ns3::CbrApplication::Interval", TimeValue(interval));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(packetSize));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::RTN_LINK,
                                          startTime,
                                          stopTime,
                                          startDelay);

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          startTime,
                                          stopTime,
                                          startDelay);

    simulationHelper->RunSimulation();

    Simulator::Destroy();

    switch (m_forwardLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        NS_TEST_ASSERT_MSG_GT(m_rxFeederPhy, 1600, "Packets should be received on feeder phy");
        NS_TEST_ASSERT_MSG_EQ(m_rxFeederMac, 0, "Packets should not be received on feeder MAC");
        NS_TEST_ASSERT_MSG_EQ(m_rxFeederNet, 0, "Packets should not be received on feeder network");
        break;
    }
    case SatEnums::REGENERATION_LINK: {
        NS_TEST_ASSERT_MSG_GT(m_rxFeederPhy, 1600, "Packets should be received on feeder phy");
        NS_TEST_ASSERT_MSG_GT(m_rxFeederMac, 1600, "Packets should be received on feeder MAC");
        NS_TEST_ASSERT_MSG_EQ(m_rxFeederNet, 0, "Packets should not be received on feeder network");
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        NS_TEST_ASSERT_MSG_GT(m_rxFeederPhy, 1600, "Packets should be received on feeder phy");
        NS_TEST_ASSERT_MSG_GT(m_rxFeederMac, 1600, "Packets should be received on feeder MAC");
        NS_TEST_ASSERT_MSG_GT(m_rxFeederNet, 1600, "Packets should be received on feeder network");
        break;
    }
    }

    switch (m_returnLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        NS_TEST_ASSERT_MSG_GT(m_rxUserPhy, 1600, "Packets should be received on user phy");
        NS_TEST_ASSERT_MSG_EQ(m_rxUserMac, 0, "Packets should not be received on user MAC");
        NS_TEST_ASSERT_MSG_EQ(m_rxUserNet, 0, "Packets should not be received on user network");
        break;
    }
    case SatEnums::REGENERATION_LINK: {
        NS_TEST_ASSERT_MSG_GT(m_rxUserPhy, 1600, "Packets should be received on user phy");
        NS_TEST_ASSERT_MSG_GT(m_rxUserMac, 1600, "Packets should be received on user MAC");
        NS_TEST_ASSERT_MSG_EQ(m_rxUserNet, 0, "Packets should not be received on user network");
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        NS_TEST_ASSERT_MSG_GT(m_rxUserPhy, 1600, "Packets should be received on user phy");
        NS_TEST_ASSERT_MSG_GT(m_rxUserMac, 1600, "Packets should be received on user MAC");
        NS_TEST_ASSERT_MSG_GT(m_rxUserNet, 1600, "Packets should be received on user network");
        break;
    }
    }
}

// The TestSuite class names the TestSuite as sat-regeneration-test, identifies what type of
// TestSuite (SYSTEM), and enables the TestCases to be run. Typically, only the constructor for this
// class must be defined
//
class SatRegenerationTestSuite : public TestSuite
{
  public:
    SatRegenerationTestSuite();
};

SatRegenerationTestSuite::SatRegenerationTestSuite()
    : TestSuite("sat-regeneration-test", SYSTEM)
{
    AddTestCase(new SatRegenerationTest1, TestCase::QUICK); // Test delay with regeneration phy
    AddTestCase(new SatRegenerationTest2, TestCase::QUICK); // Test losses with regeneration phy
    AddTestCase(new SatRegenerationTest3, TestCase::QUICK); // Test collisions with regeneration phy
    AddTestCase(new SatRegenerationTest4, TestCase::QUICK); // Test regeneration link
    AddTestCase(new SatRegenerationTest5, TestCase::QUICK); // Test ACM loop on regeneration link
    AddTestCase(new SatRegenerationTest6, TestCase::QUICK); // Test regeneration network
    AddTestCase(new SatRegenerationTest7, TestCase::QUICK); // Test ACM loop on regeneration network

    // Test all regeneration combinations, and check if packets are correctly received or not on
    // each satellite layer
    AddTestCase(new SatRegenerationTest8(SatEnums::TRANSPARENT, SatEnums::TRANSPARENT),
                TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::TRANSPARENT, SatEnums::REGENERATION_PHY),
                TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::TRANSPARENT, SatEnums::REGENERATION_LINK),
                TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::TRANSPARENT, SatEnums::REGENERATION_NETWORK),
                TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::REGENERATION_PHY, SatEnums::TRANSPARENT),
                TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::REGENERATION_PHY, SatEnums::REGENERATION_PHY),
                TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::REGENERATION_PHY, SatEnums::REGENERATION_LINK),
                TestCase::QUICK);
    AddTestCase(
        new SatRegenerationTest8(SatEnums::REGENERATION_PHY, SatEnums::REGENERATION_NETWORK),
        TestCase::QUICK);
    AddTestCase(new SatRegenerationTest8(SatEnums::REGENERATION_NETWORK, SatEnums::TRANSPARENT),
                TestCase::QUICK);
    AddTestCase(
        new SatRegenerationTest8(SatEnums::REGENERATION_NETWORK, SatEnums::REGENERATION_PHY),
        TestCase::QUICK);
    AddTestCase(
        new SatRegenerationTest8(SatEnums::REGENERATION_NETWORK, SatEnums::REGENERATION_LINK),
        TestCase::QUICK);
    AddTestCase(
        new SatRegenerationTest8(SatEnums::REGENERATION_NETWORK, SatEnums::REGENERATION_NETWORK),
        TestCase::QUICK);
}

// Allocate an instance of this TestSuite
static SatRegenerationTestSuite satRegenerationTestSuite;
