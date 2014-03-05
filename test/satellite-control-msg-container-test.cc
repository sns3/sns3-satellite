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
 * \file satellite-interference-test.cc
 * \ingroup satellite
 * \brief Test cases to unit test Satellite control message container.
 */

// Include a header file from your module to test.
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "../model/satellite-control-message.h"

using namespace ns3;

class SatCtrlMsgContBaseTestCase : public TestCase
{
public:
  SatCtrlMsgContBaseTestCase () : TestCase("") {};
  SatCtrlMsgContBaseTestCase (std::string info) : TestCase(info) {};
  virtual ~SatCtrlMsgContBaseTestCase () {}

  // add message to container
  void AddMessage (Ptr<SatControlMessage> msg);

  // get message.
  void GetMessage (uint32_t msgId);

protected:
  virtual void DoRun (void) = 0;
  Ptr<SatControlMsgContainer> m_container;
  std::vector<Ptr<SatControlMessage> >  m_msgsRead;
};

void
SatCtrlMsgContBaseTestCase::AddMessage (Ptr<SatControlMessage> msg)
{
  m_container->Add (msg);
}

void
SatCtrlMsgContBaseTestCase::GetMessage (uint32_t msgId)
{
  m_msgsRead.push_back (m_container->Get (msgId));
}

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite control message container with flag deletedOnRead set.
 *
 * This case tests that SatControlMsgContainer can be created with flag deletedOnRead set and
 * messages can be read correctly in set store window time.
 *  1.  Create SatControlMsgContainer object with deletedOnRead flag set true.
 *  2.  Set messages to container at different points of time (method Add).
 *  3.  Get messages with given IDs from container at some points of time (method Get).
 *
 *  Expected result:
 *   Message can be read only once, if the store time is not expired for the message.
 *   NULL is returned by Get method, if store time is expired or if same message is tried to read twice.
 *
 *
 */
class SatCtrlMsgContDelOnTestCase : public SatCtrlMsgContBaseTestCase
{
public:
  SatCtrlMsgContDelOnTestCase () : SatCtrlMsgContBaseTestCase ("Test satellite control message container with flag deletedOnRead set.") {}
  virtual ~SatCtrlMsgContDelOnTestCase () {}

protected:
  virtual void DoRun (void);
};

void
SatCtrlMsgContDelOnTestCase::DoRun (void)
{
  // create container with store time 100 ms and flag deletedOnRead set
  m_container = Create<SatControlMsgContainer> (Seconds (0.10), true);
  Ptr<SatControlMessage> crMsg = Create<SatCrMessage> ();
  Ptr<SatControlMessage> tbtpMsg = Create<SatTbtpMessage> ();

  // simulate message additions
  Simulator::Schedule (Seconds (0.09), &SatCtrlMsgContDelOnTestCase::AddMessage, this, crMsg ); // id 0
  Simulator::Schedule (Seconds (0.20), &SatCtrlMsgContDelOnTestCase::AddMessage, this, tbtpMsg ); // id 1
  Simulator::Schedule (Seconds (0.31), &SatCtrlMsgContDelOnTestCase::AddMessage, this, tbtpMsg ); // id 2
  Simulator::Schedule (Seconds (0.40), &SatCtrlMsgContDelOnTestCase::AddMessage, this, crMsg ); // id 3

  // simulate get operations
  Simulator::Schedule (Seconds (0.10), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 0 ); // crMsg expected
  Simulator::Schedule (Seconds (0.11), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 0 ); // NULL expected
  Simulator::Schedule (Seconds (0.21), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 1 ); // tbtpMsg expected
  Simulator::Schedule (Seconds (0.22), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 1 ); // NULL expected
  Simulator::Schedule (Seconds (0.42), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 2 ); // NULL expected
  Simulator::Schedule (Seconds (0.43), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 3 ); // crMsg expected
  Simulator::Schedule (Seconds (0.49), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 3 ); // NULL expected

  Simulator::Run ();

  // After simulation check that messages are what expected

  // ref messages created successfully
  NS_TEST_ASSERT_MSG_EQ ((crMsg == NULL ) , false, "CR message creation failed");
  NS_TEST_ASSERT_MSG_EQ ((tbtpMsg == NULL ) , false, "TBTP message failed");

  // container content correct at points messages got
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[0] == crMsg) , true, "first message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[1] == NULL) , true, "second message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[2] == tbtpMsg) , true, "third message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[3] == NULL) , true, "fourth message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[4] == NULL) , true, "fifth message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[5] == crMsg) , true, "sixth message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[6] == NULL) , true, "seventh message incorrect");

  Simulator::Destroy ();
}

