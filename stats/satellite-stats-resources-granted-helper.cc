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
#include <ns3/simulator.h>
#include <ns3/fatal-error.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>

#include <ns3/node-container.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-ut-mac.h>
#include <ns3/satellite-helper.h>

#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/bytes-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>

#include "satellite-stats-resources-granted-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsResourcesGrantedHelper");


namespace ns3 {


SatStatsResourcesGrantedHelper::SatStatsResourcesGrantedHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
    m_distributionMinValue (0.0),
    m_distributionMaxValue (20000.0),
    m_distributionBinLength (500.0)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsResourcesGrantedHelper::~SatStatsResourcesGrantedHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsResourcesGrantedHelper::DoInstall ()
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

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_UINTEGER));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
          {
            InstallProbe (*it, &ScalarCollector::TraceSinkUinteger32);
          }

        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue ("% time_sec resources_bytes"));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::UnitConversionCollector");
        m_terminalCollectors.SetAttribute ("ConversionType",
                                           EnumValue (UnitConversionCollector::TRANSPARENT));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputTimeValue",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
          {
            InstallProbe (*it, &UnitConversionCollector::TraceSinkUinteger32);
          }

        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue ("% resources_bytes freq"));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::DistributionCollector");
        DistributionCollector::OutputType_t outputType
          = DistributionCollector::OUTPUT_TYPE_HISTOGRAM;
        if (GetOutputType () == SatStatsHelper::OUTPUT_PDF_FILE)
          {
            outputType = DistributionCollector::OUTPUT_TYPE_PROBABILITY;
          }
        else if (GetOutputType () == SatStatsHelper::OUTPUT_CDF_FILE)
          {
            outputType = DistributionCollector::OUTPUT_TYPE_CUMULATIVE;
          }
        m_terminalCollectors.SetAttribute ("OutputType", EnumValue (outputType));
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (m_distributionMinValue));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (m_distributionMaxValue));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (m_distributionBinLength));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator ("OutputString",
                                                  m_aggregator,
                                                  &MultiFileAggregator::AddContextHeading);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
          {
            InstallProbe (*it, &DistributionCollector::TraceSinkUinteger32);
          }

        break;
      }

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
                                   "Resources granted (in bytes)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES_POINTS);
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

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
          {
            InstallProbe (*it, &UnitConversionCollector::TraceSinkUinteger32);
          }

        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      {
        // Setup aggregator.
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Resources granted (in bytes)",
                                   "Frequency");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
        m_aggregator = plotAggregator;

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::DistributionCollector");
        DistributionCollector::OutputType_t outputType
          = DistributionCollector::OUTPUT_TYPE_HISTOGRAM;
        if (GetOutputType () == SatStatsHelper::OUTPUT_PDF_PLOT)
          {
            outputType = DistributionCollector::OUTPUT_TYPE_PROBABILITY;
          }
        else if (GetOutputType () == SatStatsHelper::OUTPUT_CDF_PLOT)
          {
            outputType = DistributionCollector::OUTPUT_TYPE_CUMULATIVE;
          }
        m_terminalCollectors.SetAttribute ("OutputType", EnumValue (outputType));
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (m_distributionMinValue));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (m_distributionMaxValue));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (m_distributionBinLength));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &GnuplotAggregator::Write2d);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
          {
            InstallProbe (*it, &DistributionCollector::TraceSinkUinteger32);
          }

        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsResourcesGrantedHelper - Invalid output type");
      break;
    }

} // end of `void DoInstall ();`


template<typename R, typename C, typename P>
void
SatStatsResourcesGrantedHelper::InstallProbe (Ptr<Node> utNode,
                                              R (C::*collectorTraceSink) (P, P))
{
  NS_LOG_FUNCTION (this << utNode);

  const int32_t utId = GetUtId (utNode);
  NS_ASSERT_MSG (utId > 0,
                 "Node " << utNode->GetId () << " is not a valid UT");
  const uint32_t identifier = GetIdentifierForUt (utNode);

  // Create the probe.
  std::ostringstream probeName;
  probeName << utId;
  Ptr<BytesProbe> probe = CreateObject<BytesProbe> ();
  probe->SetName (probeName.str ());

  /*
   * Assuming that device #0 is for loopback device, device #1 is for
   * subscriber network device, and device #2 is for satellite beam device.
   */
  NS_ASSERT (utNode->GetNDevices () >= 3);
  Ptr<NetDevice> dev = utNode->GetDevice (2);
  Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();

  if (satDev == 0)
    {
      NS_LOG_WARN (this << " Node " << utNode->GetId ()
                        << " is not a valid UT");
    }
  else
    {
      Ptr<SatMac> satMac = satDev->GetMac ();
      NS_ASSERT (satMac != 0);
      Ptr<SatUtMac> satUtMac = satMac->GetObject<SatUtMac> ();
      NS_ASSERT (satUtMac != 0);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("DaResourcesTrace", satUtMac))
        {
          // Connect the probe to the right collector.
          if (m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                     "Output",
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
          NS_FATAL_ERROR ("Error connecting to DaResourcesTrace trace source of SatUtMac"
                          << " at node ID " << utNode->GetId () << " device #2");
        }

    } // end of else of `if (satDev == 0)`

} // end of `void InstallProbe (Ptr<Node>, R (C::*) (P, P));`


} // end of namespace ns3
