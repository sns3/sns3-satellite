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

#include <algorithm>
#include <limits>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "satellite-utils.h"
#include "satellite-frame-allocator.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameAllocator");

namespace ns3 {

// helper classes defined inside SatFrameAllocator

SatFrameAllocator::SatFrameAllocInfo::SatFrameAllocInfo ()
  : m_ctrlSlotPresent (false),
    m_craSymbols (0.0),
    m_minRbdcSymbols (0.0),
    m_rbdcSymbols (0.0),
    m_vbdcSymbols (0.0)
{
  NS_LOG_FUNCTION (this);
}

SatFrameAllocator::SatFrameAllocInfo::SatFrameAllocInfo (uint8_t countOfRcs)
  : m_ctrlSlotPresent (false),
    m_craSymbols (0.0),
    m_minRbdcSymbols (0.0),
    m_rbdcSymbols (0.0),
    m_vbdcSymbols (0.0)
{
  NS_LOG_FUNCTION (this << (uint32_t) countOfRcs);

  m_allocInfoPerRc = SatFrameAllocInfoItemContainer_t (countOfRcs, SatFrameAllocInfoItem ());
}

SatFrameAllocator::SatFrameAllocInfo::SatFrameAllocInfo (SatFrameAllocReqItemContainer_t &req, Ptr<SatWaveform> trcWaveForm,
                                                         bool ctrlSlotPresent, double ctrlSlotLength)
  : m_ctrlSlotPresent (ctrlSlotPresent),
    m_craSymbols (0.0),
    m_minRbdcSymbols (0.0),
    m_rbdcSymbols (0.0),
    m_vbdcSymbols (0.0)
{
  NS_LOG_FUNCTION (this << ctrlSlotPresent << ctrlSlotLength);

  for (SatFrameAllocReqItemContainer_t::const_iterator it = req.begin (); it != req.end (); it++ )
    {
      SatFrameAllocInfoItem  reqInSymbols;

      reqInSymbols.m_craSymbols  = trcWaveForm->GetBurstLengthInSymbols () * it->m_craBytes / trcWaveForm->GetPayloadInBytes ();
      reqInSymbols.m_minRbdcSymbols = trcWaveForm->GetBurstLengthInSymbols () * it->m_minRbdcBytes / trcWaveForm->GetPayloadInBytes ();
      reqInSymbols.m_rbdcSymbols = trcWaveForm->GetBurstLengthInSymbols () * it->m_rbdcBytes / trcWaveForm->GetPayloadInBytes ();
      reqInSymbols.m_vbdcSymbols = trcWaveForm->GetBurstLengthInSymbols () * it->m_vbdcBytes / trcWaveForm->GetPayloadInBytes ();

      // if control slot should be allocated and RC index is 0
      // add symbols needed for control slot to CRA symbols
      if ( m_ctrlSlotPresent && (it == req.begin ()) )
        {
          reqInSymbols.m_craSymbols += ctrlSlotLength;
        }

      m_craSymbols += reqInSymbols.m_craSymbols;
      m_minRbdcSymbols += reqInSymbols.m_minRbdcSymbols;
      m_rbdcSymbols += reqInSymbols.m_rbdcSymbols;
      m_vbdcSymbols += reqInSymbols.m_vbdcSymbols;

      m_allocInfoPerRc.push_back (reqInSymbols);
    }
}

SatFrameAllocator::SatFrameAllocInfoItem
SatFrameAllocator::SatFrameAllocInfo::UpdateTotalCounts ()
{
  NS_LOG_FUNCTION (this);

  m_craSymbols = 0.0;
  m_minRbdcSymbols = 0.0;
  m_rbdcSymbols = 0.0;
  m_vbdcSymbols = 0.0;

  SatFrameAllocInfoItem totalReqs;

  for (SatFrameAllocInfoItemContainer_t::const_iterator it = m_allocInfoPerRc.begin (); it != m_allocInfoPerRc.end (); it++ )
    {
      SatFrameAllocInfoItem  reqInSymbols;

      m_craSymbols += it->m_craSymbols;
      m_minRbdcSymbols += it->m_minRbdcSymbols;
      m_rbdcSymbols += it->m_rbdcSymbols;
      m_vbdcSymbols += it->m_vbdcSymbols;
    }

  totalReqs.m_craSymbols = m_craSymbols;
  totalReqs.m_minRbdcSymbols = m_minRbdcSymbols;
  totalReqs.m_rbdcSymbols = m_rbdcSymbols;
  totalReqs.m_vbdcSymbols = m_vbdcSymbols;

  return totalReqs;
}

double
SatFrameAllocator::SatFrameAllocInfo::GetTotalSymbols ()
{
  return (m_craSymbols + m_rbdcSymbols + m_vbdcSymbols);
}

SatFrameAllocator::CcReqCompare::CcReqCompare (const UtAllocContainer_t& utAllocContainer, CcReqCompare::CcReqType_t ccReqType)
  : m_utAllocContainer (utAllocContainer),
    m_ccReqType (ccReqType)
{

}

bool
SatFrameAllocator::CcReqCompare::operator() (RcAllocItem_t rcAlloc1, RcAllocItem_t rcAlloc2)
{
  bool result = false;

  switch (m_ccReqType)
    {
    case CC_TYPE_MIN_RBDC:
      result = ( m_utAllocContainer.at (rcAlloc1.first).m_request.m_allocInfoPerRc[rcAlloc1.second].m_minRbdcSymbols <
                 m_utAllocContainer.at (rcAlloc2.first).m_request.m_allocInfoPerRc[rcAlloc2.second].m_minRbdcSymbols );
      break;

    case CC_TYPE_RBDC:
      result = ( m_utAllocContainer.at (rcAlloc1.first).m_request.m_allocInfoPerRc[rcAlloc1.second].m_rbdcSymbols <
                 m_utAllocContainer.at (rcAlloc2.first).m_request.m_allocInfoPerRc[rcAlloc2.second].m_rbdcSymbols );
      break;

    case CC_TYPE_VBDC:
      result = ( m_utAllocContainer.at (rcAlloc1.first).m_request.m_allocInfoPerRc[rcAlloc1.second].m_vbdcSymbols <
                 m_utAllocContainer.at (rcAlloc2.first).m_request.m_allocInfoPerRc[rcAlloc2.second].m_vbdcSymbols );
      break;

    default:
      NS_FATAL_ERROR ("Invalid CC type!!!");
      break;
    }

  return result;
}

// SatFrameAllocator

SatFrameAllocator::SatFrameAllocator ()
  : m_allocationDenied (true),
    m_totalSymbolsInFrame (0.0),
    m_availableSymbolsInFrame (0.0),
    m_preAllocatedCraSymbols (0.0),
    m_preAllocatedMinRdbcSymbols (0.0),
    m_preAllocatedRdbcSymbols (0.0),
    m_preAllocatedVdbcSymbols (0.0),
    m_maxSymbolsPerCarrier (0),
    m_configType (SatSuperframeConf::CONFIG_TYPE_0),
    m_frameId (0)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor not supported!!!");
}

