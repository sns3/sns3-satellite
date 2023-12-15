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
 * \file satellite-handover-test.cc
 * \ingroup satellite
 * \brief Handover test case implementations.
 *
 * In this module implements the Handover Test Cases.
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
#include "ns3/satellite-id-mapper.h"
#include "ns3/satellite-isl-arbiter-unicast.h"
#include "ns3/satellite-isl-arbiter.h"
#include "ns3/satellite-phy-rx-carrier.h"
#include "ns3/satellite-ut-mac-state.h"
#include "ns3/simulation-helper.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/string.h"
#include "ns3/test.h"

#include <fstream>
#include <iostream>

using namespace ns3;

/**
 * \ingroup satellite
 * \brief 'Handover, test 1 test case implementation.
 *
 * This case tests that a application throughputs PerEntity are correct.
 * It generates a conf with 2 GEO satellites, 2 GWs, 4 UTs (3 static and 1 moving) and 1 UT users
 * per UT. Each UT user receives 40.96 kb/s.
 *
 * Static UTs are connected to:
 *     - satellite 1, beam 36
 *     - satellite 1, beam 47
 *     - satellite 2, beam 46
 *
 * Moving UT is:
 *     - At t = 8s, on satellite 1, beam 35
 *     - At t = 18s, on satellite 2, beam 20
 *
 * Expected throughputs (after 8s):
 *     - 40.96 kb/s per UT
 *     - 122.88 kb/s on satellite 1, 40.96 kb/s on satellite 2
 *     - 40.96 kb/s on beams 1/36, 1/47, 2/46 and 1/35
 *     - 81.92 kb/s per GW
 *
 * Expected throughputs (after 18s):
 *     - 40.96 kb/s per UT
 *     - 81.92 per satellite
 *     - 40.96 kb/s on beams 1/36, 1/47, 2/46 and 2/20
 *     - 40.96 kb/s on GW 1, 122.88 kb/s on GW 2
 */
class SatHandoverTest1 : public TestCase
{
  public:
    SatHandoverTest1();
    virtual ~SatHandoverTest1();

  private:
    virtual void DoRun(void);

    std::vector<std::string> Split(std::string s, char del);
    void TestFileValue(std::string path, uint32_t time, double expectedValue);

    Ptr<SatHelper> m_helper;
};

// Add some help text to this case to describe what it is intended to test
SatHandoverTest1::SatHandoverTest1()
    : TestCase("This case tests that a application throughputs PerEntity are correct.")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SatHandoverTest1::~SatHandoverTest1()
{
}

std::vector<std::string>
SatHandoverTest1::Split(std::string s, char del)
{
    std::stringstream ss(s);
    std::string word;
    std::vector<std::string> tokens;
    while (!ss.eof())
    {
        std::getline(ss, word, del);
        tokens.push_back(word);
    }
    return tokens;
}

void
SatHandoverTest1::TestFileValue(std::string path, uint32_t time, double expectedValue)
{
    std::string line;
    std::ifstream myfile(path);
    std::string delimiter = " ";
    std::string token;
    bool valueFound;

    if (myfile.is_open())
    {
        valueFound = false;
        while (std::getline(myfile, line))
        {
            std::vector<std::string> tokens = Split(line, ' ');
            if (tokens.size() == 2)
            {
                if ((uint32_t)std::stoi(tokens[0]) == time)
                {
                    NS_TEST_ASSERT_MSG_EQ_TOL(std::stof(tokens[1]),
                                              expectedValue,
                                              expectedValue / 10,
                                              "Incorrect throughput for statistic " << path);
                    valueFound = true;
                    break;
                }
            }
        }
        myfile.close();
        if (!valueFound)
        {
            NS_TEST_ASSERT_MSG_EQ(0, 1, "Cannot find time " << time << " for trace file " << path);
        }
    }
    else
    {
        NS_TEST_ASSERT_MSG_EQ(0, 1, "Cannot read trace file " << path);
    }
}

