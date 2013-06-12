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
 * \file scenario-creation.cc
 * \brief 'Scenario Creation Test Cases' implementation of TN4.
 *
 * In this module are implemented all 'Scenario Creation' Test Cases
 * defined in document TN4.
 *
 */

#include "ns3/string.h"
#include "ns3/satellite-helper.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

/**
 * \brief 'Scenario Creation, Simple' test case implementation, id: sc-1 / TN4.
 *
 * This case tests successful creation of the simple scenario creation of the reference system.
 *  1.  Simple test scenario created with helper
 *
 *  Expected result:
 *    • Satellite, GW, UT and two user nodes created.
 *    • User return/forward and feeder return/forward channels created.
 *    • Satellite connected to GW with feeder channels and to UT with user channels.
 *    • User Node-1 is attached to GW node and User Node-2 attached to UT node.
 *
 */
class ScenarioCreation1 : public TestCase
{
public:
  ScenarioCreation1 ();
  virtual ~ScenarioCreation1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreation1::ScenarioCreation1 ()
  : TestCase ("'Scenario Creation, Simple' case tests successful creation of Simple test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreation1::~ScenarioCreation1 ()
{
}

//
// ScenarioCreation1 TestCase implementation
//
void
ScenarioCreation1::DoRun (void)
{
  // Create simple scenario
  Ptr<SatHelper> helper = CreateObject<SatHelper>();
  helper->EnableCreationTraces("simple-scenario-creation.log", false);
  helper->CreateScenario(SatHelper::Simple);

  // check results what can be done at this level. More checking done in module level with traces
  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers().GetN(), 1, "GW User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers().GetN(), 1, "UT User count is not what expected!");

  Simulator::Destroy();
}

/**
 * \brief 'Scenario Creation, Larger' test case implementation, id: sc-2 / TN4.
 *
 * This case tests successful creation of the larger scenario creation of the reference system.
 *  1.  Larger test scenario created with helper
 *
 *  Expected result:
 *    • Satellite, two GWs, four UTs and two user nodes created.
 *    • User return/forward and feeder return/forward channels created.
 *    • Satellite connected to GWs with feeder channels and to UTs with user channels.
 *    • User Node-1 is attached to GW node and User Node-2 to Node-6 attached to UT nodes.
 *
 */
class ScenarioCreation2 : public TestCase
{
public:
  ScenarioCreation2 ();
  virtual ~ScenarioCreation2 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreation2::ScenarioCreation2 ()
  : TestCase ("'Scenario Creation, Simple' case tests successful creation of Simple test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreation2::~ScenarioCreation2 ()
{
}

//
// ScenarioCreation2 TestCase implementation
//
void
ScenarioCreation2::DoRun (void)
{
  // Create larger scenario
  Ptr<SatHelper> helper = CreateObject<SatHelper>();
  helper->EnableCreationTraces("larger-scenario-creation.log", false);
  helper->CreateScenario(SatHelper::Larger);

  // check results what can be done at this level. More checking done in module level with traces
  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers().GetN(), 1, "GW User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers().GetN(), 5, "UT User count is not what expected!");

  Simulator::Destroy();
}

/**
 * \brief 'Scenario Creation, Full' test case implementation, id: sc-3 / TN4.
 *
 * This case tests successful creation of the full scenario creation of the reference system.
 *  1.  Full test scenario created with helper
 *
 *  Expected result:
 *    • Satellite, seven GWs, defined number (98) UTs and defined number of the user nodes created.
 *    • User return/forward and feeder return/forward channels created according to configuration.
 *    • Satellite connected to GWs with feeder channels and to UTs with user channels.
 *    • User nodes attached to GW and UT .
 *
 */
class ScenarioCreation3 : public TestCase
{
public:
  ScenarioCreation3 ();
  virtual ~ScenarioCreation3 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreation3::ScenarioCreation3 ()
  : TestCase ("'Scenario Creation, Simple' case tests successful creation of Simple test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreation3::~ScenarioCreation3 ()
{
}

//
// ScenarioCreation3 TestCase implementation
//
void
ScenarioCreation3::DoRun (void)
{
  // Create larger scenario
  Ptr<SatHelper> helper = CreateObject<SatHelper>();
  helper->EnableCreationTraces("full-scenario-creation.log", false );
  helper->CreateScenario(SatHelper::Full);

  // check results what can be done at this level. More checking done in module level with traces
  // reference system includes 98 beams and we create three UTs with three users per UT in full scenario
  // and five GW users
  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers().GetN(), 5, "GW User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers().GetN(), 98*3*3, "UT User count is not what expected!");

  Simulator::Destroy();
}


// The TestSuite class names the TestSuite as sat-scenario-creation, identifies what type of TestSuite (SYSTEM),
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class ScenarioCreationTestSuite : public TestSuite
{
public:
  ScenarioCreationTestSuite ();
};

ScenarioCreationTestSuite::ScenarioCreationTestSuite ()
  : TestSuite ("sat-scenario-creation", SYSTEM)
{
  // add ScenarioCreation1 case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreation1);

  // add ScenarioCreation2 case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreation2);

  // add ScenarioCreation3 case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreation3);

}

// Allocate an instance of this TestSuite
static ScenarioCreationTestSuite scenarioCreationTestSuite;

