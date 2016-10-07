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
#include "ns3/ptr.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "../model/satellite-wave-form-conf.h"
#include "../model/satellite-bbframe-conf.h"
#include "../model/satellite-link-results.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-enums.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"

using namespace ns3;

/**
 * \file satellite-waveform-conf-test.cc
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
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-waveform-conf", "dvbrcs2", true);

  std::string path = Singleton<SatEnvVariables>::Get ()->GetDataPath () + "/";
  std::string fileName = "dvbRcs2Waveforms.txt";

  // Enable ACM
  Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));

  Ptr<SatLinkResultsDvbRcs2> lr = CreateObject<SatLinkResultsDvbRcs2> ();
  lr->Initialize ();

  Ptr<SatWaveformConf> wf = CreateObject<SatWaveformConf> (path + fileName);
  wf->InitializeEbNoRequirements ( lr );

  uint32_t refResults [21] = {6, 6, 7, 7, 7, 8, 8, 9, 9, 9, 10, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12};

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
      bool success = wf->GetBestWaveformId (SatUtils::DbToLinear (d), symbolRate, wfid);

      NS_TEST_ASSERT_MSG_EQ (success, true, "A suitable waveform not found");
      NS_TEST_ASSERT_MSG_EQ (wfid, refResults[i], "Not expected waveform id");
      ++i;
    }
  Singleton<SatEnvVariables>::Get ()->DoDispose ();
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
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-waveform-conf", "dvbs2", true);

  // Tested symbol rate in baud
  double symbolRate (93750000);

  Ptr<SatLinkResultsDvbS2> lr = CreateObject<SatLinkResultsDvbS2> ();
  lr->Initialize ();

  // Create BBFrame conf
  Ptr<SatBbFrameConf> bbFrameConf = CreateObject<SatBbFrameConf> (symbolRate);
  bbFrameConf->InitializeCNoRequirements (lr);

  std::vector<SatEnums::SatModcod_t> modcods;
  SatEnums::GetAvailableModcodsFwdLink (modcods);

  /**
   * Available BBFrames. Note that SHORT_FRAME is not yet supported, since
   * we do not have link results for it.
   */

  //SatEnums::SatBbFrameType_t frameTypes[2] = { SatEnums::SHORT_FRAME,
  //                                             SatEnums::NORMAL_FRAME,
  //};

  SatEnums::SatBbFrameType_t frameTypes[1] = { SatEnums::NORMAL_FRAME };

  std::cout << "BBFrame config output: " << std::endl;
  std::cout << "----------------------" << std::endl;

  // BBFrames
  for (uint32_t i = 0; i < 1; ++i)
    {
      // Modcods
      for (std::vector<SatEnums::SatModcod_t>::iterator it = modcods.begin ();
           it != modcods.end ();
           ++it)
        {
          // Get BBFrame length in Time
          Time l = bbFrameConf->GetBbFrameDuration ((*it), frameTypes[i]);

          // Get BBFrame payload in bits
          uint32_t p = bbFrameConf->GetBbFramePayloadBits (*it, frameTypes[i]);

          std::cout << "MODCOD: " << SatEnums::GetModcodTypeName (*it) <<
          ", frameType: " << frameTypes[i] <<
          ", length [s]: " << l.GetSeconds () <<
          ", payload [b]: " << p << std::endl;
        }
    }
  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}


/**
 * \ingroup satellite
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
  AddTestCase (new SatDvbRcs2WaveformTableTestCase, TestCase::QUICK);
  AddTestCase (new SatDvbS2BbFrameConfTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatWaveformConfTestSuite satWaveformConfTestSuite;

