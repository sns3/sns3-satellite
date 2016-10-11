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
 *  2. Reset allocator (method Reset)
 *  3. Allocate UT(s) with RCs to Allocator (method Allocate)
 *  4. When desired UTs allocated, pre-allocate symbols for the UT/RC(s). (method PreAllocateSymbols)
 *  5. Finally generate time slots for UTs (method GenerateTimeSlots) by
 *     enabling/disabling RC based allocation.
 *  6. Repeat steps 2 - 5 with different combination of UT/RCs.
 *
 *  Expected result:
 *     Time slots for UT/RC should be generated according to allocation requests and limits in frames.
 *
 *  NOTE:
 *      Control slot generation should be also enabled for some allocation requests.
 *      Test should executed with ACM enabled and disabled, FCA enabled and disabled.
 *
 */
class SatFrameAllocatorTestCase : public TestCase
{
public:
  SatFrameAllocatorTestCase ();
  virtual ~SatFrameAllocatorTestCase ();

private:
  typedef std::map<Address, std::pair<SatFrameAllocator::SatFrameAllocReq, uint32_t> > ReqInfo_t;

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
  void RunSingleUtTest (SatSuperframeConf::ConfigType_t configType, bool acmEnabled, bool fcaEnabled);
  void RunMultiUtTest ();

  SatFrameAllocator::SatFrameAllocReq   ContructRequestForUt (uint32_t& totalBytes, uint32_t craBytes, uint32_t minRbdcBytes,
                                                              uint32_t rbdcBytes, uint32_t vbdcBytes, uint32_t rcCount, bool controlSlot);

  void CheckSingleUtTestResults (uint32_t bytesReq, SatFrameAllocator::SatFrameAllocReq req, bool allocated, SatSuperframeConf::ConfigType_t configType,
                                 SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer,  SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer,
                                 bool rcBasedAllocation, bool fcaEnabled, bool acmEnabled);

  void CheckMultiUtTestResults ( SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer, ReqInfo_t& reqInfo );

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

  std::string dataPath = Singleton<SatEnvVariables>::Get ()->GetDataPath ();
  std::string fileNameWithPath = dataPath + "/dvbRcs2Waveforms.txt";
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
  bool checkSlotLimit = true;

  if ( configType == SatSuperframeConf::CONFIG_TYPE_0 )
    {
      defaultWaveformInUse = true;
    }

  if ( configType == SatSuperframeConf::CONFIG_TYPE_2 )
    {
      checkSlotLimit = false;
    }

  m_frameConf = Create<SatFrameConf> (10e4 * 2, MilliSeconds (125), btu, m_waveFormConf, false, defaultWaveformInUse, checkSlotLimit );
  m_frameAllocator = Create<SatFrameAllocator> (m_frameConf, 0, configType);
}

