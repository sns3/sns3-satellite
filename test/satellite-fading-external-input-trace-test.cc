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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

/**
 * \file satellite-fading-external-input-trace-test.cc
 * \ingroup satellite
 * \brief Test cases to unit test external fading traces
 */

#include <vector>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "../model/satellite-fading-external-input-trace-container.h"
#include "../model/satellite-channel.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite fading external input trace and container for these
 * objects.
 *
 */
class SatFadingExternalInputTraceTestCase : public TestCase
{
public:
  SatFadingExternalInputTraceTestCase ();
  virtual ~SatFadingExternalInputTraceTestCase ();

  void TestGetFading (uint32_t nodeId, SatEnums::ChannelType_t channelType);

private:
  virtual void DoRun (void);

  std::vector<double> m_results;
};

SatFadingExternalInputTraceTestCase::SatFadingExternalInputTraceTestCase ()
  : TestCase ("Test satellite fading external input trace and container.")
{
}

SatFadingExternalInputTraceTestCase::~SatFadingExternalInputTraceTestCase ()
{
}

void SatFadingExternalInputTraceTestCase::TestGetFading (uint32_t nodeId, SatEnums::ChannelType_t channelType)
{
  Ptr<SatMobilityModel> mobility;

  Ptr<SatFadingExternalInputTrace> trace = Singleton<SatFadingExternalInputTraceContainer>::Get ()->GetFadingTrace (nodeId, channelType, mobility);
  double fading = trace->GetFading ();
  m_results.push_back (fading);
}

void
SatFadingExternalInputTraceTestCase::DoRun (void)
{
  uint32_t numUts (2);
  uint32_t numGws (5);

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-fading-external-input-trace", "", true);

  // Test the fading traces
  bool success = Singleton<SatFadingExternalInputTraceContainer>::Get ()->TestFadingTraces (numUts,numGws);
  NS_TEST_ASSERT_MSG_EQ (success, true, "SatChannelFadingTrace test failed");

  double time [4] = {1.434, 40.923, 80.503, 140.3};
  double preCalcRes [4] = {1.020, 1.008, 1.006, 1.012};

  Simulator::Schedule (Seconds (time[0]), &SatFadingExternalInputTraceTestCase::TestGetFading, this, 1, SatEnums::RETURN_USER_CH);
  Simulator::Schedule (Seconds (time[1]), &SatFadingExternalInputTraceTestCase::TestGetFading, this, 2, SatEnums::RETURN_FEEDER_CH);
  Simulator::Schedule (Seconds (time[2]), &SatFadingExternalInputTraceTestCase::TestGetFading, this, 1, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Seconds (time[3]), &SatFadingExternalInputTraceTestCase::TestGetFading, this, 2, SatEnums::FORWARD_FEEDER_CH);

  Simulator::Run ();

  for (uint32_t i = 0; i < 4; ++i)
    {
      NS_TEST_ASSERT_MSG_EQ_TOL (m_results[i], preCalcRes[i], 0.001, "Fading not within expected tolerance");
    }

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test suite for satellite fading external input trace
 */
class SatFadingExternalInputTraceTestSuite : public TestSuite
{
public:
  SatFadingExternalInputTraceTestSuite ();
};

SatFadingExternalInputTraceTestSuite::SatFadingExternalInputTraceTestSuite ()
  : TestSuite ("sat-fading-external-input-trace-test", UNIT)
{
  AddTestCase (new SatFadingExternalInputTraceTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatFadingExternalInputTraceTestSuite satSatInterferenceTestSuite;

