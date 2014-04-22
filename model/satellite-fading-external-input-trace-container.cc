/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#include <sstream>
#include "ns3/log.h"
#include "satellite-fading-external-input-trace-container.h"

NS_LOG_COMPONENT_DEFINE ("SatFadingExternalInputTraceContainer");

namespace ns3 {

SatFadingExternalInputTraceContainer::SatFadingExternalInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);
}

SatFadingExternalInputTraceContainer::~SatFadingExternalInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  m_utFadingMap.clear ();
  m_gwFadingMap.clear ();
}

void
SatFadingExternalInputTraceContainer::CreateUtFadingTrace (uint32_t utId)
{
  NS_LOG_FUNCTION (this << utId);

  std::string path = "src/satellite/data/fadingtraces/";

  // UT identifiers start from 1
  std::stringstream ss;
  ss << utId;

  std::string fwd = path + "term_ID" + ss.str () + "_fading_fwddwn.dat";
  std::string ret = path + "term_ID" + ss.str () + "_fading_rtnup.dat";

  Ptr<SatFadingExternalInputTrace> ftRet = Create<SatFadingExternalInputTrace> (SatFadingExternalInputTrace::FT_TWO_COLUMN, ret);
  Ptr<SatFadingExternalInputTrace> ftFwd = Create<SatFadingExternalInputTrace> (SatFadingExternalInputTrace::FT_THREE_COLUMN, fwd);

  // First = RETURN_USER
  // Second = FORWARD_USER
  m_utFadingMap.insert (std::make_pair (utId, std::make_pair (ftRet, ftFwd)));
}


void
SatFadingExternalInputTraceContainer::CreateGwFadingTrace (uint32_t gwId)
{
  NS_LOG_FUNCTION (this << gwId);

  std::string path = "src/satellite/data/fadingtraces/";

  // GW identifiers start from 1
  std::stringstream ss;
  ss << gwId;

  std::string fwd = path + "GW_ID" + ss.str () + "_fading_fwdup.dat";
  std::string ret = path + "GW_ID" + ss.str () + "_fading_rtndwn.dat";

  Ptr<SatFadingExternalInputTrace> ftRet = Create<SatFadingExternalInputTrace> (SatFadingExternalInputTrace::FT_TWO_COLUMN, ret);
  Ptr<SatFadingExternalInputTrace> ftFwd = Create<SatFadingExternalInputTrace> (SatFadingExternalInputTrace::FT_TWO_COLUMN, fwd);

  // First = RETURN_FEEDER
  // Second = FORWARD_FEEDR
  m_gwFadingMap.insert (std::make_pair (gwId, std::make_pair (ftRet, ftFwd)));
}


Ptr<SatFadingExternalInputTrace>
SatFadingExternalInputTraceContainer::GetFadingTrace (uint32_t nodeId, SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << nodeId);

  Ptr<SatFadingExternalInputTrace> ft;
  switch (channelType)
  {
    case SatEnums::FORWARD_USER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_utFadingMap.find (nodeId);

        if (iter == m_utFadingMap.end ())
          {
            CreateUtFadingTrace (nodeId);
          }

        ft = m_utFadingMap.at (nodeId).second;
        break;
      }
    case SatEnums::RETURN_USER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_utFadingMap.find (nodeId);

        if (iter == m_utFadingMap.end ())
          {
            CreateUtFadingTrace (nodeId);
          }

        ft = m_utFadingMap.at (nodeId).first;
        break;
      }
    case SatEnums::FORWARD_FEEDER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_gwFadingMap.find (nodeId);

        if (iter == m_gwFadingMap.end ())
          {
            CreateGwFadingTrace (nodeId);
          }

        ft = m_gwFadingMap.at (nodeId).second;
        break;
      }
    case SatEnums::RETURN_FEEDER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_gwFadingMap.find (nodeId);

        if (iter == m_gwFadingMap.end ())
          {
            CreateGwFadingTrace (nodeId);
          }

        ft = m_gwFadingMap.at (nodeId).first;
        break;
      }
    default:
      {
        NS_LOG_ERROR (this << " not valid channel type!");
        break;
      }
  }
  return ft;
}

bool
SatFadingExternalInputTraceContainer::TestFadingTraces (uint32_t numOfUts, uint32_t numOfGws)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (numOfUts > 0);
  NS_ASSERT (numOfGws > 0);

  uint32_t ueCount = m_utFadingMap.size ();
  uint32_t gwCount = m_gwFadingMap.size ();

  for (uint32_t i = 1; i <= numOfUts; i++)
    {
      std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_utFadingMap.find (i);

      if (iter == m_utFadingMap.end ())
        {
          ueCount++;
          CreateUtFadingTrace (ueCount);
        }
    }

  for (uint32_t i = 1; i <= numOfGws; i++)
    {
      std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_gwFadingMap.find (i);

      if (iter == m_gwFadingMap.end ())
        {
          gwCount++;
          CreateGwFadingTrace (gwCount);
        }
    }

  // Go through all the created fading trace class as
  // test each one of those. If even one test fails,
  // return false.

  // Test UT fading traces
  std::map< uint32_t, ChannelTracePair_t>::const_iterator cit;
  for (cit = m_utFadingMap.begin (); cit != m_utFadingMap.end (); ++cit)
    {
      if (!cit->second.first->TestFadingTrace ())
        {
          return false;
        }
      if (!cit->second.second->TestFadingTrace ())
        {
          return false;
        }
    }

  // Test GW fading traces
  for (cit = m_gwFadingMap.begin (); cit != m_gwFadingMap.end (); ++cit)
    {
      if (!cit->second.first->TestFadingTrace ())
        {
          return false;
        }
      if (!cit->second.second->TestFadingTrace ())
        {
          return false;
        }
    }

  // All tests succeeded
  return true;
}

} // namespace ns3
