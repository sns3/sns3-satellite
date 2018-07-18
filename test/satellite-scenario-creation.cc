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
 * \file satellite-scenario-creation.cc
 * \ingroup satellite
 * \brief 'Scenario Creation Test Cases' implementation of TN4.
 *
 * In this module are implemented all 'Scenario Creation' Test Cases
 * defined in document TN4.
 *
 */

#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "../helper/satellite-helper.h"
#include "ns3/singleton.h"
#include "ns3/satellite-id-mapper.h"
#include "../utils/satellite-env-variables.h"

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
class ScenarioCreationSimple : public TestCase
{
public:
  ScenarioCreationSimple ();
  virtual ~ScenarioCreationSimple ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreationSimple::ScenarioCreationSimple ()
  : TestCase ("'Scenario Creation, Simple' case tests successful creation of Simple test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreationSimple::~ScenarioCreationSimple ()
{
}

//
// ScenarioCreationSimple TestCase implementation
//
void
ScenarioCreationSimple::DoRun (void)
{
  // Reset singletons
  Singleton<SatIdMapper>::Get ()->Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-scenario-creation", "simple-scenario", true);

  // Create simple scenario

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  // Enable creation traces
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  helper->CreatePredefinedScenario (SatHelper::SIMPLE);

  // check results what can be done at this level. More checking done in module level with traces
  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers ().GetN (), 1, "GW User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers ().GetN (), 1, "UT User count is not what expected!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  Simulator::Destroy ();
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
class ScenarioCreationLarger : public TestCase
{
public:
  ScenarioCreationLarger ();
  virtual ~ScenarioCreationLarger ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreationLarger::ScenarioCreationLarger ()
  : TestCase ("'Scenario Creation, Larger' case tests successful creation of Larger test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreationLarger::~ScenarioCreationLarger ()
{
}

//
// ScenarioCreationLarger TestCase implementation
//
void
ScenarioCreationLarger::DoRun (void)
{
  // Reset singletons
  Singleton<SatIdMapper>::Get ()->Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-scenario-creation", "larger-scenario", true);

  // Create larger scenario

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  // Enable creation traces
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  helper->CreatePredefinedScenario (SatHelper::LARGER);

  // check results what can be done at this level. More checking done in module level with traces
  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers ().GetN (), 1, "GW User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers ().GetN (), 5, "UT User count is not what expected!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  Simulator::Destroy ();
}

/**
 * \brief 'Scenario Creation, Full' test case implementation, id: sc-3 / TN4.
 *
 * This case tests successful creation of the full scenario creation of the reference system.
 *  1.  Full test scenario created with helper
 *
 *  Expected result:
 *    • Satellite, seven GWs, defined number (3*98) UTs and defined number of the user nodes created.
 *    • User return/forward and feeder return/forward channels created according to configuration.
 *    • Satellite connected to GWs with feeder channels and to UTs with user channels.
 *    • User nodes attached to GW and UT .
 *
 */
class ScenarioCreationFull : public TestCase
{
public:
  ScenarioCreationFull ();
  virtual ~ScenarioCreationFull ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreationFull::ScenarioCreationFull ()
  : TestCase ("'Scenario Creation, Full' case tests successful creation of Full test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreationFull::~ScenarioCreationFull ()
{
}

//
// ScenarioCreationFull TestCase implementation
//
void
ScenarioCreationFull::DoRun (void)
{
  // Reset singletons
  Singleton<SatIdMapper>::Get ()->Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-scenario-creation", "full-scenario", true);

  // Create full scenario

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  // Enable creation traces
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  helper->CreatePredefinedScenario (SatHelper::FULL);

  // check results what can be done at this level. More checking done in module level with traces
  // reference system includes 98 beams and we create three UTs with three users per UT in full scenario
  // and five GW users

  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers ().GetN (), 5, "GW User count is not what expected!");
  //NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers().GetN(), 98*3*3, "UT User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers ().GetN (), 72 * 3 * 3, "UT User count is not what expected!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  Simulator::Destroy ();
}

/**
 * \brief 'Scenario Creation, User Defined' test case implementation, id: tbd / TN4.
 *
 * This case tests successful creation of the user defined scenario creation of the reference system.
 *  1. Set beam info attibute(s) to satellite helper
 *  2. User defined scenario created with helper
 *
 *  Expected result:
 *    • Satellite, needed GWs, defined number UTs and defined number of the user nodes created.
 *    • User return/forward and feeder return/forward channels created according to configuration.
 *    • Satellite connected to GWs with feeder channels and to UTs with user channels.
 *    • User nodes attached to GW and UT .
 *
 */
class ScenarioCreationUser : public TestCase
{
public:
  ScenarioCreationUser ();
  virtual ~ScenarioCreationUser ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ScenarioCreationUser::ScenarioCreationUser ()
  : TestCase ("'Scenario Creation, User defined' case tests successful creation of User defined test scenario")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ScenarioCreationUser::~ScenarioCreationUser ()
{
}

//
// ScenarioCreationUser TestCase implementation
//
void
ScenarioCreationUser::DoRun (void)
{
  // Reset singletons
  Singleton<SatIdMapper>::Get ()->Reset ();

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-scenario-creation", "user-scenario", true);

  // Create user scenario

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  SatBeamUserInfo beamInfo = SatBeamUserInfo (1,1);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[8] = beamInfo;
  beamMap[3] = beamInfo;
  beamInfo.AppendUt (2);
  beamMap[2] = beamInfo;

  // Enable creation traces
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));

  helper->CreateUserDefinedScenario (beamMap);

  // check results what can be done at this level. More checking done in module level with traces
  NS_TEST_ASSERT_MSG_EQ (helper->GetGwUsers ().GetN (), 5, "GW User count is not what expected!");
  NS_TEST_ASSERT_MSG_EQ (helper->GetUtUsers ().GetN (), 5, "UT User count is not what expected!");

  Singleton<SatEnvVariables>::Get ()->DoDispose ();

  Simulator::Destroy ();
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
  // add ScenarioCreationSimple case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreationSimple, TestCase::QUICK);

  // add ScenarioCreationLarger case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreationLarger, TestCase::QUICK);

  // add ScenarioCreationFull case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreationFull, TestCase::QUICK);

  // add ScenarioCreationUser case to suite sat-scenario-creation
  AddTestCase (new ScenarioCreationUser, TestCase::QUICK);

}

// Allocate an instance of this TestSuite
static ScenarioCreationTestSuite scenarioCreationTestSuite;

