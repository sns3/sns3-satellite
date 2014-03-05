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

#include <limits>
#include "ns3/log.h"
#include "satellite-frame-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameHelper");

namespace ns3 {

SatFrameHelper::SatFrameInfo::SatFrameInfo (Ptr<SatFrameConf> frameConf)
  : m_frameConf (frameConf)
{
  NS_LOG_FUNCTION (this);

  ResetCounters ();
}

void
SatFrameHelper::SatFrameInfo::ResetCounters ()
{
  NS_LOG_FUNCTION (this);

  m_maxSymbolsPerCarrier = m_frameConf->GetBtuConf ()->GetSymbolRateInBauds () * m_frameConf->GetDurationInSeconds ();
  m_totalAvailableSymbols = m_maxSymbolsPerCarrier * m_frameConf->GetCarrierCount ();
  m_preAllocatedCraSymbols = 0;
  m_preAllocatedMinRdbcSymbols = 0;
  m_preAllocatedRdbcSymbols = 0;
  m_preAllocatedVdbcSymbols = 0;
}

double
SatFrameHelper::SatFrameInfo::GetCcLoad (CcLevel_t ccLevel)
{
  NS_LOG_FUNCTION (this);

  double load = NAN;

  switch (ccLevel)
  {
    case SatFrameInfo::CC_LEVEL_CRA:
      load = m_preAllocatedCraSymbols - m_totalAvailableSymbols;
      break;

    case SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedMinRdbcSymbols) - m_totalAvailableSymbols ;
      break;

    case SatFrameInfo::CC_LEVEL_CRA_RBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols) - m_totalAvailableSymbols;
      break;

    case SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC:
      load = (m_preAllocatedCraSymbols + m_preAllocatedRdbcSymbols + m_preAllocatedVdbcSymbols) - m_totalAvailableSymbols;
      break;
  }

  return load;
}

bool
SatFrameHelper::SatFrameInfo::Allocate (CcLevel_t ccLevel, SatFrameAllocReqInSymbols &req)
{
  NS_LOG_FUNCTION (this);

  bool allocated = false;

  switch (ccLevel)
    {
      case SatFrameInfo::CC_LEVEL_CRA:
        m_preAllocatedCraSymbols += req.m_craSymbols;
        allocated = true;
        break;

      case SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC:
        {
          double symbolsLeftInFrame = m_totalAvailableSymbols - m_preAllocatedCraSymbols - m_preAllocatedMinRdbcSymbols;
          double symbolsToUse = std::max<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

          if ( symbolsToUse >= (req.m_craSymbols + req.m_minRbdcSymbols))
            {
              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;

              allocated = true;
            }
        }
        break;

      case SatFrameInfo::CC_LEVEL_CRA_RBDC:
        {
          double symbolsLeftInFrame = m_totalAvailableSymbols - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols;
          double symbolsToUse = std::max<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

          if ( symbolsToUse >= (req.m_craSymbols + req.m_rbdcSymbols))
            {
              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;
              m_preAllocatedRdbcSymbols += req.m_rbdcSymbols;

              allocated = true;
            }
        }
        break;

      case SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC:
        {
          double symbolsLeftInFrame = m_totalAvailableSymbols - m_preAllocatedCraSymbols - m_preAllocatedRdbcSymbols- m_preAllocatedVdbcSymbols;
          double symbolsToUse = std::max<double> (symbolsLeftInFrame, m_maxSymbolsPerCarrier);

          if ( symbolsToUse >= (req.m_craSymbols + req.m_rbdcSymbols + req.m_vbdcSymbols))
            {
              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;
              m_preAllocatedRdbcSymbols += req.m_rbdcSymbols;
              m_preAllocatedVdbcSymbols += req.m_vbdcSymbols;
              allocated = true;
            }
          else if ( symbolsToUse >= (req.m_craSymbols + req.m_rbdcSymbols))
            {
              m_preAllocatedCraSymbols += req.m_craSymbols;
              m_preAllocatedMinRdbcSymbols += req.m_minRbdcSymbols;
              m_preAllocatedRdbcSymbols += req.m_rbdcSymbols;
              m_preAllocatedVdbcSymbols += symbolsToUse - (req.m_craSymbols + req.m_rbdcSymbols);
            }
        }
        break;
    }

  return allocated;
}