void
SatFrameAllocatorTestCase::RunSingleUtTest (SatSuperframeConf::ConfigType_t configType, bool acmEnabled, bool fcaEnabled)
{
  m_waveFormConf->SetAttribute ("AcmEnabled", BooleanValue (acmEnabled) );

  InitFrame (configType);

  // Test with one UT and one RC
  for ( uint32_t i = 0; i < 11; i += 2) // CRAs
    {
      for ( uint32_t j = 0; j < 15; j += 2) // minimum RBDCs
        {
          for ( uint32_t k = 0; k < 15; k += 2) // RBDCs
            {
              for ( uint32_t l = 0; l < 15; l += 2) // VBDCs
                {
                  uint32_t bytesReq = 0;

                  SatFrameAllocator::SatFrameAllocReq req = ContructRequestForUt (bytesReq, i, j, k, l, 1,(bool) std::rand () % 2 );

                  // repeat with all CC levels
                  for (uint32_t o = 0; o < m_ccLevelCount; o++ )
                    {
                      // reset first allocations
                      m_frameAllocator->Reset ();

                      uint32_t waveformId = m_waveFormConf->GetDefaultWaveformId ();
                      m_frameAllocator->GetBestWaveform (req.m_cno, waveformId);

                      // do allocation and check that result is what expected
                      bool allocationResult = m_frameAllocator->Allocate (m_ccLevels[o], &req, waveformId );

                      m_frameAllocator->PreAllocateSymbols (0.9, fcaEnabled);

                      // generate time slots and check results against request, RC based allocation off

                      Ptr<SatTbtpMessage> tptp = CreateObject<SatTbtpMessage> ();
                      SatFrameAllocator::TbtpMsgContainer_t tbtpContainer;
                      tbtpContainer.push_back (tptp);
                      SatFrameAllocator::UtAllocInfoContainer_t utAllocContainer;

                      m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, false, TracedCallback<uint32_t> (), TracedCallback<uint32_t, uint32_t> (), TracedCallback<uint32_t, double> ());

                      CheckSingleUtTestResults (bytesReq, req, allocationResult, configType, tbtpContainer, utAllocContainer, false, fcaEnabled, acmEnabled);

                      // generate time slots and check results against request, RC based allocation on

                      tbtpContainer.clear ();
                      tptp = CreateObject<SatTbtpMessage> ();
                      tbtpContainer.push_back (tptp);
                      utAllocContainer.clear ();

                      m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, true, TracedCallback<uint32_t> (), TracedCallback<uint32_t, uint32_t> (), TracedCallback<uint32_t, double> ());

                      CheckSingleUtTestResults (bytesReq, req, allocationResult, configType, tbtpContainer, utAllocContainer, true, fcaEnabled, acmEnabled);
                    }
                }
            }
        }
    }

  // test several times with random values using RC indices 2, 3, 4
  for ( uint32_t i = 0; i < 1000; i++)
    {
      for ( uint32_t m = 2; m < 4; m++) //
        {
          uint32_t bytesReq = 0;
          uint32_t divider = m_frameConf->GetCarrierMinPayloadInBytes () * 2;

          SatFrameAllocator::SatFrameAllocReq req = ContructRequestForUt (bytesReq, std::rand () % divider, std::rand () % divider, std::rand () % divider, std::rand () % divider, 2, (bool) std::rand () % 2 );

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

              m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, false, TracedCallback<uint32_t> (), TracedCallback<uint32_t, uint32_t> (), TracedCallback<uint32_t, double> ());

              CheckSingleUtTestResults (bytesReq, req, allocationResult, configType, tbtpContainer, utAllocContainer, false, fcaEnabled, acmEnabled);

              // generate time slots and check results against request, RC based allocation on

              tbtpContainer.clear ();
              tptp = CreateObject<SatTbtpMessage> ();
              tbtpContainer.push_back (tptp);
              utAllocContainer.clear ();

              m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, true, TracedCallback<uint32_t> (), TracedCallback<uint32_t, uint32_t> (), TracedCallback<uint32_t, double> () );

              CheckSingleUtTestResults (bytesReq, req, allocationResult, configType, tbtpContainer, utAllocContainer, true, fcaEnabled, acmEnabled);
            }
        }
    }

  Simulator::Destroy ();
}

