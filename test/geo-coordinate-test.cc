/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include a header file from your module to test.
#include "ns3/log.h"
#include "ns3/geo-coordinate.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

// Test case to test correctness of GeoCoordinate class
class GeoCoordinateTestCase : public TestCase
{
public:
  GeoCoordinateTestCase ();
  virtual ~GeoCoordinateTestCase ();

private:
  virtual void DoRun (void);
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
  GeoCoordinate position1 = GeoCoordinate(50, -50, 0.100);
  GeoCoordinate position2 = GeoCoordinate( position1.ToVector() );

  double alt_diff = std::abs(position1.GetAltitude() - position2.GetAltitude());
  double lon_diff = std::abs(position1.GetLongitude() - position2.GetLongitude());
  double lat_diff = std::abs(position1.GetLatitude() - position2.GetLatitude());

  NS_TEST_ASSERT_MSG_LT(alt_diff, 0.0000000001, "Altitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(lon_diff, 0.0000000001, "Longitudes difference too big!");
  NS_TEST_ASSERT_MSG_LT(lat_diff, 0.0000000001, "Latitude difference too big!");

  //TODO: more test criteria needed to add when finalizing implementation.
}

// The GeoCoordinateTestSuite for GeoCoordinate test cases
//
//
class GeoCoordinateTestSuite : public TestSuite
{
public:
  GeoCoordinateTestSuite ();
};

GeoCoordinateTestSuite::GeoCoordinateTestSuite ()
  : TestSuite ("geo-coordinate-test", UNIT)
{
  AddTestCase (new GeoCoordinateTestCase);
}

// Do allocate an instance of this TestSuite
static GeoCoordinateTestSuite geoCoordinateTestSuite;

