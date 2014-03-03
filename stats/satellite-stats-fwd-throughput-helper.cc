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
#include <ns3/node-container.h>
#include <ns3/application.h>
#include <ns3/satellite-helper.h>
#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
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


void
SatStatsFwdThroughputHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  // Create interval rate collectors.
  CreateCollectors ("ns3::IntervalRateCollector", m_intervalRateCollectors);

  // Create a probe for each UT user's application inside the container.
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();
  for (NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); ++it)
    {
      const int32_t utUserId = GetUtUserId (*it);
      const uint32_t identifier = GetUtUserIdentifier (*it);

      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          std::ostringstream probeName;
          probeName << utUserId << "-" << i;
          Ptr<Probe> probe = InstallProbe ((*it)->GetApplication (i),
                                           "ns3::Application",
                                           "Rx",
                                           probeName.str (),
                                           "ns3::ApplicationPacketProbe",
                                           "OutputBytes",
                                           identifier,
                                           m_intervalRateCollectors,
                                           &IntervalRateCollector::TraceSinkUinteger32);
          if (probe != 0)
            {
              m_probes.push_back (probe);
            }
        }
    }

  CreateAggregator ();

  // Connect the terminal collectors to the aggregator and the interval collectors.
  for (SatStatsHelper::CollectorMap_t::iterator it = m_intervalRateCollectors.begin ();
       it != m_intervalRateCollectors.end (); ++it)
    {
      Ptr<DataCollectionObject> collector = it->second;
      NS_ASSERT (it->second != 0);
      const std::string context = collector->GetName ();

      switch (GetOutputType ())
      {
        case OUTPUT_NONE:
        case OUTPUT_SCALAR_FILE:
          break;

        case OUTPUT_SCATTER_FILE:
          {
            Ptr<MultiFileAggregator> aggregator
              = GetAggregator ()->GetObject<MultiFileAggregator> ();
            NS_ASSERT (aggregator != 0);
            bool ret = collector->TraceConnect ("OutputWithTime",
                                                context,
                                                MakeCallback (&MultiFileAggregator::Write2d,
                                                              aggregator));
            NS_ASSERT_MSG (ret,
                           "Failed to connect collector " << context
                                                          << " to aggregator");
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
    }

} // end of `void DoInstall ();`


} // end of namespace ns3

