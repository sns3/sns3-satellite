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
#include "satellite-frame-allocator.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameAllocator");

namespace ns3 {

SatFrameAllocator::SatFrameInfo::SatFrameInfo (Ptr<SatFrameConf> frameConf, Ptr<SatWaveformConf> waveformConf, uint8_t frameId, SatSuperframeConf::ConfigType_t configType)
  : m_configType (configType),
    m_frameId (frameId),
    m_rcBasedAllocation (false),
    m_waveformConf (waveformConf),
    m_frameConf (frameConf)

{
  NS_LOG_FUNCTION (this);

  m_maxSymbolsPerCarrier = m_frameConf->GetBtuConf ()->GetSymbolRateInBauds () * m_frameConf->GetDuration ().GetSeconds ();
  m_totalSymbolsInFrame = m_maxSymbolsPerCarrier * m_frameConf->GetCarrierCount ();

  m_burstLenghts = waveformConf->GetSupportedBurstLengths ();

  ResetCounters ();
}

void
SatFrameAllocator::SatFrameInfo::ResetCounters ()
{
  NS_LOG_FUNCTION (this);

  m_availableSymbolsInFrame = m_totalSymbolsInFrame;
  m_preAllocatedCraSymbols = 0;
  m_preAllocatedMinRdbcSymbols = 0;
  m_preAllocatedRdbcSymbols = 0;
  m_preAllocatedVdbcSymbols = 0;

  m_utAllocs.clear ();
  m_rcAllocs.clear ();
}

SatFrameAllocator::SatFrameAllocInfoItem
SatFrameAllocator::SatFrameInfo::UpdateTotalRequests ()
{
  NS_LOG_FUNCTION (this);

  SatFrameAllocInfoItem totalReqs;

  for (UtAllocContainer_t::iterator it = m_utAllocs.begin (); it != m_utAllocs.end (); it++)
    {
      SatFrameAllocInfoItem utReqs = it->second.m_request.UpdateTotalCounts ();

      totalReqs.m_craSymbols += utReqs.m_craSymbols;
      totalReqs.m_minRbdcSymbols += utReqs.m_minRbdcSymbols;
      totalReqs.m_rbdcSymbols += utReqs.m_rbdcSymbols;
      totalReqs.m_vbdcSymbols += utReqs.m_vbdcSymbols;
    }

  return totalReqs;
}

void
SatFrameAllocator::SatFrameInfo::GenerateTimeSlots (std::vector<Ptr<SatTbtpMessage> >& tbtpContainer, uint32_t maxSizeInBytes)
{
  NS_LOG_FUNCTION (this);

  if (tbtpContainer.empty ())
    {
      NS_FATAL_ERROR ("TBTP container must contain at least one message.");
    }

  Ptr<SatTbtpMessage> tbtpToFill = tbtpContainer.back ();

  // sort UTs using random method.
  std::vector<Address> uts;

  for (UtAllocContainer_t::const_iterator it = m_utAllocs.begin (); it != m_utAllocs.end (); it++)
    {
      uts.push_back (it->first);
    }

  std::random_shuffle (uts.begin (), uts.end ());

  // sort available carriers using random methods.
  std::vector<uint16_t> carriers;

  for ( uint16_t i = 0; i < m_frameConf->GetCarrierCount (); i++ )
    {
      carriers.push_back (i);
    }

  std::random_shuffle (carriers.begin (), carriers.end ());

  std::vector<uint16_t>::const_iterator currentCarrier = carriers.begin ();
  int64_t carrierSymbolsToUse = m_maxSymbolsPerCarrier;

  for (std::vector<Address>::iterator it = uts.begin (); (it != uts.end ()) && (currentCarrier != carriers.end ()); it++ )
    {
      // sort RCs in UT using random method.
      std::random_shuffle (m_utAllocs[*it].m_allocation.m_allocInfoPerRc.begin (), m_utAllocs[*it].m_allocation.m_allocInfoPerRc.end ());
      uint8_t currentRc = 0;
      int64_t rcSymbolsLeft = m_utAllocs[*it].m_allocation.m_allocInfoPerRc[currentRc].GetTotalSymbols ();

      // generate slots here

      int64_t utSymbolsLeft = m_utAllocs[*it].m_allocation.GetTotalSymbols ();
      int64_t utSymbolsToUse = m_maxSymbolsPerCarrier;

      while ( utSymbolsLeft > 0 )
        {
          Ptr<SatTimeSlotConf> timeSlot = CreateTimeSlot (*currentCarrier, utSymbolsToUse, carrierSymbolsToUse, utSymbolsLeft, rcSymbolsLeft, m_utAllocs[*it].m_cno );

          if ( timeSlot )
            {
              if ( (tbtpToFill->GetSizeInBytes () + tbtpToFill->GetTimeSlotInfoSizeInBytes ()) > maxSizeInBytes )
                {
                  Ptr<SatTbtpMessage> newTbtp = CreateObject<SatTbtpMessage> (tbtpToFill->GetSuperframeSeqId ());
                  newTbtp->SetSuperframeCounter ( tbtpToFill->GetSuperframeCounter ());

                  tbtpContainer.push_back (newTbtp);

                  tbtpToFill = newTbtp;
                }

              timeSlot->SetRcIndex (currentRc);
              tbtpToFill->SetDaTimeslot (Mac48Address::ConvertFrom (*it), m_frameId, timeSlot);
            }

          if ( carrierSymbolsToUse <= 0)
            {
              carrierSymbolsToUse = m_maxSymbolsPerCarrier;
              currentCarrier++;
            }

          if ( rcSymbolsLeft <= 0)
            {
              currentRc++;
              rcSymbolsLeft = m_utAllocs[*it].m_allocation.m_allocInfoPerRc[currentRc].GetTotalSymbols ();
            }
        }
    }
}

Ptr<SatTimeSlotConf>
SatFrameAllocator::SatFrameInfo::CreateTimeSlot (uint16_t carrierId, int64_t& utSymbolsToUse, int64_t& carrierSymbolsToUse, int64_t& utSymbolsLeft, int64_t& rcSymbolsLeft, double cno)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatTimeSlotConf> timeSlotConf = NULL;
  int64_t symbolsToUse = std::min<int64_t> (carrierSymbolsToUse, utSymbolsToUse);

