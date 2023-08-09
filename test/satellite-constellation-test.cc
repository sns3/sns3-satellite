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

#include <iostream>
#include <fstream>

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
#include "ns3/satellite-id-mapper.h"
#include "ns3/satellite-isl-arbiter.h"
#include "ns3/satellite-isl-arbiter-unicast.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief 'Constellation, test 1' test case implementation.
 *
 * This case tests that a topology is correctly loaded.
 * It load the eutelsat-geo-2-sats configuration
 *
 *  Expected result:
 *    Correct number of entities: 2 satellites, 2 GWs, 3 UTs
 *    Correct position of entities:
 *       - satellites: (0, 7.2, 36M) and (0, 47.7, 36M)
 *       - gws: (48.85, 2.34, 0) and (55.75, 37.62, 0)
 *       - uts: (48.85341, 2.3488, 0), (55.754996, 37.621849, 0) and (55.754, 37.621, 0)
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
  Config::SetDefault ("ns3::SatHelper::SatConstellationFolder", StringValue ("eutelsat-geo-2-sats-no-isls"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::StartDateStr", StringValue ("2022-11-13 12:00:00"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionEachRequest", BooleanValue (false));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionPeriod", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::SatHelper::GwUsers", UintegerValue (3));

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

  uint32_t countNetDevices = 0;
  for (uint32_t i = 0; i < sats.GetN (); i++)
    {
      for (uint32_t j = 0; j < sats.Get (i)->GetNDevices (); j++)
        {
          if (DynamicCast<SatGeoNetDevice> (sats.Get (i)->GetDevice (j)) != nullptr)
            {
              countNetDevices += 1;
            }
        }
    }

  NS_TEST_ASSERT_MSG_EQ (sats.GetN (), 2, "Topology must contain 2 satellites");
  NS_TEST_ASSERT_MSG_EQ (countNetDevices, 2, "Topology must contain 2 satellite Geo Net Devices");
  NS_TEST_ASSERT_MSG_EQ (gws.GetN (), 2, "Topology must contain 2 GWs");
  NS_TEST_ASSERT_MSG_EQ (uts.GetN (), 3, "Topology must contain 3 UTs");
  NS_TEST_ASSERT_MSG_EQ (gwUsers.GetN (), 3, "Topology must contain 3 GW users");
  NS_TEST_ASSERT_MSG_EQ (utUsers.GetN (), 15, "Topology must contain 15 UT users");

  GeoCoordinate sat1 = GeoCoordinate (sats.Get (0)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate sat2 = GeoCoordinate (sats.Get (1)->GetObject<SatMobilityModel> ()->GetPosition ());

  GeoCoordinate gw1 = GeoCoordinate (gws.Get (0)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate gw2 = GeoCoordinate (gws.Get (1)->GetObject<SatMobilityModel> ()->GetPosition ());

  GeoCoordinate ut1 = GeoCoordinate (uts.Get (0)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate ut2 = GeoCoordinate (uts.Get (1)->GetObject<SatMobilityModel> ()->GetPosition ());
  GeoCoordinate ut3 = GeoCoordinate (uts.Get (2)->GetObject<SatMobilityModel> ()->GetPosition ());

  NS_TEST_ASSERT_MSG_EQ_TOL (sat1.GetLatitude (), 0.0130445, 0.001, "Incorrect latitude for satellite 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat1.GetLongitude (), 7.20984, 0.001, "Incorrect longitude for satellite 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat1.GetAltitude (), 35786400, 100, "Incorrect altitude for satellite 1");

  NS_TEST_ASSERT_MSG_EQ_TOL (sat2.GetLatitude (), -0.0219757, 0.001, "Incorrect latitude for satellite 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat2.GetLongitude (), 47.7062, 0.001, "Incorrect longitude for satellite 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (sat2.GetAltitude (), 35779000, 100, "Incorrect altitude for satellite 2");

  NS_TEST_ASSERT_MSG_EQ_TOL (gw1.GetLatitude (), 48.85, 0.001, "Incorrect latitude for GW 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (gw1.GetLongitude (), 2.34, 0.001, "Incorrect longitude for GW 1");
  NS_TEST_ASSERT_MSG_EQ (gw1.GetAltitude (), 0, "Incorrect altitude for GW 1");

  NS_TEST_ASSERT_MSG_EQ_TOL (gw2.GetLatitude (), 55.75, 0.001, "Incorrect latitude for GW 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (gw2.GetLongitude (), 37.62, 0.001, "Incorrect longitude for GW 2");
  NS_TEST_ASSERT_MSG_EQ (gw2.GetAltitude (), 0, "Incorrect altitude for GW 2");

  NS_TEST_ASSERT_MSG_EQ_TOL (ut1.GetLatitude (), 48.8534, 0.001, "Incorrect latitude for UT 1");
  NS_TEST_ASSERT_MSG_EQ_TOL (ut1.GetLongitude (), 2.3488, 0.001, "Incorrect longitude for UT 1");
  NS_TEST_ASSERT_MSG_EQ (ut1.GetAltitude (), 0, "Incorrect altitude for UT 1");

  NS_TEST_ASSERT_MSG_EQ_TOL (ut2.GetLatitude (), 55.755, 0.001, "Incorrect latitude for UT 2");
  NS_TEST_ASSERT_MSG_EQ_TOL (ut2.GetLongitude (), 37.6218, 0.001, "Incorrect longitude for UT 2");
  NS_TEST_ASSERT_MSG_EQ (ut2.GetAltitude (), 0, "Incorrect altitude for UT 2");

  NS_TEST_ASSERT_MSG_EQ_TOL (ut3.GetLatitude (), 55.754, 0.001, "Incorrect latitude for UT 3");
  NS_TEST_ASSERT_MSG_EQ_TOL (ut3.GetLongitude (), 37.621, 0.001, "Incorrect longitude for UT 3");
  NS_TEST_ASSERT_MSG_EQ (ut3.GetAltitude (), 0, "Incorrect altitude for UT 3");

  Simulator::Destroy ();
}

/**
 * \ingroup satellite
 * \brief 'Constellation, test 2 test case implementation.
 *
 * This case tests that a application throughputs PerEntity are correct.
 * It generates a conf with 2 satellites, 2 GWs, 3 UTs and 3 UT users per UT.
 * Each UT user receives 409.6 kb/s.
 *
 *  Expected throughputs (after 3s):
 *     - 6144 kb/s globally
 *     - 2048 kb/s and 4096 kb/s per satellite
 *     - 2048 kb/s per UT
 *     - 2048 kb/s and 4096 kb/s per GW
 *     - 409.6 kb/s per UT user
 *     - 2048 kb/s and 4096 kb/s per satellite
 */
class SatConstellationTest2 : public TestCase
{
public:
  SatConstellationTest2 ();
  virtual ~SatConstellationTest2 ();

private:
  virtual void DoRun (void);

  std::vector<std::string> Split (std::string s, char del);
  void TestFileValue (std::string path, uint32_t time, uint32_t expectedValue);

  Ptr<SatHelper> m_helper;
};

// Add some help text to this case to describe what it is intended to test
SatConstellationTest2::SatConstellationTest2 ()
  : TestCase ("This case tests that a application throughputs PerEntity are correct.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatConstellationTest2::~SatConstellationTest2 ()
{
}


std::vector<std::string>
SatConstellationTest2::Split (std::string s, char del)
{
  std::stringstream ss(s);
  std::string word;
  std::vector<std::string> tokens;
  while (!ss.eof())
    {
      std::getline(ss, word, del);
      tokens.push_back (word);
    }
  return tokens;
}

void
SatConstellationTest2::TestFileValue (std::string path, uint32_t time, uint32_t expectedValue)
{
  std::string line;
  std::ifstream myfile (path);
  std::string delimiter = " ";
  std::string token;
  bool valueFound;

  if (myfile.is_open())
    {
      valueFound = false;
      while ( std::getline (myfile, line) )
        {
          std::vector<std::string> tokens = Split (line, ' ');
          if (tokens.size () == 2)
            {
              if ((uint32_t) std::stoi(tokens[0]) == time)
                {
                  NS_TEST_ASSERT_MSG_EQ_TOL (std::stof(tokens[1]), expectedValue, expectedValue/10, "Incorrect throughput for statistic " << path);
                  valueFound = true;
                  break;
                }
            }
        }
      myfile.close();
      if (!valueFound)
        {
          NS_TEST_ASSERT_MSG_EQ (0, 1, "Cannot find time " << time << " for trace file " << path);
        }
    }
  else
    {
      NS_TEST_ASSERT_MSG_EQ (0, 1, "Cannot read trace file " << path);
    }
}

//
// SatConstellationTest2 TestCase implementation
//
void
SatConstellationTest2::DoRun (void)
{
  Config::Reset ();
  Singleton<SatIdMapper>::Get ()->Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-constellation", "test2", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatConf::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));
  Config::SetDefault ("ns3::SatConf::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));

  /// Use constellation
  Config::SetDefault ("ns3::SatHelper::SatConstellationEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::SatConstellationFolder", StringValue ("eutelsat-geo-2-sats-no-isls"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::StartDateStr", StringValue ("2022-11-13 12:00:00"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionEachRequest", BooleanValue (false));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionPeriod", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::SatHelper::GwUsers", UintegerValue (3));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("10ms"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512) );

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("test-sat-constellation/test2");

  // Creating the reference system.
  simulationHelper->SetBeamSet ({43, 30});
  simulationHelper->SetUserCountPerUt (5);

  simulationHelper->CreateSatScenario ();
  m_helper = simulationHelper->GetSatelliteHelper ();

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    Seconds (1.0), Seconds (29.0));
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (1.0), Seconds (29.0));

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->SetSimulationTime (Seconds (10));
  simulationHelper->RunSimulation ();

  Simulator::Destroy ();

  // Global throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-global-fwd-app-throughput-scatter-0.txt", 5, 6144);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-global-rtn-app-throughput-scatter-0.txt", 5, 6144);

  // Per satellite throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-sat-fwd-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-sat-fwd-app-throughput-scatter-2.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-sat-rtn-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-sat-rtn-app-throughput-scatter-2.txt", 5, 4096);

  // Per beam throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-fwd-app-throughput-scatter-1-30.txt", 5, 0);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-fwd-app-throughput-scatter-1-43.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-fwd-app-throughput-scatter-2-30.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-fwd-app-throughput-scatter-2-43.txt", 5, 0);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-rtn-app-throughput-scatter-1-30.txt", 5, 0);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-rtn-app-throughput-scatter-1-43.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-rtn-app-throughput-scatter-2-30.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-beam-rtn-app-throughput-scatter-2-43.txt", 5, 0);

  // Per GW throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-gw-fwd-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-gw-fwd-app-throughput-scatter-2.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-gw-rtn-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-gw-rtn-app-throughput-scatter-2.txt", 5, 4096);

  // Per UT throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-ut-fwd-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-ut-fwd-app-throughput-scatter-2.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-ut-fwd-app-throughput-scatter-3.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-ut-rtn-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-ut-rtn-app-throughput-scatter-2.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test2/stat-per-ut-rtn-app-throughput-scatter-3.txt", 5, 2048);
}

/**
 * \ingroup satellite
 * \brief 'Constellation, test 3 test case implementation.
 *
 * This case tests that a application throughputs PerEntity are correct, with one ISL link.
 * It generates a conf with 2 satellites, 2 GWs, 3 UTs and 3 UT users per UT.
 * Each UT user receives 409.6 kb/s.
 *
 *  Expected throughputs (after 3s):
 *     - 6144 kb/s globally
 *     - 2048 kb/s and 4096 kb/s per satellite
 *     - 2048 kb/s per UT
 *     - 2048 kb/s and 4096 kb/s per GW
 *     - 409.6 kb/s per UT user
 *     - 2048 kb/s and 4096 kb/s per satellite
 */
class SatConstellationTest3 : public TestCase
{
public:
  SatConstellationTest3 ();
  virtual ~SatConstellationTest3 ();

private:
  virtual void DoRun (void);

  std::vector<std::string> Split (std::string s, char del);
  void TestFileValue (std::string path, uint32_t time, uint32_t expectedValue);

  Ptr<SatHelper> m_helper;
};

// Add some help text to this case to describe what it is intended to test
SatConstellationTest3::SatConstellationTest3 ()
  : TestCase ("This case tests that a application throughputs PerEntity are correct.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatConstellationTest3::~SatConstellationTest3 ()
{
}


std::vector<std::string>
SatConstellationTest3::Split (std::string s, char del)
{
  std::stringstream ss(s);
  std::string word;
  std::vector<std::string> tokens;
  while (!ss.eof())
    {
      std::getline(ss, word, del);
      tokens.push_back (word);
    }
  return tokens;
}

void
SatConstellationTest3::TestFileValue (std::string path, uint32_t time, uint32_t expectedValue)
{
  std::string line;
  std::ifstream myfile (path);
  std::string delimiter = " ";
  std::string token;
  bool valueFound;

  if (myfile.is_open())
    {
      valueFound = false;
      while ( std::getline (myfile, line) )
        {
          std::vector<std::string> tokens = Split (line, ' ');
          if (tokens.size () == 2)
            {
              if ((uint32_t) std::stoi(tokens[0]) == time)
                {
                  NS_TEST_ASSERT_MSG_EQ_TOL (std::stof(tokens[1]), expectedValue, expectedValue/10, "Incorrect throughput for statistic " << path);
                  valueFound = true;
                  break;
                }
            }
        }
      myfile.close();
      if (!valueFound)
        {
          NS_TEST_ASSERT_MSG_EQ (0, 1, "Cannot find time " << time << " for trace file " << path);
        }
    }
  else
    {
      NS_TEST_ASSERT_MSG_EQ (0, 1, "Cannot read trace file " << path);
    }
}

//
// SatConstellationTest3 TestCase implementation
//
void
SatConstellationTest3::DoRun (void)
{
  Config::Reset ();
  Singleton<SatIdMapper>::Get ()->Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-constellation", "test3", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatConf::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));
  Config::SetDefault ("ns3::SatConf::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));

  /// Use constellation
  Config::SetDefault ("ns3::SatHelper::SatConstellationEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::SatConstellationFolder", StringValue ("eutelsat-geo-2-sats-isls"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::StartDateStr", StringValue ("2022-11-13 12:00:00"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionEachRequest", BooleanValue (false));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionPeriod", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::SatHelper::GwUsers", UintegerValue (3));

  Config::SetDefault ("ns3::CbrApplication::Interval", StringValue ("10ms"));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (512) );

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("test-sat-constellation/test3");

  // Creating the reference system.
  simulationHelper->SetBeamSet ({43, 30});
  simulationHelper->SetUserCountPerUt (5);

  simulationHelper->CreateSatScenario ();
  m_helper = simulationHelper->GetSatelliteHelper ();

  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
    Seconds (1.0), Seconds (29.0));
  simulationHelper->InstallTrafficModel (
    SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
    Seconds (1.0), Seconds (29.0));

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddGlobalFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerSatRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);

  simulationHelper->SetSimulationTime (Seconds (10));
  simulationHelper->RunSimulation ();

  Simulator::Destroy ();

  // Global throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-global-fwd-app-throughput-scatter-0.txt", 5, 6144);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-global-rtn-app-throughput-scatter-0.txt", 5, 6144);

  // Per satellite throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-sat-fwd-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-sat-fwd-app-throughput-scatter-2.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-sat-rtn-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-sat-rtn-app-throughput-scatter-2.txt", 5, 4096);

  // Per beam throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-fwd-app-throughput-scatter-1-30.txt", 5, 0);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-fwd-app-throughput-scatter-1-43.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-fwd-app-throughput-scatter-2-30.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-fwd-app-throughput-scatter-2-43.txt", 5, 0);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-rtn-app-throughput-scatter-1-30.txt", 5, 0);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-rtn-app-throughput-scatter-1-43.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-rtn-app-throughput-scatter-2-30.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-beam-rtn-app-throughput-scatter-2-43.txt", 5, 0);

  // Per GW throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-gw-fwd-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-gw-fwd-app-throughput-scatter-2.txt", 5, 4096);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-gw-rtn-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-gw-rtn-app-throughput-scatter-2.txt", 5, 4096);

  // Per UT throughput
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-ut-fwd-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-ut-fwd-app-throughput-scatter-2.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-ut-fwd-app-throughput-scatter-3.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-ut-rtn-app-throughput-scatter-1.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-ut-rtn-app-throughput-scatter-2.txt", 5, 2048);
  TestFileValue ("contrib/satellite/data/sims/test-sat-constellation/test3/stat-per-ut-rtn-app-throughput-scatter-3.txt", 5, 2048);
}

/**
 * \ingroup satellite
 * \brief 'Constellation, test 4' test case implementation.
 *
 * This case tests that a topology with hundreds of LEO satellites is correctly loaded.
 * It load the telesat-351-sats configuration
 *
 *  Expected result:
 *    Correct number of entities: 351 satellites, 3 GWs, 3 UTs, 702 ISLs
 *    Each satellite has 4 ISL interfaces
 *    All satellites know on which interface to send a packet depending on a target satellite
 */
class SatConstellationTest4 : public TestCase
{
public:
  SatConstellationTest4 ();
  virtual ~SatConstellationTest4 ();

private:
  virtual void DoRun (void);

  Ptr<SatHelper> m_helper;
};

// Add some help text to this case to describe what it is intended to test
SatConstellationTest4::SatConstellationTest4 ()
  : TestCase ("This case tests that a topology with hundreds of LEO satellites is correctly loaded.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatConstellationTest4::~SatConstellationTest4 ()
{
}

//
// SatConstellationTest4 TestCase implementation
//
void
SatConstellationTest4::DoRun (void)
{
  Config::Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-constellation", "test4", true);

  /// Set regeneration mode
  Config::SetDefault ("ns3::SatConf::ForwardLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));
  Config::SetDefault ("ns3::SatConf::ReturnLinkRegenerationMode", EnumValue (SatEnums::REGENERATION_NETWORK));

  /// Use constellation
  Config::SetDefault ("ns3::SatHelper::SatConstellationEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::SatConstellationFolder", StringValue ("telesat-351-sats"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::StartDateStr", StringValue ("2000-01-01 00:00:00"));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionEachRequest", BooleanValue (false));
  Config::SetDefault ("ns3::SatSGP4MobilityModel::UpdatePositionPeriod", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::SatGeoHelper::IslArbiterType", EnumValue (SatEnums::UNICAST));
  Config::SetDefault ("ns3::SatHelper::GwUsers", UintegerValue (3));

  /// Use constellation with correctly centered beams (used for testing)
  Config::SetDefault ("ns3::SatAntennaGainPatternContainer::PatternsFolder", StringValue ("SatAntennaGain72BeamsShifted"));

  /// When using 72 beams, we need a 72*nbSats network addresses for beams, so we take margin
  Config::SetDefault ("ns3::SatHelper::BeamNetworkAddress", Ipv4AddressValue ("20.1.0.0"));
  Config::SetDefault ("ns3::SatHelper::GwNetworkAddress", Ipv4AddressValue ("10.1.0.0"));
  Config::SetDefault ("ns3::SatHelper::UtNetworkAddress", Ipv4AddressValue ("250.1.0.0"));

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("test-sat-constellation");

  // Creating the reference system.
  simulationHelper->SetBeamSet ({1, 43, 60, 64});
  simulationHelper->SetUserCountPerUt (5);

  simulationHelper->CreateSatScenario ();
  m_helper = simulationHelper->GetSatelliteHelper ();

  NodeContainer sats = m_helper->GeoSatNodes ();
  NodeContainer gws = m_helper->GwNodes ();
  NodeContainer uts = m_helper->UtNodes ();
  NodeContainer gwUsers = m_helper->GetGwUsers ();
  NodeContainer utUsers = m_helper->GetUtUsers ();

  uint32_t countNetDevices = 0;
  uint32_t countIslNetDevice = 0;
  for (uint32_t i = 0; i < sats.GetN (); i++)
    {
      for (uint32_t j = 0; j < sats.Get (i)->GetNDevices (); j++)
        {
          if (DynamicCast<SatGeoNetDevice> (sats.Get (i)->GetDevice (j)) != nullptr)
            {
              countNetDevices += 1;
              countIslNetDevice += DynamicCast<SatGeoNetDevice> (sats.Get (i)->GetDevice (j))->GetIslsNetDevices ().size ();
            }
        }
    }

  NS_TEST_ASSERT_MSG_EQ (sats.GetN (), 351, "Topology must contain 351 satellites");
  NS_TEST_ASSERT_MSG_EQ (countNetDevices, 351, "Topology must contain 351 satellite Geo Net Devices");
  NS_TEST_ASSERT_MSG_EQ (countIslNetDevice, 1404, "Topology must contain 1404 (351*4) satellite ISL Net Devices");
  NS_TEST_ASSERT_MSG_EQ (gws.GetN (), 2, "Topology must contain 2 GWs");
  NS_TEST_ASSERT_MSG_EQ (uts.GetN (), 3, "Topology must contain 3 UTs");
  NS_TEST_ASSERT_MSG_EQ (gwUsers.GetN (), 3, "Topology must contain 3 GW users");
  NS_TEST_ASSERT_MSG_EQ (utUsers.GetN (), 15, "Topology must contain 15 UT users");

  for (uint32_t i = 0; i < sats.GetN (); i++)
    {
      Ptr<SatIslArbiter> arbiter = DynamicCast<SatGeoNetDevice> (sats.Get (i)->GetDevice (0))->GetArbiter ();
      Ptr<SatIslArbiterUnicast> arbiterUnicast = DynamicCast<SatIslArbiterUnicast> (arbiter);
      NS_TEST_ASSERT_MSG_NE (arbiterUnicast, nullptr, "Arbiter of satellite " << i << " is not of type SatIslArbiterUnicast");
      for (uint32_t j = 0; j < sats.GetN (); j++)
        {
          if (i != j)
            {
              NS_TEST_ASSERT_MSG_NE (arbiterUnicast->Decide (i, j, nullptr), -1, "No entry in arbiter of satellite " << i << " to satellite " << j);
            }
        }
    }

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
  AddTestCase (new SatConstellationTest2, TestCase::QUICK); // Test good throughputs without ISLs
  AddTestCase (new SatConstellationTest3, TestCase::QUICK); // Test good throughputs with ISLs
  AddTestCase (new SatConstellationTest4, TestCase::QUICK); // Test topology loading with huge constellation
}

// Allocate an instance of this TestSuite
static SatConstellationTestSuite satConstellationTestSuite;