void
SatFrameAllocatorTestCase::CheckSingleUtTestResults (uint32_t bytesReq, SatFrameAllocator::SatFrameAllocReq req, bool allocated, SatSuperframeConf::ConfigType_t configType, SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer,
                                                     SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer, bool rcBasedAllocation, bool fcaEnabled, bool acmEnabled)
{
  uint16_t slotCount = m_frameConf->GetTimeSlotCount ();
  uint16_t carrierCount = m_frameConf->GetCarrierCount ();
  uint16_t slotsPerCarrier = slotCount / carrierCount;
  uint32_t minCarrierBytes = m_frameConf->GetCarrierMinPayloadInBytes ();

  uint32_t tbtpAllocatedBytes = 0;
  uint32_t slotsAllocated = 0;

  for ( SatFrameAllocator::TbtpMsgContainer_t::const_iterator it = tbtpContainer.begin (); it != tbtpContainer.end (); it++)
    {
      SatTbtpMessage::DaTimeSlotInfoItem_t info = (*it)->GetDaTimeslots (req.m_address);

      for (SatTbtpMessage::DaTimeSlotConfContainer_t::const_iterator it2 = info.second.begin (); it2 != info.second.end (); it2++ )
        {
          tbtpAllocatedBytes += m_frameConf->GetWaveformConf ()->GetWaveform ((*it2)->GetWaveFormId ())->GetPayloadInBytes ();
        }

      slotsAllocated += info.second.size ();
    }

  // check that information is identical in TBTP container and UT allocation container
  if ( slotsAllocated > 0 )
    {
      NS_TEST_ASSERT_MSG_EQ ( utAllocContainer.at (req.m_address).second, req.m_generateCtrlSlot, "Control slot generation what expected!");
      NS_ASSERT (utAllocContainer.at (req.m_address).second == req.m_generateCtrlSlot);

      uint32_t utAllocContainerBytes = 0;

      for ( uint32_t i = 0; i < utAllocContainer.at (req.m_address).first.size (); i++ )
        {
          utAllocContainerBytes += utAllocContainer.at (req.m_address).first.at (i);
        }

      //std::cout << i << " " << j << " " << k << " " << l << " " << m << " " << m_ccLevels[o] << " " << tbtpAllocatedBytes << " " << utAllocContainerBytes << " " << std::endl;
      NS_TEST_ASSERT_MSG_EQ (tbtpAllocatedBytes, utAllocContainerBytes, " TBTP bytes=" << tbtpAllocatedBytes << ", UT bytes=" << utAllocContainerBytes );
      NS_ASSERT (tbtpAllocatedBytes == utAllocContainerBytes);

      bool minBytesAllocated = (utAllocContainerBytes >= std::min<uint32_t> (bytesReq, minCarrierBytes) );

      NS_TEST_ASSERT_MSG_EQ (minBytesAllocated, true, "Minimum bytes not allocated!" );
      NS_ASSERT (minBytesAllocated == true);
    }
  else
    {
      NS_TEST_ASSERT_MSG_EQ (utAllocContainer.empty (), true,"No allocations expected!");
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

      if ( fcaEnabled )
        {
          bool onlyCraRequested = true;

          for (uint32_t i = 0; i < req.m_reqPerRc.size (); i++)
            {
              if ((req.m_reqPerRc.at (i).m_rbdcBytes > 0) || (req.m_reqPerRc.at (i).m_vbdcBytes > 0))
                {
                  onlyCraRequested = false;
                }
            }

          if ( (( configType == SatSuperframeConf::CONFIG_TYPE_0 ) || (acmEnabled == false)) && (onlyCraRequested == false) )
            {
              slotsExpected = slotsPerCarrier;
            }
        }
    }
  //std::cout << i << " " << j << " " << k << " " << l << " " << m <<  " "<< m_ccLevels[o] << " " << slotsExpected << "=" << slotsAllocated << " " << bytesReq << " " << m_frameConf->GetCarrierMinPayloadInBytes () << std::endl;

  // check that slots are what expected
  // in other configuration than 0 or when RC based allocation is enabled,
  // it can be just checked that some slots are allocated (if request is not zero)

  if ( ( configType == SatSuperframeConf::CONFIG_TYPE_0 ) && ( rcBasedAllocation == false ) )
    {
      NS_TEST_ASSERT_MSG_EQ (slotsAllocated, slotsExpected, " slots allocated= " << slotsAllocated  << ", slots expected= " << slotsExpected );
    }
  else
    {
      if ( slotsExpected > 0 )
        {
          NS_TEST_ASSERT_MSG_GT (slotsAllocated, 0, " slots allocated= " << slotsAllocated  << ", at least one expected." );
        }
      else
        {
          NS_TEST_ASSERT_MSG_EQ (slotsAllocated, slotsExpected, " slots allocated= " << slotsAllocated  << ", slots expected= " << slotsExpected );
        }
    }
}

