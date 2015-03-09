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
 * \file satellite-geo-coordinate-test.cc
 * \ingroup satellite
 * \brief Test cases to unit test GeoCoordinate class.
 *
 */
#include "ns3/log.h"
#include "ns3/geo-coordinate.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

//#define PRINT_POSITION_INFO // uncomment to see info while executing test

#ifdef PRINT_POSITION_INFO
static void PrintPositionInfo (GeoCoordinate pos)
{
  Vector pos2 = pos.ToVector ();

  // sets number of decimal places
  std::cout.setf (std::ios::fixed, std::ios::floatfield);
  std::cout.precision (15);
  std::cout << pos.GetRefEllipsoid () << " position="  "x=" << pos2.x << ", y=" << pos2.y
            << ", z=" << pos2.z << ", longitude=" << pos.GetLongitude ()
            << ", latitude=" << pos.GetLatitude () << ", altitude=" << pos.GetAltitude ()  << std::endl;
}
#endif

/**
 * \brief Test case to unit test that GeoCoordinate can be created with valid values.
 */
class GeoCoordinateTestCase : public TestCase
{
public:
  GeoCoordinateTestCase ();
  virtual ~GeoCoordinateTestCase ();

private:
  virtual void DoRun (void);
  void Validate ( GeoCoordinate& coord1, GeoCoordinate& coord2 );
};

GeoCoordinateTestCase::GeoCoordinateTestCase ()
  : TestCase ("Test Geo Coordinate correctness")
{
}

GeoCoordinateTestCase::~GeoCoordinateTestCase ()
{
}

void
GeoCoordinateTestCase::DoRun (void)
{
  GeoCoordinate position1;
  GeoCoordinate position2;

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-geo-coordinate", "", true);

  for (int i = -180; i <= 180; i += 30)
    {
      position1 = GeoCoordinate (i / 2, i, i * 30);
      position2 = GeoCoordinate (position1.ToVector ());

      Validate ( position1, position2 );

      position1 = GeoCoordinate (i / 2, i, i * 30, GeoCoordinate::SPHERE);
      position2 = GeoCoordinate (position1.ToVector (), GeoCoordinate::SPHERE);

      Validate ( position1, position2 );

      position1 = GeoCoordinate (i / 2, i, i * 30, GeoCoordinate::WGS84);
      position2 = GeoCoordinate (position1.ToVector (), GeoCoordinate::WGS84);

      Validate ( position1, position2 );

      position1 = GeoCoordinate (i / 2, i, i * 30, GeoCoordinate::GRS80);
      position2 = GeoCoordinate (position1.ToVector (), GeoCoordinate::GRS80);

      Validate ( position1, position2 );
    }

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

void
GeoCoordinateTestCase::Validate (GeoCoordinate& position1, GeoCoordinate& position2)
{
  double altDiff, lonDiff, latDiff = 0;
  bool altSignSame, lonSignSame, latSignSame = false;

#ifdef PRINT_POSITION_INFO
  PrintPositionInfo (position1);
  PrintPositionInfo (position2);
#endif

  altDiff = std::abs (position1.GetAltitude () - position2.GetAltitude ());
  lonDiff = std::abs (position1.GetLongitude () - position2.GetLongitude ());
  latDiff = std::abs (position1.GetLatitude () - position2.GetLatitude ());

  altSignSame = ((position1.GetAltitude () > 0) ==  (position2.GetAltitude () > 0));
  lonSignSame = ((position1.GetLongitude () > 0) ==  (position2.GetLongitude () > 0));
  latSignSame = ((position1.GetLatitude () > 0) == (position2.GetLatitude () > 0));

  // check that is difference is ok
  NS_TEST_ASSERT_MSG_LT (altDiff, 0.0001, "Altitude difference too big!");
  NS_TEST_ASSERT_MSG_LT (lonDiff, 0.0001, "Longitude difference too big!");
  NS_TEST_ASSERT_MSG_LT (latDiff, 0.0001, "Latitude difference too big!");

  NS_TEST_ASSERT_MSG_EQ ( altSignSame, true, "Altitude signs are different.");
  NS_TEST_ASSERT_MSG_EQ ( lonSignSame, true, "Longitude signs are different.");
  NS_TEST_ASSERT_MSG_EQ ( latSignSame, true, "Latitude signs are different.");
}

/**
 * \brief Test suite for GeoCoordinate unit test cases.
 */
class GeoCoordinateTestSuite : public TestSuite
{
public:
  GeoCoordinateTestSuite ();
};

GeoCoordinateTestSuite::GeoCoordinateTestSuite ()
  : TestSuite ("geo-coordinate-test", UNIT)
{
  AddTestCase (new GeoCoordinateTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static GeoCoordinateTestSuite geoCoordinateTestSuite;

