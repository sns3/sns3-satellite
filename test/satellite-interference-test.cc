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
#include "../model/satellite-constant-interference.h"
#include "../model/satellite-traced-interference.h"
#include "../model/satellite-per-packet-interference.h"

using namespace ns3;

/**
 * \brief Test case to unit test satellite constant interference model.
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
  Ptr<SatConstantInterference> interference = CreateObject<SatConstantInterference>();

  interference->Set(100);

  // just test that we can call add. However, It shouldn't have any effect on interference.
  Ptr<SatInterference::Event> event =  interference->Add(Time(10), 55);

  // Test event setting with constant interference (base class SatInterference can't be instantiate)
  NS_TEST_ASSERT_MSG_EQ( event->GetDuration(), Time(10), "Event duration is incorrect");
  NS_TEST_ASSERT_MSG_EQ( event->GetEndTime() - event->GetStartTime(), Time(10), "Event start time or end time incorrect");
  NS_TEST_ASSERT_MSG_EQ( event->GetRxPower(), 55, "Event RX power incorrect");

  interference->NotifyRxStart(event);

  double finalPower;
  double firstPower = interference->Calculate(event, &finalPower);

  NS_TEST_ASSERT_MSG_EQ( firstPower, 100, "First power incorrect");
  NS_TEST_ASSERT_MSG_EQ( firstPower, finalPower, "First power and final power not equal");

  interference->NotifyRxEnd(event);

  // just test that we can set interference again
  interference->Set(50);

  interference->NotifyRxStart(event);

  firstPower = interference->Calculate(event, &finalPower);

  NS_TEST_ASSERT_MSG_EQ( firstPower, 50, "First power incorrect");
  NS_TEST_ASSERT_MSG_EQ( firstPower, finalPower, "First power and final power not equal");
}

/**
 * \brief Test case to unit test satellite per packet interference model.
 *
 */
class SatPerPacketInterferenceTestCase : public TestCase
{
public:
  SatPerPacketInterferenceTestCase ();
  virtual ~SatPerPacketInterferenceTestCase ();

  // adds interference to model object
  void AddInterference(Time duration, double power);

  // adds receivers own interference to model object and schedules receiving
  void StartReceiver(Time duration, double power);

  // receives packets i.e. calculates interference and stops receiving.
  void Receive (uint32_t rxIndex);

private:
  virtual void DoRun (void);
  Ptr<SatPerPacketInterference> m_interference;
  Ptr<SatInterference::Event> m_rxEvent[4];
  uint32_t  m_rxIndex;
  double finalPower[4];
  double firstPower[4];
};

SatPerPacketInterferenceTestCase::SatPerPacketInterferenceTestCase ()
  : TestCase ("Test satellite per packet interference model.")
{
  m_interference = CreateObject<SatPerPacketInterference>();
  m_rxIndex = 0;

  for (int i = 0; i < 4; i++)
    {
      finalPower[i] = 0;
      firstPower[i] = 0;
      m_rxEvent[i] = NULL;
    }
}

SatPerPacketInterferenceTestCase::~SatPerPacketInterferenceTestCase ()
{
}

void
SatPerPacketInterferenceTestCase::AddInterference (Time duration, double power)
{
  Ptr<SatInterference::Event> event =  m_interference->Add(duration, power);
}

void
SatPerPacketInterferenceTestCase::StartReceiver (Time duration, double power)
{
  // add own interference
  m_rxEvent[m_rxIndex] =  m_interference->Add(duration, power);

  // notify interference object of receiving
  m_interference->NotifyRxStart(m_rxEvent[m_rxIndex]);

  // schedule receiving
  Simulator::Schedule(Time(duration), &SatPerPacketInterferenceTestCase::Receive, this, m_rxIndex);

  m_rxIndex++;
}

void
SatPerPacketInterferenceTestCase::Receive (uint32_t rxIndex)
{
  firstPower[rxIndex] = m_interference->Calculate(m_rxEvent[rxIndex], &finalPower[rxIndex]);
  m_interference->NotifyRxEnd(m_rxEvent[rxIndex]);
}

