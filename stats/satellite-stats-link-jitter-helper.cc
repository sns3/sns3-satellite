/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#include "satellite-stats-link-jitter-helper.h"

#include <ns3/application-delay-probe.h>
#include <ns3/application.h>
#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/data-collection-object.h>
#include <ns3/distribution-collector.h>
#include <ns3/enum.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/probe.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-time-tag.h>
#include <ns3/scalar-collector.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/traffic-time-tag.h>
#include <ns3/unit-conversion-collector.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatStatsLinkJitterHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatStatsLinkJitterHelper);

SatStatsLinkJitterHelper::SatStatsLinkJitterHelper(Ptr<const SatHelper> satHelper)
    : SatStatsHelper(satHelper),
      m_averagingMode(false)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsLinkJitterHelper::~SatStatsLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsLinkJitterHelper")
            .SetParent<SatStatsHelper>()
            .AddAttribute("AveragingMode",
                          "If true, all samples will be averaged before passed to aggregator. "
                          "Only affects histogram, PDF, and CDF output types.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatStatsLinkJitterHelper::SetAveragingMode,
                                              &SatStatsLinkJitterHelper::GetAveragingMode),
                          MakeBooleanChecker());
    return tid;
}

void
SatStatsLinkJitterHelper::SetAveragingMode(bool averagingMode)
{
    NS_LOG_FUNCTION(this << averagingMode);
    m_averagingMode = averagingMode;
}

bool
SatStatsLinkJitterHelper::GetAveragingMode() const
{
    return m_averagingMode;
}

