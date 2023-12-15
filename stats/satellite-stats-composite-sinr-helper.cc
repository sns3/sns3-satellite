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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "satellite-stats-composite-sinr-helper.h"

#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/data-collection-object.h>
#include <ns3/distribution-collector.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/node-container.h>
#include <ns3/object-vector.h>
#include <ns3/probe.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-sinr-probe.h>
#include <ns3/scalar-collector.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/unit-conversion-collector.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatStatsCompositeSinrHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatStatsCompositeSinrHelper);

SatStatsCompositeSinrHelper::SatStatsCompositeSinrHelper(Ptr<const SatHelper> satHelper)
    : SatStatsHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsCompositeSinrHelper::~SatStatsCompositeSinrHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsCompositeSinrHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatStatsCompositeSinrHelper").SetParent<SatStatsHelper>();
    return tid;
}

void
SatStatsCompositeSinrHelper::DoInstall()
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
                                        StringValue(GetIdentifierHeading("sinr_db")));

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
                                        StringValue(GetTimeHeading("sinr_db")));

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
        // Setup aggregator.
        m_aggregator = CreateAggregator("ns3::MultiFileAggregator",
                                        "OutputFileName",
                                        StringValue(GetOutputFileName()),
                                        "GeneralHeading",
                                        StringValue(GetDistributionHeading("sinr_db")));

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
        plotAggregator->SetLegend("Time (in seconds)", "SINR (in dB)");
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
        plotAggregator->SetLegend("SINR (in dB)", "Frequency");
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
        break;
    }

    default:
        NS_FATAL_ERROR("SatStatsCompositeSinrHelper - Invalid output type");
        break;
    }

    // Setup probes and connect them to the collectors.
    InstallProbes();

} // end of `void DoInstall ();`

void
SatStatsCompositeSinrHelper::InstallProbes()
{
    // The method below is supposed to be implemented by the child class.
    DoInstallProbes();
}

bool
SatStatsCompositeSinrHelper::ConnectProbeToCollector(Ptr<Probe> probe, uint32_t identifier)
{
    NS_LOG_FUNCTION(this << probe << probe->GetName() << identifier);

    // Connect the probe to the right collector.
    bool ret = false;
    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
        ret = m_terminalCollectors.ConnectWithProbe(probe->GetObject<Probe>(),
                                                    "OutputSinr",
                                                    identifier,
                                                    &ScalarCollector::TraceSinkDouble);
        break;

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
        ret = m_terminalCollectors.ConnectWithProbe(probe->GetObject<Probe>(),
                                                    "OutputSinr",
                                                    identifier,
                                                    &UnitConversionCollector::TraceSinkDouble);
        break;

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
        ret = m_terminalCollectors.ConnectWithProbe(probe->GetObject<Probe>(),
                                                    "OutputSinr",
                                                    identifier,
                                                    &DistributionCollector::TraceSinkDouble);
        break;

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    } // end of `switch (GetOutputType ())`

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

bool
SatStatsCompositeSinrHelper::DisconnectProbeFromCollector(Ptr<Probe> probe, uint32_t identifier)
{
    NS_LOG_FUNCTION(this << probe << probe->GetName() << identifier);

    // Connect the probe to the right collector.
    bool ret = false;
    switch (GetOutputType())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
        ret = m_terminalCollectors.DisconnectWithProbe(probe->GetObject<Probe>(),
                                                       "OutputSinr",
                                                       identifier,
                                                       &ScalarCollector::TraceSinkDouble);
        break;

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
        ret = m_terminalCollectors.DisconnectWithProbe(probe->GetObject<Probe>(),
                                                       "OutputSinr",
                                                       identifier,
                                                       &UnitConversionCollector::TraceSinkDouble);
        break;

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
        ret = m_terminalCollectors.DisconnectWithProbe(probe->GetObject<Probe>(),
                                                       "OutputSinr",
                                                       identifier,
                                                       &DistributionCollector::TraceSinkDouble);
        break;

    default:
        NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                       << " is not a valid output type for this statistics.");
        break;

    } // end of `switch (GetOutputType ())`

    if (ret)
    {
        NS_LOG_INFO(this << " probe " << probe->GetName() << ", disconnected from collector "
                         << identifier);
    }
    else
    {
        NS_LOG_WARN(this << " unable to disconnect probe " << probe->GetName() << " from collector "
                         << identifier);
    }

    return ret;
}

// FORWARD LINK ///////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsFwdCompositeSinrHelper);

SatStatsFwdCompositeSinrHelper::SatStatsFwdCompositeSinrHelper(Ptr<const SatHelper> satHelper)
    : SatStatsCompositeSinrHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsFwdCompositeSinrHelper::~SatStatsFwdCompositeSinrHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsFwdCompositeSinrHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsFwdCompositeSinrHelper").SetParent<SatStatsCompositeSinrHelper>();
    return tid;
}

