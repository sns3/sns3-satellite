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
 * \file satellite-constellation-test.cc
 * \ingroup satellite
 * \brief Constellation test case implementations.
 *
 * In this module implements the Constellation Test Cases.
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
 * \brief 'Constellation, test 1' test case implementation.
 *
 * This case tests that a topology is correctly loaded.
 * It load the eutelsat-geo-2-sats configuration
 *
 *  Expected result:
 *    Correct number of entities: 2 satellites, 2 GWs, 2 UTs
 *    Correct position of entities:
 *       - satellites: (0, 7.2, 36M) and (0, 47.7, 36M)
 *       - gws: (48.85, 2.34, 0) and (55.75, 37.62, 0)
 *       - uts: (48.85341, 2.3488, 0) and (55.754996, 37.621849, 0)
 */
class SatConstellationTest1 : public TestCase
{
public:
  SatConstellationTest1 ();
  virtual ~SatConstellationTest1 ();

private:
  virtual void DoRun (void);

  Ptr<SatHelper> m_helper;
};

// Add some help text to this case to describe what it is intended to test
SatConstellationTest1::SatConstellationTest1 ()
  : TestCase ("This case tests that a topology is correctly loaded.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatConstellationTest1::~SatConstellationTest1 ()
{
}

//
// SatConstellationTest1 TestCase implementation
//
void
SatConstellationTest1::DoRun (void)
{
  Config::Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-constellation", "test1", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatConf::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));
  Config::SetDefault ("ns3::SatConf::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));

  /// Use constellation
  Config::SetDefault ("ns3::SatHelper::SatConstellationEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::SatConstellationFolder", StringValue ("eutelsat-geo-2-sats"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::StartDateStr", StringValue ("2022-11-13 12:00:00"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionEachRequest", BooleanValue (false));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionPeriod", TimeValue (Seconds (1)));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("test-sat-constellation");

  // Creating the reference system.
  simulationHelper->SetBeamSet ({43, 30});
  simulationHelper->SetUserCountPerUt (5);

  simulationHelper->CreateSatScenario ();
  m_helper = simulationHelper->GetSatelliteHelper ();

  NodeContainer sats = m_helper->GeoSatNodes ();
  NodeContainer gws = m_helper->GwNodes ();
  NodeContainer uts = m_helper->UtNodes ();
  NodeContainer gwUsers = m_helper->GetGwUsers ();
  NodeContainer utUsers = m_helper->GetUtUsers ();

  NS_TEST_ASSERT_MSG_EQ (sats.GetN (), 2, "Topology must contain 2 satellites");
  NS_TEST_ASSERT_MSG_EQ (gws.GetN (), 2, "Topology must contain 2 GWs");
  NS_TEST_ASSERT_MSG_EQ (uts.GetN (), 2, "Topology must contain 2 UTs");
  NS_TEST_ASSERT_MSG_EQ (gwUsers.GetN (), 2, "Topology must contain 2 GW users");
  NS_TEST_ASSERT_MSG_EQ (utUsers.GetN (), 10, "Topology must contain 2 UT users");

  GeoCoordinate sat1 = GeoCoordinate (sats.Get (0)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate sat2 = GeoCoordinate (sats.Get (1)->GetObject<SatMobilityModel> ()->GetPosition ());

  GeoCoordinate gw1 = GeoCoordinate (gws.Get (0)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate gw2 = GeoCoordinate (gws.Get (1)->GetObject<SatMobilityModel> ()->GetPosition ());

  GeoCoordinate ut1 = GeoCoordinate (uts.Get (0)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate ut2 = GeoCoordinate (uts.Get (1)->GetObject<SatMobilityModel> ()->GetPosition ());

  NS_TEST_ASSERT_MSG_EQ_TOL (sat1.GetLatitude (), 0.0130445, 0.001, "Incorrect latitude for satellite 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat1.GetLongitude (), 7.20984, 0.001, "Incorrect longitude for satellite 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat1.GetAltitude (), 35786400, 100, "Incorrect altitude for satellite 1");

  NS_TEST_ASSERT_MSG_EQ_TOL (sat2.GetLatitude (), -0.0219757, 0.001, "Incorrect latitude for satellite 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat2.GetLongitude (), 47.7062, 0.001, "Incorrect longitude for satellite 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat2.GetAltitude (), 35779000, 100, "Incorrect altitude for satellite 2");

  NS_TEST_ASSERT_MSG_EQ_TOL (gw1.GetLatitude (), 48.85, 0.001, "Incorrect latitude for GW 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (gw1.GetLongitude (), 2.34, 0.001, "Incorrect longitude for GW 1");
  NS_TEST_ASSERT_MSG_EQ (gw1.GetAltitude (), 0, "Incorrect altitude for GW 1");

  NS_TEST_ASSERT_MSG_EQ_TOL (gw2.GetLatitude (), 48.85, 0.001, "Incorrect latitude for GW 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (gw2.GetLongitude (), 2.34, 0.001, "Incorrect longitude for GW 2");
  NS_TEST_ASSERT_MSG_EQ (gw2.GetAltitude (), 0, "Incorrect altitude for GW 2");

  NS_TEST_ASSERT_MSG_EQ_TOL (ut1.GetLatitude (), 55.755, 0.001, "Incorrect latitude for UT 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (ut1.GetLongitude (), 37.6218, 0.001, "Incorrect longitude for UT 1");
  NS_TEST_ASSERT_MSG_EQ (ut1.GetAltitude (), 0, "Incorrect altitude for UT 1");

  NS_TEST_ASSERT_MSG_EQ_TOL (ut2.GetLatitude (), 48.8534, 0.001, "Incorrect latitude for UT 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (ut2.GetLongitude (), 2.3488, 0.001, "Incorrect longitude for UT 2");
  NS_TEST_ASSERT_MSG_EQ (ut2.GetAltitude (), 0, "Incorrect altitude for UT 2");

  Simulator::Destroy ();
}

// The TestSuite class names the TestSuite as sat-constellation-test, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run. Typically, only the constructor for this class must be defined
//
class SatConstellationTestSuite : public TestSuite
{
public:
  SatConstellationTestSuite ();
};

SatConstellationTestSuite::SatConstellationTestSuite ()
  : TestSuite ("sat-constellation-test", SYSTEM)
{
  AddTestCase (new SatConstellationTest1, TestCase::QUICK); // Test topology loading
}

// Allocate an instance of this TestSuite
static SatConstellationTestSuite satConstellationTestSuite;

