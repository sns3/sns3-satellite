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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/callback.h>

#include <ns3/singleton.h>
#include <ns3/node-container.h>
#include <ns3/satellite-ut-llc.h>
#include <ns3/satellite-request-manager.h>
#include <ns3/satellite-helper.h>

#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-rbdc-request-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsRbdcRequestHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsRbdcRequestHelper);


SatStatsRbdcRequestHelper::SatStatsRbdcRequestHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
  m_averagingMode (false)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRbdcRequestHelper::~SatStatsRbdcRequestHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
SatStatsRbdcRequestHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRbdcRequestHelper")
    .SetParent<SatStatsHelper> ()
    .AddAttribute ("AveragingMode",
                   "If true, all samples will be averaged before passed to aggregator. "
                   "Only affects histogram, PDF, and CDF output types.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatStatsRbdcRequestHelper::SetAveragingMode,
                                        &SatStatsRbdcRequestHelper::GetAveragingMode),
                   MakeBooleanChecker ())
  ;
  return tid;
}


void
SatStatsRbdcRequestHelper::SetAveragingMode (bool averagingMode)
{
  NS_LOG_FUNCTION (this << averagingMode);
  m_averagingMode = averagingMode;
}


bool
SatStatsRbdcRequestHelper::GetAveragingMode () const
{
  NS_LOG_FUNCTION (this);
  return m_averagingMode;
}


void
SatStatsRbdcRequestHelper::RbdcRateCallback (std::string identifier, uint32_t rbdcTraceKbps)
{
  std::stringstream ss (identifier);
  uint32_t identifierNum;
  if (!(ss >> identifierNum))
    {
      NS_FATAL_ERROR ("Cannot convert '" << identifier << "' to number");
    }
  Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (identifierNum);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to find collector with identifier " << identifierNum);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkUinteger32 (0, rbdcTraceKbps);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkUinteger32 (0, rbdcTraceKbps);
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
          Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
          NS_ASSERT (c != 0);
          c->TraceSinkUinteger32 (0, rbdcTraceKbps);
        }
      else
        {
          Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector> ();
          NS_ASSERT (c != 0);
          c->TraceSinkUinteger32 (0, rbdcTraceKbps);
        }
      break;

    default:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;
    }
}