SatFrameAllocator::SatFrameAllocator (Ptr<SatFrameConf> frameConf, uint8_t frameId, SatSuperframeConf::ConfigType_t configType)
  : m_allocationDenied (true),
    m_configType (configType),
    m_frameId (frameId),
    m_frameConf (frameConf)
{
  NS_LOG_FUNCTION (this << (uint32_t) frameId);

  m_waveformConf = m_frameConf->GetWaveformConf ();
  m_maxSymbolsPerCarrier = frameConf->GetCarrierMaxSymbols ();
  m_totalSymbolsInFrame = m_maxSymbolsPerCarrier * m_frameConf->GetCarrierCount ();

  switch ( m_configType )
    {
    case SatSuperframeConf::CONFIG_TYPE_0:
      {
        m_burstLenghts.push_back ( m_waveformConf->GetDefaultBurstLength ());
        m_mostRobustWaveform = m_waveformConf->GetWaveform (m_waveformConf->GetDefaultWaveformId ());
        break;
      }

    case SatSuperframeConf::CONFIG_TYPE_1:
      {
        m_burstLenghts.push_back ( m_waveformConf->GetDefaultBurstLength ());

        uint32_t mostRobustWaveformId = 0;

        if ( m_waveformConf->GetMostRobustWaveformId (mostRobustWaveformId, m_waveformConf->GetDefaultBurstLength ()) )
          {
            m_mostRobustWaveform = m_waveformConf->GetWaveform (mostRobustWaveformId);
          }
        else
          {
            NS_FATAL_ERROR ("Most robust waveform not found, error in waveform configuration ???");
          }
        break;
      }

    case SatSuperframeConf::CONFIG_TYPE_2:
      {
        if ( frameConf->GetWaveformConf ()->IsAcmEnabled () )
          {
            m_burstLenghts = frameConf->GetWaveformConf ()->GetSupportedBurstLengths ();
          }
        else
          {
            m_burstLenghts.push_back ( m_waveformConf->GetDefaultBurstLength ());
          }

        uint32_t mostRobustWaveformId = 0;

        if ( m_waveformConf->GetMostRobustWaveformId (mostRobustWaveformId, SatWaveformConf::SHORT_BURST_LENGTH) )
          {
            m_mostRobustWaveform = m_waveformConf->GetWaveform (mostRobustWaveformId);
          }
        else
          {
            NS_FATAL_ERROR ("Most robust waveform not found, error in waveform configuration ???");
          }
        break;
      }

    default:
      NS_FATAL_ERROR ("Not supported configuration type");
      break;
    }

  Reset ();
}

void
SatFrameAllocator::Reset ()
{
  NS_LOG_FUNCTION (this);

  m_availableSymbolsInFrame = m_totalSymbolsInFrame;
  m_preAllocatedCraSymbols = 0;
  m_preAllocatedMinRdbcSymbols = 0;
  m_preAllocatedRdbcSymbols = 0;
  m_preAllocatedVdbcSymbols = 0;

  m_utAllocs.clear ();
  m_rcAllocs.clear ();

  m_allocationDenied = false;
}

double
SatFrameAllocator::GetCcLoad (CcLevel_t ccLevel)
{
  NS_LOG_FUNCTION (this << ccLevel);

  double load = NAN;

  switch (ccLevel)
    {
    case CC_LEVEL_CRA:
      load = m_preAllocatedCraSymbols - m_totalSymbolsInFrame;
      break;

    case CC_LEVEL_CRA_MIN_RBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedMinRdbcSymbols) - m_totalSymbolsInFrame;
      break;

    case CC_LEVEL_CRA_RBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols) - m_totalSymbolsInFrame;
      break;

    case CC_LEVEL_CRA_RBDC_VBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols + m_preAllocatedVdbcSymbols) - m_totalSymbolsInFrame;
      break;

    default:
      NS_FATAL_ERROR ("Not supported CC level!!!");
      break;
    }

  return load;
}

bool
SatFrameAllocator::GetBestWaveform (double cno, uint32_t & waveFormId) const
{
  NS_LOG_FUNCTION (this << cno << waveFormId);

  bool cnoSupported = false;

  switch ( m_configType )
    {
    case SatSuperframeConf::CONFIG_TYPE_0:
      cnoSupported = true;
      waveFormId = m_waveformConf->GetDefaultWaveformId ();
      break;

    case SatSuperframeConf::CONFIG_TYPE_1:
      cnoSupported = m_waveformConf->GetBestWaveformId ( cno, m_frameConf->GetBtuConf ()->GetSymbolRateInBauds (), waveFormId, m_waveformConf->GetDefaultBurstLength ());
      break;

    case SatSuperframeConf::CONFIG_TYPE_2:
      cnoSupported = m_waveformConf->GetBestWaveformId ( cno, m_frameConf->GetBtuConf ()->GetSymbolRateInBauds (), waveFormId, SatWaveformConf::SHORT_BURST_LENGTH);
      break;

    default:
      NS_FATAL_ERROR ("Not supported configuration type");
      break;
    }

  return cnoSupported;
}

