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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

/**
 * \file satellite-fsl-test.cc
 * \brief Test cases to unit test Satellite Free Space Loss model.
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "../model/satellite-mobility-model.h"
#include "../model/satellite-free-space-loss.h"
#include "../helper/satellite-helper.h"

using namespace ns3;

/**
 * \brief Test case to unit test satellite constant interference model.
 *
 */
class SatFreeSpaceLossTestCase : public TestCase
{
public:
  SatFreeSpaceLossTestCase ();
  virtual ~SatFreeSpaceLossTestCase ();

private:
  virtual void DoRun (void);
};

SatFreeSpaceLossTestCase::SatFreeSpaceLossTestCase ()
  : TestCase ("Test satellite free space loss model.")
{
}

SatFreeSpaceLossTestCase::~SatFreeSpaceLossTestCase ()
{
}

void
SatFreeSpaceLossTestCase::DoRun (void)
{
  SatFreeSpaceLoss fsl;
  double frequency = 17.9 * std::pow(10.0, 9); // reference frequency

  // Create simple scenario
  Ptr<SatHelper> helper = CreateObject<SatHelper>();
  helper->EnableCreationTraces("fsl-test.log", false);
  helper->CreateScenario(SatHelper::SIMPLE);

  NodeContainer gw = helper->GwNodes();
  NodeContainer ut = helper->UtNodes();
  Ptr<Node> geo = helper->GeoSatNode();

  // get mobilities
  Ptr<SatMobilityModel> gwMob = gw.Get(0)->GetObject<SatMobilityModel> ();
  Ptr<SatMobilityModel> utMob = ut.Get(0)->GetObject<SatMobilityModel> ();
  Ptr<SatMobilityModel> geoMob = geo->GetObject<SatMobilityModel> ();

  // set reference positions fro test
  gwMob->SetGeoPosition(GeoCoordinate(54.689444, 25.20, 0.0));
  utMob->SetGeoPosition(GeoCoordinate(25.00, -26.20, 230));
  geoMob->SetGeoPosition(GeoCoordinate(33.00, 0.0, 35786000));

  // test fsl calculation in path UT - Geo Satellite
  double fsl_W = fsl.GetFslW(utMob, geoMob, frequency);
  double fsl_dB = fsl.GetFsldB(utMob, geoMob, frequency);

  NS_TEST_ASSERT_MSG_EQ_TOL( fsl_W, 754499619980809076736.0, 1,"FSL (UT-GEO) in Watts incorrect");
  NS_TEST_ASSERT_MSG_EQ_TOL( fsl_dB, 208.77659025370352, 0.1, "FSL (UT-GEO) in dBs incorrect");

  // test fsl calculation in path GW - Geo Satellite
  fsl_W = fsl.GetFslW(gwMob, geoMob, frequency);
  fsl_dB = fsl.GetFsldB(gwMob, geoMob, frequency);

  NS_TEST_ASSERT_MSG_EQ_TOL( fsl_W, 768992545596229681152.0, 1, "FSL (GW-GEO) in Watts incorrect");
  NS_TEST_ASSERT_MSG_EQ_TOL( fsl_dB, 208.8592212988971, 0.1, "FSL (GW-GEO) in dBs incorrect");

  Simulator::Destroy ();
}

/**
 * \brief Test suite for Satellite free space loss unit test cases.
 */
class SatFreeSpaceLossTestSuite : public TestSuite
{
public:
  SatFreeSpaceLossTestSuite ();
};

SatFreeSpaceLossTestSuite::SatFreeSpaceLossTestSuite ()
  : TestSuite ("sat-fsl-test", UNIT)
{
  AddTestCase (new SatFreeSpaceLossTestCase);
}

// Do allocate an instance of this TestSuite
static SatFreeSpaceLossTestSuite satSatInterferenceTestSuite;