void
SatStatsLinkJitterHelper::DoInstall()
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
                                        StringValue(GetIdentifierHeading("jitter_sec")));

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute("InputDataType",
                                          EnumValue(ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
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
                                        StringValue(GetTimeHeading("jitter_sec")));

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::UnitConversionCollector");
        m_terminalCollectors.SetAttribute("ConversionType",
                                          EnumValue(UnitConversionCollector::TRANSPARENT));
        CreateCollectorPerIdentifier(m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator("OutputTimeValue",
                                                 m_aggregator,
                                                 &MultiFileAggregator::Write2d);
        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE: {
        if (m_averagingMode)
        {
            // Setup aggregator.
            m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                            "OutputFileName",
                                            StringValue(GetOutputFileName()),
                                            "MultiFileMode",
                                            BooleanValue(false),
                                            "EnableContextPrinting",
                                            BooleanValue(false),
                                            "GeneralHeading",
                                            StringValue(GetDistributionHeading("jitter_sec")));
            Ptr<MultiFileAggregator> fileAggregator =
                m_aggregator->GetObject<MultiFileAggregator>();
            NS_ASSERT(fileAggregator != nullptr);

            // Setup the final-level collector.
            m_averagingCollector = CreateObject<DistributionCollector>();
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
            m_averagingCollector->SetOutputType(outputType);
            m_averagingCollector->SetName("0");
            m_averagingCollector->TraceConnect(
                "Output",
                "0",
                MakeCallback(&MultiFileAggregator::Write2d, fileAggregator));
            m_averagingCollector->TraceConnect(
                "OutputString",
                "0",
                MakeCallback(&MultiFileAggregator::AddContextHeading, fileAggregator));
            m_averagingCollector->TraceConnect(
                "Warning",
                "0",
                MakeCallback(&MultiFileAggregator::EnableContextWarning, fileAggregator));

            // Setup collectors.
            m_terminalCollectors.SetType("ns3::ScalarCollector");
            m_terminalCollectors.SetAttribute("InputDataType",
                                              EnumValue(ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
            m_terminalCollectors.SetAttribute(
                "OutputType",
                EnumValue(ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
            CreateCollectorPerIdentifier(m_terminalCollectors);
            Callback<void, double> callback =
                MakeCallback(&DistributionCollector::TraceSinkDouble1, m_averagingCollector);
            for (CollectorMap::Iterator it = m_terminalCollectors.Begin();
                 it != m_terminalCollectors.End();
                 ++it)
            {
                it->second->TraceConnectWithoutContext("Output", callback);
            }
        }
        else
        {
            // Setup aggregator.
            m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                            "OutputFileName",
                                            StringValue(GetOutputFileName()),
                                            "GeneralHeading",
                                            StringValue(GetDistributionHeading("jitter_sec")));

            // Setup collectors.
            m_terminalCollectors.SetType("ns3::DistributionCollector");
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
            m_terminalCollectors.SetAttribute("OutputType", EnumValue(outputType));
            CreateCollectorPerIdentifier(m_terminalCollectors);
            m_terminalCollectors.ConnectToAggregator("Output",
                                                     m_aggregator,
                                                     &MultiFileAggregator::Write2d);
            m_terminalCollectors.ConnectToAggregator("OutputString",
                                                     m_aggregator,
                                                     &MultiFileAggregator::AddContextHeading);
            m_terminalCollectors.ConnectToAggregator("Warning",
                                                     m_aggregator,
                                                     &MultiFileAggregator::EnableContextWarning);
        }

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
        plotAggregator->SetLegend("Time (in seconds)", "Packet jitter (in seconds)");
        plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::UnitConversionCollector");
        m_terminalCollectors.SetAttribute("ConversionType",
                                          EnumValue(UnitConversionCollector::TRANSPARENT));
        CreateCollectorPerIdentifier(m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin();
             it != m_terminalCollectors.End();
             ++it)
        {
            const std::string context = it->second->GetName();
            plotAggregator->Add2dDataset(context, context);
        }
        m_terminalCollectors.ConnectToAggregator("OutputTimeValue",
                                                 m_aggregator,
                                                 &MagisterGnuplotAggregator::Write2d);
        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT: {
        if (m_averagingMode)
        {
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
            plotAggregator->SetLegend("Packet jitter (in seconds)", "Frequency");
            plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);
            plotAggregator->Add2dDataset(GetName(), GetName());
            /// \todo Find a better dataset name.

            // Setup the final-level collector.
            m_averagingCollector = CreateObject<DistributionCollector>();
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
            m_averagingCollector->SetOutputType(outputType);
            m_averagingCollector->SetName("0");
            m_averagingCollector->TraceConnect(
                "Output",
                GetName(),
                MakeCallback(&MagisterGnuplotAggregator::Write2d, plotAggregator));
            /// \todo Find a better dataset name.

            // Setup collectors.
            m_terminalCollectors.SetType("ns3::ScalarCollector");
            m_terminalCollectors.SetAttribute("InputDataType",
                                              EnumValue(ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
            m_terminalCollectors.SetAttribute(
                "OutputType",
                EnumValue(ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
            CreateCollectorPerIdentifier(m_terminalCollectors);
            Callback<void, double> callback =
                MakeCallback(&DistributionCollector::TraceSinkDouble1, m_averagingCollector);
            for (CollectorMap::Iterator it = m_terminalCollectors.Begin();
                 it != m_terminalCollectors.End();
                 ++it)
            {
                it->second->TraceConnectWithoutContext("Output", callback);
            }
        }
        else
        {
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
            plotAggregator->SetLegend("Packet jitter (in seconds)", "Frequency");
            plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);

            // Setup collectors.
            m_terminalCollectors.SetType("ns3::DistributionCollector");
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
            m_terminalCollectors.SetAttribute("OutputType", EnumValue(outputType));
            CreateCollectorPerIdentifier(m_terminalCollectors);
            for (CollectorMap::Iterator it = m_terminalCollectors.Begin();
                 it != m_terminalCollectors.End();
                 ++it)
            {
                const std::string context = it->second->GetName();
                plotAggregator->Add2dDataset(context, context);
            }
            m_terminalCollectors.ConnectToAggregator("Output",
                                                     m_aggregator,
                                                     &MagisterGnuplotAggregator::Write2d);
        }

        break;
    }

    default:
        NS_FATAL_ERROR("SatStatsLinkJitterHelper - Invalid output type");
        break;
    }

    // Setup probes and connect them to the collectors.
    InstallProbes();

} // end of `void DoInstall ();`

void
SatStatsLinkJitterHelper::InstallProbes()
{
    // The method below is supposed to be implemented by the child class.
    DoInstallProbes();
}

void
SatStatsLinkJitterHelper::RxLinkJitterCallback(const Time& jitter, const Address& from)
{
    // NS_LOG_FUNCTION (this << jitter.GetSeconds () << from);

    if (from.IsInvalid())
    {
        NS_LOG_WARN(this << " discarding a packet jitter of " << jitter.GetSeconds()
                         << " from statistics collection because of"
                         << " invalid sender address");
    }
    else if (Mac48Address::ConvertFrom(from).IsBroadcast())
    {
        for (std::pair<const Address, uint32_t> item : m_identifierMap)
        {
            PassSampleToCollector(jitter, item.second);
        }
    }
    else
    {
        // Determine the identifier associated with the sender address.
        std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find(from);

        if (it != m_identifierMap.end())
        {
            PassSampleToCollector(jitter, it->second);
        }
        else
        {
            NS_LOG_WARN(this << " discarding a packet jitter of " << jitter.GetSeconds()
                             << " from statistics collection because of"
                             << " unknown sender address " << from);
        }
    }
}

bool
SatStatsLinkJitterHelper::ConnectProbeToCollector(Ptr<Probe> probe, uint32_t identifier)
{
    NS_LOG_FUNCTION(this << probe << probe->GetName() << identifier);

    bool ret = false;
    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
        ret = m_terminalCollectors.ConnectWithProbe(probe,
                                                    "OutputSeconds",
                                                    identifier,
                                                    &ScalarCollector::TraceSinkDouble);
        break;

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
        ret = m_terminalCollectors.ConnectWithProbe(probe,
                                                    "OutputSeconds",
                                                    identifier,
                                                    &UnitConversionCollector::TraceSinkDouble);
        break;

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
        if (m_averagingMode)
        {
            ret = m_terminalCollectors.ConnectWithProbe(probe,
                                                        "OutputSeconds",
                                                        identifier,
                                                        &ScalarCollector::TraceSinkDouble);
        }
        else
        {
            ret = m_terminalCollectors.ConnectWithProbe(probe,
                                                        "OutputSeconds",
                                                        identifier,
                                                        &DistributionCollector::TraceSinkDouble);
        }
        break;

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;
    }

    if (ret)
    {
        NS_LOG_INFO(this << " created probe " << probe->GetName() << ", connected to collector "
                         << identifier);
    }
    else
    {
        NS_LOG_WARN(this << " unable to connect probe " << probe->GetName() << " to collector "
                         << identifier);
    }

    return ret;
}

void
SatStatsLinkJitterHelper::PassSampleToCollector(const Time& jitter, uint32_t identifier)
{
    // NS_LOG_FUNCTION (this << jitter.GetSeconds () << identifier);

    Ptr<DataCollectionObject> collector = m_terminalCollectors.Get(identifier);
    NS_ASSERT_MSG(collector != nullptr, "Unable to find collector with identifier " << identifier);

    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT: {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, jitter.GetSeconds());
        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
        Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, jitter.GetSeconds());
        break;
    }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
        if (m_averagingMode)
        {
            Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
            NS_ASSERT(c != nullptr);
            c->TraceSinkDouble(0.0, jitter.GetSeconds());
        }
        else
        {
            Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector>();
            NS_ASSERT(c != nullptr);
            c->TraceSinkDouble(0.0, jitter.GetSeconds());
        }
        break;

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    } // end of `switch (GetOutputType ())`

} // end of `void PassSampleToCollector (Time, uint32_t)`

void
SatStatsLinkJitterHelper::SetChannelLink(SatEnums::ChannelType_t channelLink)
{
    m_channelLink = channelLink;
}

// FORWARD FEEDER LINK DEV-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdFeederDevLinkJitterHelper);

SatStatsFwdFeederDevLinkJitterHelper::SatStatsFwdFeederDevLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdFeederDevLinkJitterHelper::~SatStatsFwdFeederDevLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdFeederDevLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdFeederDevLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsFwdFeederDevLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsFwdFeederDevLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        if (satGeoDev->TraceConnectWithoutContext("RxFeederLinkJitter", callback))
        {
            NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                             << " device #" << satGeoDev->GetIfIndex());
        }
        else
        {
            NS_FATAL_ERROR("Error connecting to RxFeederLinkJitter trace source of SatNetDevice"
                           << " at node ID " << (*it)->GetId() << " device #"
                           << satGeoDev->GetIfIndex());
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);

        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

    // Enable statistics-related tags on the transmitting device.
    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    }

} // end of `void DoInstallProbes ();`