bool
SatFrameAllocator::Allocate (CcLevel_t ccLevel, SatFrameAllocReq * allocReq, uint32_t waveFormId)
{
  NS_LOG_FUNCTION (this << ccLevel << waveFormId);

  bool allocated = false;

  if (!m_allocationDenied)
    {
      // convert request in bytes to symbols based on given waveform
      SatFrameAllocator::SatFrameAllocInfo reqInSymbols = SatFrameAllocInfo (allocReq->m_reqPerRc, m_waveformConf->GetWaveform (waveFormId),
                                                                             allocReq->m_generateCtrlSlot, m_mostRobustWaveform->GetBurstLengthInSymbols () );
      if ( reqInSymbols.m_minRbdcSymbols > reqInSymbols.m_rbdcSymbols )
        {
          NS_FATAL_ERROR ("Min RBDC bytes is greater than RBDC bytes!!!");
        }

      switch (ccLevel)
        {
        case CC_LEVEL_CRA:
          {
            m_preAllocatedCraSymbols += reqInSymbols.m_craSymbols;

            if ( (reqInSymbols.m_craSymbols) <= m_maxSymbolsPerCarrier )
              {
                double symbolsLeftInCarrier = m_maxSymbolsPerCarrier - reqInSymbols.m_craSymbols;

                m_preAllocatedMinRdbcSymbols += std::min<double> ( reqInSymbols.m_minRbdcSymbols, symbolsLeftInCarrier);
                m_preAllocatedRdbcSymbols += std::min<double> ( reqInSymbols.m_rbdcSymbols, symbolsLeftInCarrier);

                if ( m_maxSymbolsPerCarrier >= (reqInSymbols.m_craSymbols + reqInSymbols.m_rbdcSymbols))
                  {
                    double vbdcSymbolsInCarrier = m_maxSymbolsPerCarrier - reqInSymbols.m_craSymbols - reqInSymbols.m_rbdcSymbols;
                    m_preAllocatedVdbcSymbols += std::min<double> (reqInSymbols.m_vbdcSymbols, vbdcSymbolsInCarrier);
                  }

                allocated = true;
              }
            else
              {
                NS_FATAL_ERROR ("CRA symbols exceeds carrier limit!!!");
              }

            allocated = true;
          }
          break;

        case CC_LEVEL_CRA_MIN_RBDC:
          {
            double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedMinRdbcSymbols;
            double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

            if ( symbolsToUse >= (reqInSymbols.m_craSymbols + reqInSymbols.m_minRbdcSymbols))
              {
                m_preAllocatedCraSymbols += reqInSymbols.m_craSymbols;
                m_preAllocatedMinRdbcSymbols += reqInSymbols.m_minRbdcSymbols;

                double symbolsLeftInCarrier = m_maxSymbolsPerCarrier - reqInSymbols.m_craSymbols - reqInSymbols.m_minRbdcSymbols;

                m_preAllocatedRdbcSymbols += std::min<double> (reqInSymbols.m_rbdcSymbols, symbolsLeftInCarrier + reqInSymbols.m_minRbdcSymbols);

                if (symbolsToUse >= (reqInSymbols.m_craSymbols + reqInSymbols.m_rbdcSymbols))
                  {
                    double vbdcSymbolsInCarrier = m_maxSymbolsPerCarrier - reqInSymbols.m_craSymbols - reqInSymbols.m_rbdcSymbols;
                    m_preAllocatedVdbcSymbols += std::min<double> (reqInSymbols.m_vbdcSymbols, vbdcSymbolsInCarrier);
                  }

                allocated = true;
              }
          }
          break;

        case CC_LEVEL_CRA_RBDC:
          {
            double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols;
            double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

            if ( symbolsToUse >= (reqInSymbols.m_craSymbols + reqInSymbols.m_rbdcSymbols))
              {
                double symbolsLeftInCarrier = m_maxSymbolsPerCarrier - reqInSymbols.m_craSymbols - reqInSymbols.m_rbdcSymbols;

                m_preAllocatedCraSymbols += reqInSymbols.m_craSymbols;
                m_preAllocatedMinRdbcSymbols += reqInSymbols.m_minRbdcSymbols;
                m_preAllocatedRdbcSymbols += reqInSymbols.m_rbdcSymbols;
                m_preAllocatedVdbcSymbols += std::min<double> (reqInSymbols.m_vbdcSymbols, symbolsLeftInCarrier);

                allocated = true;
              }
          }
          break;

        case CC_LEVEL_CRA_RBDC_VBDC:
          {
            double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols - m_preAllocatedVdbcSymbols;
            double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

            if ( symbolsToUse >= (reqInSymbols.m_craSymbols + reqInSymbols.m_rbdcSymbols + reqInSymbols.m_vbdcSymbols))
              {
                m_preAllocatedCraSymbols += reqInSymbols.m_craSymbols;
                m_preAllocatedMinRdbcSymbols += reqInSymbols.m_minRbdcSymbols;
                m_preAllocatedRdbcSymbols += reqInSymbols.m_rbdcSymbols;
                m_preAllocatedVdbcSymbols += reqInSymbols.m_vbdcSymbols;

                allocated = true;
              }
          }
          break;

        default:
          NS_FATAL_ERROR ("Not supported CC level!!!");
          break;
        }

      if ( allocated )
        {
          // update request according to carrier limit and store allocation request
          UpdateAndStoreAllocReq (allocReq->m_address, allocReq->m_cno, reqInSymbols);
        }
    }

  return allocated;
}

