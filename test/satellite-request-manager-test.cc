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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

/**
 * \file satellite-request-manager-test.cc
 * \ingroup satellite
 * \brief Test cases to test the UT request manager. Test cases:
 * - SatBaseTestCase is testing CRA. If DAMA is not configured at all
 * RM should not send CRs at all.
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "../model/satellite-request-manager.h"
#include "../model/satellite-queue.h"
#include "../model/satellite-control-message.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case to unit test the UT request manager.
 *
 */
class SatBaseTestCase : public TestCase
{
public:
  SatBaseTestCase ();
  virtual ~SatBaseTestCase ();

  /**
   * Send control message called with a callback from request manager. In
   * this test the method is used to analyze the sent requests from  RM.
   * \param msg Control msg (CR, or CNo report)
   * \param dest Destination MAC address
   * \return Boolean whether the send was successfull.
   */
  bool SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest);

  /**
   * Get queue statistics for request manager. This comes really from the
   * SatQueue, but here we send some known information as KPIs for RM.
   * \param reset Flag to reset queue statistics (not used here)
   * \return Queue statistics struct
   */
  SatQueue::QueueStats_t GetQueueStatistics (bool reset);

  /**
   * Check whether a control message transmission is possible.
   * \return Boolean indicating the possibility
   */
  bool ControlMsgTxPossible () const;

private:
  virtual void DoRun (void);

  /**
   * Received CRs
   */
  std::vector <Ptr<SatCrMessage> > m_rcvdCapacityRequests;
};

SatBaseTestCase::SatBaseTestCase ()
  : TestCase ("Test satellite request manager.")
{
}

SatBaseTestCase::~SatBaseTestCase ()
{
}

void
SatBaseTestCase::DoRun ()
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-rm", "", true);

  // Base test case tests that no Capacity Requests are generated when CRA, RBDC
  // and VBDC are disabled for all lower layer service RC indices.
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
  Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

  Ptr<SatLowerLayerServiceConf> llsConf = CreateObject<SatLowerLayerServiceConf>  ();

  Time superFrameDuration (MilliSeconds (100));

  // Create and initialize request manager
  Ptr<SatNodeInfo> nodeInfo = Create<SatNodeInfo> (SatEnums::NT_UT, 0, Mac48Address::Allocate ());
  Ptr<SatRequestManager> rm = CreateObject <SatRequestManager> ();
  rm->SetNodeInfo (nodeInfo);
  rm->Initialize (llsConf, superFrameDuration);

  rm->SetCtrlMsgTxPossibleCallback (MakeCallback (&SatBaseTestCase::ControlMsgTxPossible, this));

  // Set send control message callback
  rm->SetCtrlMsgCallback (MakeCallback (&SatBaseTestCase::SendControlMsg, this));

  // Set queue statistics callbacks
  Callback<SatQueue::QueueStats_t, bool> cb = MakeCallback (&SatBaseTestCase::GetQueueStatistics, this);
  for (uint8_t rc = 0; rc < llsConf->GetDaServiceCount (); ++rc)
    {
      rm->AddQueueCallback (rc, cb);
    }

  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  NS_TEST_ASSERT_MSG_EQ (m_rcvdCapacityRequests.empty (), true, "Capacity requests received!" );

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

bool
SatBaseTestCase::SendControlMsg (Ptr<SatControlMessage> msg, const Address& dest)
{
  if (msg->GetMsgType () == SatControlMsgTag::SAT_CR_CTRL_MSG)
    {
      Ptr<SatCrMessage> cr = DynamicCast<SatCrMessage> (msg);
      if (cr == NULL)
        {
          NS_FATAL_ERROR ("Dynamic cast to CR message failed!");
        }
      m_rcvdCapacityRequests.push_back (cr);
    }
  else
    {
      // Also CNo reports arrive here
    }
  return true;
}


SatQueue::QueueStats_t
SatBaseTestCase::GetQueueStatistics (bool reset)
{
  SatQueue::QueueStats_t queueStats;

  queueStats.m_incomingRateKbps = 10;
  queueStats.m_outgoingRateKbps = 10;
  queueStats.m_volumeInBytes = 10;
  queueStats.m_volumeOutBytes = 10;
  queueStats.m_queueSizeBytes = 10;

  return queueStats;
}

bool
SatBaseTestCase::ControlMsgTxPossible () const
{
  return true;
}

/**
 * \brief Test suite for Satellite Request Manager unit test cases.
 */
class SatRequestManagerTestSuite : public TestSuite
{
public:
  SatRequestManagerTestSuite ();
};

SatRequestManagerTestSuite::SatRequestManagerTestSuite ()
  : TestSuite ("sat-rm-test", UNIT)
{
  AddTestCase (new SatBaseTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatRequestManagerTestSuite satRmTestSuite;