// FORWARD USER LINK DEV-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdUserDevLinkJitterHelper);

SatStatsFwdUserDevLinkJitterHelper::SatStatsFwdUserDevLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdUserDevLinkJitterHelper::~SatStatsFwdUserDevLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdUserDevLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdUserDevLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsFwdUserDevLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        const int32_t utId = GetUtId(*it);
        NS_ASSERT_MSG(utId > 0, "Node " << (*it)->GetId() << " is not a valid UT");
        const uint32_t identifier = GetIdentifierForUt(*it);

        // Create the probe.
        std::ostringstream probeName;
        probeName << utId;
        Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe>();
        probe->SetName(probeName.str());

        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);

        // Connect the object to the probe.
        if (probe->ConnectByObject("RxLinkJitter", satDev) &&
            ConnectProbeToCollector(probe, identifier))
        {
            m_probes.push_back(probe->GetObject<Probe>());

            // Enable statistics-related tags and trace sources on the device.
            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        else
        {
            NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatMac"
                           << " at node ID " << (*it)->GetId() << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

    // Enable statistics-related tags on the transmitting device.
    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    }

} // end of `void DoInstallProbes ();`

// FORWARD FEEDER LINK MAC-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdFeederMacLinkJitterHelper);

SatStatsFwdFeederMacLinkJitterHelper::SatStatsFwdFeederMacLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdFeederMacLinkJitterHelper::~SatStatsFwdFeederMacLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdFeederMacLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdFeederMacLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsFwdFeederMacLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsFwdFeederMacLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatMac>> satGeoFeederMacs = satGeoDev->GetFeederMac();
        Ptr<SatMac> satMac;
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin();
             it2 != satGeoFeederMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));

            // Connect the object to the probe.
            if (satMac->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satGeoDev->GetIfIndex());

                // Enable statistics-related tags and trace sources on the device.
                satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satGeoDev->GetIfIndex());
            }
        }
        std::map<uint32_t, Ptr<SatMac>> satGeoUserMacs = satGeoDev->GetUserMac();
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin();
             it2 != satGeoUserMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatMac> satMac = satDev->GetMac();
        NS_ASSERT(satMac != nullptr);

        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

    // Enable statistics-related tags on the transmitting device.
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

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    }

} // end of `void DoInstallProbes ();`