void
SatFrameAllocator::PreAllocateSymbols (double targetLoad, bool fcaEnabled)
{
  NS_LOG_FUNCTION (this << targetLoad << fcaEnabled);

  if (!m_allocationDenied)
    {
      if ( (targetLoad >= 0)  && (targetLoad <= 1) )
        {
          m_availableSymbolsInFrame = targetLoad * m_totalSymbolsInFrame;
          ShareSymbols (fcaEnabled);
        }
      else
        {
          NS_FATAL_ERROR ("target load must be between 0 and 1.");
        }

      m_allocationDenied = true;
    }
}

void
SatFrameAllocator::GenerateTimeSlots (SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer, uint32_t maxSizeInBytes, UtAllocInfoContainer_t& utAllocContainer,
                                      bool rcBasedAllocationEnabled, TracedCallback<uint32_t> waveformTrace, TracedCallback<uint32_t, uint32_t> utLoadTrace, TracedCallback<uint32_t, double> loadTrace)
{
  NS_LOG_FUNCTION (this);

  // variable to watch limit for maximum allowed time slot
  uint32_t timeslotCount = 0;

  if (tbtpContainer.empty ())
    {
      NS_FATAL_ERROR ("TBTP container must contain at least one message.");
    }

  Ptr<SatTbtpMessage> tbtpToFill = tbtpContainer.back ();

  // sort UTs
  std::vector<Address> uts = SortUts ();

  // sort available carriers in the frame
  std::vector<uint16_t> carriers = SortCarriers ();

  // go through all allocated UT until there is available carriers

  std::vector<uint16_t>::const_iterator currentCarrier = carriers.begin ();
  int64_t carrierSymbolsToUse = m_maxSymbolsPerCarrier;
  uint32_t utCount = 0;
  uint32_t symbolsAllocated = 0;

  for (std::vector<Address>::iterator it = uts.begin (); (it != uts.end ()) && (currentCarrier != carriers.end ()); it++ )
    {
      // check before the first slot addition that frame info fit in TBTP in addition to time slot
      if ( (tbtpToFill->GetSizeInBytes () + tbtpToFill->GetTimeSlotInfoSizeInBytes () + tbtpToFill->GetFrameInfoSize ()) > maxSizeInBytes )
        {
          tbtpToFill = CreateNewTbtp (tbtpContainer);
        }

      // sort RCs in UT using random method.
      std::vector<uint32_t> rcIndices = SortUtRcs (*it);
      std::vector<uint32_t>::const_iterator currentRcIndex = rcIndices.begin ();

      int64_t rcSymbolsLeft = m_utAllocs[*it].m_allocation.m_allocInfoPerRc[*currentRcIndex].GetTotalSymbols ();

      // generate slots here

      int64_t utSymbolsLeft = m_utAllocs[*it].m_allocation.GetTotalSymbols ();
      int64_t utSymbolsToUse = m_maxSymbolsPerCarrier;

      bool waveformIdTraced = false;

      while ( utSymbolsLeft > 0 )
        {
          Ptr<SatTimeSlotConf> timeSlot = NULL;

          // try to first create Control slot if present in request and is not already created
          // otherwise create TRC slot
          if ( (currentRcIndex == rcIndices.begin ()) && m_utAllocs[*it].m_request.m_ctrlSlotPresent
               && (m_utAllocs[*it].m_allocation.m_ctrlSlotPresent == false ))
            {
              timeSlot = CreateCtrlTimeSlot (*currentCarrier, utSymbolsToUse, carrierSymbolsToUse, utSymbolsLeft, rcSymbolsLeft, rcBasedAllocationEnabled );

              // if control slot creation fails try to allocate TRC slot,
              // this i because control and TRC slot may use different waveforms (different amount of symbols)
              if ( timeSlot )
                {
                  m_utAllocs[*it].m_allocation.m_ctrlSlotPresent = true;
                }
              else
                {
                  timeSlot = CreateTimeSlot (*currentCarrier, utSymbolsToUse, carrierSymbolsToUse, utSymbolsLeft, rcSymbolsLeft, m_utAllocs[*it].m_cno, rcBasedAllocationEnabled );
                }
            }
          else
            {
              timeSlot = CreateTimeSlot (*currentCarrier, utSymbolsToUse, carrierSymbolsToUse, utSymbolsLeft, rcSymbolsLeft, m_utAllocs[*it].m_cno, rcBasedAllocationEnabled );
            }

          // if creation succeeded, add slot to TBTP and update allocation info container
          if ( timeSlot )
            {
              // trace first used wave form per UT
              if ( !waveformIdTraced )
                {
                  waveformIdTraced = true;
                  waveformTrace (timeSlot->GetWaveFormId ());
                  utCount++;
                }

              if ( (tbtpToFill->GetSizeInBytes () + tbtpToFill->GetTimeSlotInfoSizeInBytes () ) > maxSizeInBytes )
                {
                  tbtpToFill = CreateNewTbtp (tbtpContainer);
                }

              timeSlot->SetRcIndex (*currentRcIndex);

              if (timeslotCount > SatFrameConf::m_maxTimeSlotCount)
                {
                  //NS_FATAL_ERROR ("Maximum limit for time slots in a frame reached. Check frame configuration!!!");
                }

              tbtpToFill->SetDaTimeslot (Mac48Address::ConvertFrom (*it), m_frameId, timeSlot);
              timeslotCount++;

              // store needed information to UT allocation container
              Ptr<SatWaveform> waveform = m_waveformConf->GetWaveform (timeSlot->GetWaveFormId ());

              UtAllocInfoContainer_t::iterator utAlloc = GetUtAllocItem (utAllocContainer, *it);
              utAlloc->second.first.at (*currentRcIndex) += waveform->GetPayloadInBytes ();
              utAlloc->second.second |= m_utAllocs[*it].m_allocation.m_ctrlSlotPresent;

              symbolsAllocated += waveform->GetBurstLengthInSymbols ();
            }

          // select new carrier to use
          if ( carrierSymbolsToUse <= 0)
            {
              carrierSymbolsToUse = m_maxSymbolsPerCarrier;
              currentCarrier++;

              if ( currentCarrier == carriers.end () )
                {
                  // stop if no more carriers left
                  utSymbolsLeft = 0;
                }
            }

          // select new RC to use
          if ( rcSymbolsLeft <= 0)
            {
              currentRcIndex++;

              if ( currentRcIndex == rcIndices.end () )
                {
                  // stop if last RC handled
                  utSymbolsLeft = 0;
                }
              else
                {
                  rcSymbolsLeft = m_utAllocs[*it].m_allocation.m_allocInfoPerRc[*currentRcIndex].GetTotalSymbols ();

                }
            }

          // carrier limit for UT reached, so we need to stop because time slot cannot generated anymore
          if ( (utSymbolsToUse <= 0 ) || (currentCarrier == carriers.end ()) )
            {
              utSymbolsLeft = 0;
            }
        }

      m_utAllocs[*it].m_allocation.m_ctrlSlotPresent = false;
    }

  // trace out frame UT load
  utLoadTrace ((uint32_t) m_frameId, utCount);

  // trace out frame load
  loadTrace ((uint32_t) m_frameId, symbolsAllocated / m_totalSymbolsInFrame );
}

