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
 * \file satellite-antenna-pattern-test.cc
 * \brief Test cases to unit test Satellite Antenna Pattern
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include <ns3/ptr.h>
#include "../helper/satellite-wave-form-conf.h"
#include "../model/satellite-link-results.h"
#include "../model/satellite-utils.h"


using namespace ns3;

/**
 * \brief Test case to unit test satellite antenna patterns
 *
 */
class SatWaveformTableTestCase : public TestCase
{
public:
  SatWaveformTableTestCase ();
  virtual ~SatWaveformTableTestCase ();

private:
  virtual void DoRun (void);

};

SatWaveformTableTestCase::SatWaveformTableTestCase ()
  : TestCase ("Test satellite antenna gain pattern.")
{
}

SatWaveformTableTestCase::~SatWaveformTableTestCase ()
{
}


void
SatWaveformTableTestCase::DoRun (void)
{
  std::string path = "src/satellite/data/";
  std::string fileName = "dvbRcs2Waveforms.txt";

  Ptr<SatLinkResultsDvbRcs2> lr = CreateObject<SatLinkResultsDvbRcs2> ();
  lr->Initialize ();

  Ptr<SatWaveformConf> wf = CreateObject<SatWaveformConf> (path+fileName);
  wf->InitializeEsNoRequirements( lr );

  uint32_t refResults [23] = {4, 5, 6, 7, 7, 7, 7, 7, 8, 8, 9, 9, 10, 11, 11, 12, 12, 12, 12, 12, 12};

  // 250 kbaud
  double symbolRate (250000);
  uint32_t i (0);

  // Method capable of dumping the waveform conf to standard output
  /*
  double rollOff (0.2);
  double carrierBandwidth = (1.0 + rollOff) * symbolRate;
  wf->Dump (carrierBandwidth, symbolRate);
  */

  for (double d = 60.0; d <= 70.0; d += 0.5)
    {
      uint32_t wfid (0);
      bool success = wf->GetBestWaveformId(SatUtils::DbToLinear (d), symbolRate, wfid);
      NS_TEST_ASSERT_MSG_EQ(success, true, "A suitable waveform not found");
      NS_TEST_ASSERT_MSG_EQ(wfid, refResults[i], "Not expected waveform id");
      ++i;
    }
}

/**
 * \brief Test suite for Satellite free space loss unit test cases.
 */
class SatWaveformTableTraceSuite : public TestSuite
{
public:
  SatWaveformTableTraceSuite ();
};

SatWaveformTableTraceSuite::SatWaveformTableTraceSuite ()
  : TestSuite ("sat-waveform-table-test", UNIT)
{
  AddTestCase (new SatWaveformTableTestCase);
}

// Do allocate an instance of this TestSuite
static SatWaveformTableTraceSuite satWaveformTableTestSuite;

