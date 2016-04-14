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

#include <fstream>
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"
#include "satellite-fading-external-input-trace-container.h"

NS_LOG_COMPONENT_DEFINE ("SatFadingExternalInputTraceContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFadingExternalInputTraceContainer);

TypeId
SatFadingExternalInputTraceContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingExternalInputTraceContainer")
    .SetParent<Object> ()
    .AddConstructor<SatFadingExternalInputTraceContainer> ()
    .AddAttribute ("UtInputMode",
                   "Input mode to read trace source files from given index table.",
                   EnumValue (SatFadingExternalInputTraceContainer::LIST_MODE),
                   MakeEnumAccessor (&SatFadingExternalInputTraceContainer::m_utInputMode),
                   MakeEnumChecker (SatFadingExternalInputTraceContainer::LIST_MODE, "ListMode",
                                    SatFadingExternalInputTraceContainer::POSITION_MODE, "PositionMode",
                                    SatFadingExternalInputTraceContainer::RANDOM_MODE, "RandomMode"))
    .AddAttribute ("UtRtnUpIndexFileName",
                   "Index file defining trace source files for return up link/UTs.",
                   StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"),
                   MakeStringAccessor (&SatFadingExternalInputTraceContainer::m_utRtnUpIndexFileName),
                   MakeStringChecker ())
    .AddAttribute ("UtFwdDownIndexFileName",
                   "Index file defining trace source files for forward down link/UTs.",
                   StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"),
                   MakeStringAccessor (&SatFadingExternalInputTraceContainer::m_utFwdDownIndexFileName),
                   MakeStringChecker ())
    .AddAttribute ("GwFwdUpIndexFileName",
                   "Index file defining trace source files for forward up link/GWs.",
                   StringValue ("GW_fading_fwdup_traces.txt"),
                   MakeStringAccessor (&SatFadingExternalInputTraceContainer::m_gwFwdUpIndexFileName),
                   MakeStringChecker ())
    .AddAttribute ("GwRtnDownIndexFileName",
                   "Index file defining trace source files for return down link/GWs.",
                   StringValue ("GW_fading_rtndwn_traces.txt"),
                   MakeStringAccessor (&SatFadingExternalInputTraceContainer::m_gwRtnDownIndexFileName),
                   MakeStringChecker ())
    .AddAttribute ("MaxDistance",
                   "Maximum distance allowed to fading source in position based mode [m].",
                   DoubleValue (5000),
                   MakeDoubleAccessor (&SatFadingExternalInputTraceContainer::m_maxDistanceToFading),
                   MakeDoubleChecker<double> ());
  return tid;
}

TypeId SatFadingExternalInputTraceContainer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatFadingExternalInputTraceContainer::SatFadingExternalInputTraceContainer ()
  : m_utInputMode (LIST_MODE),
    m_indexFilesLoaded (false),
    m_maxDistanceToFading (0)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());
  m_dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory () + "/ext-fadingtraces/input/";
}

SatFadingExternalInputTraceContainer::~SatFadingExternalInputTraceContainer ()
{
  NS_LOG_FUNCTION (this);

  m_utFadingMap.clear ();
  m_gwFadingMap.clear ();
}

void
SatFadingExternalInputTraceContainer::LoadIndexFiles ()
{
  NS_LOG_FUNCTION (this);

  if ( !m_indexFilesLoaded )
    {
      ReadIndexFile (m_utRtnUpIndexFileName, m_utRtnUpFileNames);
      ReadIndexFile (m_utFwdDownIndexFileName, m_utFwdDownFileNames);
      ReadIndexFile (m_gwRtnDownIndexFileName, m_gwRtnDownFileNames);
      ReadIndexFile (m_gwFwdUpIndexFileName, m_gwFwdUpFileNames);

      m_indexFilesLoaded = true;
    }
}