void SatFrameAllocator::ShareSymbols (bool fcaEnabled)
{
  NS_LOG_FUNCTION (this);

  if ( (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols + m_preAllocatedVdbcSymbols) <= m_availableSymbolsInFrame )
    {
      AcceptRequests (CC_LEVEL_CRA_RBDC_VBDC);

      if ( fcaEnabled )
        {
          // share additional VBDC resources

          // calculate how many symbols left over
          double vbdcSymbolsLeft = m_availableSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols - m_preAllocatedVdbcSymbols;

          m_preAllocatedVdbcSymbols += vbdcSymbolsLeft;

          // sort RCs according to VBDC requests
          CcReqCompare vbdcCompare = CcReqCompare (m_utAllocs, CcReqCompare::CC_TYPE_VBDC);
          m_rcAllocs.sort (vbdcCompare);

          uint32_t rcAllocsLeft = m_rcAllocs.size ();

          // do share by adding a share to all RC/VBDC allocations
          for (RcAllocContainer_t::iterator it = m_rcAllocs.begin (); it != m_rcAllocs.end () && (vbdcSymbolsLeft > 0); it++)
            {
              double freeUtSymbols = std::max<double> (0.0, m_maxSymbolsPerCarrier - m_utAllocs.at (it->first).m_allocation.GetTotalSymbols ());
              double symbolsToAdd = std::min<double> (freeUtSymbols, (vbdcSymbolsLeft / rcAllocsLeft));

              // only share symbols to RCs requested RBDC or VBDC
              if ( ( m_utAllocs.at (it->first).m_request.m_allocInfoPerRc[it->second].m_rbdcSymbols > 0 )
                   || ( m_utAllocs.at (it->first).m_request.m_allocInfoPerRc[it->second].m_vbdcSymbols > 0 ) )
                {
                  m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_vbdcSymbols += symbolsToAdd;
                  m_utAllocs.at (it->first).m_allocation.m_vbdcSymbols += symbolsToAdd;
                  vbdcSymbolsLeft -= symbolsToAdd;
                }

              rcAllocsLeft--;
            }
        }
    }
  else if ( (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols) <= m_availableSymbolsInFrame )
    {
      // share VBDC resources

      // calculate how many symbols left over
      double vbdcSymbolsLeft = m_availableSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols;
      m_preAllocatedVdbcSymbols = vbdcSymbolsLeft;

      // sort RCs according to VBDC requests
      CcReqCompare vbdcCompare = CcReqCompare (m_utAllocs, CcReqCompare::CC_TYPE_VBDC);
      m_rcAllocs.sort (vbdcCompare);

      AcceptRequests (CC_LEVEL_CRA_RBDC);

      uint32_t rcAllocsLeft = m_rcAllocs.size ();

      // do share by setting a share to all RC/VBDC allocations
      for (RcAllocContainer_t::iterator it = m_rcAllocs.begin (); it != m_rcAllocs.end () && (vbdcSymbolsLeft > 0); it++)
        {
          double freeUtSymbols = std::max<double> (0.0, m_maxSymbolsPerCarrier - m_utAllocs.at (it->first).m_allocation.GetTotalSymbols ());
          double symbolsToAdd = std::min<double> (freeUtSymbols, (vbdcSymbolsLeft / rcAllocsLeft));

          // only share symbols to RCs requested VBDC
          if ( m_utAllocs.at (it->first).m_request.m_allocInfoPerRc[it->second].m_vbdcSymbols > 0 )
            {
              m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_vbdcSymbols = symbolsToAdd;
              m_utAllocs.at (it->first).m_allocation.m_vbdcSymbols += symbolsToAdd;
              vbdcSymbolsLeft -= symbolsToAdd;
            }

          rcAllocsLeft--;
        }
    }
  else if ( (m_preAllocatedCraSymbols + m_preAllocatedMinRdbcSymbols) <= m_availableSymbolsInFrame )
    {
      // share RBDC resources

      // calculate how many symbols left over
      double rbdcSymbolsLeft = m_availableSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedMinRdbcSymbols;

      m_preAllocatedVdbcSymbols = 0;
      m_preAllocatedRdbcSymbols = rbdcSymbolsLeft;

      // sort RCs according to RBDC requests
      CcReqCompare rbdcCompare = CcReqCompare (m_utAllocs, CcReqCompare::CC_TYPE_RBDC);
      m_rcAllocs.sort (rbdcCompare);

      AcceptRequests (CC_LEVEL_CRA_MIN_RBDC);

      uint32_t rcAllocsLeft = m_rcAllocs.size ();

      // do share by setting a share to all RC/RBDC allocations
      for (RcAllocContainer_t::iterator it = m_rcAllocs.begin (); it != m_rcAllocs.end () && (rbdcSymbolsLeft > 0); it++)
        {
          double freeUtSymbols = std::max<double> (0.0, m_maxSymbolsPerCarrier - m_utAllocs.at (it->first).m_allocation.GetTotalSymbols ());
          double symbolsToAdd = std::min<double> (freeUtSymbols, (rbdcSymbolsLeft / rcAllocsLeft));

          // only share symbols to RCs requested RBDC
          if ( m_utAllocs.at (it->first).m_request.m_allocInfoPerRc[it->second].m_rbdcSymbols > 0 )
            {
              m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_rbdcSymbols = symbolsToAdd + m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_minRbdcSymbols;
              m_utAllocs.at (it->first).m_allocation.m_rbdcSymbols += symbolsToAdd + m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_minRbdcSymbols;
              rbdcSymbolsLeft -= symbolsToAdd;
            }

          rcAllocsLeft--;
        }
    }
  else if ( (m_preAllocatedCraSymbols) <= m_availableSymbolsInFrame )
    {
      // share minimum RBDC resources

      // calculate how many symbols left over
      double minRbdcSymbolsLeft = m_availableSymbolsInFrame - m_preAllocatedCraSymbols;

      m_preAllocatedVdbcSymbols = 0;
      m_preAllocatedRdbcSymbols = minRbdcSymbolsLeft;
      m_preAllocatedMinRdbcSymbols = minRbdcSymbolsLeft;

      // sort RCs according to RBDC requests
      CcReqCompare minRbdcCompare = CcReqCompare (m_utAllocs, CcReqCompare::CC_TYPE_MIN_RBDC);
      m_rcAllocs.sort (minRbdcCompare);

      AcceptRequests (CC_LEVEL_CRA);

      uint32_t rcAllocsLeft = m_rcAllocs.size ();

      // do share by setting a share to all RC/Minimum RBDC and RC/RBDC allocations
      for (RcAllocContainer_t::iterator it = m_rcAllocs.begin (); it != m_rcAllocs.end () && (minRbdcSymbolsLeft > 0); it++)
        {
          double freeUtSymbols = std::max<double> (0.0, m_maxSymbolsPerCarrier - m_utAllocs.at (it->first).m_allocation.GetTotalSymbols ());
          double symbolsToAdd = std::min<double> (freeUtSymbols, (minRbdcSymbolsLeft / rcAllocsLeft));

          // only share symbols to RCs requested RBDC
          if ( m_utAllocs.at (it->first).m_request.m_allocInfoPerRc[it->second].m_rbdcSymbols > 0 )
            {
              m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_minRbdcSymbols = symbolsToAdd;
              m_utAllocs.at (it->first).m_allocation.m_minRbdcSymbols += symbolsToAdd;

              m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_rbdcSymbols = symbolsToAdd;
              m_utAllocs.at (it->first).m_allocation.m_rbdcSymbols += symbolsToAdd;

              minRbdcSymbolsLeft -= symbolsToAdd;
            }

          rcAllocsLeft--;
        }
    }
  else
    {
      NS_FATAL_ERROR ("CRAs don't fit to frame CAC or configuration error???");
    }
}

