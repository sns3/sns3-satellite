/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include <ns3/log.h>
#include <ns3/fatal-error.h>
#include <ns3/boolean.h>
#include <ns3/string.h>
#include <ns3/enum.h>
#include <ns3/callback.h>

#include <ns3/node-container.h>
#include <ns3/satellite-beam-scheduler.h>
#include <ns3/satellite-ncc.h>
#include <ns3/satellite-beam-helper.h>
#include <ns3/satellite-helper.h>

#include <ns3/data-collection-object.h>
#include <ns3/satellite-frame-symbol-load-probe.h>
#include <ns3/satellite-frame-user-load-probe.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <utility>

#include "satellite-stats-frame-load-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsFrameLoadHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsFrameLoadHelper);

std::string // static
SatStatsFrameLoadHelper::GetUnitTypeName (SatStatsFrameLoadHelper::UnitType_t unitType)
{
  switch (unitType)
    {
    case SatStatsFrameLoadHelper::UNIT_SYMBOLS:
      return "UNIT_SYMBOLS";
    case SatStatsFrameLoadHelper::UNIT_USERS:
      return "UNIT_USERS";
    default:
      NS_FATAL_ERROR ("SatStatsFrameLoadHelper - Invalid unit type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsFrameLoadHelper - Invalid unit type");
  return "";
}


SatStatsFrameLoadHelper::SatStatsFrameLoadHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
    m_unitType (SatStatsFrameLoadHelper::UNIT_SYMBOLS),
    m_shortLabel (""),
    m_longLabel (""),
    m_objectTraceSourceName (""),
    m_probeTraceSourceName ("")
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFrameLoadHelper::~SatStatsFrameLoadHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFrameLoadHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFrameLoadHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsFrameLoadHelper::SetUnitType (SatStatsFrameLoadHelper::UnitType_t unitType)
{
  NS_LOG_FUNCTION (this << GetUnitTypeName (unitType));
  m_unitType = unitType;

  // Update unit-specific attributes.
  if (unitType == SatStatsFrameLoadHelper::UNIT_SYMBOLS)
    {
      m_shortLabel = "allocated_symbol_ratio";
      m_longLabel = "Number of allocated symbols over total number of symbols";
      m_objectTraceSourceName = "FrameLoadTrace";
      m_probeTraceSourceName = "Output";
    }
  else if (unitType == SatStatsFrameLoadHelper::UNIT_USERS)
    {
      m_shortLabel = "allocated_users";
      m_longLabel = "Number of scheduled users";
      m_objectTraceSourceName = "FrameUtLoadTrace";
      m_probeTraceSourceName = "Output";
    }
  else
    {
      NS_FATAL_ERROR ("SatStatsFrameLoadHelper - Invalid unit type");
    }
}


SatStatsFrameLoadHelper::UnitType_t
SatStatsFrameLoadHelper::GetUnitType () const
{
  return m_unitType;
}


void
SatStatsFrameLoadHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  if (GetOutputType () != SatStatsHelper::OUTPUT_SCALAR_FILE)
    {
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ())
                      << " is not a valid output type for this statistics.");
    }

  if (GetIdentifierType () == SatStatsHelper::IDENTIFIER_UT
      || GetIdentifierType () == SatStatsHelper::IDENTIFIER_UT_USER)
    {
      NS_FATAL_ERROR (GetIdentifierTypeName (GetIdentifierType ())
                      << " is not a valid identifier type for this statistics.");
    }

  // Setup aggregator.
  m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                   "OutputFileName", StringValue (GetOutputFileName ()),
                                   "MultiFileMode", BooleanValue (false),
                                   "EnableContextPrinting", BooleanValue (true),
                                   "GeneralHeading", StringValue (GetIdentifierHeading (m_shortLabel)));

  // Setup probes.
  Ptr<SatBeamHelper> beamHelper = GetSatHelper ()->GetBeamHelper ();
  NS_ASSERT (beamHelper != 0);
  Ptr<SatNcc> ncc = beamHelper->GetNcc ();
  NS_ASSERT (ncc != 0);
  std::list<uint32_t> beams = beamHelper->GetBeams ();
  bool ret = false;

  for (std::list<uint32_t>::const_iterator it = beams.begin ();
       it != beams.end (); ++it)
    {
      if (m_unitType == SatStatsFrameLoadHelper::UNIT_SYMBOLS)
        {
          ret = SetupProbe<SatFrameSymbolLoadProbe> (ncc->GetBeamScheduler (*it),
                                                     GetIdentifierForBeam (*it),
                                                     &SatStatsFrameLoadHelper::FrameSymbolLoadCallback);
        }
      else
        {
          NS_ASSERT_MSG (m_unitType == SatStatsFrameLoadHelper::UNIT_USERS,
                         "Invalid unit type");
          ret = SetupProbe<SatFrameUserLoadProbe> (ncc->GetBeamScheduler (*it),
                                                   GetIdentifierForBeam (*it),
                                                   &SatStatsFrameLoadHelper::FrameUserLoadCallback);
        }

      if (ret)
        {
          NS_LOG_INFO (this << " successfully connected"
                            << " with beam " << *it);
        }
      else
        {
          NS_LOG_WARN (this << " unable to connect to beam " << *it);
        }

    } // end of `for (it: beams)`


} // end of `void DoInstall ();`


