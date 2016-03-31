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
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <sstream>
#include <utility>
#include <list>

#include "satellite-stats-waveform-usage-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsWaveformUsageHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsWaveformUsageHelper);

SatStatsWaveformUsageHelper::SatStatsWaveformUsageHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsWaveformUsageHelper::~SatStatsWaveformUsageHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsWaveformUsageHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsWaveformUsageHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsWaveformUsageHelper::DoInstall ()
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
                                   "GeneralHeading", StringValue (GetIdentifierHeading ("usage_count")));

  Callback<void, std::string, uint32_t> waveformUsageCallback
    = MakeCallback (&SatStatsWaveformUsageHelper::WaveformUsageCallback, this);

  // Setup probes.
  Ptr<SatBeamHelper> beamHelper = GetSatHelper ()->GetBeamHelper ();
  NS_ASSERT (beamHelper != 0);
  Ptr<SatNcc> ncc = beamHelper->GetNcc ();
  NS_ASSERT (ncc != 0);
  std::list<uint32_t> beams = beamHelper->GetBeams ();

  for (std::list<uint32_t>::const_iterator it = beams.begin ();
       it != beams.end (); ++it)
    {
      std::ostringstream context;
      context << GetIdentifierForBeam (*it);

      Ptr<SatBeamScheduler> s = ncc->GetBeamScheduler (*it);
      NS_ASSERT_MSG (s != 0, "Error finding beam " << *it);
      const bool ret = s->TraceConnect ("WaveformTrace",
                                        context.str (), waveformUsageCallback);
      NS_ASSERT_MSG (ret,
                     "Error connecting to WaveformTrace of beam " << *it);
      NS_UNUSED (ret);
      NS_LOG_INFO (this << " successfully connected"
                        << " with beam " << *it);
    }

} // end of `void DoInstall ();`


std::string
SatStatsWaveformUsageHelper::GetIdentifierHeading (std::string dataLabel) const
{
  switch (GetIdentifierType ())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return "% global waveform_id " + dataLabel;

    case SatStatsHelper::IDENTIFIER_GW:
      return "% gw_id waveform_id " + dataLabel;

    case SatStatsHelper::IDENTIFIER_BEAM:
      return "% beam_id waveform_id " + dataLabel;

    default:
      NS_FATAL_ERROR ("SatStatsWaveformUsageHelper - Invalid identifier type");
      break;
    }
  return "";
}


void
SatStatsWaveformUsageHelper::WaveformUsageCallback (std::string context,
                                                    uint32_t waveformId)
{
  NS_LOG_FUNCTION (this << context << waveformId);

  // convert context to number
  std::stringstream ss (context);
  uint32_t identifier;
  if (!(ss >> identifier))
    {
      NS_FATAL_ERROR ("Cannot convert '" << context << "' to number");
    }

  std::map<uint32_t, CollectorMap>::iterator it = m_collectors.find (waveformId);
  if (it == m_collectors.end ())
    {
      // Newly discovered waveform ID
      NS_LOG_INFO (this << " Creating new collectors for waveform ID " << waveformId);
      CollectorMap collectorMap;
      collectorMap.SetType ("ns3::ScalarCollector");
      collectorMap.SetAttribute ("InputDataType",
                                 EnumValue (ScalarCollector::INPUT_DATA_TYPE_UINTEGER));
      collectorMap.SetAttribute ("OutputType",
                                 EnumValue (ScalarCollector::OUTPUT_TYPE_SUM));
      uint32_t n = 0;

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
            name << "0 " << waveformId;
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
                name << gwId << " " << waveformId;
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
                name << beamId << " " << waveformId;
                collectorMap.SetAttribute ("Name", StringValue (name.str ()));
                collectorMap.Create (beamId);
                n++;
              }
            break;
          }

        default:
          NS_FATAL_ERROR ("SatStatsWaveformUsageHelper - Invalid identifier type");
          break;
        }

      collectorMap.ConnectToAggregator ("Output",
                                        m_aggregator,
                                        &MultiFileAggregator::Write1d);
      NS_LOG_INFO (this << " created " << n << " instance(s)"
                        << " of " << collectorMap.GetType ().GetName ()
                        << " for " << GetIdentifierTypeName (GetIdentifierType ()));

      std::pair<std::map<uint32_t, CollectorMap>::iterator, bool> ret;
      ret = m_collectors.insert (std::make_pair (waveformId, collectorMap));
      NS_ASSERT (ret.second);
      it = ret.first;

    } // end of `if (it == m_collectors.end ())`

  NS_ASSERT (it != m_collectors.end ());

  // Find the collector with the right identifier.
  Ptr<DataCollectionObject> collector = it->second.Get (identifier);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to find collector with identifier " << identifier);
  Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
  NS_ASSERT (c != 0);

  // Pass the sample to the collector.
  c->TraceSinkUinteger32 (0, 1);

} // end of `void WaveformUsageCallback (std::string, uint32_t)`


} // end of namespace ns3
