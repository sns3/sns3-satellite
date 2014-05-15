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
 * \file satellite-frame-allocator-test.cc
 * \ingroup satellite
 * \brief Test cases to unit test Satellite Frame Allocator and its related objects.
 */

#include <algorithm>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "../utils/satellite-env-variables.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-superframe-allocator.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Test case to unit test Satellite Frame Allocator.
 *
 * This case tests that SatFrameAllocator object allocates symbols (time slots)
 * for UT/RCs correctly with different configurations.
 *
 *  1. Create frame configuration and frame allocator.
 *  2. Reset allocator (Reset method))
 *  3. Allocate UT(s) with RCs to Allocator (method Allocate)
 *  4. When desired UTs allocated, pre-allocate symbols for the UT/RC(s). (method PreAllocateSymbols)
 *  5. Finally generate time slots for UTs (method GenerateTimeSlots) by
 *     enabling/disabling RC based allocation.
 *  6. Repeat steps 2 - 5 with different combination of UT/RCs.
 * *
 *  Expected result:
 *     Time slots for UT/RC should be generated according to allocation requests and limits in frames.
 *
 *  NOTE:
 *      Control slot generation should be also enabled for some allocation requests.
 *      Test should executed with ACM enabled and disabled.
 *
 */
class SatFrameAllocatorTestCase : public TestCase
{
public:
  SatFrameAllocatorTestCase ();
  virtual ~SatFrameAllocatorTestCase ();

private:
  static const uint32_t m_ccLevelCount = 4;
  static const uint32_t m_cnoValueCount = 10;

  SatFrameAllocator::CcLevel_t m_ccLevels[m_ccLevelCount];
  double m_cnoValues[m_cnoValueCount];
  uint32_t m_cnoIndex;
  Ptr<SatWaveformConf> m_waveFormConf;
  Ptr<SatFrameConf> m_frameConf;
  Ptr<SatFrameAllocator> m_frameAllocator;

  virtual void DoRun (void);

  void InitFrame (SatSuperframeConf::ConfigType_t configType);
  void RunSingleUtTest (SatSuperframeConf::ConfigType_t configType, bool acmEnabled);
  void RunMultiUtTest (SatSuperframeConf::ConfigType_t configType, bool acmEnabled);
  SatFrameAllocator::SatFrameAllocReq
  ContructRequestForSinleUtTest (uint32_t& totalBytes, uint32_t craBytes, uint32_t minRbdcBytes,
                                 uint32_t rbdcBytes, uint32_t vbdcBytes, uint32_t rcCount);
  void CheckSingleUtTestResults (uint32_t bytesReq, SatFrameAllocator::SatFrameAllocReq req, bool allocated, SatSuperframeConf::ConfigType_t configType,
                                 SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer,  SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer, bool rcBasedAllocation);

};

SatFrameAllocatorTestCase::SatFrameAllocatorTestCase ()
  : TestCase ("Test satellite frame allocator, configuration 0.")
{
  m_ccLevels[0] = SatFrameAllocator::CC_LEVEL_CRA;
  m_ccLevels[1] = SatFrameAllocator::CC_LEVEL_CRA_MIN_RBDC;
  m_ccLevels[2] = SatFrameAllocator::CC_LEVEL_CRA_RBDC;
  m_ccLevels[3] = SatFrameAllocator::CC_LEVEL_CRA_RBDC_VBDC;

  m_cnoIndex = 0;

  m_cnoValues[0] = SatUtils::DbToLinear (125.0);
  m_cnoValues[1] = SatUtils::DbToLinear (35.0);
  m_cnoValues[2] = SatUtils::DbToLinear (120.0);
  m_cnoValues[3] = NAN;
  m_cnoValues[4] = SatUtils::DbToLinear (77.0);
  m_cnoValues[5] = SatUtils::DbToLinear (10.0);
  m_cnoValues[6] = SatUtils::DbToLinear (66.0);
  m_cnoValues[7] = SatUtils::DbToLinear (135.0);
  m_cnoValues[8] = NAN;
  m_cnoValues[9] = SatUtils::DbToLinear (60.0);

  std::string fileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory( "src/satellite/data") + "/dvbRcs2Waveforms.txt";
  m_waveFormConf = CreateObject<SatWaveformConf> (fileNameWithPath);
}

SatFrameAllocatorTestCase::~SatFrameAllocatorTestCase ()
{

}

