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
#include "ns3/simulation-helper.h"
#include "ns3/satellite-helper.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-ut-mac-state.h"
#include "ns3/satellite-gw-mac.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-geo-feeder-phy.h"
#include "ns3/satellite-geo-user-phy.h"
#include "ns3/satellite-phy-rx-carrier.h"

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

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest1::~SatRegenerationTest1 ()
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

//
// SatRegenerationTest1 TestCase implementation
//
void
SatRegenerationTest1::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-regeneration", "test1", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100));

  // Enable traces
  Config::SetDefault ("ns3::SatPhy::EnableStatisticsTags", BooleanValue (true));
  Config::SetDefault ("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue (true));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("500ms"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512));

  // Creating the reference system.
  m_helper = CreateObject<SatHelper> ();
  m_helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer utUsers = m_helper->GetUtUsers ();
  NodeContainer gwUsers = m_helper->GetGwUsers ();
  uint16_t port = 9;

  // Install forward traffic
  CbrHelper cbrForward ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (utUsers.Get (0)), port)));
  ApplicationContainer gwAppsForward = cbrForward.Install (gwUsers);
  gwAppsForward.Start (Seconds (1.0));
  gwAppsForward.Stop (Seconds (5.0));

  PacketSinkHelper sinkForward ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (utUsers.Get (0)), port)));
  ApplicationContainer utAppsForward = sinkForward.Install (utUsers);
  utAppsForward.Start (Seconds (1.0));
  utAppsForward.Stop (Seconds (10.0));

  // Install return traffic
  CbrHelper cbrReturn ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer utAppsReturn = cbrReturn.Install (utUsers);
  utAppsReturn.Start (Seconds (1.0));
  utAppsReturn.Stop (Seconds (5.0));

  PacketSinkHelper sinkReturn ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer gwAppsReturn = sinkReturn.Install (gwUsers);
  gwAppsReturn.Start (Seconds (1.0));
  gwAppsReturn.Stop (Seconds (10.0));

  m_gwAddress = m_helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();
  m_stAddress = m_helper->UtNodes ().Get (0)->GetDevice (2)->GetAddress ();

  Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetFeederPhy (8));
  Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetUserPhy (8));

  // TODO update with link delay
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
 * \brief 'Regeneration, test 2' test case implementation.
 *
 * This case tests physical regeneration on satellite. It is based on a SIMPLE scenario, with losses on uplink, forward and return.
 *
 *  Expected result:
 *    Packets are lost on uplink with expect drop rate
 *    No packets are lost on downlink
 *    All packets received on uplink are received on GW/UT
 */
