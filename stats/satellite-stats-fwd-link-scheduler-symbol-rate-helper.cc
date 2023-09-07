/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#include "satellite-stats-fwd-link-scheduler-symbol-rate-helper.h"

#include <ns3/boolean.h>
#include <ns3/data-collection-object.h>
#include <ns3/distribution-collector.h>
#include <ns3/enum.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/log.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/node-container.h>
#include <ns3/object-map.h>
#include <ns3/object-vector.h>
#include <ns3/pointer.h>
#include <ns3/satellite-fwd-link-scheduler.h>
#include <ns3/satellite-gw-mac.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-net-device.h>
#include <ns3/scalar-collector.h>
#include <ns3/string.h>
#include <ns3/unit-conversion-collector.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatStatsFwdLinkSchedulerSymbolRateHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdLinkSchedulerSymbolRateHelper);

SatStatsFwdLinkSchedulerSymbolRateHelper::SatStatsFwdLinkSchedulerSymbolRateHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsHelper(satHelper),
      m_traceSinkCallback(
          MakeCallback(&SatStatsFwdLinkSchedulerSymbolRateHelper::SymbolRateCallback, this))
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdLinkSchedulerSymbolRateHelper::~SatStatsFwdLinkSchedulerSymbolRateHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdLinkSchedulerSymbolRateHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdLinkSchedulerSymbolRateHelper").SetParent<SatStatsHelper>();
    return tid;
}

void
SatStatsFwdLinkSchedulerSymbolRateHelper::SymbolRateCallback(uint8_t sliceId, double symbolRate)
{
    NS_LOG_FUNCTION(this << sliceId << " " << symbolRate);

    Ptr<DataCollectionObject> collector = NULL;

    switch (GetIdentifierType())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL: {
        collector = m_collectors.Get(0);
        NS_ASSERT_MSG(collector != nullptr, "Unable to find collector with identifier 0");
        break;
    }
    case SatStatsHelper::IDENTIFIER_SLICE: {
        collector = m_collectors.Get(static_cast<uint32_t>(sliceId));
        NS_ASSERT_MSG(collector != nullptr, "Unable to find collector with identifier " << sliceId);
        break;
    }
    default:
        NS_FATAL_ERROR("SatStatsFwdLinkSchedulerSymbolRateHelper - Invalid identifier type");
        break;
    }

    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT: {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, symbolRate);
        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
        Ptr<IntervalRateCollector> c = collector->GetObject<IntervalRateCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, symbolRate);
        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT: {
        Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, symbolRate);
        break;
    }

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    } // end of `switch (GetOutputType ())`

} // end of `void RxPowerCallback (double);`

Callback<void, uint8_t, double>
SatStatsFwdLinkSchedulerSymbolRateHelper::GetTraceSinkCallback() const
{
    return m_traceSinkCallback;
}