void SatFrameAllocatorTestCase::InitFrame (SatSuperframeConf::ConfigType_t configType)
{
  // create first frame with two carriers and each carrier having three slots
    Ptr<SatBtuConf> btu = Create<SatBtuConf> (10e4, 0.4, 0.1);

    bool defaultWaveformInUse = false;

    if ( configType == SatSuperframeConf::CONFIG_TYPE_0 )
      {
        defaultWaveformInUse = true;
      }

    m_frameConf = Create<SatFrameConf> (10e4 * 2, MilliSeconds (125), btu, m_waveFormConf, false, defaultWaveformInUse );
    m_frameAllocator = Create<SatFrameAllocator> (m_frameConf, 0, configType);
}

void
SatFrameAllocatorTestCase::RunSingleUtTest (SatSuperframeConf::ConfigType_t configType, bool acmEnabled)
{
  m_waveFormConf->SetAttribute ("AcmEnabled", BooleanValue (acmEnabled) );

  InitFrame (configType);

  // Test with one UT
  for ( uint32_t i = 0; i < 10; i++) // CRAs
    {
      for ( uint32_t j = 0; j < 15; j++) // minimum RBDCs
        {
          for ( uint32_t k = 0; k < 15; k++) // RBDCs
            {
              for ( uint32_t l = 0; l < 15; l++) // VBDCs
                {
                  for ( uint32_t m = 0; m < 9; m++) // RC indices
                    {
                      uint32_t bytesReq = 0;

                      SatFrameAllocator::SatFrameAllocReq req = ContructRequestForSinleUtTest (bytesReq, i, j, k, l, m + 1 );

                      // repeat with all CC levels
                      for (uint32_t o = 0; o < m_ccLevelCount; o++ )
                        {
                          // reset first allocations
                          m_frameAllocator->Reset ();

                          uint32_t waveformId = m_waveFormConf->GetDefaultWaveformId ();
                          m_frameAllocator->GetBestWaveform (req.m_cno, waveformId);

                          // do allocation and check that result is what expected
                          bool allocationResult = m_frameAllocator->Allocate (m_ccLevels[o], &req, waveformId );

                          m_frameAllocator->PreAllocateSymbols (0.9, false);

                          // generate time slots and check results against request, RC based allocation off

                          Ptr<SatTbtpMessage> tptp = CreateObject<SatTbtpMessage> ();
                          SatFrameAllocator::TbtpMsgContainer_t tbtpContainer;
                          tbtpContainer.push_back (tptp);
                          SatFrameAllocator::UtAllocInfoContainer_t utAllocContainer;

                          m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, false, TracedCallback<uint32_t> (), TracedCallback<uint32_t, long> ());

                          CheckSingleUtTestResults (bytesReq, req, allocationResult, configType, tbtpContainer, utAllocContainer, false);

                          // generate time slots and check results against request, RC based allocation on

                          tbtpContainer.clear ();
                          tptp = CreateObject<SatTbtpMessage> ();
                          tbtpContainer.push_back (tptp);
                          utAllocContainer.clear ();

                          m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, true, TracedCallback<uint32_t> (), TracedCallback<uint32_t, long> ());

                          CheckSingleUtTestResults (bytesReq, req, allocationResult, configType, tbtpContainer, utAllocContainer, true);
                        }
                    }
                }
            }
        }
    }

  Simulator::Destroy ();
}