class SatRegenerationTest2 : public TestCase
{
public:
  SatRegenerationTest2 ();
  virtual ~SatRegenerationTest2 ();

private:
  virtual void DoRun (void);
  void GeoPhyTraceCb (Time time,
                      SatEnums::SatPacketEvent_t event,
                      SatEnums::SatNodeType_t type,
                      uint32_t nodeId,
                      Mac48Address address,
                      SatEnums::SatLogLevel_t level,
                      SatEnums::SatLinkDir_t dir,
                      std::string packetInfo);
  void PhyTraceCb (Time time,
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
SatRegenerationTest2::SatRegenerationTest2 ()
  : TestCase ("This case tests physical regeneration on satellite. It is based on a SIMPLE scenario, with losses on uplink, forward and return."),
  m_packetsReceivedFeeder (0),
  m_packetsDroppedFeeder (0),
  m_packetsReceivedUser (0),
  m_packetsDroppedUser (0),
  m_packetsReceivedUt (0),
  m_packetsDroppedUt (0),
  m_packetsReceivedGw (0),
  m_packetsDroppedGw (0)
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest2::~SatRegenerationTest2 ()
{
}

void
SatRegenerationTest2::GeoPhyTraceCb (Time time,
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
SatRegenerationTest2::PhyTraceCb (Time time,
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
SatRegenerationTest2::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-regeneration", "test2", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100));

  /// Set constant 10% losses on Uplink
  Config::SetDefault ("ns3::SatGeoHelper::FwdLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_CONSTANT));
  Config::SetDefault ("ns3::SatGeoHelper::FwdLinkConstantErrorRate", DoubleValue (0.1));
  Config::SetDefault ("ns3::SatGeoHelper::RtnLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_CONSTANT));
  Config::SetDefault ("ns3::SatGeoHelper::RtnLinkConstantErrorRate", DoubleValue (0.1));

  // Enable SatMac traces
  Config::SetDefault ("ns3::SatPhy::EnableStatisticsTags", BooleanValue (true));
  Config::SetDefault ("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue (true));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("10ms"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512));

  // Creating the reference system.
  m_helper = CreateObject<SatHelper> ();
  m_helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  NodeContainer utUsers = m_helper->GetUtUsers ();
  NodeContainer gwUsers = m_helper->GetGwUsers ();
  uint16_t port = 9;

  // Install forward traffic
  CbrHelper cbrForward ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (utUsers.Get (0)), port)));
  ApplicationContainer gwAppsForward = cbrForward.Install (gwUsers);
  gwAppsForward.Start (Seconds (1.0));
  gwAppsForward.Stop (Seconds (59.0));

  PacketSinkHelper sinkForward ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (utUsers.Get (0)), port)));
  ApplicationContainer utAppsForward = sinkForward.Install (utUsers);
  utAppsForward.Start (Seconds (1.0));
  utAppsForward.Stop (Seconds (60.0));

  // Install return traffic
  CbrHelper cbrReturn ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer utAppsReturn = cbrReturn.Install (utUsers);
  utAppsReturn.Start (Seconds (1.0));
  utAppsReturn.Stop (Seconds (59.0));

  PacketSinkHelper sinkReturn ("ns3::UdpSocketFactory", Address (InetSocketAddress (m_helper->GetUserAddress (gwUsers.Get (0)), port)));
  ApplicationContainer gwAppsReturn = sinkReturn.Install (gwUsers);
  gwAppsReturn.Start (Seconds (1.0));
  gwAppsReturn.Stop (Seconds (60.0));

  Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetFeederPhy (8));
  Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetUserPhy (8));

  satGeoFeederPhy->TraceConnectWithoutContext ("PacketTrace", MakeCallback (&SatRegenerationTest2::GeoPhyTraceCb, this));
  satGeoUserPhy->TraceConnectWithoutContext ("PacketTrace", MakeCallback (&SatRegenerationTest2::GeoPhyTraceCb, this));

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatPhy/PacketTrace", MakeCallback (&SatRegenerationTest2::PhyTraceCb, this));

  Simulator::Stop (Seconds (60));
  Simulator::Run ();

  Simulator::Destroy ();


  double dropRateForwardFeeder = (m_packetsDroppedFeeder*100.0/(m_packetsReceivedFeeder+m_packetsDroppedFeeder));
  double dropRateReturnUser = (m_packetsDroppedUser*100.0/(m_packetsReceivedUser+m_packetsDroppedUser));

  double forwardDifference = std::abs((1.0*m_packetsReceivedUt - m_packetsReceivedFeeder)/m_packetsReceivedUt);
  double returnDifference = std::abs((1.0*m_packetsReceivedGw - m_packetsReceivedUser)/m_packetsReceivedGw);

  NS_TEST_ASSERT_MSG_GT (dropRateForwardFeeder, 8.0, "Not enough losses on FWD feeder");
  NS_TEST_ASSERT_MSG_LT (dropRateForwardFeeder, 12.0, "Too much losses on FWD feeder");
  NS_TEST_ASSERT_MSG_GT (dropRateReturnUser, 8.0, "Not enough losses on RTN user");
  NS_TEST_ASSERT_MSG_LT (dropRateReturnUser, 12.0, "Too much losses on RTN user");

  NS_TEST_ASSERT_MSG_NE (m_packetsReceivedGw, 0, "Packets must be received by GW");
  NS_TEST_ASSERT_MSG_NE (m_packetsReceivedUt, 0, "Packets must be received by UT");
  NS_TEST_ASSERT_MSG_EQ (m_packetsDroppedGw, 0, "Packets must not be dropped by GW");
  NS_TEST_ASSERT_MSG_EQ (m_packetsDroppedUt, 0, "Packets must not be dropped by UT");

  NS_TEST_ASSERT_MSG_LT (forwardDifference, 0.01, "Number of packets received on FWD should be almost the same between SAT and UT");
  NS_TEST_ASSERT_MSG_LT (returnDifference, 0.01, "Number of packets received on RTN should be almost the same between SAT and GW");
}

