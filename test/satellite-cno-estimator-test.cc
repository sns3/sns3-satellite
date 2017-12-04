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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

/**
 * \file satellite-cno-estimator.cc
 * \ingroup satellite
 * \brief Test cases to unit test Satellite C/N0 estimator.
 */

// Include a header file from your module to test.
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "../model/satellite-cno-estimator.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

class SatEstimatorBaseTestCase : public TestCase
{
public:
  SatEstimatorBaseTestCase () : TestCase ("")
  {
  }
  SatEstimatorBaseTestCase (std::string info) : TestCase (info)
  {
  }
  virtual ~SatEstimatorBaseTestCase ()
  {
  }

  // add sample to estimator
  void AddSample (double cno);

  // get C/N0 estimation.
  void GetCnoEstimation ();

  // create C/N0 estimator.
  void CreateEstimator (SatCnoEstimator::EstimationMode_t mode, Time window);

protected:
  virtual void DoRun (void) = 0;
  Ptr<SatCnoEstimator> m_estimator;
  std::vector<double>  m_cnoEstimations;
};

void
SatEstimatorBaseTestCase::AddSample (double cno)
{
  m_estimator->AddSample (cno);
}

void
SatEstimatorBaseTestCase::GetCnoEstimation ()
{
  m_cnoEstimations.push_back (m_estimator->GetCnoEstimation ());
}

void
SatEstimatorBaseTestCase::CreateEstimator (SatCnoEstimator::EstimationMode_t mode, Time window)
{
  m_estimator = Create<SatBasicCnoEstimator> (mode, window);
}

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite C/N0 estimator with mode LAST.
 *
 * This case tests that SatBasicCnoEstimator can be created in mode LAST and
 * C/N0 is estimated correctly in set window.
 *  1.  Create SatBasicCnoEstimator object with LAST mode using different window values.
 *  2.  Set samples to estimator at different points of time (method AddSample).
 *  3.  Get C/N0 estimation from estimator at some points of time (method GetCnoEstimation).
 *
 *  Expected result:
 *   Returned C/N0 estimation must be the value of last added sample, if the last sample is
 *   added inside time window.
 *
 *   C/N0 estimation must be NAN, if no samples are got during time window.
 *
 *
 */
class SatBasicEstimatorLastTestCase : public SatEstimatorBaseTestCase
{
public:
  SatBasicEstimatorLastTestCase () : SatEstimatorBaseTestCase ("Test satellite C per N0 basic estimator with mode LAST.")
  {
  }
  virtual ~SatBasicEstimatorLastTestCase ()
  {
  }

protected:
  virtual void DoRun (void);
};

