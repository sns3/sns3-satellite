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
 * \file sat-mobility-test.cc
 * \brief Test cases to unit test Satellite Mobility.
 */

// Include a header file from your module to test.
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/mobility-helper.h"
#include "../model/satellite-mobility-model.h"
#include "../model/satellite-position-allocator.h"

using namespace ns3;

static void
SatCourseChange (std::string context, Ptr<const SatMobilityModel> position)
{
  GeoCoordinate pos = position->GetGeoPosition();
  Vector pos2 = pos.ToVector();

   // sets number of decimal places

  std::cout << std::cout.precision(15) << std::cout.setf(std::ios::fixed, std::ios::floatfield) << Simulator::Now () << ", pos=" << pos << ", x=" << pos2.x << ", y=" << pos2.y
                                 << ", z=" << pos2.z << ", longitude=" << pos.GetLongitude()
                                 << ", latitude=" << pos.GetLatitude() << ", altitude=" << pos.GetAltitude()  << std::endl;
}

/**
 * \brief Test case to unit test satellite mobility's position
 *        setting from random box position allocator.
 *        (Constant position mobility model used).
 */
class SatMobilityRandomTestCase : public TestCase
{
public:
  SatMobilityRandomTestCase ();
  virtual ~SatMobilityRandomTestCase ();

private:
  virtual void DoRun (void);
};

SatMobilityRandomTestCase::SatMobilityRandomTestCase ()
  : TestCase ("Test satellite mobility (constant model) with random box allocator.")
{
}

SatMobilityRandomTestCase::~SatMobilityRandomTestCase ()
{
}

void
SatMobilityRandomTestCase::DoRun (void)
{
  Config::Connect ("/NodeList/*/$ns3::SatMobilityModel/SatCourseChange",
                                   MakeCallback (&SatCourseChange));

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::SatRandomBoxPositionAllocator",
                                 "Longitude",StringValue ("ns3::UniformRandomVariable[Min=-10.0|Max=180.0]"),
                                 "Latitude", StringValue ("ns3::UniformRandomVariable[Min=-45.0|Max=80.0]"),
                                 "Altitude", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));


  NodeContainer c;
  c.Create (10);

  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (c);

  for ( uint32_t i; i < c.GetN(); i++ )
    {
      Ptr<SatMobilityModel> model = c.Get(i)->GetObject<SatMobilityModel>();
      GeoCoordinate pos = model->GetGeoPosition();

      // check that position is in limit
      NS_TEST_ASSERT_MSG_LT( pos.GetLongitude(), 180.1, "Longitude is too big.");
      NS_TEST_ASSERT_MSG_LT( pos.GetLatitude(), 80.1, "Latitude is too big.");
      NS_TEST_ASSERT_MSG_LT( pos.GetAltitude(), 100.1, "Altitude is too big.");

      NS_TEST_ASSERT_MSG_GT( pos.GetLongitude(), -10.1, "Longitude is too small.");
      NS_TEST_ASSERT_MSG_GT( pos.GetLatitude(), -45.1, "Latitude is too small.");
      NS_TEST_ASSERT_MSG_GT( pos.GetAltitude(), -0.1, "Altitude is too small.");
    }
}


/**
 * \brief Test case to unit test satellite mobility's position
 *        setting from list position allocator. No conversion expected to happen.
 *        (Constant position mobility model used).
 */
class SatMobilityList1TestCase : public TestCase
{
public:
  SatMobilityList1TestCase ();
  virtual ~SatMobilityList1TestCase ();

private:
  virtual void DoRun (void);
};

SatMobilityList1TestCase::SatMobilityList1TestCase ()
  : TestCase ("Test satellite mobility (constant model, no conversion) with list position allocator.")
{
}

SatMobilityList1TestCase::~SatMobilityList1TestCase ()
{
}