void
SatStatsRbdcRequestHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("rbdc_Kbps")));

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
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetTimeHeading ("rbdc_Kbps")));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::UnitConversionCollector");
        m_terminalCollectors.SetAttribute ("ConversionType",
                                           EnumValue (UnitConversionCollector::TRANSPARENT));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputTimeValue",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      {
        if (m_averagingMode)
          {
            // Setup aggregator.
            m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                             "OutputFileName", StringValue (GetOutputFileName ()),
                                             "MultiFileMode", BooleanValue (false),
                                             "EnableContextPrinting", BooleanValue (false),
                                             "GeneralHeading", StringValue (GetDistributionHeading ("rbdc_Kbps")));
            Ptr<MultiFileAggregator> fileAggregator = m_aggregator->GetObject<MultiFileAggregator> ();
            NS_ASSERT (fileAggregator != 0);

            // Setup the final-level collector.
            m_averagingCollector = CreateObject<DistributionCollector> ();
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
            m_averagingCollector->SetOutputType (outputType);
            m_averagingCollector->SetName ("0");
            m_averagingCollector->TraceConnect ("Output", "0",
                                                MakeCallback (&MultiFileAggregator::Write2d,
                                                              fileAggregator));
            m_averagingCollector->TraceConnect ("OutputString", "0",
                                                MakeCallback (&MultiFileAggregator::AddContextHeading,
                                                              fileAggregator));
            m_averagingCollector->TraceConnect ("Warning", "0",
                                                MakeCallback (&MultiFileAggregator::EnableContextWarning,
                                                              fileAggregator));

            // Setup collectors.
            m_terminalCollectors.SetType ("ns3::ScalarCollector");
            m_terminalCollectors.SetAttribute ("InputDataType",
                                               EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
            m_terminalCollectors.SetAttribute ("OutputType",
                                               EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
            CreateCollectorPerIdentifier (m_terminalCollectors);
            Callback<void, double> callback
              = MakeCallback (&DistributionCollector::TraceSinkDouble1,
                              m_averagingCollector);
            for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
                 it != m_terminalCollectors.End (); ++it)
              {
                it->second->TraceConnectWithoutContext ("Output", callback);
              }
          }
        else
          {
            // Setup aggregator.
            m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                             "OutputFileName", StringValue (GetOutputFileName ()),
                                             "GeneralHeading", StringValue (GetDistributionHeading ("rbdc_Kbps")));

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
                                   "RBDC requested (in kbps)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

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
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      {
        if (m_averagingMode)
          {
            // Setup aggregator.
            m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                             "OutputPath", StringValue (GetOutputPath ()),
                                             "OutputFileName", StringValue (GetName ()));
            Ptr<MagisterGnuplotAggregator> plotAggregator
              = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
            NS_ASSERT (plotAggregator != 0);
            //plot->SetTitle ("");
            plotAggregator->SetLegend ("RBDC requested (in kbps)",
                                       "Frequency");
            plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
            plotAggregator->Add2dDataset (GetName (), GetName ());
            /// \todo Find a better dataset name.

            // Setup the final-level collector.
            m_averagingCollector = CreateObject<DistributionCollector> ();
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
            m_averagingCollector->SetOutputType (outputType);
            m_averagingCollector->SetName ("0");
            m_averagingCollector->TraceConnect ("Output",
                                                GetName (),
                                                MakeCallback (&MagisterGnuplotAggregator::Write2d,
                                                              plotAggregator));
            /// \todo Find a better dataset name.

            // Setup collectors.
            m_terminalCollectors.SetType ("ns3::ScalarCollector");
            m_terminalCollectors.SetAttribute ("InputDataType",
                                               EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
            m_terminalCollectors.SetAttribute ("OutputType",
                                               EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
            CreateCollectorPerIdentifier (m_terminalCollectors);
            Callback<void, double> callback
              = MakeCallback (&DistributionCollector::TraceSinkDouble1,
                              m_averagingCollector);
            for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
                 it != m_terminalCollectors.End (); ++it)
              {
                it->second->TraceConnectWithoutContext ("Output", callback);
              }
          }
        else
          {
            // Setup aggregator.
            m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                             "OutputPath", StringValue (GetOutputPath ()),
                                             "OutputFileName", StringValue (GetName ()));
            Ptr<MagisterGnuplotAggregator> plotAggregator
              = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
            NS_ASSERT (plotAggregator != 0);
            //plot->SetTitle ("");
            plotAggregator->SetLegend ("RBDC requested (in kbps)",
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
          }

        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsDelayHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to the collectors.
  InstallProbes ();
}


void
SatStatsRbdcRequestHelper::InstallProbes ()
{
  NS_LOG_FUNCTION (this);

  Callback<void, std::string, uint32_t> callback
    = MakeCallback (&SatStatsRbdcRequestHelper::RbdcRateCallback, this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      std::ostringstream context;
      switch (GetIdentifierType ())
        {
        case SatStatsHelper::IDENTIFIER_GLOBAL:
          context << "0";
          break;
        case SatStatsHelper::IDENTIFIER_GW:
        case SatStatsHelper::IDENTIFIER_BEAM:
        case SatStatsHelper::IDENTIFIER_UT:
          context << GetIdentifierForUt (*it);
          break;
        default:
          NS_FATAL_ERROR ("SatStatsRbdcRequestHelper - Invalid identifier type");
          break;
        }

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatLlc> satLlc = satDev->GetLlc ();
      NS_ASSERT (satLlc != 0);
      Ptr<SatUtLlc> utLlc = satLlc->GetObject<SatUtLlc> ();
      NS_ASSERT (utLlc != 0);
      Ptr<SatRequestManager> requestManager = utLlc->GetRequestManager ();

      const bool ret = requestManager->TraceConnect ("RbdcTrace", context.str (), callback);
      NS_ASSERT_MSG (ret, "Error connecting to CrTraceLog of node " << (*it)->GetId ());
      NS_UNUSED (ret);
      NS_LOG_INFO (this << " successfully connected"
                        << " with node ID " << (*it)->GetId ());
    }
}

}  // namespace ns-3
