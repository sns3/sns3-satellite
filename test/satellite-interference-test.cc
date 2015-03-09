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
 * \file satellite-interference-test.cc
 * \ingroup satellite
 * \brief Test cases to unit test Satellite Interference Models.
 */

// Include a header file from your module to test.
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "../model/satellite-constant-interference.h"
#include "../model/satellite-traced-interference.h"
#include "../model/satellite-per-packet-interference.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite constant interference model.
 *
 * This case tests that SatConstantInterference object can be created successfully and interference value set is correct.
 *  1.  Create SatConstantInterference object.
 *  2.  Set constant interference.
 *  3.  Create event and notify SatConstantInterference about it.
 *  4.  Get interference with calculate method with the event.
 *
 *  Expected result:
 *   Calculate method should return set constant interference value.
 *
 */
class SatConstantInterferenceTestCase : public TestCase
{
public:
  SatConstantInterferenceTestCase ();
  virtual ~SatConstantInterferenceTestCase ();

private:
  virtual void DoRun (void);
};

SatConstantInterferenceTestCase::SatConstantInterferenceTestCase ()
  : TestCase ("Test satellite constant interference model.")
{
}

SatConstantInterferenceTestCase::~SatConstantInterferenceTestCase ()
{
}

void
SatConstantInterferenceTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-if-unit", "constant", true);

  Ptr<SatConstantInterference> interference = CreateObject<SatConstantInterference> ();
  interference->SetAttribute ("ConstantInterferencePower", DoubleValue (100.0));

  // just test that we can call add. However, It shouldn't have any effect on interference.
  Ptr<SatInterference::InterferenceChangeEvent> event =  interference->Add (Time (10), 55, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));

  // Test event setting with constant interference (base class SatInterference can't be instantiate)
  NS_TEST_ASSERT_MSG_EQ (event->GetDuration (), Time (10), "Event duration is incorrect");
  NS_TEST_ASSERT_MSG_EQ (event->GetEndTime () - event->GetStartTime (), Time (10), "Event start time or end time incorrect");
  NS_TEST_ASSERT_MSG_EQ (event->GetRxPower (), 55, "Event RX power incorrect");

  interference->NotifyRxStart (event);

  double power = interference->Calculate (event);

  NS_TEST_ASSERT_MSG_EQ (100, power, "Calculated power not correct");

  interference->NotifyRxEnd (event);

  // just test that we can set interference again
  interference->SetAttribute ("ConstantInterferencePower", DoubleValue (50.0));

  interference->NotifyRxStart (event);

  power = interference->Calculate (event);

  NS_TEST_ASSERT_MSG_EQ (50, power, "Calculated power not correct");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite per packet interference model.
 *
 * This case tests that SatPerPacketInterference object can be created successfully and interference value calculated correctly.
 *  1.  Create SatPerPacketInterference object.
 *  2.  Create events, add them to SatPerPacketInterference about them.
 *  3.  Notify SatPerPacketInterference about the event wanted to calculate.
 *  4.  Get interference with calculate method with the event to calculate.
 *
 *  Expected result:
 *   Value should be correctly calculated. According to interfering events added.
 *
 */
class SatPerPacketInterferenceTestCase : public TestCase
{
public:
  SatPerPacketInterferenceTestCase ();
  virtual ~SatPerPacketInterferenceTestCase ();

  // adds interference to model object
  void AddInterference (Time duration, double power, Address rxAddress);

  // adds receivers own interference to model object and schedules receiving
  void StartReceiver (Time duration, double power, Address rxAddress);

  // receives packets i.e. calculates interference and stops receiving.
  void Receive (uint32_t rxIndex);

private:
  virtual void DoRun (void);
  Ptr<SatPerPacketInterference> m_interference;
  Ptr<SatInterference::InterferenceChangeEvent> m_rxEvent[4];
  uint32_t  m_rxIndex;
  double finalPower[4];
};