void
SatStatsFwdLinkSchedulerSymbolRateHelper::DoInstall()
{
    NS_LOG_FUNCTION(this);

    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_NONE:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    case SatStatsHelper::OUTPUT_SCALAR_FILE: {
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                        "OutputFileName",
                                        StringValue(GetOutputFileName()),
                                        "MultiFileMode",
                                        BooleanValue(false),
                                        "EnableContextPrinting",
                                        BooleanValue(true),
                                        "GeneralHeading",
                                        StringValue(GetIdentifierHeading("symbol_rate_baud")));
        Ptr<MultiFileAggregator> aggregator = m_aggregator->GetObject<MultiFileAggregator>();

        // Setup collectors.
        m_collectors.SetType("ns3::ScalarCollector");
        m_collectors.SetAttribute("InputDataType",
                                  EnumValue(ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_collectors.SetAttribute("OutputType",
                                  EnumValue(ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        CreateCollectorPerIdentifier(m_collectors);
        m_collectors.ConnectToAggregator("Output", m_aggregator, &MultiFileAggregator::Write1d);

        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE: {
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                        "OutputFileName",
                                        StringValue(GetOutputFileName()),
                                        "GeneralHeading",
                                        StringValue(GetTimeHeading("symbol_rate_baud")));
        Ptr<MultiFileAggregator> aggregator = m_aggregator->GetObject<MultiFileAggregator>();

        // Setup collectors
        m_collectors.SetType("ns3::IntervalRateCollector");
        m_collectors.SetAttribute("InputDataType",
                                  EnumValue(IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier(m_collectors);
        m_collectors.ConnectToAggregator("OutputWithTime",
                                         m_aggregator,
                                         &MultiFileAggregator::Write2d);
        m_collectors.ConnectToAggregator("OutputString",
                                         m_aggregator,
                                         &MultiFileAggregator::AddContextHeading);

        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE: {
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                        "OutputFileName",
                                        StringValue(GetOutputFileName()),
                                        "GeneralHeading",
                                        StringValue(GetDistributionHeading("symbol_rate_baud")));
        Ptr<MultiFileAggregator> aggregator = m_aggregator->GetObject<MultiFileAggregator>();

        // Setup collectors.
        m_collectors.SetType("ns3::DistributionCollector");
        DistributionCollector::OutputType_t outputType =
            DistributionCollector::OUTPUT_TYPE_HISTOGRAM;
        if (GetOutputType() == SatStatsHelper::OUTPUT_PDF_FILE)
        {
            outputType = DistributionCollector::OUTPUT_TYPE_PROBABILITY;
        }
        else if (GetOutputType() == SatStatsHelper::OUTPUT_CDF_FILE)
        {
            outputType = DistributionCollector::OUTPUT_TYPE_CUMULATIVE;
        }
        m_collectors.SetAttribute("OutputType", EnumValue(outputType));
        CreateCollectorPerIdentifier(m_collectors);
        m_collectors.ConnectToAggregator("Output", m_aggregator, &MultiFileAggregator::Write2d);
        m_collectors.ConnectToAggregator("OutputString",
                                         m_aggregator,
                                         &MultiFileAggregator::AddContextHeading);
        m_collectors.ConnectToAggregator("Warning",
                                         m_aggregator,
                                         &MultiFileAggregator::EnableContextWarning);

        break;
    }

    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
        /// \todo Add support for boxes in Gnuplot.
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MagisterGnuplotAggregator",
                                        "OutputPath",
                                        StringValue(GetOutputPath()),
                                        "OutputFileName",
                                        StringValue(GetName()));
        Ptr<MagisterGnuplotAggregator> plotAggregator =
            m_aggregator->GetObject<MagisterGnuplotAggregator>();
        NS_ASSERT(plotAggregator != nullptr);
        // plot->SetTitle ("");
        plotAggregator->SetLegend("Time (in seconds)", "Latency (in seconds)");
        plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_collectors.SetType("ns3::IntervalRateCollector");
        m_collectors.SetAttribute("InputDataType",
                                  EnumValue(IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier(m_collectors);
        for (CollectorMap::Iterator it = m_collectors.Begin(); it != m_collectors.End(); ++it)
        {
            const std::string context = it->second->GetName();
            plotAggregator->Add2dDataset(context, context);
        }
        m_collectors.ConnectToAggregator("OutputWithTime",
                                         m_aggregator,
                                         &MagisterGnuplotAggregator::Write2d);

        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT: {
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MagisterGnuplotAggregator",
                                        "OutputPath",
                                        StringValue(GetOutputPath()),
                                        "OutputFileName",
                                        StringValue(GetName()));
        Ptr<MagisterGnuplotAggregator> plotAggregator =
            m_aggregator->GetObject<MagisterGnuplotAggregator>();
        NS_ASSERT(plotAggregator != nullptr);
        // plot->SetTitle ("");
        plotAggregator->SetLegend("Latency (in seconds)", "Frequency");
        plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_collectors.SetType("ns3::DistributionCollector");
        DistributionCollector::OutputType_t outputType =
            DistributionCollector::OUTPUT_TYPE_HISTOGRAM;
        if (GetOutputType() == SatStatsHelper::OUTPUT_PDF_PLOT)
        {
            outputType = DistributionCollector::OUTPUT_TYPE_PROBABILITY;
        }
        else if (GetOutputType() == SatStatsHelper::OUTPUT_CDF_PLOT)
        {
            outputType = DistributionCollector::OUTPUT_TYPE_CUMULATIVE;
        }
        m_collectors.SetAttribute("OutputType", EnumValue(outputType));
        CreateCollectorPerIdentifier(m_collectors);
        for (CollectorMap::Iterator it = m_collectors.Begin(); it != m_collectors.End(); ++it)
        {
            const std::string context = it->second->GetName();
            plotAggregator->Add2dDataset(context, context);
        }
        m_collectors.ConnectToAggregator("Output",
                                         m_aggregator,
                                         &MagisterGnuplotAggregator::Write2d);

        break;
    }

    default:
        NS_FATAL_ERROR("SatStatsFwdLinkSchedulerSymbolRateHelper - Invalid output type");
        break;
    }

    // Setup probes and connect them to the collectors.
    InstallProbes();

} // end of `void DoInstall ();`

void
SatStatsFwdLinkSchedulerSymbolRateHelper::InstallProbes()
{
    NS_LOG_FUNCTION(this);

    // Connect to trace sources at GW nodes.
    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();

    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);
            Ptr<SatMac> satMac = satDev->GetMac();
            NS_ASSERT(satMac != nullptr);
            Ptr<SatGwMac> satGwMac = satMac->GetObject<SatGwMac>();
            NS_ASSERT(satGwMac != nullptr);
            PointerValue scheduler;
            satGwMac->GetAttribute("Scheduler", scheduler);
            Ptr<SatFwdLinkScheduler> fwdLinkScheduler = scheduler.Get<SatFwdLinkScheduler>();
            NS_ASSERT(fwdLinkScheduler != nullptr);

            if (!fwdLinkScheduler->TraceConnectWithoutContext("SymbolRate", GetTraceSinkCallback()))
            {
                NS_FATAL_ERROR("Error connecting to Symbol Rate trace source"
                               << " of SatFwdLinkScheduler"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << (*itDev)->GetIfIndex());
            }

        } // end of `for (NetDeviceCOntainer::Iterator itDev = devs)`

    } // end of `for (it = gws.Begin (); it != gws.End (); ++it)`
}

} // end of namespace ns3