/**
 * \ingroup satellite
 * \brief 'Regeneration, test 3' test case implementation.
 *
 * This case tests physical regeneration on satellite. It is based on a LARGER scenario, with collisions on return uplink.
 *
 *  Expected result:
 *    TODO
 */
class SatRegenerationTest3 : public TestCase
{
public:
  SatRegenerationTest3 ();
  virtual ~SatRegenerationTest3 ();

private:
  virtual void DoRun (void);
  void GeoPhyTraceErrorCb (std::string, uint32_t nPackets, const Address & address , bool hasError);
  void GeoPhyTraceCollisionCb (std::string, uint32_t nPackets, const Address & address , bool hasCollision);
  void GeoPhyTraceCb (Time time,
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
SatRegenerationTest3::SatRegenerationTest3 ()
  : TestCase ("This case tests physical regeneration on satellite. It is based on a LARGER scenario, with collisions on return uplink."),
  m_nbErrorpacketsFwd (0),
  m_nbErrorpacketsRtn (0),
  m_nbCollisionPacketsFwd (0),
  m_nbCollisionPacketsRtn (0),
  m_packetsReceivedFeeder (0),
  m_packetsDroppedFeeder (0),
  m_packetsReceivedUser (0),
  m_packetsDroppedUser (0)
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatRegenerationTest3::~SatRegenerationTest3 ()
{
}

void
SatRegenerationTest3::GeoPhyTraceErrorCb (std::string, uint32_t nPackets, const Address & address , bool hasError)
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
SatRegenerationTest3::GeoPhyTraceCollisionCb (std::string, uint32_t nPackets, const Address & address , bool hasCollision)
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
SatRegenerationTest3::GeoPhyTraceCb (Time time,
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
SatRegenerationTest3::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-regeneration", "test3", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatBeamHelper::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));
  Config::SetDefault ("ns3::SatBeamHelper::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_PHY));

  Config::SetDefault ("ns3::SatGeoFeederPhy::QueueSize", UintegerValue (100));

  /// Enable SatMac traces
  Config::SetDefault ("ns3::SatPhy::EnableStatisticsTags", BooleanValue (true));
  Config::SetDefault ("ns3::SatNetDevice::EnableStatisticsTags", BooleanValue (true));

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  // Enable Random Access
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));

  // Set Random Access interference model
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));

  // Disable periodic control slots
  Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (false));

  // Set dynamic load control parameters
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (false));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize", UintegerValue (10));

  // Set random access parameters
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed", UintegerValue (6));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock", UintegerValue (2));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds", UintegerValue (250));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability", UintegerValue (10000));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability", UintegerValue (30000));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances", UintegerValue (3));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold", DoubleValue (0.5));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval", TimeValue (MilliSeconds (100)));
  Config::SetDefault ("ns3::SatRandomAccessConf::CrdsaSignalingOverheadInBytes", UintegerValue (5));
  Config::SetDefault ("ns3::SatRandomAccessConf::SlottedAlohaSignalingOverheadInBytes", UintegerValue (3));

  // Disable CRA and DA
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
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

  Config::SetDefault ("ns3::SatGeoHelper::FwdLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_NONE));
  Config::SetDefault ("ns3::SatGeoHelper::RtnLinkErrorModel", EnumValue (SatPhyRxCarrierConf::EM_AVI));
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("test-sat-regeneration3");
  simulationHelper->SetSimulationTime (Seconds (5));
  simulationHelper->SetUserCountPerUt (1);
  simulationHelper->SetUtCountPerBeam (50);
  simulationHelper->SetBeamSet ({1});
  simulationHelper->CreateSatScenario ();

  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512));
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR,
    SimulationHelper::UDP,
    SimulationHelper::RTN_LINK,
    Seconds (0.01), Seconds (5), Seconds (0.01));

  m_helper = simulationHelper->GetSatelliteHelper ();

  Ptr<SatGeoFeederPhy> satGeoFeederPhy = DynamicCast<SatGeoFeederPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetFeederPhy (1));
  Ptr<SatGeoUserPhy> satGeoUserPhy = DynamicCast<SatGeoUserPhy> (DynamicCast<SatGeoNetDevice> (m_helper->GeoSatNode ()->GetDevice (0))->GetUserPhy (1));

  m_gwAddress = m_helper->GwNodes ().Get (0)->GetDevice (1)->GetAddress ();

  satGeoFeederPhy->TraceConnectWithoutContext ("PacketTrace", MakeCallback (&SatRegenerationTest3::GeoPhyTraceCb, this));
  satGeoUserPhy->TraceConnectWithoutContext ("PacketTrace", MakeCallback (&SatRegenerationTest3::GeoPhyTraceCb, this));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxError", MakeCallback (&SatRegenerationTest3::GeoPhyTraceErrorCb, this));
  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxCollision", MakeCallback (&SatRegenerationTest3::GeoPhyTraceCollisionCb, this));
  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxError", MakeCallback (&SatRegenerationTest3::GeoPhyTraceErrorCb, this));
  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/1/PhyRx/RxCarrierList/*/SlottedAlohaRxCollision", MakeCallback (&SatRegenerationTest3::GeoPhyTraceCollisionCb, this));

  Simulator::Stop (Seconds (5));
  Simulator::Run ();

  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_NE (m_packetsReceivedFeeder, 0, "Packets received on FWD feeder link");
  NS_TEST_ASSERT_MSG_NE (m_packetsReceivedUser, 0, "Packets received on RTN user link due to collisions");
  NS_TEST_ASSERT_MSG_EQ (m_packetsDroppedFeeder, 0, "No packets dropped on FWD feeder link");
  NS_TEST_ASSERT_MSG_NE (m_packetsDroppedUser, 0, "Packets dropped on RTN user link due to collisions");

  NS_TEST_ASSERT_MSG_EQ (m_nbErrorpacketsFwd, 0, "No errors on FWD feeder link");
  NS_TEST_ASSERT_MSG_EQ (m_nbCollisionPacketsFwd, 0, "No collisions on FWD feeder link");
  NS_TEST_ASSERT_MSG_NE (m_nbErrorpacketsRtn, 0, "Need errors on RTN feeder link");
  NS_TEST_ASSERT_MSG_NE (m_nbCollisionPacketsRtn, 0, "Need collisions on RTN feeder link");
}

// The TestSuite class names the TestSuite as sat-regeneration-test, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run. Typically, only the constructor for this class must be defined
//
class SatRegenerationTestSuite : public TestSuite
{
public:
  SatRegenerationTestSuite ();
};

SatRegenerationTestSuite::SatRegenerationTestSuite ()
  : TestSuite ("sat-regeneration-test", SYSTEM)
{
  //AddTestCase (new SatRegenerationTest1, TestCase::QUICK); // Test delay with regeneration phy
  //AddTestCase (new SatRegenerationTest2, TestCase::QUICK); // Test losses with regeneration phy
  AddTestCase (new SatRegenerationTest3, TestCase::QUICK); // Test collisions with regeneration phy
}

// Allocate an instance of this TestSuite
static SatRegenerationTestSuite satRegenerationTestSuite;