void
SatPerPacketInterferenceTestCase::DoRun (void)
{
  // simulate interferences and receiving (4 receivers), adding and calculation done in callback routines
  Simulator::Schedule(Time(0), &SatPerPacketInterferenceTestCase::AddInterference, this, Time(60), 60);
  Simulator::Schedule(Time(10), &SatPerPacketInterferenceTestCase::AddInterference, this, Time(40), 70);
  Simulator::Schedule(Time(30), &SatPerPacketInterferenceTestCase::AddInterference, this, Time(50), 10);
  Simulator::Schedule(Time(50), &SatPerPacketInterferenceTestCase::AddInterference, this, Time(40), 20);
  Simulator::Schedule(Time(10), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time(90), 50);  //event[0]
  Simulator::Schedule(Time(20), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time(50), 5);   //event[1]
  Simulator::Schedule(Time(30), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time(60), 30);  //event[2]
  Simulator::Schedule(Time(50), &SatPerPacketInterferenceTestCase::StartReceiver, this, Time(20), 40);  //event[3]

  Simulator::Run ();

  // After simulation check that calculated results are as planned

  // check first receiver
  double finalDiff = std::abs((double)995/(double)9 - finalPower[0]);

  NS_TEST_ASSERT_MSG_EQ( firstPower[0], 130, "First power incorrect");
  NS_TEST_ASSERT_MSG_LT( finalDiff, 0.0000000000001, "Final power incorrect");

  // check second receiver
  finalDiff = std::abs((double)196 - finalPower[1]);

  NS_TEST_ASSERT_MSG_EQ( firstPower[1], 180, "First power incorrect");
  NS_TEST_ASSERT_MSG_LT( finalDiff, 0.0000000000001, "Final power incorrect");

  // check third receiver
  finalDiff = std::abs((double)850/(double)6 - finalPower[2]);

  NS_TEST_ASSERT_MSG_EQ( firstPower[2], 195, "First power incorrect");
  NS_TEST_ASSERT_MSG_LT( finalDiff, 0.0000000000001, "Final power incorrect");

  // check fourth receiver
  finalDiff = std::abs((double)145 - finalPower[3]);

  NS_TEST_ASSERT_MSG_EQ( firstPower[3], 175, "First power incorrect");
  NS_TEST_ASSERT_MSG_LT( finalDiff, 0.00000000000001, "Final power incorrect");

  Simulator::Destroy ();
}

/**
 * \brief Test case to unit test satellite constant interference model.
 *
 */
class SatTracedInterferenceTestCase : public TestCase
{
public:
  SatTracedInterferenceTestCase ();
  virtual ~SatTracedInterferenceTestCase ();

private:
  virtual void DoRun (void);
};

SatTracedInterferenceTestCase::SatTracedInterferenceTestCase ()
  : TestCase ("Test satellite traced interference model.")
{
}

SatTracedInterferenceTestCase::~SatTracedInterferenceTestCase ()
{
}

void
SatTracedInterferenceTestCase::DoRun (void)
{
  // traced implementation is just place holder currently, so just test interface (interference 0)
  Ptr<SatTracedInterference> interference = CreateObject<SatTracedInterference>();
  Ptr<SatInterference::Event> event =  interference->Add(Time(10), 55);

  interference->NotifyRxStart(event);

  double finalPower = 0;
  double firstPower = interference->Calculate(event, &finalPower);

  NS_TEST_ASSERT_MSG_EQ( firstPower, 0, "First power incorrect");
  NS_TEST_ASSERT_MSG_EQ( firstPower, finalPower, "First power and final power not equal");

  interference->NotifyRxEnd(event);
}

/**
 * \brief Test suite for Satellite interference unit test cases.
 */
class SatInterferenceTestSuite : public TestSuite
{
public:
  SatInterferenceTestSuite ();
};

SatInterferenceTestSuite::SatInterferenceTestSuite ()
  : TestSuite ("sat-interference-test", UNIT)
{
  AddTestCase (new SatConstantInterferenceTestCase);
  AddTestCase (new SatTracedInterferenceTestCase);
  AddTestCase (new SatPerPacketInterferenceTestCase);
}

// Do allocate an instance of this TestSuite
static SatInterferenceTestSuite satSatInterferenceTestSuite;