Ptr<SatTimeSlotConf>
SatFrameAllocator::CreateTimeSlot (uint16_t carrierId, int64_t& utSymbolsToUse, int64_t& carrierSymbolsToUse,
                                   int64_t& utSymbolsLeft, int64_t& rcSymbolsLeft, double cno, bool rcBasedAllocationEnabled)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatTimeSlotConf> timeSlotConf = NULL;
  int64_t symbolsToUse = std::min<int64_t> (carrierSymbolsToUse, utSymbolsToUse);
  uint32_t waveformId = 0;
  int64_t timeSlotSymbols = 0;

  if ( rcBasedAllocationEnabled || (symbolsToUse < utSymbolsLeft))
    {
      timeSlotSymbols = GetOptimalBurtsLengthInSymbols (symbolsToUse, rcSymbolsLeft, cno, waveformId);
    }
  else
    {
      timeSlotSymbols = GetOptimalBurtsLengthInSymbols (symbolsToUse, utSymbolsLeft, cno, waveformId);
    }

  if ( timeSlotSymbols == 0 )
    {
      if ( rcSymbolsLeft > 0)
        {
          if (carrierSymbolsToUse <= utSymbolsToUse)
            {
              carrierSymbolsToUse -= symbolsToUse;
            }

          utSymbolsToUse -= symbolsToUse;
        }
    }
  else if (rcSymbolsLeft > 0)
    {
      switch (m_configType)
        {
        case SatSuperframeConf::CONFIG_TYPE_0:
          {
            uint16_t index = (m_maxSymbolsPerCarrier - carrierSymbolsToUse) / timeSlotSymbols;
            timeSlotConf = m_frameConf->GetTimeSlotConf (carrierId, index);
          }
          break;

        case SatSuperframeConf::CONFIG_TYPE_1:
        case SatSuperframeConf::CONFIG_TYPE_2:
          {
            Time startTime = Seconds ( (m_maxSymbolsPerCarrier - carrierSymbolsToUse) / m_frameConf->GetBtuConf ()->GetSymbolRateInBauds ());
            timeSlotConf = Create<SatTimeSlotConf> (startTime, waveformId, carrierId, SatTimeSlotConf::SLOT_TYPE_TRC);
          }
          break;

        case SatSuperframeConf::CONFIG_TYPE_3:
        default:
          NS_FATAL_ERROR ("Not supported configuration type!!!");
          break;
        }

      if (timeSlotConf)
        {
          carrierSymbolsToUse -= timeSlotSymbols;
          utSymbolsToUse -= timeSlotSymbols;

          if ( rcBasedAllocationEnabled )
            {
              utSymbolsLeft -= std::min (rcSymbolsLeft, timeSlotSymbols);
            }
          else
            {
              utSymbolsLeft -= timeSlotSymbols;
            }

          rcSymbolsLeft -= timeSlotSymbols;
        }
    }

  return timeSlotConf;
}

