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
 * \file satellite-antenna-pattern-test.cc
 * \brief Test cases to unit test Satellite Antenna Pattern
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "../model/satellite-antenna-gain-pattern.h"
#include "../model/satellite-antenna-gain-pattern-container.h"
using namespace ns3;

/**
 * \brief Test case to unit test satellite antenna patterns
 *
 */
class SatAntennaPatternTestCase : public TestCase
{
public:
  SatAntennaPatternTestCase ();
  virtual ~SatAntennaPatternTestCase ();

private:
  virtual void DoRun (void);
};

SatAntennaPatternTestCase::SatAntennaPatternTestCase ()
  : TestCase ("Test satellite antenna gain pattern.")
{
}

SatAntennaPatternTestCase::~SatAntennaPatternTestCase ()
{
}

void
SatAntennaPatternTestCase::DoRun (void)
{
  // Create antenna gain container
  SatAntennaGainPatternContainer gpContainer;

  // Get one example spot-beam pattern = beam 3
  uint32_t beamId (3);
  Ptr<SatAntennaGainPattern> gainPattern = gpContainer.GetAntennaGainPattern (beamId);

  // Test positions (= GW positions from 72 spot-beam reference system)
  std::vector<GeoCoordinate> coordinates;
  GeoCoordinate g1 = GeoCoordinate(51.50, -0.13, 0.0);
  coordinates.push_back (g1);
  GeoCoordinate g2 = GeoCoordinate(59.91, 10.75, 0.0);
  coordinates.push_back (g2);
  GeoCoordinate g3 = GeoCoordinate(40.42, -3.70, 0.0);
  coordinates.push_back (g3);
  GeoCoordinate g4 = GeoCoordinate(41.90, 12.48, 0.0);
  coordinates.push_back (g4);
  GeoCoordinate g5 = GeoCoordinate(37.98, 23.73, 0.0);
  coordinates.push_back (g5);

  // Expected spot-beam gains from beam 3
  std::vector<double> expectedGains;
  expectedGains.push_back (36.8857);
  expectedGains.push_back (19.3728);
  expectedGains.push_back (2.06321);
  expectedGains.push_back (9.26611);
  expectedGains.push_back (10.7818);

  // Expected best spot-beams
  std::vector<uint32_t> expectedBeamIds;
  expectedBeamIds.push_back (12);
  expectedBeamIds.push_back (6);
  expectedBeamIds.push_back (22);
  expectedBeamIds.push_back (46);
  expectedBeamIds.push_back (58);

  NS_ASSERT (coordinates.size () == expectedGains.size());
  NS_ASSERT (coordinates.size () == expectedBeamIds.size());

  // Check that the gains and best beam IDs are as expected
  double gain (0.0);
  uint32_t bestBeamId (0);
  for ( uint32_t i = 0; i < coordinates.size (); ++i)
    {
      gain = gainPattern->GetAntennaGain_lin (coordinates[i]);
      double gain_dB = 10.0 * log10 (gain);

      bestBeamId = gpContainer.GetBestBeamId (coordinates[i]);

      NS_TEST_ASSERT_MSG_EQ_TOL( gain_dB, expectedGains[i], 0.001, "Expected gain not within tolerance");
      NS_TEST_ASSERT_MSG_EQ( bestBeamId, expectedBeamIds[i], "Not expected best spot-beam id");
    }
}

/**
 * \brief Test suite for Satellite free space loss unit test cases.
 */
class SatAntennaPatternTestSuite : public TestSuite
{
public:
  SatAntennaPatternTestSuite ();
};

SatAntennaPatternTestSuite::SatAntennaPatternTestSuite ()
  : TestSuite ("sat-antenna-gain-pattern-test", UNIT)
{
  AddTestCase (new SatAntennaPatternTestCase);
}

// Do allocate an instance of this TestSuite
static SatAntennaPatternTestSuite satSatInterferenceTestSuite;