void
SatMobilityList1TestCase::DoRun (void)
{
    MobilityHelper mobility;
    Ptr<SatListPositionAllocator> positionAlloc = CreateObject<SatListPositionAllocator> ();

    // set positions to allocator
    int j=0;
    for (int i = -180; i <= 180; i += 30)
      {
        positionAlloc->Add(GeoCoordinate(i, i/2, i*30));
        j++;
      }

    // create nodes
    NodeContainer c;
    c.Create (j);

    // set allocator and mobility model to helper
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");

    // install mobility to nodes
    mobility.Install (c);

    // now check that model is set and position is correct
    for (int i = 0; i < j; i++)
      {
        Ptr<SatMobilityModel> model = c.Get(i)->GetObject<SatMobilityModel>();
        GeoCoordinate pos = model->GetGeoPosition();

        double longitude = -180 + i*30;
        double latitude = longitude/2;
        double altitude = longitude*30;

        // check that position is equal with set one
        NS_TEST_ASSERT_MSG_EQ( pos.GetLongitude(), longitude, "Longitude is different.");
        NS_TEST_ASSERT_MSG_EQ( pos.GetLatitude(), latitude, "Latitude is different.");
        NS_TEST_ASSERT_MSG_EQ( pos.GetAltitude(), altitude, "Altitude is different.");
      }
}

/**
 * \brief Test case to unit test satellite mobility's position
 *        setting from list position allocator. Conversion expected to happen.
 *        (Constant position mobility model used).
 */
class SatMobilityList2TestCase : public TestCase
{
public:
  SatMobilityList2TestCase ();
  virtual ~SatMobilityList2TestCase ();

private:
  virtual void DoRun (void);
};

SatMobilityList2TestCase::SatMobilityList2TestCase ()
  : TestCase ("Test satellite mobility (constant model, conversion) with list position allocator.")
{
}

SatMobilityList2TestCase::~SatMobilityList2TestCase ()
{
}

void
SatMobilityList2TestCase::DoRun (void)
{
  // now do same with Cartesian coordinates used by SatMobilityModel SetPosition
  // and SatPositionAllocator GetNext
  Config::SetDefault ("ns3::SatMobilityModel::AsGeoCoordinates", BooleanValue (false));
  Config::SetDefault ("ns3::SatPositionAllocator::AsGeoCoordinates", BooleanValue (false));

  MobilityHelper mobility;
  Ptr<SatListPositionAllocator> positionAlloc = CreateObject<SatListPositionAllocator> ();

  positionAlloc->Add(GeoCoordinate(30, 45, 1000));

  // create node
  NodeContainer c;
  c.Create (1);

  // set allocator and mobility model to helper
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");

  // install mobility to nodes
  mobility.Install (c);

  // now check that model is set and position is correct but not too accurate because now
  // is made conversion from Cartesian to Geodetic and back
  // lon=39, lat=45 alt=1000 is value which doesn't converts exact
  Ptr<SatMobilityModel> model = c.Get(0)->GetObject<SatMobilityModel>();
  GeoCoordinate pos = model->GetGeoPosition();

   // check that position is exactly equal with set one
  NS_TEST_ASSERT_MSG_NE( pos.GetLongitude(), 30, "Longitude is not  different.");
  NS_TEST_ASSERT_MSG_NE( pos.GetLatitude(), 45, "Latitude is not different.");
  NS_TEST_ASSERT_MSG_NE( pos.GetAltitude(), 1000, "Altitude is not different.");

  // anyway it should close enough
  NS_TEST_ASSERT_MSG_LT(std::abs(pos.GetLongitude() - 30), 0.000001, "Longitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(std::abs(pos.GetLatitude() - 45), 0.000001, "Latitude difference too big!");
  NS_TEST_ASSERT_MSG_LT(std::abs(pos.GetAltitude()- 1000), 0.000001, "Altitude difference too big!");
}

/**
 * \brief Test suite for Satellite mobility unit test cases.
 */
class SatMobilityTestSuite : public TestSuite
{
public:
  SatMobilityTestSuite ();
};

SatMobilityTestSuite::SatMobilityTestSuite ()
  : TestSuite ("sat-mobility-test", UNIT)
{
  AddTestCase (new SatMobilityRandomTestCase);
  AddTestCase (new SatMobilityList1TestCase);
  AddTestCase (new SatMobilityList2TestCase);
}

// Do allocate an instance of this TestSuite
static SatMobilityTestSuite satSatMobilityTestSuite;