Ptr<SatTimeSlotConf>
SatFrameAllocator::CreateCtrlTimeSlot (uint16_t carrierId, int64_t& utSymbolsToUse, int64_t& carrierSymbolsToUse,
                                       int64_t& utSymbolsLeft, int64_t& rcSymbolsLeft, bool rcBasedAllocationEnabled)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatTimeSlotConf> timeSlotConf = NULL;
  int64_t symbolsToUse = std::min<int64_t> (carrierSymbolsToUse, utSymbolsToUse);

  int64_t timeSlotSymbols = m_mostRobustWaveform->GetBurstLengthInSymbols ();

  if ( timeSlotSymbols <= symbolsToUse )
    {
      Time startTime = Seconds ( (m_maxSymbolsPerCarrier - carrierSymbolsToUse) / m_frameConf->GetBtuConf ()->GetSymbolRateInBauds ());
      timeSlotConf = Create<SatTimeSlotConf> (startTime, m_mostRobustWaveform->GetWaveformId (), carrierId, SatTimeSlotConf::SLOT_TYPE_C);

      carrierSymbolsToUse -= timeSlotSymbols;
      utSymbolsToUse -= timeSlotSymbols;

      if ( rcBasedAllocationEnabled )
        {
          utSymbolsLeft -= std::min (rcSymbolsLeft, timeSlotSymbols);
        }
      else
        {
          utSymbolsLeft -= timeSlotSymbols;
        }

      rcSymbolsLeft -= timeSlotSymbols;
    }

  return timeSlotConf;
}

uint32_t
SatFrameAllocator::GetOptimalBurtsLengthInSymbols (int64_t symbolsToUse, int64_t symbolsLeft, double cno, uint32_t& waveformId)
{
  NS_LOG_FUNCTION (this);

  uint32_t burstLength = 0;

  for (SatWaveformConf::BurstLengthContainer_t::const_iterator it = m_burstLenghts.begin (); it != m_burstLenghts.end (); it++)
    {
      uint32_t newLength = *it;
      uint32_t selectedWaveformId = 0;

      if ( m_configType == SatSuperframeConf::CONFIG_TYPE_0 )
        {
          waveformId = m_waveformConf->GetDefaultWaveformId ();
        }
      else
        {
          bool waveformFound = m_waveformConf->GetBestWaveformId (cno, m_frameConf->GetBtuConf ()->GetSymbolRateInBauds (), selectedWaveformId, *it );

          if ( waveformFound )
            {
              newLength = m_waveformConf->GetWaveform (selectedWaveformId)->GetBurstLengthInSymbols ();
            }
        }

      if ( symbolsToUse >= newLength )
        {
          if ( burstLength < symbolsLeft)
            {
              if ( burstLength < newLength )
                {
                  burstLength = newLength;
                  waveformId = selectedWaveformId;
                }
            }
          else if ( (newLength - symbolsLeft) < (burstLength - symbolsLeft))
            {
              burstLength = newLength;
              waveformId = selectedWaveformId;
            }
        }
    }

  return burstLength;
}

void
SatFrameAllocator::AcceptRequests (CcLevel_t ccLevel)
{
  NS_LOG_FUNCTION (this);

  for ( UtAllocContainer_t::iterator it = m_utAllocs.begin (); it != m_utAllocs.end (); it++ )
    {
      // accept first UT level total requests by updating allocation counters
      switch (ccLevel)
        {
        case CC_LEVEL_CRA:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = 0.0;
          it->second.m_allocation.m_rbdcSymbols = 0.0;
          it->second.m_allocation.m_vbdcSymbols = 0.0;
          break;

        case CC_LEVEL_CRA_MIN_RBDC:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = it->second.m_request.m_minRbdcSymbols;
          it->second.m_allocation.m_rbdcSymbols = 0.0;
          it->second.m_allocation.m_vbdcSymbols = 0.0;
          break;

        case CC_LEVEL_CRA_RBDC:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = it->second.m_request.m_minRbdcSymbols;
          it->second.m_allocation.m_rbdcSymbols = it->second.m_request.m_rbdcSymbols;
          it->second.m_allocation.m_vbdcSymbols = 0.0;
          break;

        case CC_LEVEL_CRA_RBDC_VBDC:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = it->second.m_request.m_minRbdcSymbols;
          it->second.m_allocation.m_rbdcSymbols = it->second.m_request.m_rbdcSymbols;
          it->second.m_allocation.m_vbdcSymbols = it->second.m_request.m_vbdcSymbols;
          break;

        default:
          NS_FATAL_ERROR ("Not supported CC level!!!");
          break;
        }

      // accept first RC specific requests by updating allocation counters
      for ( uint32_t i = 0; i < it->second.m_request.m_allocInfoPerRc.size (); i++ )
        {
          switch (ccLevel)
            {
            case CC_LEVEL_CRA:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = 0.0;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = 0.0;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = 0.0;
              break;

            case CC_LEVEL_CRA_MIN_RBDC:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_minRbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = 0.0;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = 0.0;
              break;

            case CC_LEVEL_CRA_RBDC:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_minRbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_rbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = 0.0;
              break;

            case CC_LEVEL_CRA_RBDC_VBDC:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_minRbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_rbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_vbdcSymbols;
              break;

            default:
              NS_FATAL_ERROR ("Not supported CC level!!!");
              break;
            }
        }
    }
}

