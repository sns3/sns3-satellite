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

  /* Reference results
  GW: 1, Lat: 51.5, Lon: -0.13, bestBeamId: 12, gain: 48.9788
  GW: 2, Lat: 59.91, Lon: 10.75, bestBeamId: 6, gain: 49.2983
  GW: 3, Lat: 40.42, Lon: -3.7, bestBeamId: 22, gain: 49.1886
  GW: 4, Lat: 41.9, Lon: 12.48, bestBeamId: 46, gain: 48.8911
  GW: 5, Lat: 37.98, Lon: 23.73, bestBeamId: 58, gain: 51.0815
  */

  // Expected spot-beam gains from beam 3
  double expectedGains[5] = { 48.9788, 49.2983, 49.1886, 48.8911, 51.0815 };

  // Expected best spot-beams
  uint32_t expectedBeamIds[5] = {12, 6, 22, 46, 58};

  // Check that the gains and best beam IDs are as expected
  double gain (0.0);
  uint32_t bestBeamId (0);
  for ( uint32_t i = 0; i < coordinates.size (); ++i)
    {
      bestBeamId = gpContainer.GetBestBeamId (coordinates[i]);

      Ptr<SatAntennaGainPattern> gainPattern = gpContainer.GetAntennaGainPattern (bestBeamId);

      gain = gainPattern->GetAntennaGain_lin (coordinates[i]);
      double gain_dB = 10.0 * log10 (gain);

      /*
      std::cout << "GW: " << i+1 <<
          ", Lat: " << coordinates[i].GetLatitude () <<
          ", Lon: " << coordinates[i].GetLongitude () <<
          ", bestBeamId: " << bestBeamId <<
          ", gain: " << gain_dB << std::endl;
      */

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