SatPerPacketInterferenceTestCase::SatPerPacketInterferenceTestCase ()
  : TestCase ("Test satellite per packet interference model.")
{
  m_interference = CreateObject<SatPerPacketInterference> ();
  m_rxIndex = 0;

  for (int i = 0; i < 4; i++)
    {
      finalPower[i] = 0;
      m_rxEvent[i] = NULL;
    }
}

SatPerPacketInterferenceTestCase::~SatPerPacketInterferenceTestCase ()
{
}

void
SatPerPacketInterferenceTestCase::AddInterference (Time duration, double power, Address rxAddress)
{
  Ptr<SatInterference::InterferenceChangeEvent> event =  m_interference->Add (duration, power, rxAddress);
}

void
SatPerPacketInterferenceTestCase::StartReceiver (Time duration, double power, Address rxAddress)
{
  // add own interference
  m_rxEvent[m_rxIndex] =  m_interference->Add (duration, power, rxAddress);

  // notify interference object of receiving
  m_interference->NotifyRxStart (m_rxEvent[m_rxIndex]);

  // schedule receiving
  Simulator::Schedule (Time (duration), &SatPerPacketInterferenceTestCase::Receive, this, m_rxIndex);

  m_rxIndex++;
}

void
SatPerPacketInterferenceTestCase::Receive (uint32_t rxIndex)
{
  finalPower[rxIndex] = m_interference->Calculate (m_rxEvent[rxIndex]);
  m_interference->NotifyRxEnd (m_rxEvent[rxIndex]);
}

void
SatPerPacketInterferenceTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-if-unit", "perpacket", true);

  // simulate interferences and receiving (4 receivers), adding and calculation done in callback routines
  Simulator::Schedule (Time (0), &SatPerPacketInterferenceTestCase::AddInterference, this, Time (60), 60, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
  Simulator::Schedule (Time (10), &SatPerPacketInterferenceTestCase::AddInterference, this, Time (40), 70, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
  Simulator::Schedule (Time (30), &SatPerPacketInterferenceTestCase::AddInterference, this, Time (50), 10, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
  Simulator::Schedule (Time (50), &SatPerPacketInterferenceTestCase::AddInterference, this, Time (40), 20, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
  Simulator::Schedule (Time (10), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time (90), 50, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));  //event[0]
  Simulator::Schedule (Time (20), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time (50), 5, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));   //event[1]
  Simulator::Schedule (Time (30), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time (60), 30, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));  //event[2]
  Simulator::Schedule (Time (50), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time (20), 40, Mac48Address::ConvertFrom (Mac48Address::Allocate ()));  //event[3]

  Simulator::Run ();

  // After simulation check that calculated results are as planned

  // check first receiver
  double finalDiff = std::abs ((double)995 / (double)9 - finalPower[0]);

  NS_TEST_ASSERT_MSG_LT ( finalDiff, 0.0000000000001, "Final power incorrect");

  // check second receiver
  finalDiff = std::abs ((double)196 - finalPower[1]);

  NS_TEST_ASSERT_MSG_LT ( finalDiff, 0.0000000000001, "Final power incorrect");

  // check third receiver
  finalDiff = std::abs ((double)850 / (double)6 - finalPower[2]);

  NS_TEST_ASSERT_MSG_LT ( finalDiff, 0.0000000000001, "Final power incorrect");

  // check fourth receiver
  finalDiff = std::abs ((double)145 - finalPower[3]);

  NS_TEST_ASSERT_MSG_LT ( finalDiff, 0.00000000000001, "Final power incorrect");

  Simulator::Destroy ();
  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test suite for Satellite interference unit test cases.
 */
class SatInterferenceTestSuite : public TestSuite
{
public:
  SatInterferenceTestSuite ();
};

SatInterferenceTestSuite::SatInterferenceTestSuite ()
  : TestSuite ("sat-if-unit-test", UNIT)
{
  AddTestCase (new SatConstantInterferenceTestCase, TestCase::QUICK);
  AddTestCase (new SatPerPacketInterferenceTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatInterferenceTestSuite satSatInterferenceTestSuite;