void
SatStatsFwdCompositeSinrHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);
    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();

    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        const int32_t utId = GetUtId(*it);
        NS_ASSERT_MSG(utId > 0, "Node " << (*it)->GetId() << " is not a valid UT");
        const uint32_t identifier = GetIdentifierForUt(*it);

        // Create the probe.
        std::ostringstream probeName;
        probeName << utId;
        Ptr<SatSinrProbe> probe = CreateObject<SatSinrProbe>();
        probe->SetName(probeName.str());

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
            // Connect the object to the probe.
            if (probe->ConnectByObject("Sinr", itCarrier->second) &&
                ConnectProbeToCollector(probe, identifier))
            {
                m_probes.insert(
                    std::make_pair(probe->GetObject<Probe>(), std::make_pair(*it, identifier)));
            } // end of `if (probe->ConnectByObject ("Sinr", itCarrier->second))`
            else
            {
                NS_FATAL_ERROR("Error connecting to Sinr trace source"
                               << " of SatPhyRxCarrier"
                               << " at node ID " << (*it)->GetId() << " device #"
                               << dev->GetIfIndex() << " RX carrier #" << itCarrier->first);
            }

        } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`

void
SatStatsFwdCompositeSinrHelper::UpdateIdentifierOnProbes()
{
    NS_LOG_FUNCTION(this);

    std::map<Ptr<Probe>, std::pair<Ptr<Node>, uint32_t>>::iterator it;

    for (it = m_probes.begin(); it != m_probes.end(); it++)
    {
        Ptr<Probe> probe = it->first;
        Ptr<Node> node = it->second.first;
        uint32_t identifier = it->second.second;

        if (!DisconnectProbeFromCollector(probe, identifier))
        {
            NS_FATAL_ERROR("Error disconnecting trace file on handover");
        }

        identifier = GetIdentifierForUtUser(node);

        if (!ConnectProbeToCollector(probe, identifier))
        {
            NS_FATAL_ERROR("Error connecting trace file on handover");
        }

        it->second.second = identifier;
    }
} // end of `void UpdateIdentifierOnProbes ();`

// RETURN LINK ////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(SatStatsRtnCompositeSinrHelper);

SatStatsRtnCompositeSinrHelper::SatStatsRtnCompositeSinrHelper(Ptr<const SatHelper> satHelper)
    : SatStatsCompositeSinrHelper(satHelper)
{
    NS_LOG_FUNCTION(this << satHelper);
}

SatStatsRtnCompositeSinrHelper::~SatStatsRtnCompositeSinrHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId // static
SatStatsRtnCompositeSinrHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatStatsRtnCompositeSinrHelper").SetParent<SatStatsCompositeSinrHelper>();
    return tid;
}

void
SatStatsRtnCompositeSinrHelper::DoInstallProbes()
{
    NS_LOG_FUNCTION(this);

    NodeContainer uts = GetSatHelper()->GetBeamHelper()->GetUtNodes();
    for (NodeContainer::Iterator it = uts.Begin(); it != uts.End(); ++it)
    {
        // Create a map of UT addresses and identifiers.
        SaveAddressAndIdentifier(*it);
    }

    // Connect to trace sources at GW nodes.

    NodeContainer gws = GetSatHelper()->GetBeamHelper()->GetGwNodes();
    Callback<void, double, const Address&> callback =
        MakeCallback(&SatStatsRtnCompositeSinrHelper::SinrCallback, this);

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
                if (itCarrier->second->TraceConnectWithoutContext("Sinr", callback))
                {
                    NS_LOG_INFO(this << " successfully connected with node ID " << (*it)->GetId()
                                     << " device #" << (*itDev)->GetIfIndex() << " RX carrier #"
                                     << itCarrier->first);
                }
                else
                {
                    NS_FATAL_ERROR("Error connecting to Sinr trace source"
                                   << " of SatPhyRxCarrier"
                                   << " at node ID " << (*it)->GetId() << " device #"
                                   << (*itDev)->GetIfIndex() << " RX carrier #"
                                   << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`

void
SatStatsRtnCompositeSinrHelper::SinrCallback(double sinrDb, const Address& from)
{
    // NS_LOG_FUNCTION (this << sinrDb << from);

    if (from.IsInvalid())
    {
        NS_LOG_WARN(this << " discarding a SINR trace of " << sinrDb << " dB"
                         << " from statistics collection because of"
                         << " invalid sender address");
    }
    else
    {
        // Determine the identifier associated with the sender address.
        std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find(from);

        if (it == m_identifierMap.end())
        {
            NS_LOG_WARN(this << " discarding a SINR trace of " << sinrDb << " dB"
                             << " from statistics collection because of"
                             << " unknown sender address " << from);
        }
        else
        {
            // Find the collector with the right identifier.
            Ptr<DataCollectionObject> collector = m_terminalCollectors.Get(it->second);
            NS_ASSERT_MSG(collector != nullptr,
                          "Unable to find collector with identifier " << it->second);

            switch (GetOutputType())
            {
            case SatStatsHelper::OUTPUT_SCALAR_FILE:
            case SatStatsHelper::OUTPUT_SCALAR_PLOT: {
                Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector>();
                NS_ASSERT(c != nullptr);
                c->TraceSinkDouble(0.0, sinrDb);
                break;
            }

            case SatStatsHelper::OUTPUT_SCATTER_FILE:
            case SatStatsHelper::OUTPUT_SCATTER_PLOT: {
                Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector>();
                NS_ASSERT(c != nullptr);
                c->TraceSinkDouble(0.0, sinrDb);
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
                c->TraceSinkDouble(0.0, sinrDb);
                break;
            }

            default:
                NS_FATAL_ERROR(GetOutputTypeName(GetOutputType())
                               << " is not a valid output type for this statistics.");
                break;

            } // end of `switch (GetOutputType ())`

        } // end of `if (it == m_identifierMap.end ())`

    } // end of else of `if (from.IsInvalid ())`

} // end of `void SinrCallback (double, const Address &);`

} // end of namespace ns3