// FORWARD USER LINK MAC-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdUserMacLinkJitterHelper);

SatStatsFwdUserMacLinkJitterHelper::SatStatsFwdUserMacLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdUserMacLinkJitterHelper::~SatStatsFwdUserMacLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdUserMacLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdUserMacLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsFwdUserMacLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatMac>> satGeoFeederMacs = satGeoDev->GetFeederMac();
        Ptr<SatMac> satMac;
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin();
             it2 != satGeoFeederMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        std::map<uint32_t, Ptr<SatMac>> satGeoUserMacs = satGeoDev->GetUserMac();
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin();
             it2 != satGeoUserMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        const int32_t utId = GetUtId(*it);
        NS_ASSERT_MSG(utId > 0, "Node " << (*it)->GetId() << " is not a valid UT");
        const uint32_t identifier = GetIdentifierForUt(*it);

        // Create the probe.
        std::ostringstream probeName;
        probeName << utId;
        Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe>();
        probe->SetName(probeName.str());

        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatMac> satMac = satDev->GetMac();
        NS_ASSERT(satMac != nullptr);

        // Connect the object to the probe.
        if (probe->ConnectByObject("RxLinkJitter", satMac) &&
            ConnectProbeToCollector(probe, identifier))
        {
            m_probes.push_back(probe->GetObject<Probe>());

            // Enable statistics-related tags and trace sources on the device.
            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        else
        {
            NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatMac"
                           << " at node ID " << (*it)->GetId() << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

    // Enable statistics-related tags on the transmitting device.
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

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    }

} // end of `void DoInstallProbes ();`