void
SatFrameAllocator::UpdateAndStoreAllocReq (Address address, double cno, SatFrameAllocInfo &req)
{
  NS_LOG_FUNCTION (this);

  if ( (m_maxSymbolsPerCarrier - req.m_craSymbols) < 0 )
    {
      NS_FATAL_ERROR ("CRA does not fit in to carrier. Error in configuration or CAC?");
    }
  else if ( (m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_minRbdcSymbols) < 0 )
    {
      double minRbdcSymbolsLeft = m_maxSymbolsPerCarrier - req.m_craSymbols;

      // share symbols left between minimum RBDC requests in RCs in relation of the request
      for (SatFrameAllocInfoItemContainer_t::iterator it = req.m_allocInfoPerRc.begin (); it != req.m_allocInfoPerRc.end (); it++)
        {
          it->m_vbdcSymbols = 0.0;
          it->m_minRbdcSymbols = (it->m_minRbdcSymbols / req.m_minRbdcSymbols) * minRbdcSymbolsLeft;
          it->m_rbdcSymbols = it->m_minRbdcSymbols;
        }

      // update UT total request
      req.m_minRbdcSymbols = minRbdcSymbolsLeft;
      req.m_rbdcSymbols = minRbdcSymbolsLeft;
      req.m_vbdcSymbols = 0;
    }
  else if ( (m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols) < 0 )
    {
      double rbdcSymbolsLeft = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_minRbdcSymbols;
      double rbdcReqOverMinRbdc = req.m_rbdcSymbols - req.m_minRbdcSymbols;

      if (rbdcReqOverMinRbdc > 0)
        {
          // share symbols left over minimum RBDC between RBDC requests in RCs in relation of the request
          for (SatFrameAllocInfoItemContainer_t::iterator it = req.m_allocInfoPerRc.begin (); it != req.m_allocInfoPerRc.end (); it++)
            {
              it->m_vbdcSymbols = 0.0;
              double rcRbdcReqOverMinRbdc = std::max (0.0, (it->m_rbdcSymbols - it->m_minRbdcSymbols ));

              it->m_rbdcSymbols = ((rcRbdcReqOverMinRbdc / rbdcReqOverMinRbdc) * rbdcSymbolsLeft) + it->m_minRbdcSymbols;
            }
        }

      // update UT total request
      req.m_rbdcSymbols = rbdcSymbolsLeft + req.m_minRbdcSymbols;
      req.m_vbdcSymbols = 0;
    }
  else if ( (m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols - req.m_vbdcSymbols) < 0 )
    {
      double vbdcSymbolsLeft = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols;

      // share symbols left over RBDC between VBDC requests in RCs in relation of the request
      for (SatFrameAllocInfoItemContainer_t::iterator it = req.m_allocInfoPerRc.begin (); it != req.m_allocInfoPerRc.end (); it++)
        {
          it->m_vbdcSymbols = (it->m_vbdcSymbols / req.m_vbdcSymbols) * vbdcSymbolsLeft;
        }

      req.m_vbdcSymbols = vbdcSymbolsLeft;
    }

  // add request and empty allocation info container
  UtAllocItem_t utAlloc;
  utAlloc.m_request = req;
  utAlloc.m_allocation = SatFrameAllocInfo (req.m_allocInfoPerRc.size ());
  utAlloc.m_cno = cno;

  for (uint8_t i = 0; i < req.m_allocInfoPerRc.size (); i++)
    {
      RcAllocItem_t rcAlloc = std::make_pair (address, i);
      m_rcAllocs.push_back (rcAlloc);
    }

  m_utAllocs.insert (std::make_pair (address, utAlloc));
}

std::vector<Address>
SatFrameAllocator::SortUts ()
{
  NS_LOG_FUNCTION (this);

  std::vector<Address> uts;

  for (UtAllocContainer_t::const_iterator it = m_utAllocs.begin (); it != m_utAllocs.end (); it++)
    {
      uts.push_back (it->first);
    }

  // sort UTs using random method.
  std::random_shuffle (uts.begin (), uts.end ());

  return uts;
}

std::vector<uint16_t>
SatFrameAllocator::SortCarriers ()
{
  NS_LOG_FUNCTION (this);

  std::vector<uint16_t> carriers;

  for ( uint16_t i = 0; i < m_frameConf->GetCarrierCount (); i++ )
    {
      carriers.push_back (i);
    }

  // sort available carriers using random methods.
  std::random_shuffle (carriers.begin (), carriers.end ());

  return carriers;
}

std::vector<uint32_t>
SatFrameAllocator::SortUtRcs (Address ut)
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> rcIndices;

  for (uint32_t i = 0; i < m_utAllocs[ut].m_allocation.m_allocInfoPerRc.size (); i++)
    {
      rcIndices.push_back (i);
    }

  // we need to sort (or shuffle) only when there are at least two RCs in addition to RC 0,
  // because RC 0 is always first in the list
  if ( rcIndices.size () > 2)
    {
      // sort RCs in UT using random method.
      std::random_shuffle (rcIndices.begin () + 1, rcIndices.end ());
    }

  return rcIndices;
}


SatFrameAllocator::UtAllocInfoContainer_t::iterator
SatFrameAllocator::GetUtAllocItem (UtAllocInfoContainer_t& allocContainer, Address ut)
{
  NS_LOG_FUNCTION (this);
  UtAllocInfoContainer_t::iterator utAlloc = allocContainer.find (ut);

  if ( utAlloc == allocContainer.end () )
    {
      UtAllocInfoItem_t rcAllocs;

      rcAllocs.second = false;
      rcAllocs.first = std::vector<uint32_t> (m_utAllocs[ut].m_allocation.m_allocInfoPerRc.size (), 0);

      std::pair<UtAllocInfoContainer_t::iterator, bool> result = allocContainer.insert (std::make_pair (ut, rcAllocs ));

      if ( result.second )
        {
          utAlloc = result.first;
        }
      else
        {
          NS_FATAL_ERROR ("UT cannot be added to map!!!");
        }
    }

  return utAlloc;
}

Ptr<SatTbtpMessage>
SatFrameAllocator::CreateNewTbtp (TbtpMsgContainer_t& tbtpContainer)
{
  NS_LOG_FUNCTION (this);

  if (tbtpContainer.empty ())
    {
      NS_FATAL_ERROR ("TBTP container is empty");
    }

  Ptr<SatTbtpMessage> newTbtp = CreateObject<SatTbtpMessage> (tbtpContainer.back ()->GetSuperframeSeqId ());
  newTbtp->SetSuperframeCounter ( tbtpContainer.back ()->GetSuperframeCounter ());

  tbtpContainer.push_back (newTbtp);

  return newTbtp;
}

} // namespace ns3
