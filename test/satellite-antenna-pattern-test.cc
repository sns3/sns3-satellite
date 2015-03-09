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

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "../model/satellite-antenna-gain-pattern.h"
#include "../model/satellite-antenna-gain-pattern-container.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \file satellite-antenna-pattern-test.cc
 * \ingroup satellite
 * \brief Satellite antenna pattern test case implementation.
 *
 * This case creates the antenna gain patterns classes and compares the
 * antenna gain values and best beam ids for the test positions (= GW positions
 * of the 72 beam reference system).
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
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-antenna-gain-pattern", "", true);

  // Create antenna gain container
  SatAntennaGainPatternContainer gpContainer;

  // Test positions (= GW positions from 72 spot-beam reference system)
  std::vector<GeoCoordinate> coordinates;
  GeoCoordinate g1 = GeoCoordinate (50.25, 3.75, 0.0);
  coordinates.push_back (g1);
  GeoCoordinate g2 = GeoCoordinate (64.00, 8.25, 0.0);
  coordinates.push_back (g2);
  GeoCoordinate g3 = GeoCoordinate (42.25, -4.50, 0.0);
  coordinates.push_back (g3);
  GeoCoordinate g4 = GeoCoordinate (44.50, 13.50, 0.0);
  coordinates.push_back (g4);
  GeoCoordinate g5 = GeoCoordinate (37.25, 23.75, 0.0);
  coordinates.push_back (g5);

  /* Reference results
  GW: 1, Lat: 50.25, Lon: 3.75, bestBeamId: 12, gain: 51.5528
  GW: 2, Lat: 64, Lon: 8.25, bestBeamId: 6, gain: 51.4735
  GW: 3, Lat: 42.25, Lon: -4.5, bestBeamId: 22, gain: 51.3403
  GW: 4, Lat: 44.5, Lon: 13.5, bestBeamId: 39, gain: 51.6553
  GW: 5, Lat: 37.25, Lon: 23.75, bestBeamId: 58, gain: 51.4738
  */

  // Expected spot-beam gains from beam 3
  double expectedGains[5] = { 51.5528, 51.4735, 51.3403, 51.6553, 51.4738 };

  // Expected best spot-beams
  uint32_t expectedBeamIds[5] = {12, 6, 22, 39, 58};

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

      NS_TEST_ASSERT_MSG_EQ_TOL ( gain_dB, expectedGains[i], 0.001, "Expected gain not within tolerance");
      NS_TEST_ASSERT_MSG_EQ ( bestBeamId, expectedBeamIds[i], "Not expected best spot-beam id");
    }

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Satellite antenna pattern test suite
 */
class SatAntennaPatternTestSuite : public TestSuite
{
public:
  SatAntennaPatternTestSuite ();
};

SatAntennaPatternTestSuite::SatAntennaPatternTestSuite ()
  : TestSuite ("sat-antenna-gain-pattern-test", UNIT)
{
  AddTestCase (new SatAntennaPatternTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatAntennaPatternTestSuite satSatInterferenceTestSuite;