// FORWARD FEEDER LINK PHY-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdFeederPhyLinkJitterHelper);

SatStatsFwdFeederPhyLinkJitterHelper::SatStatsFwdFeederPhyLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);

    SetChannelLink(SatEnums::FORWARD_FEEDER_CH);
}

SatStatsFwdFeederPhyLinkJitterHelper::~SatStatsFwdFeederPhyLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdFeederPhyLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdFeederPhyLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsFwdFeederPhyLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsFwdFeederPhyLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatPhy>> satGeoFeederPhys = satGeoDev->GetFeederPhy();
        Ptr<SatPhy> satPhy;
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin();
             it2 != satGeoFeederPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));

            // Connect the object to the probe.
            if (satPhy->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satGeoDev->GetIfIndex());

                // Enable statistics-related tags and trace sources on the device.
                satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satGeoDev->GetIfIndex());
            }
        }
        std::map<uint32_t, Ptr<SatPhy>> satGeoUserPhys = satGeoDev->GetUserPhy();
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin();
             it2 != satGeoUserPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatPhy> satPhy = satDev->GetPhy();
        NS_ASSERT(satPhy != nullptr);

        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

    // Enable statistics-related tags on the transmitting device.
    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);
            Ptr<SatPhy> satPhy = satDev->GetPhy();
            NS_ASSERT(satPhy != nullptr);

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    }

} // end of `void DoInstallProbes ();`

// FORWARD USER LINK PHY-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdUserPhyLinkJitterHelper);

SatStatsFwdUserPhyLinkJitterHelper::SatStatsFwdUserPhyLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);

    SetChannelLink(SatEnums::FORWARD_USER_CH);
}

SatStatsFwdUserPhyLinkJitterHelper::~SatStatsFwdUserPhyLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdUserPhyLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdUserPhyLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsFwdUserPhyLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatPhy>> satGeoFeederPhys = satGeoDev->GetFeederPhy();
        Ptr<SatPhy> satPhy;
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin();
             it2 != satGeoFeederPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        std::map<uint32_t, Ptr<SatPhy>> satGeoUserPhys = satGeoDev->GetUserPhy();
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin();
             it2 != satGeoUserPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        const int32_t utId = GetUtId(*it);
        NS_ASSERT_MSG(utId > 0, "Node " << (*it)->GetId() << " is not a valid UT");
        const uint32_t identifier = GetIdentifierForUt(*it);

        // Create the probe.
        std::ostringstream probeName;
        probeName << utId;
        Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe>();
        probe->SetName(probeName.str());

        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatPhy> satPhy = satDev->GetPhy();
        NS_ASSERT(satPhy != nullptr);

        // Connect the object to the probe.
        if (probe->ConnectByObject("RxLinkJitter", satPhy) &&
            ConnectProbeToCollector(probe, identifier))
        {
            m_probes.push_back(probe->GetObject<Probe>());

            // Enable statistics-related tags and trace sources on the device.
            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        else
        {
            NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatPhy"
                           << " at node ID " << (*it)->GetId() << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

    // Enable statistics-related tags on the transmitting device.
    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);
            Ptr<SatPhy> satPhy = satDev->GetPhy();
            NS_ASSERT(satPhy != nullptr);

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    }

} // end of `void DoInstallProbes ();`

