/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 */

#include "satellite-stats-packet-drop-rate-helper.h"

#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/data-collection-object.h>
#include <ns3/distribution-collector.h>
#include <ns3/enum.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/log.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/probe.h>
#include <ns3/satellite-handover-module.h>
#include <ns3/satellite-helper.h>
#include <ns3/scalar-collector.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/unit-conversion-collector.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatStatsPacketDropRateHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatStatsPacketDropRateHelper);

SatStatsPacketDropRateHelper::SatStatsPacketDropRateHelper(Ptr<const SatHelper> satHelper)
    : SatStatsHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsPacketDropRateHelper::~SatStatsPacketDropRateHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SatStatsPacketDropRateHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatStatsPacketDropRateHelper").SetParent<SatStatsHelper>();
    return tid;
}

void
SatStatsPacketDropRateHelper::PacketDropRateCallback(uint32_t nPackets,
                                                     Ptr<Node> satSrc,
                                                     Ptr<Node> satDst,
                                                     bool isError)
{
    NS_LOG_FUNCTION(this << nPackets << satSrc << satDst << isError);

    uint32_t identifier = GetIdentifierForIsl(satSrc, satDst);

    // Find the first-level collector with the right identifier.
    Ptr<DataCollectionObject> collector = m_terminalCollectors.Get(identifier);
    NS_ASSERT_MSG(collector != nullptr, "Unable to find collector with identifier " << identifier);

    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT: {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkBoolean(false, isError);
        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
        Ptr<IntervalRateCollector> c = collector->GetObject<IntervalRateCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkBoolean(false, isError);
        break;
    }

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    } // end of `switch (GetOutputType ())`
}

void
SatStatsPacketDropRateHelper::DoInstall()
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
                                        StringValue(GetIdentifierHeading("packet_drop_rate")));

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute("InputDataType",
                                          EnumValue(ScalarCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute(
            "OutputType",
            EnumValue(ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier(m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator("Output",
                                                 m_aggregator,
                                                 &MultiFileAggregator::Write1d);
        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE: {
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                        "OutputFileName",
                                        StringValue(GetOutputFileName()),
                                        "GeneralHeading",
                                        StringValue(GetTimeHeading("packet_drop_rate")));

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute(
            "InputDataType",
            EnumValue(IntervalRateCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute(
            "OutputType",
            EnumValue(IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier(m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator("OutputWithTime",
                                                 m_aggregator,
                                                 &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator("OutputString",
                                                 m_aggregator,
                                                 &MultiFileAggregator::AddContextHeading);
        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

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
        plotAggregator->SetLegend("Time (in seconds)", "Packet drop rate");
        plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute(
            "InputDataType",
            EnumValue(IntervalRateCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute(
            "OutputType",
            EnumValue(IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier(m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin();
             it != m_terminalCollectors.End();
             ++it)
        {
            const std::string context = it->second->GetName();
            plotAggregator->Add2dDataset(context, context);
        }
        m_terminalCollectors.ConnectToAggregator("OutputWithTime",
                                                 m_aggregator,
                                                 &MagisterGnuplotAggregator::Write2d);
        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    default:
        NS_FATAL_ERROR("SatStatsPacketErrorHelper - Invalid output type");
        break;
    }

    InstallProbes();
}

void
SatStatsPacketDropRateHelper::InstallProbes()
{
    NS_LOG_FUNCTION(this);

    Callback<void, uint32_t, Ptr<Node>, Ptr<Node>, bool> callback =
        MakeCallback(&SatStatsPacketDropRateHelper::PacketDropRateCallback, this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<SatGeoNetDevice> satGeoNetDevice =
            DynamicCast<SatGeoNetDevice>(GetSatSatGeoNetDevice(*it));
        std::vector<Ptr<PointToPointIslNetDevice>> islNetDevices =
            satGeoNetDevice->GetIslsNetDevices();
        for (std::vector<Ptr<PointToPointIslNetDevice>>::iterator itIsl = islNetDevices.begin();
             itIsl != islNetDevices.end();
             itIsl++)
        {
            std::ostringstream oss;
            oss << GetIdentifierForIsl(*it, (*itIsl)->GetDestinationNode());

            if ((*itIsl)->TraceConnectWithoutContext("PacketDropRateTrace", callback))
            {
                NS_LOG_INFO(this << " successfully connected with ISL " << oss.str());
            }
            else
            {
                NS_LOG_WARN(this << " unable to connect to ISL " << oss.str());
            }
        }
    }
}

} // namespace ns3