void
SatFrameAllocatorTestCase::RunMultiUtTest ()
{
  m_waveFormConf->SetAttribute ("AcmEnabled", BooleanValue (false) );

  // test with configuration type 0
  InitFrame (SatSuperframeConf::CONFIG_TYPE_0);

  uint32_t utBytesReq[6];
  SatFrameAllocator::SatFrameAllocReq req[6];

  req[0] = ContructRequestForUt (utBytesReq[0], 4, 1, 6, 0, 1, false);
  req[1] = ContructRequestForUt (utBytesReq[1], 4, 0, 0, 6, 1, false );
  req[2] = ContructRequestForUt (utBytesReq[2], 4, 1, 3, 3, 1, false );
  req[3] = ContructRequestForUt (utBytesReq[3], 1, 0, 1, 0, 1, false);
  req[4] = ContructRequestForUt (utBytesReq[4], 2, 0, 0, 1, 1, false );
  req[5] = ContructRequestForUt (utBytesReq[5], 1, 1, 1, 1, 1, false );

  uint32_t waveformId[6];

  for (uint32_t i = 0; i < 6; i++ )
    {
      waveformId[i] = m_waveFormConf->GetDefaultWaveformId ();
      m_frameAllocator->GetBestWaveform (req[i].m_cno, waveformId[i]);
    }

  bool allocationResult;

  // test with two UTs
  for (uint32_t n = 0; n < 5; n++ )
    {
      for (uint32_t m = n + 1; m < 6; m++ )
        {
          // repeat case all CC levels
          for (uint32_t o = 0; o < m_ccLevelCount; o++ )
            {
              m_frameAllocator->Reset ();

              allocationResult = m_frameAllocator->Allocate (m_ccLevels[o], &req[n], waveformId[n]);
              NS_TEST_ASSERT_MSG_EQ (true, allocationResult, "Allocation failed!" );

              allocationResult = m_frameAllocator->Allocate (m_ccLevels[o], &req[m], waveformId[m]);
              NS_TEST_ASSERT_MSG_EQ (true, allocationResult, "Allocation failed!" );

              m_frameAllocator->PreAllocateSymbols (1.0, false);

              Ptr<SatTbtpMessage> tptp = CreateObject<SatTbtpMessage> ();
              SatFrameAllocator::TbtpMsgContainer_t tbtpContainer;
              tbtpContainer.push_back (tptp);
              SatFrameAllocator::UtAllocInfoContainer_t utAllocContainer;

              m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, false, TracedCallback<uint32_t> (), TracedCallback<uint32_t, uint32_t> (), TracedCallback<uint32_t, double> ());

              ReqInfo_t reqInfo;
              reqInfo.insert (std::make_pair ( req[n].m_address, std::make_pair (req[n], utBytesReq[n])) );
              reqInfo.insert (std::make_pair ( req[m].m_address, std::make_pair (req[m], utBytesReq[m])) );

              CheckMultiUtTestResults (utAllocContainer, reqInfo);
            }
        }
    }

  // test with three UTs
  for (uint32_t n = 0; n < 4; n++ )
    {
      for (uint32_t m = n + 1; m < 5; m++ )
        {
          for (uint32_t o = m + 2; o < 6; o++ )
            {
              m_frameAllocator->Reset ();

              allocationResult = m_frameAllocator->Allocate (SatFrameAllocator::CC_LEVEL_CRA, &req[n], waveformId[n]);
              NS_TEST_ASSERT_MSG_EQ (true, allocationResult, "Allocation failed!" );

              allocationResult = m_frameAllocator->Allocate (SatFrameAllocator::CC_LEVEL_CRA, &req[m], waveformId[m]);
              NS_TEST_ASSERT_MSG_EQ (true, allocationResult, "Allocation failed!" );

              allocationResult = m_frameAllocator->Allocate (SatFrameAllocator::CC_LEVEL_CRA, &req[o], waveformId[o]);
              NS_TEST_ASSERT_MSG_EQ (true, allocationResult, "Allocation failed!" );

              m_frameAllocator->PreAllocateSymbols (1.0, false);

              Ptr<SatTbtpMessage> tptp = CreateObject<SatTbtpMessage> ();
              SatFrameAllocator::TbtpMsgContainer_t tbtpContainer;
              tbtpContainer.push_back (tptp);
              SatFrameAllocator::UtAllocInfoContainer_t utAllocContainer;

              m_frameAllocator->GenerateTimeSlots (tbtpContainer, 1000, utAllocContainer, false, TracedCallback<uint32_t> (), TracedCallback<uint32_t, uint32_t> (), TracedCallback<uint32_t, double> ());

              ReqInfo_t reqInfo;
              reqInfo.insert (std::make_pair ( req[n].m_address, std::make_pair (req[n], utBytesReq[n])) );
              reqInfo.insert (std::make_pair ( req[m].m_address, std::make_pair (req[m], utBytesReq[m])) );
              reqInfo.insert (std::make_pair ( req[m].m_address, std::make_pair (req[o], utBytesReq[o])) );

              CheckMultiUtTestResults (utAllocContainer, reqInfo);
            }
        }
    }

}

