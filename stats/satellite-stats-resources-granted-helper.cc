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
#include <ns3/magister-gnuplot-aggregator.h>

#include "satellite-stats-resources-granted-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsResourcesGrantedHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsResourcesGrantedHelper);

SatStatsResourcesGrantedHelper::SatStatsResourcesGrantedHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsResourcesGrantedHelper::~SatStatsResourcesGrantedHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsResourcesGrantedHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsResourcesGrantedHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
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
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "MultiFileMode", BooleanValue (false),
                                         "EnableContextPrinting", BooleanValue (true),
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("resources_bytes")));

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
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
          {
            InstallProbe (*it, &ScalarCollector::TraceSinkUinteger32);
          }

        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetTimeHeading ("resources_bytes")));

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
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
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
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetDistributionHeading ("resources_bytes")));

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
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator ("OutputString",
                                                  m_aggregator,
                                                  &MultiFileAggregator::AddContextHeading);
        m_terminalCollectors.ConnectToAggregator ("Warning",
                                                  m_aggregator,
                                                  &MultiFileAggregator::EnableContextWarning);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
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
        m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                         "OutputPath", StringValue (GetOutputPath ()),
                                         "OutputFileName", StringValue (GetName ()));
        Ptr<MagisterGnuplotAggregator> plotAggregator
          = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
        NS_ASSERT (plotAggregator != 0);
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Resources granted (in bytes)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES_POINTS);

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
                                                  &MagisterGnuplotAggregator::Write2d);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
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
        m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                         "OutputPath", StringValue (GetOutputPath ()),
                                         "OutputFileName", StringValue (GetName ()));
        Ptr<MagisterGnuplotAggregator> plotAggregator
          = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
        NS_ASSERT (plotAggregator != 0);
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Resources granted (in bytes)",
                                   "Frequency");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

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
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MagisterGnuplotAggregator::Write2d);

        // Setup a probe in each UT MAC.
        NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
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
                                              R (C::*collectorTraceSink)(P, P))
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

  Ptr<NetDevice> dev = GetUtSatNetDevice (utNode);
  Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
  NS_ASSERT (satDev != 0);
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
          NS_LOG_INFO (this << " created probe " << probeName.str ()
                            << ", connected to collector " << identifier);
          m_probes.push_back (probe->GetObject<Probe> ());
        }
      else
        {
          NS_LOG_WARN (this << " unable to connect probe " << probeName.str ()
                            << " to collector " << identifier);
        }

    }
  else
    {
      NS_FATAL_ERROR ("Error connecting to DaResourcesTrace trace source of SatUtMac"
                      << " at node ID " << utNode->GetId ()
                      << " device #" << satDev->GetIfIndex ());
    }

} // end of `void InstallProbe (Ptr<Node>, R (C::*) (P, P));`


} // end of namespace ns3