void
SatFadingExternalInputTraceContainer::CreateUtFadingTrace (uint32_t utId, Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this << utId);

  if ( !m_indexFilesLoaded )
    {
      LoadIndexFiles ();
    }

  Ptr<SatFadingExternalInputTrace> ftRet = CreateFadingTrace (SatFadingExternalInputTrace::FT_TWO_COLUMN, m_utInputMode, m_utRtnUpFileNames, utId - 1, mobility);
  Ptr<SatFadingExternalInputTrace> ftFwd = CreateFadingTrace (SatFadingExternalInputTrace::FT_THREE_COLUMN, m_utInputMode, m_utFwdDownFileNames, utId - 1, mobility);

  // First = RETURN_USER
  // Second = FORWARD_USER
  m_utFadingMap.insert (std::make_pair (utId, std::make_pair (ftRet, ftFwd)));
}


void
SatFadingExternalInputTraceContainer::CreateGwFadingTrace (uint32_t gwId, Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this << gwId);

  if ( !m_indexFilesLoaded )
    {
      LoadIndexFiles ();
    }

  Ptr<SatFadingExternalInputTrace> ftRet = CreateFadingTrace (SatFadingExternalInputTrace::FT_TWO_COLUMN, LIST_MODE, m_gwRtnDownFileNames, gwId - 1, mobility);
  Ptr<SatFadingExternalInputTrace> ftFwd = CreateFadingTrace (SatFadingExternalInputTrace::FT_TWO_COLUMN, LIST_MODE, m_gwFwdUpFileNames, gwId - 1, mobility);

  // First = RETURN_FEEDER
  // Second = FORWARD_FEEDR
  m_gwFadingMap.insert (std::make_pair (gwId, std::make_pair (ftRet, ftFwd)));
}


