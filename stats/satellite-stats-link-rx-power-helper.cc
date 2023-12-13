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

#include "satellite-stats-link-rx-power-helper.h"

#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/data-collection-object.h>
#include <ns3/distribution-collector.h>
#include <ns3/double-probe.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/node.h>
#include <ns3/object-map.h>
#include <ns3/object-vector.h>
#include <ns3/probe.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy.h>
#include <ns3/scalar-collector.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/unit-conversion-collector.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatStatsLinkRxPowerHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatStatsLinkRxPowerHelper);

SatStatsLinkRxPowerHelper::SatStatsLinkRxPowerHelper(Ptr<const SatHelper> satHelper)
    : SatStatsHelper(satHelper),
      m_traceSinkCallback(MakeCallback(&SatStatsLinkRxPowerHelper::RxPowerCallback, this))
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsLinkRxPowerHelper::~SatStatsLinkRxPowerHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsLinkRxPowerHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatStatsLinkRxPowerHelper").SetParent<SatStatsHelper>();
    return tid;
}

void
SatStatsLinkRxPowerHelper::SetAveragingMode(bool averagingMode)
{
    NS_LOG_FUNCTION(this << averagingMode);
    m_averagingMode = averagingMode;
}

Callback<void, double, const Address&>
SatStatsLinkRxPowerHelper::GetTraceSinkCallback() const
{
    return m_traceSinkCallback;
}

void
SatStatsLinkRxPowerHelper::DoInstall()
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
                                        StringValue(GetIdentifierHeading("rx_power_db")));

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
                                        StringValue(GetTimeHeading("rx_power_db")));

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
                                            StringValue(GetDistributionHeading("rx_power_db")));
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
                                            StringValue(GetDistributionHeading("rx_power_db")));

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
        plotAggregator->SetLegend("RX Power (in dB)", "Frequency");
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
            plotAggregator->SetLegend("RX Power (in dB)", "Frequency");
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
            plotAggregator->SetLegend("RX Power (in dB)", "Frequency");
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
        NS_FATAL_ERROR("SatStatsLinkDelayHelper - Invalid output type");
        break;
    }

    // Setup probes and connect them to the collectors.
    InstallProbes();

} // end of `void DoInstall ();`

void
SatStatsLinkRxPowerHelper::RxPowerCallback(double rxPowerDb, const Address& from)
{
    // NS_LOG_FUNCTION (this << rxPowerDb << from);

    if (from.IsInvalid())
    {
        NS_LOG_WARN(this << " discarding a packet RX power of " << rxPowerDb << "dB"
                         << " from statistics collection because of"
                         << " invalid sender address");
    }
    else if (Mac48Address::ConvertFrom(from).IsBroadcast())
    {
        for (std::pair<const Address, uint32_t> item : m_identifierMap)
        {
            PassSampleToCollector(rxPowerDb, item.second);
        }
    }
    else
    {
        // Determine the identifier associated with the sender address.
        std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find(from);

        if (it != m_identifierMap.end())
        {
            PassSampleToCollector(rxPowerDb, it->second);
        }
        else
        {
            NS_LOG_WARN(this << " discarding a packet RX power of " << rxPowerDb << "dB"
                             << " from statistics collection because of"
                             << " unknown sender address " << from);
        }
    }
}

bool
SatStatsLinkRxPowerHelper::ConnectProbeToCollector(Ptr<Probe> probe, uint32_t identifier)
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
SatStatsLinkRxPowerHelper::PassSampleToCollector(double rxPowerDb, uint32_t identifier)
{
    // NS_LOG_FUNCTION (this << rxPowerDb << identifier);

    Ptr<DataCollectionObject> collector = m_terminalCollectors.Get(identifier);
    NS_ASSERT_MSG(collector != nullptr, "Unable to find collector with identifier " << identifier);

    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT: {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, rxPowerDb);
        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
        Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkDouble(0.0, rxPowerDb);
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
            c->TraceSinkDouble(0.0, rxPowerDb);
        }
        else
        {
            Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector>();
            NS_ASSERT(c != nullptr);
            c->TraceSinkDouble(0.0, rxPowerDb);
        }
        break;

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    } // end of `switch (GetOutputType ())`

} // end of `void PassSampleToCollector (double, uint32_t)`

void
SatStatsLinkRxPowerHelper::InstallProbes()
{
    // The method below is supposed to be implemented by the child class.
    DoInstallProbes();
}

