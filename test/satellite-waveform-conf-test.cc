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
 * \file satellite-waveform-conf-test.cc
 * \brief Waveform conf test suite
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include <ns3/ptr.h>
#include "../helper/satellite-wave-form-conf.h"
#include "../helper/satellite-bbframe-conf.h"
#include "../model/satellite-link-results.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-enums.h"


using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case to unit test the waveform configuration table for DVB-RCS2
 *
 * Expected result:
 * - Creates link results and waveform config instances for DVB-RCS2
 * - Calculates the best waveform ids for a range of C/Nos from 60 to 70 dBs
 * - If waveform is not found or the waveform id is not expected, the test
 *   case shall fail
 */
class SatDvbRcs2WaveformTableTestCase : public TestCase
{
public:
  SatDvbRcs2WaveformTableTestCase ();
  virtual ~SatDvbRcs2WaveformTableTestCase ();

private:
  virtual void DoRun (void);

};

SatDvbRcs2WaveformTableTestCase::SatDvbRcs2WaveformTableTestCase ()
  : TestCase ("Test DVB-RCS2 waveform configuration table.")
{
}

SatDvbRcs2WaveformTableTestCase::~SatDvbRcs2WaveformTableTestCase ()
{
}


void
SatDvbRcs2WaveformTableTestCase::DoRun (void)
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
 * \ingroup satellite
 * \brief Test case to unit test to create BBFrame conf and its public methods.
 *
 * Expected result:
 * - Creates SatBbFrameConf
 * - Calculate the BBFrame length in Time for all the MODCODs and BBFrame types
 * - Calculate the BBFrame payload in bits for all the MODCODs and BBFrame types
 */
class SatDvbS2BbFrameConfTestCase : public TestCase
{
public:
  SatDvbS2BbFrameConfTestCase ();
  virtual ~SatDvbS2BbFrameConfTestCase ();

private:
  virtual void DoRun (void);

};

SatDvbS2BbFrameConfTestCase::SatDvbS2BbFrameConfTestCase ()
  : TestCase ("Test DVB-S2 BBFrame configuration.")
{
}

SatDvbS2BbFrameConfTestCase::~SatDvbS2BbFrameConfTestCase ()
{
}


void
SatDvbS2BbFrameConfTestCase::DoRun (void)
{
  // Create BBFrame conf
  Ptr<SatBbFrameConf> bbFrameConf = CreateObject<SatBbFrameConf> ();

  // Tested symbol rate in baud
  double symbolRate (93750000);

  // Available modcods
  SatEnums::SatModcod_t modcods[24] = { SatEnums::SAT_MODCOD_QPSK_1_TO_2,
                                        SatEnums::SAT_MODCOD_QPSK_2_TO_3,
                                        SatEnums::SAT_MODCOD_QPSK_3_TO_4,
                                        SatEnums::SAT_MODCOD_QPSK_3_TO_5,
                                        SatEnums::SAT_MODCOD_QPSK_4_TO_5,
                                        SatEnums::SAT_MODCOD_QPSK_5_TO_6,
                                        SatEnums::SAT_MODCOD_QPSK_8_TO_9,
                                        SatEnums::SAT_MODCOD_QPSK_9_TO_10,
                                        SatEnums::SAT_MODCOD_8PSK_2_TO_3,
                                        SatEnums::SAT_MODCOD_8PSK_3_TO_4,
                                        SatEnums::SAT_MODCOD_8PSK_3_TO_5,
                                        SatEnums::SAT_MODCOD_8PSK_5_TO_6,
                                        SatEnums::SAT_MODCOD_8PSK_8_TO_9,
                                        SatEnums::SAT_MODCOD_8PSK_9_TO_10,
                                        SatEnums::SAT_MODCOD_16APSK_2_TO_3,
                                        SatEnums::SAT_MODCOD_16APSK_3_TO_4,
                                        SatEnums::SAT_MODCOD_16APSK_4_TO_5,
                                        SatEnums::SAT_MODCOD_16APSK_5_TO_6,
                                        SatEnums::SAT_MODCOD_16APSK_8_TO_9,
                                        SatEnums::SAT_MODCOD_16APSK_9_TO_10,
                                        SatEnums::SAT_MODCOD_32APSK_3_TO_4,
                                        SatEnums::SAT_MODCOD_32APSK_4_TO_5,
                                        SatEnums::SAT_MODCOD_32APSK_5_TO_6,
                                        SatEnums::SAT_MODCOD_32APSK_8_TO_9,
  };

  // Available BBFrames
  SatEnums::SatBbFrameType_t frameTypes[2] = { SatEnums::SHORT_FRAME,
                                               SatEnums::NORMAL_FRAME,
  };

  std::cout << "BBFrame config output: " << std::endl;
  std::cout << "----------------------" << std::endl;

  // BBFrames
  for (uint32_t i = 0; i < 2; ++i)
    {
      // Modcods
      for (uint32_t j = 0; j < 24; ++j)
        {
          // Get BBFrame length in Time
          Time l = bbFrameConf->GetBbFrameLength (modcods[j], frameTypes[i], symbolRate);

          // Get BBFrame payload in bits
          uint32_t p = bbFrameConf->GetBbFramePayloadBits (modcods[j], frameTypes[i]);

          std::cout << "MODCOD: " << SatEnums::GetModcodTypeName(modcods[j]) <<
              ", frameType: " << frameTypes[i] <<
              ", length [s]: " << l.GetSeconds () <<
              ", payload [B]: " << p << std::endl;
        }
    }
}


/**
 * \brief Test suite for Satellite free space loss unit test cases.
 */
class SatWaveformConfTestSuite : public TestSuite
{
public:
  SatWaveformConfTestSuite ();
};

SatWaveformConfTestSuite::SatWaveformConfTestSuite ()
  : TestSuite ("sat-waveform-conf-test", UNIT)
{
  AddTestCase (new SatDvbRcs2WaveformTableTestCase);
  AddTestCase (new SatDvbS2BbFrameConfTestCase);
}

// Do allocate an instance of this TestSuite
static SatWaveformConfTestSuite satWaveformConfTestSuite;

