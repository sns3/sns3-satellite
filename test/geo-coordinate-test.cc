/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include a header file from your module to test.
#include "ns3/log.h"
#include "ns3/geo-coordinate.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

static void SatCourseChange (GeoCoordinate pos)
{
  Vector pos2 = pos.ToVector();

   // sets number of decimal places

  std::cout << std::cout.precision(15) << std::cout.setf(std::ios::fixed, std::ios::floatfield) << ", position="  "x=" << pos2.x << ", y=" << pos2.y
                                 << ", z=" << pos2.z << ", longitude=" << pos.GetLongitude()
                                 << ", latitude=" << pos.GetLatitude() << ", altitude=" << pos.GetAltitude()  << std::endl;
}

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
  GeoCoordinate position1 = GeoCoordinate(50, -50, 35000000);
  GeoCoordinate position2 = GeoCoordinate( position1.ToVector() );

  SatCourseChange(position1);
  SatCourseChange(position2);

  double altDiff = std::abs(position1.GetAltitude() - position2.GetAltitude());
  double lonDiff = std::abs(position1.GetLongitude() - position2.GetLongitude());
  double latDiff = std::abs(position1.GetLatitude() - position2.GetLatitude());

  bool altSignSame = ((position1.GetAltitude() > 0) ==  (position2.GetAltitude() > 0));
  bool lonSignSame = ((position1.GetLongitude() > 0) ==  (position2.GetLongitude() > 0));
  bool latSignSame = ((position1.GetLatitude() > 0) == (position2.GetLatitude() > 0));

  // check that is difference is ok
  NS_TEST_ASSERT_MSG_LT(altDiff, 250, "Altitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(lonDiff, 0.0003, "Longitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(latDiff, 0.0003, "Latitude difference too big!");

  // check that values have same signs +/-
  NS_TEST_ASSERT_MSG_EQ( altSignSame, true, "Altitude signs are different.");
  NS_TEST_ASSERT_MSG_EQ( lonSignSame, true, "Longitude signs are different.");
  NS_TEST_ASSERT_MSG_EQ( latSignSame, true, "Latitude signs are different.");

  position1 = GeoCoordinate(50, -50, 1000);
  position2 = GeoCoordinate( position1.ToVector() );

  SatCourseChange(position1);
  SatCourseChange(position2);

  altDiff = std::abs(position1.GetAltitude() - position2.GetAltitude());
  lonDiff = std::abs(position1.GetLongitude() - position2.GetLongitude());
  latDiff = std::abs(position1.GetLatitude() - position2.GetLatitude());

  altSignSame = ((position1.GetAltitude() > 0) ==  (position2.GetAltitude() > 0));
  lonSignSame = ((position1.GetLongitude() > 0) ==  (position2.GetLongitude() > 0));
  latSignSame = ((position1.GetLatitude() > 0) == (position2.GetLatitude() > 0));

  // check that is difference is ok
  NS_TEST_ASSERT_MSG_LT(altDiff, 0.000001, "Altitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(lonDiff, 0.000001, "Longitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(latDiff, 0.000001, "Latitude difference too big!");

  NS_TEST_ASSERT_MSG_EQ( altSignSame, true, "Altitude signs are different.");
  NS_TEST_ASSERT_MSG_EQ( lonSignSame, true, "Longitude signs are different.");
  NS_TEST_ASSERT_MSG_EQ( latSignSame, true, "Latitude signs are different.");

  for (int i = -180; i <= 180; i += 30)
    {
      position1 = GeoCoordinate(i, i/2, i*30);
      position2 = GeoCoordinate( position1.ToVector() );

      SatCourseChange(position1);
      SatCourseChange(position2);

      altDiff = std::abs(position1.GetAltitude() - position2.GetAltitude());
      lonDiff = std::abs(position1.GetLongitude() - position2.GetLongitude());
      latDiff = std::abs(position1.GetLatitude() - position2.GetLatitude());

      altSignSame = ((position1.GetAltitude() > 0) ==  (position2.GetAltitude() > 0));
      lonSignSame = ((position1.GetLongitude() > 0) ==  (position2.GetLongitude() > 0));
      latSignSame = ((position1.GetLatitude() > 0) == (position2.GetLatitude() > 0));

      // check that is difference is ok
      NS_TEST_ASSERT_MSG_LT(altDiff, 0.0001, "Altitude difference too big!");
      NS_TEST_ASSERT_MSG_LT(lonDiff, 0.0001, "Longitude difference too big!");
      NS_TEST_ASSERT_MSG_LT(latDiff, 0.0001, "Latitude difference too big!");

      NS_TEST_ASSERT_MSG_EQ( altSignSame, true, "Altitude signs are different.");
      NS_TEST_ASSERT_MSG_EQ( lonSignSame, true, "Longitude signs are different.");
      NS_TEST_ASSERT_MSG_EQ( latSignSame, true, "Latitude signs are different.");
    }

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

