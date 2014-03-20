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

#include "satellite-stats-fwd-mac-throughput-helper.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/node-container.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-mac.h>
#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsFwdMacThroughputHelper");


namespace ns3 {

SatStatsFwdMacThroughputHelper::SatStatsFwdMacThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdMacThroughputHelper::~SatStatsFwdMacThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdMacThroughputHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_NONE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCALAR_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName () + ".txt"),
                                         "MultiFileMode", BooleanValue (false));

        // Setup terminal collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);

        // Setup conversion collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &ScalarCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue ("% time_sec throughput_kbps"));

        // Setup terminal collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);

        // Setup conversion collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      /// \todo Add support for boxes in Gnuplot.
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        // Setup aggregator.
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Received throughput (in kilobits per second)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
        m_aggregator = plotAggregator;

        // Setup terminal collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &GnuplotAggregator::Write2d);

        // Setup conversion collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to conversion collectors.
  InstallProbes (m_conversionCollectors,
                 &UnitConversionCollector::TraceSinkUinteger32);

} // end of `void DoInstall ();`


template<typename R, typename C, typename P>
void
SatStatsFwdMacThroughputHelper::InstallProbes (CollectorMap &collectorMap,
                                               R (C::*collectorTraceSink) (P, P))
{
  NS_LOG_FUNCTION (this);
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();

  for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
    {
      const int32_t utId = GetUtId (*it);
      NS_ASSERT_MSG (utId > 0,
                     "Node " << (*it)->GetId () << " is not a valid UT");
      const uint32_t identifier = GetIdentifierForUt (*it);

      // Create the probe.
      std::ostringstream probeName;
      probeName << utId;
      Ptr<ApplicationPacketProbe> probe = CreateObject<ApplicationPacketProbe> ();
      probe->SetName (probeName.str ());

      /*
       * Assuming that device #0 is for loopback device, device #1 is for
       * subscriber network device, and device #2 is for satellite beam device.
       */
      NS_ASSERT ((*it)->GetNDevices () >= 3);
      Ptr<NetDevice> dev = (*it)->GetDevice (2);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();

      if (satDev == 0)
        {
          NS_LOG_WARN (this << " Node " << (*it)->GetId ()
                            << " is not a valid UT");
        }
      else
        {
          Ptr<SatMac> satMac = satDev->GetMac ();
          NS_ASSERT (satMac != 0);

          // Connect the object to the probe.
          if (probe->ConnectByObject ("Rx", satMac))
            {
              // Connect the probe to the right collector.
              if (collectorMap.ConnectWithProbe (probe->GetObject<Probe> (),
                                                 "OutputBytes",
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

            } // end of `if (probe->ConnectByObject ("Rx", satMac))`

        } // end of else of `if (satDev == 0)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void InstallProbes (CollectorMap, R (C::*) (P, P));`


} // end of namespace ns3