void
SatBasicEstimatorLastTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-cno-estimator-unit", "last", true);

  // create estimator with window 100 ms
  Simulator::Schedule (Seconds (0.05), &SatBasicEstimatorLastTestCase::CreateEstimator, this, SatCnoEstimator::LAST, Seconds (0.10) );

  // simulate sample additions with window 100 ms
  Simulator::Schedule (Seconds (0.10), &SatBasicEstimatorLastTestCase::AddSample, this, -5.0 );
  Simulator::Schedule (Seconds (0.20), &SatBasicEstimatorLastTestCase::AddSample, this, 5.0 );
  Simulator::Schedule (Seconds (0.30), &SatBasicEstimatorLastTestCase::AddSample, this, -15.0 );

  // simulate C/N0 estimations with window 100 ms
  Simulator::Schedule (Seconds (0.09), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected
  Simulator::Schedule (Seconds (0.19), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -5.0 expected
  Simulator::Schedule (Seconds (0.25), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // 5.0 expected
  Simulator::Schedule (Seconds (0.31), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -15.0 expected
  Simulator::Schedule (Seconds (0.41), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected

  // create estimator with window 300 ms
  Simulator::Schedule (Seconds (0.5), &SatBasicEstimatorLastTestCase::CreateEstimator, this, SatCnoEstimator::LAST, Seconds (0.30) );

  // simulate sample additions with window 300 ms
  Simulator::Schedule (Seconds (0.60), &SatBasicEstimatorLastTestCase::AddSample, this, -6.0 );
  Simulator::Schedule (Seconds (0.70), &SatBasicEstimatorLastTestCase::AddSample, this, 1.0 );
  Simulator::Schedule (Seconds (1.10), &SatBasicEstimatorLastTestCase::AddSample, this, -5.0 );

  // simulate C/N0 estimations with window 300 ms
  Simulator::Schedule (Seconds (0.59), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected
  Simulator::Schedule (Seconds (0.69), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -6.0 expected
  Simulator::Schedule (Seconds (0.75), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // 1.0 expected
  Simulator::Schedule (Seconds (0.91), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // 1.0 expected
  Simulator::Schedule (Seconds (1.39), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -5.0 expected
  Simulator::Schedule (Seconds (1.41), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected

  Simulator::Run ();

  // After simulation check that estimations are as expected

  // estimations with window 100 ms
  NS_TEST_ASSERT_MSG_EQ (std::isnan (m_cnoEstimations[0]), true, "first estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ (m_cnoEstimations[1], -5.0, "second estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ (m_cnoEstimations[2], 5.0, "third estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ (m_cnoEstimations[3], -15.0, "fourth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ (std::isnan (m_cnoEstimations[4]), true, "fifth estimation incorrect");

  // estimations with window 300 ms
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (m_cnoEstimations[5]), true, "sixth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[6], -6.0, "seventh estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[7], 1.0, "eight estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[8], 1.0, "ninth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[9], -5.0, "tenth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (m_cnoEstimations[10]), true, "eleventh estimation incorrect");

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite C/N0 estimator with mode MINIMUM.
 *
 * This case tests that SatBasicCnoEstimator can be created in mode MINIMUM and
 * C/N0 is estimated correctly in set window.
 *  1.  Create SatBasicCnoEstimator object with MINIMUM mode using different window values.
 *  2.  Set samples to estimator at different points of time (method AddSample).
 *  3.  Get C/N0 estimation from estimator at some points of time (method GetCnoEstimation).
 *
 *  Expected result:
 *   Returned C/N0 estimation must be the minimum value of samples in window.
 *
 *   C/N0 estimation must be NAN, if no samples are got during time window.
 *
 *
 */
class SatBasicEstimatorMinTestCase : public SatEstimatorBaseTestCase
{
public:
  SatBasicEstimatorMinTestCase () : SatEstimatorBaseTestCase ("Test satellite C per N0 basic estimator with mode MINIMUM.")
  {
  }
  virtual ~SatBasicEstimatorMinTestCase ()
  {
  }

protected:
  virtual void DoRun (void);
};

void
SatBasicEstimatorMinTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-cno-estimator-unit", "min", true);

  // create estimator with window 200 ms
  Simulator::Schedule (Seconds (0.05), &SatBasicEstimatorLastTestCase::CreateEstimator, this, SatCnoEstimator::MINIMUM, Seconds (0.20) );

  // simulate sample additions with window 100 ms
  Simulator::Schedule (Seconds (0.17), &SatBasicEstimatorLastTestCase::AddSample, this, -4.2 );
  Simulator::Schedule (Seconds (0.22), &SatBasicEstimatorLastTestCase::AddSample, this, 8.1 );
  Simulator::Schedule (Seconds (0.46), &SatBasicEstimatorLastTestCase::AddSample, this, -15.7 );
  Simulator::Schedule (Seconds (0.48), &SatBasicEstimatorLastTestCase::AddSample, this, 2.4 );

  // simulate C/N0 estimations with window 200 ms
  Simulator::Schedule (Seconds (0.09), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected
  Simulator::Schedule (Seconds (0.19), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -4.2 expected
  Simulator::Schedule (Seconds (0.35), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -4.2 expected
  Simulator::Schedule (Seconds (0.41), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -8.1 expected
  Simulator::Schedule (Seconds (0.49), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -15.7 expected
  Simulator::Schedule (Seconds (0.69), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected

  Simulator::Run ();

  // After simulation check that estimations are as expected

  // estimations with window 200 ms
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (m_cnoEstimations[0]), true, "first estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[1], -4.2, "second estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[2], -4.2, "third estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[3], 8.1, "fourth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( m_cnoEstimations[4], -15.7, "fifth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (m_cnoEstimations[5]), true, "sixth estimation incorrect");

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite C/N0 estimator with mode AVERAGE.
 *
 * This case tests that SatBasicCnoEstimator can be created in mode AVERAGE and
 * C/N0 is estimated correctly in set window.
 *  1.  Create SatBasicCnoEstimator object with AVERAGE mode using different window values.
 *  2.  Set samples to estimator at different points of time (method AddSample).
 *  3.  Get C/N0 estimation from estimator at some points of time (method GetCnoEstimation).
 *
 *  Expected result:
 *   Returned C/N0 estimation must be the average value of samples in window.
 *
 *   C/N0 estimation must be NAN, if no samples are got during time window.
 *
 *
 */
class SatBasicEstimatorAverageTestCase : public SatEstimatorBaseTestCase
{
public:
  SatBasicEstimatorAverageTestCase () : SatEstimatorBaseTestCase ("Test satellite C per N0 basic estimator with mode AVERAGE.")
  {
  }
  virtual ~SatBasicEstimatorAverageTestCase ()
  {
  }

protected:
  virtual void DoRun (void);
};

void
SatBasicEstimatorAverageTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-cno-estimator-unit", "average", true);

  // create estimator with window 200 ms
  Simulator::Schedule (Seconds (0.05), &SatBasicEstimatorLastTestCase::CreateEstimator, this, SatCnoEstimator::AVERAGE, Seconds (0.20) );

  // simulate sample additions with window 100 ms
  Simulator::Schedule (Seconds (0.17), &SatBasicEstimatorLastTestCase::AddSample, this, -4.2 );
  Simulator::Schedule (Seconds (0.22), &SatBasicEstimatorLastTestCase::AddSample, this, 8.1 );
  Simulator::Schedule (Seconds (0.26), &SatBasicEstimatorLastTestCase::AddSample, this, -15.7 );
  Simulator::Schedule (Seconds (0.43), &SatBasicEstimatorLastTestCase::AddSample, this, 2.4 );

  // simulate C/N0 estimations with window 200 ms
  Simulator::Schedule (Seconds (0.09), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected
  Simulator::Schedule (Seconds (0.19), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // -4.2 expected
  Simulator::Schedule (Seconds (0.35), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // (-4.2 + 8.1 -15.7) / 3 expected
  Simulator::Schedule (Seconds (0.41), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // (8.1 -15.7) / 2 expected
  Simulator::Schedule (Seconds (0.49), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // 2.4 expected
  Simulator::Schedule (Seconds (0.69), &SatBasicEstimatorLastTestCase::GetCnoEstimation, this ); // NAN expected

  Simulator::Run ();

  // After simulation check that estimations are as expected

  // estimations with window 200 ms
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (m_cnoEstimations[0]), true, "first estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ_TOL ( m_cnoEstimations[1], -4.2, 0.0001, "second estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ_TOL ( m_cnoEstimations[2], (-4.2 + 8.1 - 15.7) / 3, 0.0001, "third estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ_TOL ( m_cnoEstimations[3], (8.1 - 15.7) / 2, 0.0001, "fourth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ_TOL ( m_cnoEstimations[4], 2.4, 0.0001, "fifth estimation incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (m_cnoEstimations[5]), true, "sixth estimation incorrect");

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}


/**
 * \ingroup satellite
 * \brief Test suite for Satellite C/N0 estimator unit test cases.
 */
class SatBasicCnoEstimatorTestSuite : public TestSuite
{
public:
  SatBasicCnoEstimatorTestSuite ();
};

SatBasicCnoEstimatorTestSuite::SatBasicCnoEstimatorTestSuite ()
  : TestSuite ("sat-cno-estimator-unit-test", UNIT)
{
  AddTestCase (new SatBasicEstimatorLastTestCase, TestCase::QUICK);
  AddTestCase (new SatBasicEstimatorMinTestCase, TestCase::QUICK);
  AddTestCase (new SatBasicEstimatorAverageTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatBasicCnoEstimatorTestSuite satCnoEstimatorUnit;