std::string
SatStatsFrameLoadHelper::GetIdentifierHeading (std::string dataLabel) const
{
  switch (GetIdentifierType ())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return "% global frame_number " + dataLabel;

    case SatStatsHelper::IDENTIFIER_GW:
      return "% gw_id frame_number " + dataLabel;

    case SatStatsHelper::IDENTIFIER_BEAM:
      return "% beam_id frame_number " + dataLabel;

    default:
      NS_FATAL_ERROR ("SatStatsFrameLoadHelper - Invalid identifier type");
      break;
    }
  return "";
}


void
SatStatsFrameLoadHelper::FrameSymbolLoadCallback (std::string context,
                                                  uint32_t frameId,
                                                  double loadRatio)
{
  //NS_LOG_FUNCTION (this << context << frameId << loadRatio);

  // Get the right collector for this frame ID and identifier.
  Ptr<ScalarCollector> collector = GetCollector (frameId, context);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to get/create collector"
                 << " for frame ID "  << frameId
                 << " and beam " << context);

  // Pass the sample to the collector.
  collector->TraceSinkDouble (0, loadRatio);
}


void
SatStatsFrameLoadHelper::FrameUserLoadCallback (std::string context,
                                                uint32_t frameId,
                                                uint32_t utCount)
{
  //NS_LOG_FUNCTION (this << context << frameId << utCount);

  // Get the right collector for this frame ID and identifier.
  Ptr<ScalarCollector> collector = GetCollector (frameId, context);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to get/create collector"
                 << " for frame ID "  << frameId
                 << " and identifier " << context);

  // Pass the sample to the collector.
  collector->TraceSinkUinteger32 (0, utCount);
}