// FORWARD FEEDER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdFeederLinkRxPowerHelper);

SatStatsFwdFeederLinkRxPowerHelper::SatStatsFwdFeederLinkRxPowerHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkRxPowerHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdFeederLinkRxPowerHelper::~SatStatsFwdFeederLinkRxPowerHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdFeederLinkRxPowerHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdFeederLinkRxPowerHelper").SetParent<SatStatsLinkRxPowerHelper>();
    return tid;
}

void
SatStatsFwdFeederLinkRxPowerHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);
    }

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        std::map<uint32_t, Ptr<SatPhy>> satGeoFeederPhys = satGeoDev->GetFeederPhy();
        ObjectMapValue phy;
        satGeoDev->GetAttribute("FeederPhy", phy);
        NS_LOG_DEBUG(this << " GeoSat Node ID " << (*it)->GetId() << " device #"
                          << dev->GetIfIndex() << " has " << phy.GetN() << " PHY instance(s)");

        for (ObjectMapValue::Iterator itPhy = phy.Begin(); itPhy != phy.End(); ++itPhy)
        {
            Ptr<SatPhy> satPhy = itPhy->second->GetObject<SatPhy>();
            NS_ASSERT(satPhy != nullptr);
            Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx();
            NS_ASSERT(satPhyRx != nullptr);
            ObjectVectorValue carriers;
            satPhyRx->GetAttribute("RxCarrierList", carriers);
            NS_LOG_DEBUG(this << " PHY #" << itPhy->first << " has " << carriers.GetN()
                              << " RX carrier(s)");

            for (ObjectVectorValue::Iterator itCarrier = carriers.Begin();
                 itCarrier != carriers.End();
                 ++itCarrier)
            {
                // NS_ASSERT (itCarrier->second->m_channelType == SatEnums::FORWARD_FEEDER_CH)
                if (!itCarrier->second->TraceConnectWithoutContext("RxPowerTrace",
                                                                   GetTraceSinkCallback()))
                {
                    NS_FATAL_ERROR("Error connecting to RxPowerTrace trace source"
                                   << " of SatPhyRxCarrier"
                                   << " at GeoSat node ID " << (*it)->GetId() << " device #"
                                   << dev->GetIfIndex() << " PHY #" << itPhy->first
                                   << " RX carrier #" << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (ObjectMapValue::Iterator itPhy = phys)`
    }     // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

} // end of `void DoInstallProbes ();`

// FORWARD USER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdUserLinkRxPowerHelper);

SatStatsFwdUserLinkRxPowerHelper::SatStatsFwdUserLinkRxPowerHelper(Ptr<const SatHelper> satHelper)
    : SatStatsLinkRxPowerHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdUserLinkRxPowerHelper::~SatStatsFwdUserLinkRxPowerHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdUserLinkRxPowerHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdUserLinkRxPowerHelper").SetParent<SatStatsLinkRxPowerHelper>();
    return tid;
}

void
SatStatsFwdUserLinkRxPowerHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);

        // const int32_t utId = GetUtId (*it);
        // NS_ASSERT_MSG (utId > 0,
        //                "Node " << (*it)->GetId () << " is not a valid UT");
        // const uint32_t identifier = GetIdentifierForUt (*it);
        Ptr<NetDevice> dev = GetUtSatNetDevice(*it);
        Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice>();
        NS_ASSERT(satDev != nullptr);
        Ptr<SatPhy> satPhy = satDev->GetPhy();
        NS_ASSERT(satPhy != nullptr);
        Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx();
        NS_ASSERT(satPhyRx != nullptr);
        ObjectVectorValue carriers;
        satPhyRx->GetAttribute("RxCarrierList", carriers);
        NS_LOG_DEBUG(this << " Node ID " << (*it)->GetId() << " device #" << dev->GetIfIndex()
                          << " has " << carriers.GetN() << " RX carriers");

        for (ObjectVectorValue::Iterator itCarrier = carriers.Begin(); itCarrier != carriers.End();
             ++itCarrier)
        {
            // NS_ASSERT (itCarrier->second->m_channelType == SatEnums::FORWARD_USER_CH)
            if (!itCarrier->second->TraceConnectWithoutContext("RxPowerTrace",
                                                               GetTraceSinkCallback()))
            {
                NS_FATAL_ERROR("Error connecting to RxPowerTrace trace source"
                               << " of SatPhyRxCarrier"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << dev->GetIfIndex() << " RX carrier #" << itCarrier->first);
            }

        } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`

// RETURN FEEDER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnFeederLinkRxPowerHelper);

SatStatsRtnFeederLinkRxPowerHelper::SatStatsRtnFeederLinkRxPowerHelper(
    Ptr<const SatHelper> satHelper)
    : SatStatsLinkRxPowerHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnFeederLinkRxPowerHelper::~SatStatsRtnFeederLinkRxPowerHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnFeederLinkRxPowerHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnFeederLinkRxPowerHelper").SetParent<SatStatsLinkRxPowerHelper>();
    return tid;
}

void
SatStatsRtnFeederLinkRxPowerHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);
    }

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
            Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx();
            NS_ASSERT(satPhyRx != nullptr);
            ObjectVectorValue carriers;
            satPhyRx->GetAttribute("RxCarrierList", carriers);
            NS_LOG_DEBUG(this << " Node ID " << (*it)->GetId() << " device #"
                              << (*itDev)->GetIfIndex() << " has " << carriers.GetN()
                              << " RX carriers");

            for (ObjectVectorValue::Iterator itCarrier = carriers.Begin();
                 itCarrier != carriers.End();
                 ++itCarrier)
            {
                // NS_ASSERT (itCarrier->second->m_channelType == SatEnums::RETURN_FEEDER_CH)
                if (!itCarrier->second->TraceConnectWithoutContext("RxPowerTrace",
                                                                   GetTraceSinkCallback()))
                {
                    NS_FATAL_ERROR("Error connecting to RxPowerTrace trace source"
                                   << " of SatPhyRxCarrier"
                                   << " at node ID " << (*it)->GetId() << " device #"
                                   << (*itDev)->GetIfIndex() << " RX carrier #"
                                   << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (it = gws.Begin(); it != gws.End (); ++it)`

    } // end of `for (itDev = devs.Begin (); itDev != devs.End (); ++itDev)`

} // end of `void DoInstallProbes ();`

// RETURN USER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnUserLinkRxPowerHelper);

SatStatsRtnUserLinkRxPowerHelper::SatStatsRtnUserLinkRxPowerHelper(Ptr<const SatHelper> satHelper)
    : SatStatsLinkRxPowerHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnUserLinkRxPowerHelper::~SatStatsRtnUserLinkRxPowerHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnUserLinkRxPowerHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnUserLinkRxPowerHelper").SetParent<SatStatsLinkRxPowerHelper>();
    return tid;
}

void
SatStatsRtnUserLinkRxPowerHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);
    }

    NodeContainer sats = GetSatHelper()->GetBeamHelper()->GetGeoSatNodes();

    for (NodeContainer::Iterator it = sats.Begin(); it != sats.End(); ++it)
    {
        Ptr<NetDevice> dev = GetSatSatGeoNetDevice(*it);
        Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice>();
        NS_ASSERT(satGeoDev != nullptr);
        Ptr<SatPhy> satPhy;
        std::map<uint32_t, Ptr<SatPhy>> satGeoUserPhys = satGeoDev->GetUserPhy();
        for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin();
             it2 != satGeoUserPhys.end();
             ++it2)
        {
            satPhy = it2->second;
            NS_ASSERT(satPhy != nullptr);
            Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx();
            NS_ASSERT(satPhyRx != nullptr);
            ObjectVectorValue carriers;
            satPhyRx->GetAttribute("RxCarrierList", carriers);
            NS_LOG_DEBUG(this << " Node ID " << (*it)->GetId() << " device #"
                              << satGeoDev->GetIfIndex() << " has " << carriers.GetN()
                              << " RX carriers");

            for (ObjectVectorValue::Iterator itCarrier = carriers.Begin();
                 itCarrier != carriers.End();
                 ++itCarrier)
            {
                // NS_ASSERT (itCarrier->second->m_channelType == SatEnums::RETURN_FEEDER_CH)
                if (!itCarrier->second->TraceConnectWithoutContext("RxPowerTrace",
                                                                   GetTraceSinkCallback()))
                {
                    NS_FATAL_ERROR("Error connecting to RxPowerTrace trace source"
                                   << " of SatPhyRxCarrier"
                                   << " at node ID " << (*it)->GetId() << " device #"
                                   << satGeoDev->GetIfIndex() << " RX carrier #"
                                   << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

} // end of `void DoInstallProbes ();`

} // end of namespace ns3
