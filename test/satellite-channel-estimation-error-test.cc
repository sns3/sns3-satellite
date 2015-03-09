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
 * \file satellite-channel-estimation-error-test.cc
 * \ingroup satellite
 * \brief Channel estimation error test suite
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "../model/satellite-channel-estimation-error-container.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case for FWD link channel estimation error
 */

class SatFwdChannelEstimationErrorTestCase : public TestCase
{
public:
  SatFwdChannelEstimationErrorTestCase ();
  virtual ~SatFwdChannelEstimationErrorTestCase ();

private:
  virtual void DoRun (void);

};

SatFwdChannelEstimationErrorTestCase::SatFwdChannelEstimationErrorTestCase ()
  : TestCase ("Test FWD link SINR channel estimation error.")
{
}

SatFwdChannelEstimationErrorTestCase::~SatFwdChannelEstimationErrorTestCase ()
{
}


void
SatFwdChannelEstimationErrorTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-channel-estimation-error", "fwd", true);

  Ptr<SatChannelEstimationErrorContainer> ceError = Create<SatFwdLinkChannelEstimationErrorContainer> ();

  for (double in = -20.0; in < 20.0; in += 0.1)
    {
      double out = ceError->AddError (in);

      std::cout << "SINR in: " << in << ", SINR out: " << out << std::endl;
    }

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test case for RTN link channel estimation error
 */
class SatRtnChannelEstimationErrorTestCase : public TestCase
{
public:
  SatRtnChannelEstimationErrorTestCase ();
  virtual ~SatRtnChannelEstimationErrorTestCase ();

private:
  virtual void DoRun (void);

};

SatRtnChannelEstimationErrorTestCase::SatRtnChannelEstimationErrorTestCase ()
  : TestCase ("Test RTN link SINR channel estimation error.")
{
}

SatRtnChannelEstimationErrorTestCase::~SatRtnChannelEstimationErrorTestCase ()
{
}

void
SatRtnChannelEstimationErrorTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-channel-estimation-error", "rtn", true);

  Ptr<SatChannelEstimationErrorContainer> ceError = Create<SatFwdLinkChannelEstimationErrorContainer> ();

  for (uint32_t wf = 3; wf <= 23; ++wf)
    {
      std::cout << "--- WF:  " << wf << std::endl;
      for (double in = -20.0; in < 20.0; in += 0.1)
        {
          double out = ceError->AddError (in);

          std::cout << "SINR in: " << in << ", SINR out: " << out << std::endl;
        }
    }
  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test suite for satellite channel estimation error
 */
class SatChannelEstimationErrorTestSuite : public TestSuite
{
public:
  SatChannelEstimationErrorTestSuite ();
};

SatChannelEstimationErrorTestSuite::SatChannelEstimationErrorTestSuite ()
  : TestSuite ("sat-channel-estimation-error-test", UNIT)
{
  AddTestCase (new SatFwdChannelEstimationErrorTestCase, TestCase::QUICK);
  AddTestCase (new SatRtnChannelEstimationErrorTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatChannelEstimationErrorTestSuite satChannelEstimationErrorTestSuite;