Ptr<ScalarCollector>
SatStatsFrameLoadHelper::GetCollector (uint32_t frameId, std::string identifier)
{
  //NS_LOG_FUNCTION (this << frameId);

  // convert context to number
  std::stringstream ss (identifier);
  uint32_t identifierNum;
  if (!(ss >> identifierNum))
    {
      NS_FATAL_ERROR ("Cannot convert '" << identifier << "' to number");
    }

  std::map<uint32_t, CollectorMap>::iterator it = m_collectors.find (frameId);
  if (it == m_collectors.end ())
    {
      // Newly discovered frame ID
      uint32_t n = 0;
      CollectorMap collectorMap;
      collectorMap.SetType ("ns3::ScalarCollector");
      if (m_unitType == SatStatsFrameLoadHelper::UNIT_SYMBOLS)
        {
          collectorMap.SetAttribute ("InputDataType",
                                     EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
          collectorMap.SetAttribute ("OutputType",
                                     EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        }
      else
        {
          NS_ASSERT_MSG (m_unitType == SatStatsFrameLoadHelper::UNIT_USERS,
                         "Invalid unit type");
          collectorMap.SetAttribute ("InputDataType",
                                     EnumValue (ScalarCollector::INPUT_DATA_TYPE_UINTEGER));
          collectorMap.SetAttribute ("OutputType",
                                     EnumValue (ScalarCollector::OUTPUT_TYPE_SUM));
        }

      /*
       * Create a new set of collectors. Its name consists of two integers:
       *   - the first is the identifier ID (beam ID, GW ID, or simply zero for
       *     global);
       *   - the second is the frame ID.
       */
      switch (GetIdentifierType ())
        {
        case SatStatsHelper::IDENTIFIER_GLOBAL:
          {
            std::ostringstream name;
            name << "0 " << frameId;
            collectorMap.SetAttribute ("Name", StringValue (name.str ()));
            collectorMap.Create (0);
            n++;
            break;
          }

        case SatStatsHelper::IDENTIFIER_GW:
          {
            NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
            for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
              {
                const uint32_t gwId = GetGwId (*it);
                std::ostringstream name;
                name << gwId << " " << frameId;
                collectorMap.SetAttribute ("Name", StringValue (name.str ()));
                collectorMap.Create (gwId);
                n++;
              }
            break;
          }

        case SatStatsHelper::IDENTIFIER_BEAM:
          {
            std::list<uint32_t> beams = GetSatHelper ()->GetBeamHelper ()->GetBeams ();
            for (std::list<uint32_t>::const_iterator it = beams.begin ();
                 it != beams.end (); ++it)
              {
                const uint32_t beamId = (*it);
                std::ostringstream name;
                name << beamId << " " << frameId;
                collectorMap.SetAttribute ("Name", StringValue (name.str ()));
                collectorMap.Create (beamId);
                n++;
              }
            break;
          }

        default:
          NS_FATAL_ERROR ("SatStatsFrameLoadHelper - Invalid identifier type");
          break;
        }

      collectorMap.ConnectToAggregator ("Output",
                                        m_aggregator,
                                        &MultiFileAggregator::Write1d);
      NS_LOG_INFO (this << " created " << n << " instance(s)"
                        << " of " << collectorMap.GetType ().GetName ()
                        << " for " << GetIdentifierTypeName (GetIdentifierType ()));

      std::pair<std::map<uint32_t, CollectorMap>::iterator, bool> ret;
      ret = m_collectors.insert (std::make_pair (frameId, collectorMap));
      NS_ASSERT (ret.second);
      it = ret.first;

    } // end of `if (it == m_collectors.end ())`

  NS_ASSERT (it != m_collectors.end ());

  // Find the collector with the right identifier.
  Ptr<DataCollectionObject> collector = it->second.Get (identifierNum);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to find collector with identifier " << identifier);
  Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
  return c;

} // `Ptr<ScalarCollector> GetCollector (uint32_t, std::string)`


// IN SYMBOL UNIT /////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFrameSymbolLoadHelper);

SatStatsFrameSymbolLoadHelper::SatStatsFrameSymbolLoadHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFrameLoadHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsFrameLoadHelper::UNIT_SYMBOLS);
}


SatStatsFrameSymbolLoadHelper::~SatStatsFrameSymbolLoadHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFrameSymbolLoadHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFrameSymbolLoadHelper")
    .SetParent<SatStatsFrameLoadHelper> ()
  ;
  return tid;
}


// IN USER UNIT ///////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFrameUserLoadHelper);

SatStatsFrameUserLoadHelper::SatStatsFrameUserLoadHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFrameLoadHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsFrameLoadHelper::UNIT_USERS);
}


SatStatsFrameUserLoadHelper::~SatStatsFrameUserLoadHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFrameUserLoadHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFrameUserLoadHelper")
    .SetParent<SatStatsFrameLoadHelper> ()
  ;
  return tid;
}


} // end of namespace ns3
