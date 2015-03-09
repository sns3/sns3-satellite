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
 * \ingroup satellite
 * \file satellite-rle-test.cc
 * \brief Return Link Encapsulator test suite
 */

#include <vector>
#include <deque>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "../model/satellite-arq-sequence-number.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \file satellite-arq-seqno-test.cc
 * \ingroup satellite
 * \brief ARQ sequence number handler test
 */
class SatSeqNoTestCase : public TestCase
{
public:
  SatSeqNoTestCase ();
  virtual ~SatSeqNoTestCase ();

private:
  virtual void DoRun (void);

};

SatSeqNoTestCase::SatSeqNoTestCase ()
  : TestCase ("Test ARQ sequence numbers.")
{
}

SatSeqNoTestCase::~SatSeqNoTestCase ()
{
}


void
SatSeqNoTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-arq-seqno", "", true);

  uint32_t windowSize (10);
  Ptr<SatArqSequenceNumber> seqNo = Create<SatArqSequenceNumber> (windowSize);

  std::deque<uint32_t> seqNoWindow;
  std::vector<uint32_t> allSeqNos;

  for (uint32_t i = 0; i < 550; ++i)
    {
      if (seqNo->SeqNoAvailable ())
        {
          uint32_t sn = uint32_t (seqNo->NextSequenceNumber ());
          allSeqNos.push_back (sn);
          seqNoWindow.push_back (sn);
        }
      else
        {
          uint32_t oldest = seqNoWindow.front ();
          seqNo->Release (oldest);
          seqNoWindow.pop_front ();
        }
    }

  for (uint32_t i = 0; i < allSeqNos.size (); ++i)
    {
      std::cout << "SN: " << allSeqNos[i] << std::endl;
    }

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test suite for RLE.
 */
class SatArqSeqNoTraceSuite : public TestSuite
{
public:
  SatArqSeqNoTraceSuite ();
};

SatArqSeqNoTraceSuite::SatArqSeqNoTraceSuite ()
  : TestSuite ("sat-arq-seqno-test", UNIT)
{
  AddTestCase (new SatSeqNoTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatArqSeqNoTraceSuite SatSeqNoTestSuite;