Ptr<SatFadingExternalInputTrace>
SatFadingExternalInputTraceContainer::GetFadingTrace (uint32_t nodeId, SatEnums::ChannelType_t channelType, Ptr<MobilityModel> mobility)
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
            CreateUtFadingTrace (nodeId, mobility);
          }

        ft = m_utFadingMap.at (nodeId).second;
        break;
      }
    case SatEnums::RETURN_USER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_utFadingMap.find (nodeId);

        if (iter == m_utFadingMap.end ())
          {
            CreateUtFadingTrace (nodeId, mobility);
          }

        ft = m_utFadingMap.at (nodeId).first;
        break;
      }
    case SatEnums::FORWARD_FEEDER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_gwFadingMap.find (nodeId);

        if (iter == m_gwFadingMap.end ())
          {
            CreateGwFadingTrace (nodeId, mobility);
          }

        ft = m_gwFadingMap.at (nodeId).second;
        break;
      }
    case SatEnums::RETURN_FEEDER_CH:
      {
        std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_gwFadingMap.find (nodeId);

        if (iter == m_gwFadingMap.end ())
          {
            CreateGwFadingTrace (nodeId, mobility);
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

  if ( !m_indexFilesLoaded )
    {
      ReadIndexFile (m_gwRtnDownIndexFileName, m_gwRtnDownFileNames);
      ReadIndexFile (m_gwFwdUpIndexFileName, m_gwFwdUpFileNames);
      ReadIndexFile (m_utRtnUpIndexFileName, m_utRtnUpFileNames);
      ReadIndexFile (m_utFwdDownIndexFileName, m_utFwdDownFileNames);
      m_indexFilesLoaded = true;
    }

  for (uint32_t i = 1; i <= numOfUts; i++)
    {
      std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_utFadingMap.find (i);

      if (iter == m_utFadingMap.end ())
        {
          ueCount++;
          CreateUtFadingTrace (ueCount, NULL);
        }
    }

  for (uint32_t i = 1; i <= numOfGws; i++)
    {
      std::map< uint32_t, ChannelTracePair_t>::iterator iter = m_gwFadingMap.find (i);

      if (iter == m_gwFadingMap.end ())
        {
          gwCount++;
          CreateGwFadingTrace (gwCount, NULL);
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

void
SatFadingExternalInputTraceContainer::ReadIndexFile (std::string indexFile, TraceFileContainer_t& container)
{
  NS_LOG_FUNCTION (this << indexFile);

  // READ FROM THE GIVEN INDEX FILE
  std::ifstream *ifs = new std::ifstream ((m_dataPath + indexFile).c_str (), std::ios::in);

  if (ifs->is_open ())
    {
      double lat, lon, alt;
      uint32_t id;
      std::string fileName;

      *ifs >> id >> fileName >> lat >> lon >> alt;

      while (ifs->good ())
        {
          NS_LOG_DEBUG (this <<
                        " id = " << id <<
                        " file = " << fileName <<
                        " latitude [deg] = " << lat <<
                        " longitude [deg] = " << lon);

          // Store the values
          TraceFileContainerItem_t item = std::make_pair ( fileName, GeoCoordinate (lat, lon, alt) );
          container.push_back ( item );

          // get next row
          *ifs >> id >> fileName >> lat >> lon >> alt;
        }

      ifs->close ();
    }
  else
    {
      NS_FATAL_ERROR (m_dataPath << indexFile << " index file not found.");
    }

  delete ifs;
}

Ptr<SatFadingExternalInputTrace>
SatFadingExternalInputTraceContainer::CreateFadingTrace (SatFadingExternalInputTrace::TraceFileType_e fileType, InputMode_t inputMode,
                                                         TraceFileContainer_t& container, uint32_t id, Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this << mobility);

  Ptr<SatFadingExternalInputTrace> trace;
  std::string fileName;

  switch (inputMode)
    {
    case LIST_MODE:
      if ( container.empty () || ( id > container.size () ))
        {
          NS_FATAL_ERROR ("No input available!");
        }
      else
        {
          fileName = container.at (id).first;
        }
      break;

    case RANDOM_MODE:
      if ( container.empty ())
        {
          NS_FATAL_ERROR ("No input available!");
        }
      else
        {
          fileName = container.at (std::rand () % container.size ()).first;
        }
      break;

    case POSITION_MODE:
      fileName = FindSourceBasedOnPosition (container, id, mobility);
      break;

    default:
      NS_FATAL_ERROR ("Not supported mode.");
      break;
    }

  NS_LOG_INFO ("SatFadingExternalInputTraceContainer -> Creation info: Mode=" << m_utInputMode << ", ID (GW/UT)=" << id << ", FileName=" << fileName);

  // find from loaded list

  TraceInputContainer_t::iterator it = m_loadedTraces.find (fileName);

  if ( it == m_loadedTraces.end ())
    {
      // create if not found
      trace = Create<SatFadingExternalInputTrace> (fileType, m_dataPath + fileName);
    }
  else
    {
      trace = it->second;
    }

  return trace;
}

std::string
SatFadingExternalInputTraceContainer::FindSourceBasedOnPosition (TraceFileContainer_t& container, uint32_t id, Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this << mobility);

  std::string fileName;
  double currentDistanceToFading = std::numeric_limits<double>::max ();
  Vector position = mobility->GetPosition ();

  for (TraceFileContainer_t::iterator it = container.begin (); it != container.end (); it++)
    {
      Vector fadingPosition = it->second.ToVector ();

      double distanceToFading = CalculateDistance ( position, fadingPosition );

      if ( distanceToFading < currentDistanceToFading )
        {
          currentDistanceToFading = distanceToFading;
          fileName = it->first;
        }
    }

  if ( currentDistanceToFading > m_maxDistanceToFading )
    {
      NS_FATAL_ERROR ("No valid fading based on position (min found distance, max allowed distance): " << currentDistanceToFading << ", " << m_maxDistanceToFading);
    }
  else
    {
      NS_LOG_INFO ("SatFadingExternalInputTraceContainer -> Minimum distance to fading trace source: " << currentDistanceToFading);
    }

  return fileName;
}

} // namespace ns3