  int64_t timeSlotSymbols = GetOptimalBurtsLengthInSymbols (symbolsToUse, rcSymbolsLeft);

    if ( timeSlotSymbols == 0 )
      {
        if (carrierSymbolsToUse < utSymbolsLeft)
          {
            carrierSymbolsToUse -= symbolsToUse;
          }

        utSymbolsToUse -= symbolsToUse;
      }
    else
      {
        switch (m_configType)
        {
          case 0:
            {
              uint16_t index = (carrierId * m_frameConf->GetTimeSlotCount ()/m_frameConf->GetCarrierCount ()) + ((m_maxSymbolsPerCarrier - carrierSymbolsToUse) / timeSlotSymbols);
              timeSlotConf = m_frameConf->GetTimeSlotConf (index);

              carrierSymbolsToUse -= timeSlotSymbols;
              utSymbolsToUse -= timeSlotSymbols;

              if (m_rcBasedAllocation )
                {
                  utSymbolsLeft -= std::min (rcSymbolsLeft, timeSlotSymbols);
                }
              else
                {
                  utSymbolsLeft -= timeSlotSymbols;
                }

              rcSymbolsLeft -= timeSlotSymbols;
            }
            break;

          case 1:
          case 2:
            {
              uint32_t waveformId = 0;
              bool waveformFound = m_waveformConf->GetBestWaveformId (cno, m_frameConf->GetDuration ().GetSeconds (), waveformId, timeSlotSymbols );

              if ( waveformFound )
                {
                  double startTime = (m_maxSymbolsPerCarrier - carrierSymbolsToUse) / m_frameConf->GetBtuConf ()->GetSymbolRateInBauds ();
                  timeSlotConf = Create<SatTimeSlotConf> (startTime, waveformId, carrierId);

                  carrierSymbolsToUse -= timeSlotSymbols;
                  utSymbolsToUse -= timeSlotSymbols;

                  if (m_rcBasedAllocation )
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
            break;

          case 3:
          default:
            NS_FATAL_ERROR ("Not supported configuration type!!!");
            break;
        }
      }

  return timeSlotConf;
}

uint32_t
SatFrameAllocator::SatFrameInfo::GetOptimalBurtsLengthInSymbols (int64_t symbolsToUse, int64_t rcSymbolsLeft)
{
  NS_LOG_FUNCTION (this);

  uint32_t burstLength = 0;

  switch (m_configType)
    {
      case 0:
      case 1:
        if (symbolsToUse >= m_waveformConf->GetDefaultBurstLength ())
          {
            burstLength = m_waveformConf->GetDefaultBurstLength ();
          }
        break;

      case 2:
        {
          for (SatWaveformConf::BurstLengthContainer_t::const_iterator it = m_burstLenghts.begin (); it != m_burstLenghts.end (); it++)
            {
              if ( symbolsToUse >= *it )
                {
                  if ( burstLength < rcSymbolsLeft)
                    {
                      if ( burstLength < *it )
                        {
                          burstLength = *it;
                        }
                    }
                  else if ( (*it - rcSymbolsLeft) < (burstLength - rcSymbolsLeft))
                    {
                      burstLength = *it;
                    }
                }
            }
        }
        break;

      case 3:
      default:
        NS_FATAL_ERROR ("Not supported configuration type!!!");
        break;
    }

  return burstLength;

}

void
SatFrameAllocator::SatFrameInfo::AllocateSymbols (double targetLoad, bool fcaEnabled)
{
  NS_LOG_FUNCTION (this << targetLoad << fcaEnabled);

  if ( (targetLoad >= 0)  && (targetLoad <= 1) )
    {
      m_availableSymbolsInFrame = targetLoad * m_totalSymbolsInFrame;
      ShareSymbols (fcaEnabled);
    }
  else
    {
      NS_FATAL_ERROR ("target load must be between 0 and 1.");
    }
}

void SatFrameAllocator::SatFrameInfo::ShareSymbols (bool fcaEnabled)
{
  NS_LOG_FUNCTION (this);

  if ( (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols + m_preAllocatedVdbcSymbols) <= m_availableSymbolsInFrame )
    {
      if ( fcaEnabled )
        {
          // share additional VBDC resources

          // calculate how many symbols left over
          double vbdcSymbolsLeft = m_availableSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols - m_preAllocatedVdbcSymbols;

          m_preAllocatedVdbcSymbols += vbdcSymbolsLeft;

          // sort RCs according to VBDC requests
          CcReqCompare vbdcCompare = CcReqCompare (m_utAllocs, CcReqCompare::CC_TYPE_VBDC);
          m_rcAllocs.sort (vbdcCompare);

          AcceptRequests (CC_LEVEL_CRA_RBDC_VBDC);

          uint32_t rcAllocsLeft = m_rcAllocs.size ();

          // do share by adding a share to all RC/VBDC allocations
          for (RcAllocContainer_t::iterator it = m_rcAllocs.begin (); it != m_rcAllocs.end () && (vbdcSymbolsLeft > 0); it++)
            {
              double freeUtSymbols = std::max<double> (0.0, m_maxSymbolsPerCarrier - m_utAllocs.at (it->first).m_allocation.GetTotalSymbols ());
              double symbolsToAdd = std::min<double> (freeUtSymbols, (vbdcSymbolsLeft / rcAllocsLeft));

              m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_vbdcSymbols += symbolsToAdd;
              m_utAllocs.at (it->first).m_allocation.m_vbdcSymbols += symbolsToAdd;

              vbdcSymbolsLeft -= symbolsToAdd;
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

          m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_vbdcSymbols = symbolsToAdd;
          m_utAllocs.at (it->first).m_allocation.m_vbdcSymbols += symbolsToAdd;

          vbdcSymbolsLeft -= symbolsToAdd;
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

          m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_rbdcSymbols = symbolsToAdd + m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_minRbdcSymbols;
          m_utAllocs.at (it->first).m_allocation.m_rbdcSymbols += symbolsToAdd + m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_minRbdcSymbols;

          rbdcSymbolsLeft -= symbolsToAdd;
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

          m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_minRbdcSymbols = symbolsToAdd;
          m_utAllocs.at (it->first).m_allocation.m_minRbdcSymbols += symbolsToAdd;

          m_utAllocs.at (it->first).m_allocation.m_allocInfoPerRc[it->second].m_rbdcSymbols = symbolsToAdd;
          m_utAllocs.at (it->first).m_allocation.m_rbdcSymbols += symbolsToAdd;

          minRbdcSymbolsLeft -= symbolsToAdd;
          rcAllocsLeft--;
        }
    }
  else
    {
      NS_FATAL_ERROR ("CRAs don't fit to frame CAC or configuration error???");
    }
}

void
SatFrameAllocator::SatFrameInfo::AcceptRequests (CcLevel_t ccLevel)
{
  NS_LOG_FUNCTION (this);

  for ( UtAllocContainer_t::iterator it = m_utAllocs.begin (); it != m_utAllocs.end (); it++ )
    {
      // accept first UT level total requests by updating allocation counters
      switch (ccLevel)
      {
        case SatFrameInfo::CC_LEVEL_CRA:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = 0.0;
          it->second.m_allocation.m_rbdcSymbols = 0.0;
          it->second.m_allocation.m_vbdcSymbols = 0.0;
          break;

        case SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = it->second.m_request.m_minRbdcSymbols;
          it->second.m_allocation.m_rbdcSymbols = 0.0;
          it->second.m_allocation.m_vbdcSymbols = 0.0;
          break;

        case SatFrameInfo::CC_LEVEL_CRA_RBDC:
          it->second.m_allocation.m_craSymbols = it->second.m_request.m_craSymbols;
          it->second.m_allocation.m_minRbdcSymbols = it->second.m_request.m_minRbdcSymbols;
          it->second.m_allocation.m_rbdcSymbols = it->second.m_request.m_rbdcSymbols;
          it->second.m_allocation.m_vbdcSymbols = 0.0;
          break;

        case SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC:
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
            case SatFrameInfo::CC_LEVEL_CRA:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = 0.0;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = 0.0;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = 0.0;
              break;

            case SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_minRbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = 0.0;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = 0.0;
              break;

            case SatFrameInfo::CC_LEVEL_CRA_RBDC:
              it->second.m_allocation.m_allocInfoPerRc[i].m_craSymbols = it->second.m_request.m_allocInfoPerRc[i].m_craSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_minRbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_minRbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_rbdcSymbols = it->second.m_request.m_allocInfoPerRc[i].m_rbdcSymbols;
              it->second.m_allocation.m_allocInfoPerRc[i].m_vbdcSymbols = 0.0;
              break;

            case SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC:
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

double
SatFrameAllocator::SatFrameInfo::GetCcLoad (CcLevel_t ccLevel)
{
  NS_LOG_FUNCTION (this);

  double load = NAN;

  switch (ccLevel)
  {
    case SatFrameInfo::CC_LEVEL_CRA:
      load = m_preAllocatedCraSymbols - m_totalSymbolsInFrame;
      break;

    case SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedMinRdbcSymbols) - m_totalSymbolsInFrame ;
      break;

    case SatFrameInfo::CC_LEVEL_CRA_RBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols) - m_totalSymbolsInFrame;
      break;

    case SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols + m_preAllocatedVdbcSymbols) - m_totalSymbolsInFrame;
      break;
  }

  return load;
}

bool
SatFrameAllocator::SatFrameInfo::Allocate (CcLevel_t ccLevel, Address address, double cno, SatFrameAllocInfo &req)
{
  NS_LOG_FUNCTION (this);

  bool allocated = false;

  switch (ccLevel)
    {
      case SatFrameInfo::CC_LEVEL_CRA:
        {
        m_preAllocatedCraSymbols += req.m_craSymbols;

        double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols;
        double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

        if ( symbolsToUse >= (req.m_craSymbols))
          {
            double symbolsLeftInCarrier = m_maxSymbolsPerCarrier - req.m_craSymbols;

            m_preAllocatedMinRdbcSymbols += std::min<double> ( req.m_minRbdcSymbols, symbolsLeftInCarrier);
            m_preAllocatedRdbcSymbols += std::min<double> ( req.m_rbdcSymbols, symbolsLeftInCarrier);

            if ( symbolsToUse >= (req.m_craSymbols + req.m_rbdcSymbols))
              {
                double vbdcSymbolsInCarrier = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_minRbdcSymbols;
                m_preAllocatedVdbcSymbols += std::min<double> (req.m_vbdcSymbols, vbdcSymbolsInCarrier);
              }

            allocated = true;
          }

        allocated = true;
        }
        break;

      case SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC:
        {
          double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedMinRdbcSymbols;
          double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

          if ( symbolsToUse >= (req.m_craSymbols + req.m_minRbdcSymbols))
            {
              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;

              double symbolsLeftInCarrier = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_minRbdcSymbols;

              m_preAllocatedRdbcSymbols += std::min<double> (req.m_rbdcSymbols, symbolsLeftInCarrier);

              if (symbolsToUse >= (req.m_craSymbols + req.m_minRbdcSymbols))
                {
                  double vbdcSymbolsInCarrier = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols;
                  m_preAllocatedVdbcSymbols += std::min<double> (req.m_vbdcSymbols, vbdcSymbolsInCarrier);
                }

              allocated = true;
            }
        }
        break;

      case SatFrameInfo::CC_LEVEL_CRA_RBDC:
        {
          double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols;
          double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

          if ( symbolsToUse >= (req.m_craSymbols + req.m_rbdcSymbols))
            {
              double symbolsLeftInCarrier = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols;

              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;
              m_preAllocatedRdbcSymbols += req.m_rbdcSymbols;
              m_preAllocatedVdbcSymbols += std::min<double> (req.m_vbdcSymbols, symbolsLeftInCarrier);

              allocated = true;
            }
        }
        break;

      case SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC:
        {
          double symbolsLeftInFrame = m_totalSymbolsInFrame - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols- m_preAllocatedVdbcSymbols;
          double symbolsToUse = std::min<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

          if ( symbolsToUse >= (req.m_craSymbols + req.m_rbdcSymbols + req.m_vbdcSymbols))
            {
              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;
              m_preAllocatedRdbcSymbols += req.m_rbdcSymbols;
              m_preAllocatedVdbcSymbols += req.m_vbdcSymbols;

              allocated = true;
            }
        }
        break;
    }

  if ( allocated )
    {
      // update request according to carrier limit
      UpdateAllocReq (req);

      // add request and empty allocation info container
      UtAllocItem_t utAlloc;
      utAlloc.m_request = req;
      utAlloc.m_allocation = SatFrameAllocInfo (req.m_allocInfoPerRc.size ());
      utAlloc.m_cno = cno;

      m_utAllocs.insert (std::make_pair (address, utAlloc));

      for (uint8_t i = 0; i < req.m_allocInfoPerRc.size (); i++)
        {
          RcAllocItem_t rcAlloc = std::make_pair (address, i);
          m_rcAllocs.push_back (rcAlloc);
        }
    }

  return allocated;
}

void
SatFrameAllocator::SatFrameInfo::UpdateAllocReq (SatFrameAllocInfo &req)
{
  NS_LOG_FUNCTION (this);

  if ( (m_maxSymbolsPerCarrier - req.m_craSymbols) < 0 )
    {
      NS_FATAL_ERROR ("CRA does not fit in to carrier. Error in configuration or CAC?");
    }
  else if ( (m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_minRbdcSymbols) < 0 )
    {
      double minRbdcSymbolsLeft = m_maxSymbolsPerCarrier - req.m_craSymbols;

      // update UT total request
      req.m_minRbdcSymbols = minRbdcSymbolsLeft;
      req.m_rbdcSymbols = minRbdcSymbolsLeft;
      req.m_vbdcSymbols = 0;

      // share symbols left between minimum RBDC requests in RCs in relation of the request
      for (SatFrameAllocInfoItemContainer_t::iterator it = req.m_allocInfoPerRc.begin (); it != req.m_allocInfoPerRc.end (); it++)
        {
          it->m_vbdcSymbols = 0.0;
          it->m_minRbdcSymbols = (it->m_minRbdcSymbols / req.m_minRbdcSymbols) * minRbdcSymbolsLeft;
          it->m_rbdcSymbols = it->m_minRbdcSymbols;
        }
    }
  else if ( (m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols) < 0 )
    {
      double rbdcSymbolsLeft = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_minRbdcSymbols;
      double rbdcReqOverMinRbdc = req.m_rbdcSymbols - req.m_minRbdcSymbols;

      // update UT total request
      req.m_rbdcSymbols = rbdcSymbolsLeft;
      req.m_vbdcSymbols = 0;

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
    }
  else if ( (m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols - req.m_vbdcSymbols) < 0 )
    {
      double vbdcSymbolsLeft = m_maxSymbolsPerCarrier - req.m_craSymbols - req.m_rbdcSymbols;
      req.m_vbdcSymbols = vbdcSymbolsLeft;

      // share symbols left over RBDC between VBDC requests in RCs in relation of the request
      for (SatFrameAllocInfoItemContainer_t::iterator it = req.m_allocInfoPerRc.begin (); it != req.m_allocInfoPerRc.end (); it++)
        {
          it->m_vbdcSymbols = (it->m_vbdcSymbols / req.m_vbdcSymbols) * vbdcSymbolsLeft;
        }
    }
}


NS_OBJECT_ENSURE_REGISTERED (SatFrameAllocator);

TypeId
SatFrameAllocator::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatFrameAllocator")
      .SetParent<Object> ()
      .AddAttribute ("TargetLoad",
                     "Target load limits upper bound of the symbols in a frame.",
                      DoubleValue (0.9),
                      MakeDoubleAccessor (&SatFrameAllocator::m_targetLoad),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("FcaEnabled",
                     "Free capacity allocation (FCA) enable status.",
                      BooleanValue (true),
                      MakeBooleanAccessor (&SatFrameAllocator::m_fcaEnabled),
                      MakeBooleanChecker ())

    ;
    return tid;
}

TypeId
SatFrameAllocator::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatFrameAllocator::SatFrameAllocator (Ptr<SatSuperframeConf> superFrameConf, Ptr<SatWaveformConf> waveformConf, uint8_t maxRcCount)
 : m_waveformConf (waveformConf),
   m_targetLoad (0.0),
   m_fcaEnabled (false),
   m_maxRcCount (maxRcCount)
{
  NS_LOG_FUNCTION (this);

  for (uint8_t i = 0; i < superFrameConf->GetFrameCount (); i++ )
    {
      if (superFrameConf->GetFrameConf (i)->IsRandomAccess () == false )
        {
          std::pair<FrameInfoContainer_t::const_iterator, bool> result = m_frameInfos.insert ( std::make_pair( i, SatFrameInfo (superFrameConf->GetFrameConf (i), waveformConf, i, superFrameConf->GetConfigType ())));

          if ( result.second == false )
            {
              NS_FATAL_ERROR ("Frame info insertion failed!!!");
            }
        }
    }
}

SatFrameAllocator::~SatFrameAllocator ()
{
  NS_LOG_FUNCTION (this);
}

void
SatFrameAllocator::RemoveAllocations ()
{
  NS_LOG_FUNCTION (this);

  for (FrameInfoContainer_t::iterator it = m_frameInfos.begin (); it != m_frameInfos.end (); it++  )
    {
      it->second.ResetCounters ();
    }
}

void
SatFrameAllocator::GenerateTimeSlots (std::vector<Ptr<SatTbtpMessage> >& tbtpContainer, uint32_t maxSizeInBytes)
{
  NS_LOG_FUNCTION (this);

  if (tbtpContainer.empty ())
    {
      NS_FATAL_ERROR ("TBTP container must contain at least one message.");
    }

  for (FrameInfoContainer_t::iterator it = m_frameInfos.begin (); it != m_frameInfos.end (); it++  )
    {
      it->second.GenerateTimeSlots (tbtpContainer, maxSizeInBytes);
    }
}

void
SatFrameAllocator::AllocateSymbols ()
{
  NS_LOG_FUNCTION (this);

  for (FrameInfoContainer_t::iterator it = m_frameInfos.begin (); it != m_frameInfos.end (); it++  )
    {
      it->second.AllocateSymbols (m_targetLoad, m_fcaEnabled);
    }
}

bool
SatFrameAllocator::AllocateToFrame (double cno, SatFrameAllocReq& allocReq, SatFrameAllocResp& allocResp)
{
  NS_LOG_FUNCTION (this << cno);

  bool allocated = false;
  allocResp = SatFrameAllocResp ();

  SupportedFrameInfo_t supportedFrames;

  // find supported symbol rates (frames)
  for (FrameInfoContainer_t::iterator it = m_frameInfos.begin (); it != m_frameInfos.end (); it++  )
    {
      uint32_t waveFormId = 0;

      switch ( it->second.GetConfigType () )
      {
        case SatSuperframeConf::CONFIG_TYPE_0:
          supportedFrames.insert (std::make_pair (it->first, m_waveformConf->GetDefaultWaveformId ()));
          break;

        case SatSuperframeConf::CONFIG_TYPE_1:
            if ( m_waveformConf->GetBestWaveformId ( cno, it->second.GetSymbolRateInBauds (), waveFormId, m_waveformConf->GetDefaultBurstLength ()))
              {
                supportedFrames.insert (std::make_pair (it->first, waveFormId));
              }
          break;

        case SatSuperframeConf::CONFIG_TYPE_2:
          if ( m_waveformConf->GetBestWaveformId ( cno, it->second.GetSymbolRateInBauds (), waveFormId, SatWaveformConf::SHORT_BURST_LENGTH) )
            {
              supportedFrames.insert (std::make_pair (it->first, waveFormId));
            }
          break;

        default:
          NS_FATAL_ERROR ("Not supported configuration type");
          break;
      }
    }

  if ( supportedFrames.empty () == false )
    {
      // allocate with CC level CRA + RBDC + VBDC
      allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC, cno, allocReq, allocResp, supportedFrames );

      if ( allocated == false )
        {
          // allocate with CC level CRA + RBDC
          allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA_RBDC, cno, allocReq, allocResp, supportedFrames );

          if ( allocated == false )
            {
              // allocate with CC level CRA + MIM RBDC
              allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC, cno, allocReq, allocResp, supportedFrames );

              if ( allocated == false )
                {
                  // allocate with CC level CRA
                  allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA, cno, allocReq, allocResp, supportedFrames );
                }
            }
        }
    }

  return allocated;
}