//
// SatHandoverTest1 TestCase implementation
//
void
SatHandoverTest1::DoRun(void)
{
    Config::Reset();
    Singleton<SatIdMapper>::Get()->Reset();

    // Set simulation output details
    Singleton<SatEnvVariables>::Get()->DoInitialize();
    Singleton<SatEnvVariables>::Get()->SetOutputVariables("test-sat-handover", "test1", true);

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::SatGeoFeederPhy::QueueSize", UintegerValue(100000));

    Config::SetDefault("ns3::SatHelper::HandoversEnabled", BooleanValue(true));
    Config::SetDefault("ns3::SatHandoverModule::NumberClosestSats", UintegerValue(2));

    Config::SetDefault("ns3::SatGwMac::DisableSchedulingIfNoDeviceConnected", BooleanValue(true));
    Config::SetDefault("ns3::SatGeoMac::DisableSchedulingIfNoDeviceConnected", BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    /// Enable packet trace
    Config::SetDefault("ns3::SatHelper::PacketTraceEnabled", BooleanValue(true));
    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("test-sat-handover/test1");
    Ptr<SimulationHelperConf> simulationConf = CreateObject<SimulationHelperConf>();
    simulationHelper->SetGwUserCount(1);
    simulationHelper->SetUserCountPerUt(1);
    simulationHelper->SetBeamSet({0,  20, 21, 22, 32, 33, 34, 35, 36, 37, 44, 45, 46,
                                  47, 53, 54, 55, 56, 59, 60, 61, 66, 67, 68, 71, 72});
    simulationHelper->SetUserCountPerMobileUt(simulationConf->m_utMobileUserCount);

    simulationHelper->LoadScenario("constellation-eutelsat-geo-2-sats-handovers");

    std::string mobileUtFolder = Singleton<SatEnvVariables>::Get()->LocateDataDirectory() +
                                 "/additional-input/utpositions/mobiles/scenario6";
    Ptr<SatHelper> helper = simulationHelper->CreateSatScenario(SatHelper::NONE, mobileUtFolder);

    helper->PrintTopology(std::cout);

    Config::SetDefault("ns3::CbrApplication::Interval", StringValue("100ms"));
    Config::SetDefault("ns3::CbrApplication::PacketSize", UintegerValue(512));

    simulationHelper->InstallTrafficModel(SimulationHelper::CBR,
                                          SimulationHelper::UDP,
                                          SimulationHelper::FWD_LINK,
                                          Seconds(1.0),
                                          Seconds(25.0));

    Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer();

    s->AddPerSatFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerBeamFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddPerSatFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerBeamFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerGwFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    simulationHelper->SetSimulationTime(Seconds(30));
    simulationHelper->RunSimulation();

    Simulator::Destroy();

    // Per satellite APP throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-app-throughput-scatter-1.txt",
                  8,
                  122.88);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-app-throughput-scatter-2.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-app-throughput-scatter-1.txt",
                  18,
                  81.92);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-app-throughput-scatter-2.txt",
                  18,
                  81.92);

    // Per satellite MAC throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-user-mac-throughput-scatter-1.txt",
                  8,
                  122.88);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-user-mac-throughput-scatter-2.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-user-mac-throughput-scatter-1.txt",
                  18,
                  81.92);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-sat-fwd-user-mac-throughput-scatter-2.txt",
                  18,
                  81.92);

    // Per beam APP throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-app-throughput-scatter-1-35.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-app-throughput-scatter-2-20.txt",
                  8,
                  0);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-app-throughput-scatter-1-35.txt",
                  18,
                  0);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-app-throughput-scatter-2-20.txt",
                  18,
                  40.96);

    // Per beam MAC throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-user-mac-throughput-scatter-1-35.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-user-mac-throughput-scatter-2-20.txt",
                  8,
                  0);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-user-mac-throughput-scatter-1-35.txt",
                  18,
                  0);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-beam-fwd-user-mac-throughput-scatter-2-20.txt",
                  18,
                  40.96);

    // Per GW APP throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-app-throughput-scatter-1.txt",
                  8,
                  81.92);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-app-throughput-scatter-2.txt",
                  8,
                  81.92);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-app-throughput-scatter-1.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-app-throughput-scatter-2.txt",
                  18,
                  122.88);

    // Per GW MAC throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-user-mac-throughput-scatter-1.txt",
                  8,
                  81.92);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-user-mac-throughput-scatter-2.txt",
                  8,
                  81.92);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-user-mac-throughput-scatter-1.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-gw-fwd-user-mac-throughput-scatter-2.txt",
                  18,
                  122.88);

    // Per UT APP throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-1.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-2.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-3.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-4.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-1.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-2.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-3.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-app-throughput-scatter-4.txt",
                  18,
                  40.96);

    // Per UT MAC throughput
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-1.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-2.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-3.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-4.txt",
                  8,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-1.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-2.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-3.txt",
                  18,
                  40.96);
    TestFileValue("contrib/satellite/data/sims/test-sat-handover/test1/"
                  "stat-per-ut-fwd-user-mac-throughput-scatter-4.txt",
                  18,
                  40.96);
}

// The TestSuite class names the TestSuite as sat-handover-test, identifies what type of
// TestSuite (SYSTEM), and enables the TestCases to be run. Typically, only the constructor for this
// class must be defined
//
class SatHandoverTestSuite : public TestSuite
{
  public:
    SatHandoverTestSuite();
};

SatHandoverTestSuite::SatHandoverTestSuite()
    : TestSuite("sat-handover-test", SYSTEM)
{
    AddTestCase(
        new SatHandoverTest1,
        TestCase::QUICK); // This case tests that a application throughputs PerEntity are correct.
}

// Allocate an instance of this TestSuite
static SatHandoverTestSuite satHandoverTestSuite;
