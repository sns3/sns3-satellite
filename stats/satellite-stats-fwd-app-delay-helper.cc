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

#include "satellite-stats-fwd-app-delay-helper.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/node-container.h>
#include <ns3/application.h>
#include <ns3/satellite-helper.h>
#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-delay-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsFwdAppDelayHelper");


namespace ns3 {

SatStatsFwdAppDelayHelper::SatStatsFwdAppDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}


SatStatsFwdAppDelayHelper::~SatStatsFwdAppDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdAppDelayHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_NONE:
      break;

    case SatStatsHelper::OUTPUT_SCALAR_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName () + ".txt"),
                                         "MultiFileMode", BooleanValue (false));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);

        // Setup probes.
        InstallProbes (m_terminalCollectors,
                       &ScalarCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::UnitConversionCollector");
        m_terminalCollectors.SetAttribute ("ConversionType",
                                           EnumValue (UnitConversionCollector::TRANSPARENT));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputTimeValue",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);

        // Setup probes.
        InstallProbes (m_terminalCollectors,
                       &UnitConversionCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      break;

    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      // TODO: Add support for boxes in Gnuplot.
      break;

    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        // Setup aggregator.
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Packet delay (in seconds)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
        m_aggregator = plotAggregator;

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::UnitConversionCollector");
        m_terminalCollectors.SetAttribute ("ConversionType",
                                           EnumValue (UnitConversionCollector::TRANSPARENT));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("OutputTimeValue",
                                                  m_aggregator,
                                                  &GnuplotAggregator::Write2d);

        // Setup probes.
        InstallProbes (m_terminalCollectors,
                       &UnitConversionCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
    }

} // end of `void DoInstall ();`


template<typename R, typename C, typename P>
void
SatStatsFwdAppDelayHelper::InstallProbes (CollectorMap &collectorMap,
                                          R (C::*collectorTraceSink) (P, P))
{
  NS_LOG_FUNCTION (this);
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();

  for (NodeContainer::Iterator it = utUsers.Begin(); it != utUsers.End (); ++it)
    {
      const int32_t utUserId = GetUtUserId (*it);
      const uint32_t identifier = GetIdentifierForUtUser (*it);

      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          // Create the probe.
          std::ostringstream probeName;
          probeName << utUserId << "-" << i;
          Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
          probe->SetName (probeName.str ());

          // Connect the object to the probe.
          if (probe->ConnectByObject ("RxDelay", (*it)->GetApplication (i)))
            {
              // Connect the probe to the right collector.
              if (collectorMap.ConnectWithProbe (probe->GetObject<Probe> (),
                                                 "OutputSeconds",
                                                 identifier,
                                                 collectorTraceSink))
                {
                  NS_LOG_INFO (this << " created probe " << probeName
                                    << ", connected to collector " << identifier);
                  m_probes.push_back (probe->GetObject<Probe> ());
                }
              else
                {
                  NS_LOG_WARN (this << " unable to connect probe " << probeName
                                    << " to collector " << identifier);
                }
            }
          else
            {
              NS_LOG_WARN (this << " unable to connect probe " << probeName
                                << " to the source application");
            }

        } // end of `for (i = 0; i < (*it)->GetNApplications (); i++)`

    } // end of `for (it = utUsers.Begin(); it != utUsers.End (); ++it)`

} // end of `void InstallProbes (CollectorMap, R (C::*) (P, P));`


} // end of namespace ns3