SatFrameHelper::SatFrameHelper (Ptr<SatSuperframeConf> superFrameConf, Ptr<SatWaveformConf> waveformConf)
 : m_waveformConf (waveformConf)
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < superFrameConf->GetFrameCount (); i++ )
    {
      if (superFrameConf->GetFrameConf (i)->IsRandomAccess () == false )
        {
          std::pair<FrameInfoContainer_t::const_iterator, bool> result = m_frameInfos.insert ( std::make_pair( i, SatFrameInfo (superFrameConf->GetFrameConf (i))));

          if ( result.second == false )
            {
              NS_FATAL_ERROR ("Frame info insertion failed!!!");
            }
        }
    }
}

void
SatFrameHelper::ResetCounters ()
{
  NS_LOG_FUNCTION (this);

  for (FrameInfoContainer_t::iterator it = m_frameInfos.begin (); it != m_frameInfos.end (); it++  )
    {
      it->second.ResetCounters ();
    }
}

SatFrameHelper::~SatFrameHelper ()
{
  NS_LOG_FUNCTION (this);
}

bool
SatFrameHelper::AllocateToFrame (double cno, SatFrameAllocReq& allocReq, SatFrameAllocResp& allocResp)
{
  NS_LOG_FUNCTION (this << cno);

  bool allocated = false;
  allocResp = SatFrameAllocResp ();

  SupportedFrameInfo_t supportedFrames;

  // find supported symbol rates (frames)
  for (FrameInfoContainer_t::iterator it = m_frameInfos.begin (); it != m_frameInfos.end (); it++  )
    {
      uint32_t waveFormId = 0;

      if ( m_waveformConf->GetBestWaveformId ( cno, it->second.m_maxSymbolsPerCarrier, waveFormId, SatWaveformConf::SHORT_BURST_LENGTH) )
        {
          supportedFrames.insert (std::make_pair (it->first, waveFormId));
        }
    }

  // allocate with CC level CRA + RBDC + VBDC
  allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA_RBDC_VBDC, allocReq, allocResp, supportedFrames );

  if ( allocated == false )
    {
      // allocate with CC level CRA + RBDC
      allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA_RBDC, allocReq, allocResp, supportedFrames );

      if ( allocated == false )
        {
          // allocate with CC level CRA + MIM RBDC
          allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA_MIN_RBDC, allocReq, allocResp, supportedFrames );

          if ( allocated == false )
            {
              // allocate with CC level CRA
              allocated = AllocateBasedOnCc (SatFrameInfo::CC_LEVEL_CRA, allocReq, allocResp, supportedFrames );
            }
        }
    }

  return allocated;
}

bool
SatFrameHelper::AllocateBasedOnCc (SatFrameInfo::CcLevel_t ccLevel, SatFrameAllocReq& allocReq, SatFrameAllocResp& allocResp, const SupportedFrameInfo_t &frames)
{
  uint32_t loadInSymbols = 0;
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
        }
      else if ( m_frameInfos.at(it->first).GetCcLoad (ccLevel) < loadInSymbols)
        {
          selectedFrame = it->second;
        }

      loadInSymbols = m_frameInfos.at(it->first).GetCcLoad (ccLevel);
    }

  // convert bytes to symbols based on wave form
  SatFrameAllocReqInSymbols reqSymbols = SatFrameAllocReqInSymbols (allocReq, m_waveformConf->GetWaveform (frames.at (selectedFrame)));

  if (m_frameInfos.at(selectedFrame).Allocate (ccLevel, reqSymbols))
    {
      // initialize needed information for the selected frame
      allocResp.m_frameId = selectedFrame;
      allocResp.m_waveformId = frames.at (selectedFrame);

      allocated = true;
    }

  return allocated;
}


} // namespace ns3
