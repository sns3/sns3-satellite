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

#include "satellite-stats-fwd-throughput-helper.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/node-container.h>
#include <ns3/application.h>
#include <ns3/satellite-helper.h>
#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
#include <ns3/scalar-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsFwdThroughputHelper");


namespace ns3 {

SatStatsFwdThroughputHelper::SatStatsFwdThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}


SatStatsFwdThroughputHelper::~SatStatsFwdThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


template<typename R, typename C, typename P1, typename P2>
void
SatStatsFwdThroughputHelper::InstallProbes (NodeContainer userNodes,
                                            CollectorMap_t &collectorMap,
                                            R (C::*collectorTraceSink) (P1, P2))
{
  NS_LOG_FUNCTION (this);

  for (NodeContainer::Iterator it = userNodes.Begin ();
       it != userNodes.End (); ++it)
    {
      const int32_t utUserId = GetUtUserId (*it);
      const uint32_t identifier = GetUtUserIdentifier (*it);

      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          std::ostringstream probeName;
          probeName << utUserId << "-" << i;
          Ptr<Probe> probe = InstallProbe ((*it)->GetApplication (i),
                                           "Rx",
                                           probeName.str (),
                                           "ns3::ApplicationPacketProbe",
                                           "OutputBytes",
                                           identifier,
                                           collectorMap,
                                           collectorTraceSink);
          if (probe != 0)
            {
              NS_LOG_INFO (this << " created probe " << probeName
                                << ", connected to collector " << identifier);
              m_probes.push_back (probe);
            }
          else
            {
              NS_LOG_WARN (this << " unable to create probe " << probeName
                                << " nor connect it to collector " << identifier);
            }
        }
    }
}


template<typename R, typename C, typename P1, typename V1>
void
SatStatsFwdThroughputHelper::ConnectCollectorsToAggregator (CollectorMap_t &collectorMap,
                                                            std::string collectorTraceSourceName,
                                                            Ptr<DataCollectionObject> aggregator,
                                                            R (C::*aggregatorTraceSink) (P1, V1)) const
{
  NS_LOG_FUNCTION (this);

  for (SatStatsHelper::CollectorMap_t::iterator it = collectorMap.begin ();
       it != collectorMap.end (); ++it)
    {
      Ptr<DataCollectionObject> collector = it->second;
      NS_ASSERT (it->second != 0);
      const std::string context = collector->GetName ();
      Ptr<C> target = aggregator->GetObject<C> ();
      NS_ASSERT (target != 0);
      if (collector->TraceConnect (collectorTraceSourceName,
                                   context,
                                   MakeCallback (aggregatorTraceSink,
                                                 target)))
        {
          NS_LOG_INFO (this << " succesfully connected collector " << context
                            << " to aggregator");
        }
      else
        {
          NS_LOG_WARN (this << " unable to connect collector " << context
                            << " to aggregator");
        }
    }
}


template<typename R, typename C, typename P1, typename V1, typename V2>
void
SatStatsFwdThroughputHelper::ConnectCollectorsToAggregator (CollectorMap_t &collectorMap,
                                                            std::string collectorTraceSourceName,
                                                            Ptr<DataCollectionObject> aggregator,
                                                            R (C::*aggregatorTraceSink) (P1, V1, V2)) const
{
  NS_LOG_FUNCTION (this);

  for (SatStatsHelper::CollectorMap_t::iterator it = collectorMap.begin ();
       it != collectorMap.end (); ++it)
    {
      Ptr<DataCollectionObject> collector = it->second;
      NS_ASSERT (it->second != 0);
      const std::string context = collector->GetName ();
      Ptr<C> target = aggregator->GetObject<C> ();
      NS_ASSERT (target != 0);
      if (collector->TraceConnect (collectorTraceSourceName,
                                   context,
                                   MakeCallback (aggregatorTraceSink,
                                                 target)))
        {
          NS_LOG_INFO (this << " succesfully connected collector " << context
                            << " to aggregator");
        }
      else
        {
          NS_LOG_WARN (this << " unable to connect collector " << context
                            << " to aggregator");
        }
    }
}


void
SatStatsFwdThroughputHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case OUTPUT_NONE:
      break;

    case OUTPUT_SCALAR_FILE:
      {
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName () + ".txt"),
                                         "MultiFileMode", BooleanValue (false));
        CreateCollectors ("ns3::ScalarCollector",
                          m_terminalCollectors,
                          "InputDataType",
                          EnumValue (ScalarCollector::INPUT_DATA_TYPE_UINTEGER),
                          "OutputType",
                          EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        ConnectCollectorsToAggregator (m_terminalCollectors,
                                       "Output",
                                       m_aggregator,
                                       &MultiFileAggregator::Write1d);
        InstallProbes (GetSatHelper ()->GetUtUsers (),
                       m_terminalCollectors,
                       &ScalarCollector::TraceSinkUinteger32);
        break;
      }

    case OUTPUT_SCATTER_FILE:
      {
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()));
        CreateCollectors ("ns3::IntervalRateCollector",
                          m_terminalCollectors,
                          "InputDataType",
                          EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_UINTEGER));
        ConnectCollectorsToAggregator (m_terminalCollectors,
                                       "OutputWithTime",
                                       m_aggregator,
                                       &MultiFileAggregator::Write2d);
        InstallProbes (GetSatHelper ()->GetUtUsers (),
                       m_terminalCollectors,
                       &IntervalRateCollector::TraceSinkUinteger32);
        break;
      }

    case OUTPUT_HISTOGRAM_FILE:
    case OUTPUT_PDF_FILE:
    case OUTPUT_CDF_FILE:
    case OUTPUT_SCALAR_PLOT:
    case OUTPUT_SCATTER_PLOT:
    case OUTPUT_HISTOGRAM_PLOT:
    case OUTPUT_PDF_PLOT:
    case OUTPUT_CDF_PLOT:
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
    }

} // end of `void DoInstall ();`


} // end of namespace ns3