void
SatFrameAllocatorTestCase::CheckMultiUtTestResults ( SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer, ReqInfo_t& reqInfo )
{
  uint16_t carrierCount = m_frameConf->GetCarrierCount ();
  uint32_t minCarrierBytes = m_frameConf->GetCarrierMinPayloadInBytes ();

  for (SatFrameAllocator::UtAllocInfoContainer_t::const_iterator it = utAllocContainer.begin (); it != utAllocContainer.end (); it++)
    {
      uint32_t bytesAllocated = 0;

      for ( std::vector<uint32_t>::const_iterator it2 = it->second.first.begin (); it2 != it->second.first.end (); it2++)
        {
          bytesAllocated += *it2;
        }

      // check results can be easily checked by general way
      if (utAllocContainer.size () > carrierCount)
        {
          NS_TEST_ASSERT_MSG_GT (bytesAllocated, 0, "Allocation not what expected!" );
        }
      else if (reqInfo.at (it->first).second == minCarrierBytes )
        {
          NS_TEST_ASSERT_MSG_EQ (minCarrierBytes, bytesAllocated, "Allocation not what expected!" );
        }
      else
        {
          NS_TEST_ASSERT_MSG_NE (minCarrierBytes, bytesAllocated, "Allocation not what expected!" );
        }
    }
}