void
SatFrameAllocatorTestCase::CheckSingleUtTestResults (uint32_t bytesReq, SatFrameAllocator::SatFrameAllocReq req, bool allocated, SatSuperframeConf::ConfigType_t configType,
                                                     SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer, SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer, bool rcBasedAllocation)
{
  uint16_t slotCount = m_frameConf->GetTimeSlotCount ();
  uint16_t carrierCount = m_frameConf->GetCarrierCount ();
  uint16_t slotsPerCarrier = slotCount / carrierCount;
  uint32_t minCarrierBytes = m_frameConf->GetCarrierMinPayloadInBytes ();

  uint32_t tbtpAllocatedBytes = 0;
  uint32_t slotsAllocated = 0;

  for ( SatFrameAllocator::TbtpMsgContainer_t::const_iterator it = tbtpContainer.begin (); it != tbtpContainer.end (); it++)
    {
      SatTbtpMessage::DaTimeSlotInfoContainer_t slots = (*it)->GetDaTimeslots (req.m_address);

      for (SatTbtpMessage::DaTimeSlotInfoContainer_t::const_iterator it2 = slots.begin (); it2 != slots.end (); it2++ )
        {
          tbtpAllocatedBytes += m_frameConf->GetWaveformConf ()->GetWaveform(it2->second->GetWaveFormId ())->GetPayloadInBytes ();
        }

      slotsAllocated += slots.size ();
    }

    // check that information is identical in TBTP container and UT allocation container
  if ( slotsAllocated > 0 )
    {
      NS_TEST_ASSERT_MSG_EQ ( utAllocContainer.at (req.m_address).second, req.m_generateCtrlSlot, "Control slot generation what expected!");
      NS_ASSERT (utAllocContainer.at (req.m_address).second == req.m_generateCtrlSlot);

      uint32_t utAllocContainerBytes = 0;

      for ( uint32_t n = 0; n < utAllocContainer.at (req.m_address).first.size (); n++ )
        {
          utAllocContainerBytes += utAllocContainer.at (req.m_address).first.at (n);
        }

      //std::cout << i << " " << j << " " << k << " " << l << " " << m << " " << m_ccLevels[o] << " " << tbtpAllocatedBytes << " " << utAllocContainerBytes << " " << std::endl;
      NS_TEST_ASSERT_MSG_EQ (tbtpAllocatedBytes , utAllocContainerBytes, " TBTP bytes=" << tbtpAllocatedBytes << ", UT bytes=" << utAllocContainerBytes );
      NS_ASSERT (tbtpAllocatedBytes == utAllocContainerBytes);

      bool minBytesAllocated = (utAllocContainerBytes >= std::min<uint32_t> (bytesReq, minCarrierBytes) );

      NS_TEST_ASSERT_MSG_EQ (minBytesAllocated , true, "Minimum bytes not allocated!" );
      NS_ASSERT (minBytesAllocated == true);
    }
  else
    {
      NS_TEST_ASSERT_MSG_EQ (utAllocContainer.empty (), true ,"No allocations expected!");
      NS_ASSERT (utAllocContainer.empty () == true);
    }

  uint32_t slotsExpected = 0;

  if ( allocated )
    {
      slotsExpected = std::ceil ((double)bytesReq / (double) minCarrierBytes * (double) slotsPerCarrier);

      if (req.m_generateCtrlSlot)
        {
          slotsExpected++;
        }

      slotsExpected = std::min<uint32_t> (slotsExpected, slotsPerCarrier);
    }
  //std::cout << i << " " << j << " " << k << " " << l << " " << m <<  " "<< m_ccLevels[o] << " " << slotsExpected << "=" << slotsAllocated << " " << bytesReq << " " << m_frameConf->GetCarrierMinPayloadInBytes () << std::endl;

  // check that slots are what expected
    // in other configuration than 0 or when RC based allocation is enabled,
    // it can be just checked that some slots allocated (if request is not zero)
  if ( ( configType == SatSuperframeConf::CONFIG_TYPE_0 ) && ( rcBasedAllocation == false ) )
    {
      NS_TEST_ASSERT_MSG_EQ (slotsAllocated, slotsExpected, " slots allocated= " << slotsAllocated  << ", slots expected= " <<slotsExpected );
    }
  else
    {
      if ( slotsExpected > 0 )
        {
          NS_TEST_ASSERT_MSG_GT (slotsAllocated, 0, " slots allocated= " << slotsAllocated  << ", at least one expected." );
        }
      else
        {
          NS_TEST_ASSERT_MSG_EQ (slotsAllocated, slotsExpected, " slots allocated= " << slotsAllocated  << ", slots expected= " <<slotsExpected );
        }
    }
}

void
SatFrameAllocatorTestCase::RunMultiUtTest (SatSuperframeConf::ConfigType_t configType, bool acmEnabled)
{
  // multi UT test case to be implemented
}