bool
SatFrameAllocator::AllocateBasedOnCc (SatFrameInfo::CcLevel_t ccLevel, double cno, SatFrameAllocReq& allocReq, SatFrameAllocResp& allocResp, const SupportedFrameInfo_t &frames)
{
  double loadInSymbols = 0;
  uint8_t selectedFrame = 0;
  bool allocated = false;

  if (frames.empty ())
    {
      NS_FATAL_ERROR ("Tried to allocate without frames!!!");
    }

  // find the lowest load frame
  for (SupportedFrameInfo_t::const_iterator it = frames.begin (); it != frames.end (); it++  )
    {
      if ( it == frames.begin () )
        {
          selectedFrame = it->first;
          loadInSymbols = m_frameInfos.at (it->first).GetCcLoad (ccLevel);
        }
      else if ( m_frameInfos.at (it->first).GetCcLoad (ccLevel) < loadInSymbols)
        {
          selectedFrame = it->first;
          loadInSymbols = m_frameInfos.at (it->first).GetCcLoad (ccLevel);
        }
    }

  Time frameDuration = m_frameInfos.at (selectedFrame).GetDuration ();

  // convert bytes to symbols based on wave form
  SatFrameAllocInfo reqSymbols = SatFrameAllocInfo (allocReq.m_reqPerRc, m_waveformConf->GetWaveform (frames.at (selectedFrame)), frameDuration );

  if (m_frameInfos.at (selectedFrame).Allocate (ccLevel, allocReq.m_address, cno, reqSymbols))
    {
      // initialize needed information for the selected frame
      allocResp.m_frameId = selectedFrame;
      allocResp.m_waveformId = frames.at (selectedFrame);

      allocated = true;
    }

  return allocated;
}


} // namespace ns3