SatFrameAllocator::SatFrameAllocReq
SatFrameAllocatorTestCase::ContructRequestForUt ( uint32_t& totalBytes, uint32_t craBytes, uint32_t minRbdcBytes,
                                                  uint32_t rbdcBytes, uint32_t vbdcBytes, uint32_t rcCount, bool controlSlot)
{
  SatFrameAllocator::SatFrameAllocReqItemContainer_t rcReqs = SatFrameAllocator::SatFrameAllocReqItemContainer_t (rcCount, SatFrameAllocator::SatFrameAllocReqItem ());

  SatFrameAllocator::SatFrameAllocReq req;
  req = SatFrameAllocator::SatFrameAllocReq (rcReqs);
  req.m_address = Mac48Address::Allocate ();
  req.m_generateCtrlSlot = controlSlot;

  m_cnoIndex++;

  if ( m_cnoIndex >= m_cnoValueCount)
    {
      m_cnoIndex = 0;
    }

  req.m_cno = m_cnoValues[m_cnoIndex];

  uint32_t craBytesReq = 0;
  totalBytes = 0;

  uint32_t maxCraBytes = m_frameConf->GetCarrierMinPayloadInBytes ();
  uint32_t minCarrierBytes = maxCraBytes;

  if ( req.m_generateCtrlSlot )
    {
      maxCraBytes -= m_frameAllocator->GetMostRobustWaveform ()->GetPayloadInBytes ();
    }

  if ( craBytesReq < maxCraBytes )
    {
      uint32_t craBytesLeft = maxCraBytes - craBytesReq;
      req.m_reqPerRc[0].m_craBytes = std::min<uint32_t> (minCarrierBytes * craBytes / 10, craBytesLeft);
    }
  else
    {
      req.m_reqPerRc[0].m_craBytes = 0;
    }

  req.m_reqPerRc[0].m_minRbdcBytes = minCarrierBytes * minRbdcBytes / 10;
  req.m_reqPerRc[0].m_rbdcBytes = minCarrierBytes * rbdcBytes / 10;
  req.m_reqPerRc[0].m_vbdcBytes = minCarrierBytes * vbdcBytes / 10;

  if ( req.m_reqPerRc[0].m_minRbdcBytes > req.m_reqPerRc[0].m_rbdcBytes)
    {
      req.m_reqPerRc[0].m_rbdcBytes = req.m_reqPerRc[0].m_minRbdcBytes;
    }

  craBytesReq += req.m_reqPerRc[0].m_craBytes;

  totalBytes += req.m_reqPerRc[0].m_craBytes;
  totalBytes += req.m_reqPerRc[0].m_rbdcBytes;
  totalBytes += req.m_reqPerRc[0].m_vbdcBytes;

  uint32_t divider = minCarrierBytes + 1;

  // if more than one RC is given, then random request value is set for the RC 1 to RC n
  for ( uint32_t i = 1; i < rcCount; i++ )
    {
      if ( craBytesReq < maxCraBytes )
        {
          uint32_t craBytesLeft = maxCraBytes - craBytesReq;
          req.m_reqPerRc[i].m_craBytes = std::min<uint32_t> (std::rand () % divider, craBytesLeft);
        }
      else
        {
          req.m_reqPerRc[i].m_craBytes = 0;
        }

      req.m_reqPerRc[i].m_minRbdcBytes = std::rand () % divider;
      req.m_reqPerRc[i].m_rbdcBytes = std::rand () % divider;
      req.m_reqPerRc[i].m_vbdcBytes = std::rand () % divider;

      if ( req.m_reqPerRc[i].m_minRbdcBytes > req.m_reqPerRc[i].m_rbdcBytes)
        {
          req.m_reqPerRc[i].m_rbdcBytes = req.m_reqPerRc[i].m_minRbdcBytes;
        }

      craBytesReq += req.m_reqPerRc[i].m_craBytes;
      totalBytes += req.m_reqPerRc[i].m_craBytes;
      totalBytes += req.m_reqPerRc[i].m_rbdcBytes;
      totalBytes += req.m_reqPerRc[i].m_vbdcBytes;
    }

  return req;
}

void
SatFrameAllocatorTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("sat-frame-allocator", "", true);

  // test single UT with all configuration types, ACM and FCA disabled
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_0, false, false);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_1, false, false);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_2, false, false);

  // test single UT with all configuration types, ACM disabled and FCA enabled
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_0, false, true);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_1, false, true);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_2, false, true);

  // test single UT with all configuration types, ACM enabled and FCA disabled
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_0, true, false);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_1, true, false);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_2, true, false);

  // test single UT with all configuration types, ACM enabled and FCA enabled
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_0, true, true);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_1, true, true);
  RunSingleUtTest (SatSuperframeConf::CONFIG_TYPE_2, true, true);

  // test with two and three UTs
  RunMultiUtTest ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
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
  AddTestCase (new SatFrameAllocatorTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatFrameAllocatorTestSuite satFrameAllocatorTestSuite;