/**
 * \ingroup satellite
 * \brief Test case to unit test satellite control message container with flag deletedOnRead not set.
 *
 * This case tests that SatControlMsgContainer can be created with flag deletedOnRead not set and
 * messages can be read correctly in set store window time.
 *  1.  Create SatControlMsgContainer object with deletedOnRead flag set false.
 *  2.  Set messages to container at different points of time (method Add).
 *  3.  Get messages with given IDs from container at some points of time (method Get).
 *
 *  Expected result:
 *   Message can be read until the store time is not expired for the message.
 *   NULL is returned by Get method, if store time is expired.
 *
 *
 */
class SatCtrlMsgContDelOffTestCase : public SatCtrlMsgContBaseTestCase
{
public:
  SatCtrlMsgContDelOffTestCase () : SatCtrlMsgContBaseTestCase ("Test satellite control message container with flag deletedOnRead NOT set.") {}
  virtual ~SatCtrlMsgContDelOffTestCase () {}

protected:
  virtual void DoRun (void);
};

void
SatCtrlMsgContDelOffTestCase::DoRun (void)
{
  // create container with store time 100 ms and flag deletedOnRead NOT set
  m_container = Create<SatControlMsgContainer> (Seconds (0.10), false);
  Ptr<SatControlMessage> crMsg = Create<SatCrMessage> ();
  Ptr<SatControlMessage> tbtpMsg = Create<SatTbtpMessage> ();

  // simulate message additions
  Simulator::Schedule (Seconds (0.09), &SatCtrlMsgContDelOnTestCase::AddMessage, this, crMsg ); // id 0
  Simulator::Schedule (Seconds (0.20), &SatCtrlMsgContDelOnTestCase::AddMessage, this, tbtpMsg ); // id 1
  Simulator::Schedule (Seconds (0.31), &SatCtrlMsgContDelOnTestCase::AddMessage, this, tbtpMsg ); // id 2
  Simulator::Schedule (Seconds (0.40), &SatCtrlMsgContDelOnTestCase::AddMessage, this, crMsg ); // id 3

  // simulate get operations
  Simulator::Schedule (Seconds (0.10), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 0 ); // crMsg expected
  Simulator::Schedule (Seconds (0.11), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 0 ); // crMsg expected
  Simulator::Schedule (Seconds (0.21), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 1 ); // tbtpMsg expected
  Simulator::Schedule (Seconds (0.22), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 1 ); // tbtpMsg expected
  Simulator::Schedule (Seconds (0.42), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 2 ); // NULL expected
  Simulator::Schedule (Seconds (0.43), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 3 ); // crMsg expected
  Simulator::Schedule (Seconds (0.49), &SatCtrlMsgContDelOnTestCase::GetMessage, this, 3 ); // crMsg expected

  Simulator::Run ();

  // After simulation check that messages are what expected

  // ref messages created successfully
  NS_TEST_ASSERT_MSG_EQ ((crMsg == NULL ) , false, "CR message creation failed");
  NS_TEST_ASSERT_MSG_EQ ((tbtpMsg == NULL ) , false, "TBTP message failed");

  // container content correct at points messages got
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[0] == crMsg) , true, "first message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[1] == crMsg) , true, "second message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[2] == tbtpMsg) , true, "third message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[3] == tbtpMsg) , true, "fourth message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[4] == NULL) , true, "fifth message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[5] == crMsg) , true, "sixth message incorrect");
  NS_TEST_ASSERT_MSG_EQ ((m_msgsRead[6] == crMsg) , true, "seventh message incorrect");

  Simulator::Destroy ();
}

/**
 * \ingroup satellite
 * \brief Test suite for Satellite control message container unit test cases.
 */
class SatCtrlMsgContainerTestSuite : public TestSuite
{
public:
  SatCtrlMsgContainerTestSuite ();
};

SatCtrlMsgContainerTestSuite::SatCtrlMsgContainerTestSuite ()
  : TestSuite ("sat-ctrl-msg-container-unit-test", UNIT)
{
  AddTestCase (new SatCtrlMsgContDelOnTestCase, TestCase::QUICK);
  AddTestCase (new SatCtrlMsgContDelOffTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatCtrlMsgContainerTestSuite satCtrlMsgContUnit;

