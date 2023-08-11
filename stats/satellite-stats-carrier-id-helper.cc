/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include "satellite-stats-carrier-id-helper.h"

#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/data-collection-object.h>
#include <ns3/enum.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/node-container.h>
#include <ns3/object-vector.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-phy-rx-carrier-marsala.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy.h>
#include <ns3/scalar-collector.h>
#include <ns3/singleton.h>
#include <ns3/string.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatStatsCarrierIdHelper");

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsCarrierIdHelper);

SatStatsCarrierIdHelper::SatStatsCarrierIdHelper(Ptr<const SatHelper> satHelper)
    : SatStatsHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
    SetTraceSourceName("DaRxCarrierId");
    SetValidCarrierType(SatPhyRxCarrier::DEDICATED_ACCESS);
}

SatStatsCarrierIdHelper::~SatStatsCarrierIdHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsCarrierIdHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatStatsCarrierIdHelper").SetParent<SatStatsHelper>();
    return tid;
}

void
SatStatsCarrierIdHelper::SetTraceSourceName(std::string traceSourceName)
{
    NS_LOG_FUNCTION(this << traceSourceName);
    m_traceSourceName = traceSourceName;
}

std::string
SatStatsCarrierIdHelper::GetTraceSourceName() const
{
    return m_traceSourceName;
}

void
SatStatsCarrierIdHelper::DoInstall()
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
                                        StringValue(GetIdentifierHeading("carrier_id")));

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute("InputDataType",
                                          EnumValue(ScalarCollector::INPUT_DATA_TYPE_UINTEGER));
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
                                        StringValue(GetTimeHeading("carrier_id")));

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute(
            "InputDataType",
            EnumValue(IntervalRateCollector::INPUT_DATA_TYPE_UINTEGER));
        m_terminalCollectors.SetAttribute(
            "OutputType",
            EnumValue(IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        m_terminalCollectors.SetAttribute("IntervalLength", TimeValue(MilliSeconds(5)));
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
        plotAggregator->SetLegend("Time (in seconds)", "Correlations");
        plotAggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_terminalCollectors.SetType("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute(
            "InputDataType",
            EnumValue(IntervalRateCollector::INPUT_DATA_TYPE_UINTEGER));
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
        NS_FATAL_ERROR("SatStatsCarrierIdHelper - Invalid output type");
        break;
    }

    // Create a map of UT addresses and identifiers.
    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        SaveAddressAndIdentifier(*it);
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    Callback<void, uint32_t, const Address&> callback =
        MakeCallback(&SatStatsCarrierIdHelper::CarrierIdRxCallback, this);

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
                Ptr<SatPhyRxCarrierPerSlot> c =
                    DynamicCast<SatPhyRxCarrierPerSlot>(itCarrier->second);
                if (!c)
                {
                    continue;
                }

                SatPhyRxCarrier::CarrierType ct =
                    DynamicCast<SatPhyRxCarrier>(itCarrier->second)->GetCarrierType();
                if (ct != GetValidCarrierType())
                {
                    continue;
                }

                const bool ret =
                    itCarrier->second->TraceConnectWithoutContext(GetTraceSourceName(), callback);
                if (ret)
                {
                    NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                     << " device #" << (*itDev)->GetIfIndex() << " RX carrier #"
                                     << itCarrier->first);
                }
                else
                {
                    NS_FATAL_ERROR("Error connecting to " << GetTraceSourceName() << " trace source"
                                                          << " of SatPhyRxCarrier"
                                                          << " at node ID " << (*it)->GetId()
                                                          << " device #" << (*itDev)->GetIfIndex()
                                                          << " RX carrier #" << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstall ();`

void
SatStatsCarrierIdHelper::CarrierIdRxCallback(uint32_t carrierId, const Address& from)
{
    NS_LOG_FUNCTION(this << carrierId << from);

    if (from.IsInvalid())
    {
        NS_LOG_WARN(this << " discarding a packet"
                         << " from statistics collection because of"
                         << " invalid sender address");
        return;
    }

    // Determine the identifier associated with the sender address.
    std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find(from);

    if (it == m_identifierMap.end())
    {
        NS_LOG_WARN(this << " discarding a packet"
                         << " from statistics collection because of"
                         << " unknown sender address " << from);
        return;
    }

    // Find the first-level collector with the right identifier.
    Ptr<DataCollectionObject> collector = m_terminalCollectors.Get(it->second);
    NS_ASSERT_MSG(collector != nullptr, "Unable to find collector with identifier " << it->second);

    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT: {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkUinteger32(0, carrierId);
        break;
    }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
        Ptr<IntervalRateCollector> c = collector->GetObject<IntervalRateCollector>();
        NS_ASSERT(c != nullptr);
        c->TraceSinkUinteger32(0, carrierId);
        break;
    }

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;
    }
}

void
SatStatsCarrierIdHelper::SaveAddressAndIdentifier(Ptr<Node> utNode)
{
    NS_LOG_FUNCTION(this << utNode->GetId());

    const SatIdMapper* satIdMapper = Singleton<SatIdMapper>::Get();
    const Address addr = satIdMapper->GetUtMacWithNode(utNode);

    if (addr.IsInvalid())
    {
        NS_LOG_WARN(this << " Node " << utNode->GetId() << " is not a valid UT");
    }
    else
    {
        const uint32_t identifier = GetIdentifierForUt(utNode);
        m_identifierMap[addr] = identifier;
        NS_LOG_INFO(this << " associated address " << addr << " with identifier " << identifier);
    }
}

} // end of namespace ns3