// RETURN FEEDER LINK DEV-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnFeederDevLinkJitterHelper);

SatStatsRtnFeederDevLinkJitterHelper::SatStatsRtnFeederDevLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnFeederDevLinkJitterHelper::~SatStatsRtnFeederDevLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnFeederDevLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnFeederDevLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsRtnFeederDevLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<SatMac> satMac;
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // Enable statistics-related tags and trace sources on the device.
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsRtnFeederDevLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);

            // Connect the object to the probe.
            if (satDev->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satDev->GetIfIndex());

                // Enable statistics-related tags and trace sources on the device.
                satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satDev->GetIfIndex());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

// RETURN USER LINK DEV-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnUserDevLinkJitterHelper);

SatStatsRtnUserDevLinkJitterHelper::SatStatsRtnUserDevLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnUserDevLinkJitterHelper::~SatStatsRtnUserDevLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnUserDevLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnUserDevLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsRtnUserDevLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsRtnUserDevLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));

        // Connect the object to the probe.
        if (satGeoDev->TraceConnectWithoutContext("RxUserLinkJitter", callback))
        {
            NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                             << " device #" << satGeoDev->GetIfIndex());
        }
        else
        {
            NS_FATAL_ERROR("Error connecting to RxUserLinkJitter trace source of SatNetDevice"
                           << " at node ID " << (*it)->GetId() << " device #"
                           << satGeoDev->GetIfIndex());
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // Enable statistics-related tags and trace sources on the device.
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();

    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

// RETURN FEEDER LINK MAC-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnFeederMacLinkJitterHelper);

SatStatsRtnFeederMacLinkJitterHelper::SatStatsRtnFeederMacLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnFeederMacLinkJitterHelper::~SatStatsRtnFeederMacLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnFeederMacLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnFeederMacLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsRtnFeederMacLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<SatMac> satMac;
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatMac>> satGeoFeederMacs = satGeoDev->GetFeederMac();
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin();
             it2 != satGeoFeederMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        std::map<uint32_t, Ptr<SatMac>> satGeoUserMacs = satGeoDev->GetUserMac();
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin();
             it2 != satGeoUserMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // Enable statistics-related tags and trace sources on the device.
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatMac> satMac = satDev->GetMac();
        NS_ASSERT(satMac != nullptr);
        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsRtnFeederMacLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);
            Ptr<SatMac> satMac = satDev->GetMac();
            NS_ASSERT(satMac != nullptr);

            // Connect the object to the probe.
            if (satMac->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satDev->GetIfIndex());

                // Enable statistics-related tags and trace sources on the device.
                satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
                satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satDev->GetIfIndex());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

// RETURN USER LINK MAC-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnUserMacLinkJitterHelper);

SatStatsRtnUserMacLinkJitterHelper::SatStatsRtnUserMacLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnUserMacLinkJitterHelper::~SatStatsRtnUserMacLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnUserMacLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnUserMacLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsRtnUserMacLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsRtnUserMacLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatMac>> satGeoFeederMacs = satGeoDev->GetFeederMac();
        Ptr<SatMac> satMac;
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin();
             it2 != satGeoFeederMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        std::map<uint32_t, Ptr<SatMac>> satGeoUserMacs = satGeoDev->GetUserMac();
        for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin();
             it2 != satGeoUserMacs.end();
             ++it2)
        {
            satMac = it2->second;
            NS_ASSERT(satMac != nullptr);
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));

            // Connect the object to the probe.
            if (satMac->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satGeoDev->GetIfIndex());
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satGeoDev->GetIfIndex());
            }
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // Enable statistics-related tags and trace sources on the device.
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatMac> satMac = satDev->GetMac();
        NS_ASSERT(satMac != nullptr);
        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    }

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

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satMac->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