SatFrameAllocator::SatFrameAllocReq
SatFrameAllocatorTestCase::ContructRequestForSinleUtTest ( uint32_t& totalBytes, uint32_t craBytes, uint32_t minRbdcBytes,
                                                           uint32_t rbdcBytes, uint32_t vbdcBytes, uint32_t rcCount)
{
  SatFrameAllocator::SatFrameAllocReqItemContainer_t rcReqs = SatFrameAllocator::SatFrameAllocReqItemContainer_t (rcCount, SatFrameAllocator::SatFrameAllocReqItem ());

  SatFrameAllocator::SatFrameAllocReq req;
  req = SatFrameAllocator::SatFrameAllocReq (rcReqs);
  req.m_address = Mac48Address::Allocate ();
  req.m_generateCtrlSlot = (bool) std::rand() % 2;

  m_cnoIndex++;

  if ( m_cnoIndex >= m_cnoValueCount)
    {
      m_cnoIndex = 0;
    }

  req.m_cno = m_cnoValues[m_cnoIndex];

  uint32_t craBytesReq = 0;
  uint32_t minRbdcBytesReq = 0;
  uint32_t rbdcBytesReq = 0;
  uint32_t vbdcBytesReq = 0;

  totalBytes = 0;

  uint32_t maxCraBytes = m_frameConf->GetCarrierMinPayloadInBytes ();
  uint32_t minCarrierBytes = maxCraBytes;

  if ( req.m_generateCtrlSlot )
    {
      maxCraBytes -= m_frameAllocator->GetMostRobustWaveform ()->GetPayloadInBytes ();
    }

  for ( uint32_t i = 0; i < rcCount; i++ )
    {
      if ( craBytesReq < maxCraBytes )
        {
          uint32_t craBytesLeft = maxCraBytes - craBytesReq;
          req.m_reqPerRc[i].m_craBytes = std::min<uint32_t> (minCarrierBytes * craBytes / 10, craBytesLeft);
        }
      else
        {
          req.m_reqPerRc[i].m_craBytes = 0;
        }

      req.m_reqPerRc[i].m_minRbdcBytes = minCarrierBytes * minRbdcBytes / 10;
      req.m_reqPerRc[i].m_rbdcBytes = minCarrierBytes * rbdcBytes / 10;
      req.m_reqPerRc[i].m_vbdcBytes = minCarrierBytes * vbdcBytes / 10;

      if ( req.m_reqPerRc[i].m_minRbdcBytes > req.m_reqPerRc[i].m_rbdcBytes)
        {
          req.m_reqPerRc[i].m_rbdcBytes = req.m_reqPerRc[i].m_minRbdcBytes;
        }

      craBytesReq += req.m_reqPerRc[i].m_craBytes;
      minRbdcBytesReq += req.m_reqPerRc[i].m_minRbdcBytes;
      rbdcBytesReq += req.m_reqPerRc[i].m_rbdcBytes;
      vbdcBytesReq += req.m_reqPerRc[i].m_vbdcBytes;
    }

  if ( rcCount > 1 )
    {
      std::random_shuffle (req.m_reqPerRc.begin (), req.m_reqPerRc.end () );
    }

  totalBytes = craBytesReq + rbdcBytesReq + vbdcBytesReq;

  return req;
}

void
SatFrameAllocatorTestCase::DoRun (void)
{
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_0, false);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_1, false);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_2, false);

  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_0, true);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_1, true);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_2, true);
}

/**
 * \ingroup satellite
 * \brief Test case to unit test Satellite Superframe Allocator and its related objects.
 *
 * This case tests that SatSuperframeAllocator work correctly.
 *
 *   1.  -
 *   2.  -
 *
 *   Expected result:
 *     -
 *
 */
class SatSuperframeAllocatorTestCase : public TestCase
{
public:
  SatSuperframeAllocatorTestCase ();
  virtual ~SatSuperframeAllocatorTestCase ();

private:
  virtual void DoRun (void);
};

SatSuperframeAllocatorTestCase::SatSuperframeAllocatorTestCase ()
  : TestCase ("Test satellite superframe and frame allocators.")
{
  // test case to be implemented
}

SatSuperframeAllocatorTestCase::~SatSuperframeAllocatorTestCase ()
{
}

void
SatSuperframeAllocatorTestCase::DoRun (void)
{
  Simulator::Destroy ();
}

/**
 * \brief Test suite for Satellite Frame Allocator unit test cases.
 */
class SatFrameAllocatorTestSuite : public TestSuite
{
public:
  SatFrameAllocatorTestSuite ();
};

SatFrameAllocatorTestSuite::SatFrameAllocatorTestSuite ()
  : TestSuite ("sat-frame-allocator-test", UNIT)
{
  AddTestCase (new SatFrameAllocatorTestCase, TestCase::EXTENSIVE);
  AddTestCase (new SatSuperframeAllocatorTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatFrameAllocatorTestSuite satFrameAllocatorTestSuite;