// RETURN FEEDER LINK PHY-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnFeederPhyLinkJitterHelper);

SatStatsRtnFeederPhyLinkJitterHelper::SatStatsRtnFeederPhyLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);

    SetChannelLink(SatEnums::RETURN_FEEDER_CH);
}

SatStatsRtnFeederPhyLinkJitterHelper::~SatStatsRtnFeederPhyLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnFeederPhyLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnFeederPhyLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsRtnFeederPhyLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<SatPhy> satPhy;
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatPhy>> satGeoFeederPhys = satGeoDev->GetFeederPhy();
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin();
             it2 != satGeoFeederPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        std::map<uint32_t, Ptr<SatPhy>> satGeoUserPhys = satGeoDev->GetUserPhy();
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin();
             it2 != satGeoUserPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // Enable statistics-related tags and trace sources on the device.
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatPhy> satPhy = satDev->GetPhy();
        NS_ASSERT(satPhy != nullptr);
        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsRtnFeederPhyLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);
            Ptr<SatPhy> satPhy = satDev->GetPhy();
            NS_ASSERT(satPhy != nullptr);

            // Connect the object to the probe.
            if (satPhy->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satDev->GetIfIndex());

                // Enable statistics-related tags and trace sources on the device.
                satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
                satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satDev->GetIfIndex());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

// RETURN USER LINK PHY-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnUserPhyLinkJitterHelper);

SatStatsRtnUserPhyLinkJitterHelper::SatStatsRtnUserPhyLinkJitterHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkJitterHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);

    SetChannelLink(SatEnums::RETURN_USER_CH);
}

SatStatsRtnUserPhyLinkJitterHelper::~SatStatsRtnUserPhyLinkJitterHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnUserPhyLinkJitterHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnUserPhyLinkJitterHelper").SetParent<SatStatsLinkJitterHelper>();
    return tid;
}

void
SatStatsRtnUserPhyLinkJitterHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();
    Callback<void, const Time&, const Address&> callback =
        MakeCallback(&SatStatsRtnUserPhyLinkJitterHelper::RxLinkJitterCallback, this);

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        satGeoDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        std::map<uint32_t, Ptr<SatPhy>> satGeoFeederPhys = satGeoDev->GetFeederPhy();
        Ptr<SatPhy> satPhy;
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin();
             it2 != satGeoFeederPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        }
        std::map<uint32_t, Ptr<SatPhy>> satGeoUserPhys = satGeoDev->GetUserPhy();
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin();
             it2 != satGeoUserPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));

            // Connect the object to the probe.
            if (satPhy->TraceConnectWithoutContext("RxLinkJitter", callback))
            {
                NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                 << " device #" << satGeoDev->GetIfIndex());
            }
            else
            {
                NS_FATAL_ERROR("Error connecting to RxLinkJitter trace source of SatNetDevice"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << satGeoDev->GetIfIndex());
            }
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // Enable statistics-related tags and trace sources on the device.
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatPhy> satPhy = satDev->GetPhy();
        NS_ASSERT(satPhy != nullptr);
        satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();

    for (NodeContainer::Iterator it = gws.Begin(); it != gws.End(); ++it)
    {
        NetDeviceContainer devs = GetGwSatNetDevice(*it);

        for (NetDeviceContainer::Iterator itDev = devs.Begin(); itDev != devs.End(); ++itDev)
        {
            Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice>();
            NS_ASSERT(satDev != nullptr);
            Ptr<SatPhy> satPhy = satDev->GetPhy();
            NS_ASSERT(satPhy != nullptr);

            satDev->SetAttribute("EnableStatisticsTags", BooleanValue(true));
            satPhy->SetAttribute("EnableStatisticsTags", BooleanValue(true));
        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

} // end of namespace ns3
