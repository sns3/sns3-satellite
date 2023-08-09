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

#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/object-map.h>
#include <ns3/object-vector.h>
#include <ns3/singleton.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/probe.h>
#include <ns3/double-probe.h>
#include <ns3/mac48-address.h>

#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-geo-feeder-phy.h>
#include <ns3/satellite-geo-user-phy.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>

#include <ns3/data-collection-object.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-satellite-queue-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsSatelliteQueueHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsSatelliteQueueHelper);

std::string // static
SatStatsSatelliteQueueHelper::GetUnitTypeName (SatStatsSatelliteQueueHelper::UnitType_t unitType)
{
  switch (unitType)
    {
    case SatStatsSatelliteQueueHelper::UNIT_BYTES:
      return "UNIT_BYTES";
    case SatStatsSatelliteQueueHelper::UNIT_NUMBER_OF_PACKETS:
      return "UNIT_NUMBER_OF_PACKETS";
    default:
      NS_FATAL_ERROR ("SatStatsSatelliteQueueHelper - Invalid unit type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsSatelliteQueueHelper - Invalid unit type");
  return "";
}

SatStatsSatelliteQueueHelper::SatStatsSatelliteQueueHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsSatelliteQueueHelper::~SatStatsSatelliteQueueHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsSatelliteQueueHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsSatelliteQueueHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsSatelliteQueueHelper::SetUnitType (SatStatsSatelliteQueueHelper::UnitType_t unitType)
{
  NS_LOG_FUNCTION (this << GetUnitTypeName (unitType));
  m_unitType = unitType;

  // Update presentation-based attributes.
  if (unitType == SatStatsSatelliteQueueHelper::UNIT_BYTES)
    {
      m_shortLabel = "size_bytes";
      m_longLabel = "Queue size (in bytes)";
    }
  else if (unitType == SatStatsSatelliteQueueHelper::UNIT_NUMBER_OF_PACKETS)
    {
      m_shortLabel = "num_packets";
      m_longLabel = "Queue size (in number of packets)";
    }
  else
    {
      NS_FATAL_ERROR ("SatStatsSatelliteQueueHelper - Invalid unit type");
    }
}


void
SatStatsSatelliteQueueHelper::SetAveragingMode (bool averagingMode)
{
  NS_LOG_FUNCTION (this << averagingMode);
  m_averagingMode = averagingMode;
}


void
SatStatsSatelliteQueueHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue (GetIdentifierHeading (m_shortLabel)));

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
                                         "GeneralHeading", StringValue (GetTimeHeading (m_shortLabel)));

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
                                             "GeneralHeading", StringValue (GetDistributionHeading (m_shortLabel)));
            Ptr<MultiFileAggregator> fileAggregator = m_aggregator->GetObject<MultiFileAggregator> ();
            NS_ASSERT (fileAggregator != nullptr);

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
                                             "GeneralHeading", StringValue (GetDistributionHeading (m_shortLabel)));

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
        NS_ASSERT (plotAggregator != nullptr);
        //plot->SetTitle ("");
        plotAggregator->SetLegend (m_longLabel,
                                   "Frequency");
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
            NS_ASSERT (plotAggregator != nullptr);
            //plot->SetTitle ("");
            plotAggregator->SetLegend (m_longLabel,
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
            NS_ASSERT (plotAggregator != nullptr);
            //plot->SetTitle ("");
            plotAggregator->SetLegend (m_longLabel,
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
      NS_FATAL_ERROR ("SatStatsLinkDelayHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to the collectors.
  InstallProbes ();



} // end of `void DoInstall ();`


void
SatStatsSatelliteQueueHelper::QueueSizeCallback (uint32_t size, const Address &from)
{
  //NS_LOG_FUNCTION (this << size << from);

  if (from.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding a packet"
                        << " from statistics collection because of"
                        << " invalid sender address");
    }
  else if (Mac48Address::ConvertFrom (from).IsBroadcast ())
    {
      for (std::pair<const Address, uint32_t> item : m_identifierMap)
        {
          PassSampleToCollector (size, item.second);
        }
    }
  else
    {
      // Determine the identifier associated with the sender address.
      std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find (from);

      if (it != m_identifierMap.end ())
        {
          PassSampleToCollector (size, it->second);
        }
      else
        {
          NS_LOG_WARN (this << " discarding a packet"
                            << " from statistics collection because of"
                            << " unknown sender address " << from);
        }
    }
}


void
SatStatsSatelliteQueueHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
{
  NS_LOG_FUNCTION (this << utNode->GetId ());

  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
  const Address addr = satIdMapper->GetUtMacWithNode (utNode);

  if (addr.IsInvalid ())
    {
      NS_LOG_WARN (this << " Node " << utNode->GetId ()
                        << " is not a valid UT");
    }
  else
    {
      const uint32_t identifier = GetIdentifierForUt (utNode);
      m_identifierMap[addr] = identifier;
      NS_LOG_INFO (this << " associated address " << addr
                        << " with identifier " << identifier);

    }
}


bool
SatStatsSatelliteQueueHelper::ConnectProbeToCollector (Ptr<Probe> probe, uint32_t identifier)
{
  NS_LOG_FUNCTION (this << probe << probe->GetName () << identifier);

  bool ret = false;
  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      ret = m_terminalCollectors.ConnectWithProbe (probe,
                                                   "OutputSeconds",
                                                   identifier,
                                                   &ScalarCollector::TraceSinkDouble);
      break;

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      ret = m_terminalCollectors.ConnectWithProbe (probe,
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
          ret = m_terminalCollectors.ConnectWithProbe (probe,
                                                       "OutputSeconds",
                                                       identifier,
                                                       &ScalarCollector::TraceSinkDouble);
        }
      else
        {
          ret = m_terminalCollectors.ConnectWithProbe (probe,
                                                       "OutputSeconds",
                                                       identifier,
                                                       &DistributionCollector::TraceSinkDouble);
        }
      break;

    default:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;
    }

  if (ret)
    {
      NS_LOG_INFO (this << " created probe " << probe->GetName ()
                        << ", connected to collector " << identifier);
    }
  else
    {
      NS_LOG_WARN (this << " unable to connect probe " << probe->GetName ()
                        << " to collector " << identifier);
    }

  return ret;
}


void
SatStatsSatelliteQueueHelper::PassSampleToCollector (uint32_t size, uint32_t identifier)
{
  //NS_LOG_FUNCTION (this << size << identifier);

  Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (identifier);
  NS_ASSERT_MSG (collector != nullptr,
                 "Unable to find collector with identifier " << identifier);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
        NS_ASSERT (c != nullptr);
        c->TraceSinkDouble (0.0, size);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
        NS_ASSERT (c != nullptr);
        c->TraceSinkDouble (0.0, size);
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
          NS_ASSERT (c != nullptr);
          c->TraceSinkDouble (0.0, size);
        }
      else
        {
          Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector> ();
          NS_ASSERT (c != nullptr);
          c->TraceSinkDouble (0.0, size);
        }
      break;

    default:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    } // end of `switch (GetOutputType ())`

} // end of `void PassSampleToCollector (double, uint32_t)`


void
SatStatsSatelliteQueueHelper::InstallProbes ()
{
  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


// RTN FEEDER QUEUE IN BYTES ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnFeederQueueBytesHelper);

SatStatsRtnFeederQueueBytesHelper::SatStatsRtnFeederQueueBytesHelper (Ptr<const SatHelper> satHelper)
  : SatStatsSatelliteQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsSatelliteQueueHelper::UNIT_BYTES);
}


SatStatsRtnFeederQueueBytesHelper::~SatStatsRtnFeederQueueBytesHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnFeederQueueBytesHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnFeederQueueBytesHelper")
    .SetParent<SatStatsSatelliteQueueHelper> ()
  ;
  return tid;
}


void
SatStatsRtnFeederQueueBytesHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);
    }

  NodeContainer sats = GetSatHelper ()->GetBeamHelper ()->GetGeoSatNodes ();
  Callback<void, uint32_t, const Address &> callback
    = MakeCallback (&SatStatsRtnFeederQueueBytesHelper::QueueSizeCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != nullptr);
      Ptr<SatPhy> satPhy;
      std::map<uint32_t, Ptr<SatPhy> > satGeoFeederPhys = satGeoDev->GetFeederPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin (); it2 != satGeoFeederPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != nullptr);
          Ptr<SatGeoFeederPhy> satGeoFeederPhy = satPhy->GetObject<SatGeoFeederPhy> ();
          NS_ASSERT (satGeoFeederPhy != nullptr);

          if (satGeoFeederPhy->TraceConnectWithoutContext ("QueueSizeBytes", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to QueueSizeBytes trace source"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }

        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

} // end of `void DoInstallProbes ();`


// RTN FEEDER QUEUE IN PACKETS ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnFeederQueuePacketsHelper);

SatStatsRtnFeederQueuePacketsHelper::SatStatsRtnFeederQueuePacketsHelper (Ptr<const SatHelper> satHelper)
  : SatStatsSatelliteQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsSatelliteQueueHelper::UNIT_NUMBER_OF_PACKETS);
}


SatStatsRtnFeederQueuePacketsHelper::~SatStatsRtnFeederQueuePacketsHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnFeederQueuePacketsHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnFeederQueuePacketsHelper")
    .SetParent<SatStatsSatelliteQueueHelper> ()
  ;
  return tid;
}


void
SatStatsRtnFeederQueuePacketsHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);
    }

  NodeContainer sats = GetSatHelper ()->GetBeamHelper ()->GetGeoSatNodes ();
  Callback<void, uint32_t, const Address &> callback
    = MakeCallback (&SatStatsRtnFeederQueuePacketsHelper::QueueSizeCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != nullptr);
      Ptr<SatPhy> satPhy;
      std::map<uint32_t, Ptr<SatPhy> > satGeoFeederPhys = satGeoDev->GetFeederPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin (); it2 != satGeoFeederPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != nullptr);
          Ptr<SatGeoFeederPhy> satGeoFeederPhy = satPhy->GetObject<SatGeoFeederPhy> ();
          NS_ASSERT (satGeoFeederPhy != nullptr);

          if (satGeoFeederPhy->TraceConnectWithoutContext ("QueueSizePackets", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to QueueSizePackets trace source"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

} // end of `void DoInstallProbes ();`


// FWD USER QUEUE IN BYTES ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdUserQueueBytesHelper);

SatStatsFwdUserQueueBytesHelper::SatStatsFwdUserQueueBytesHelper (Ptr<const SatHelper> satHelper)
  : SatStatsSatelliteQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsSatelliteQueueHelper::UNIT_BYTES);
}


SatStatsFwdUserQueueBytesHelper::~SatStatsFwdUserQueueBytesHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdUserQueueBytesHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdUserQueueBytesHelper")
    .SetParent<SatStatsSatelliteQueueHelper> ()
  ;
  return tid;
}


void
SatStatsFwdUserQueueBytesHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);
    }

  NodeContainer sats = GetSatHelper ()->GetBeamHelper ()->GetGeoSatNodes ();
  Callback<void, uint32_t, const Address &> callback
    = MakeCallback (&SatStatsFwdUserQueueBytesHelper::QueueSizeCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != nullptr);
      Ptr<SatPhy> satPhy;
      std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin (); it2 != satGeoUserPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != nullptr);
          Ptr<SatGeoUserPhy> satGeoUserPhy = satPhy->GetObject<SatGeoUserPhy> ();
          NS_ASSERT (satGeoUserPhy != nullptr);

          if (satGeoUserPhy->TraceConnectWithoutContext ("QueueSizeBytes", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to QueueSizeBytes trace source"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }

        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

} // end of `void DoInstallProbes ();`


// FWD USER QUEUE IN PACKETS ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdUserQueuePacketsHelper);

SatStatsFwdUserQueuePacketsHelper::SatStatsFwdUserQueuePacketsHelper (Ptr<const SatHelper> satHelper)
  : SatStatsSatelliteQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsSatelliteQueueHelper::UNIT_NUMBER_OF_PACKETS);
}


SatStatsFwdUserQueuePacketsHelper::~SatStatsFwdUserQueuePacketsHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdUserQueuePacketsHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdUserQueuePacketsHelper")
    .SetParent<SatStatsSatelliteQueueHelper> ()
  ;
  return tid;
}


void
SatStatsFwdUserQueuePacketsHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);
    }

  NodeContainer sats = GetSatHelper ()->GetBeamHelper ()->GetGeoSatNodes ();
  Callback<void, uint32_t, const Address &> callback
    = MakeCallback (&SatStatsFwdUserQueuePacketsHelper::QueueSizeCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != nullptr);
      Ptr<SatPhy> satPhy;
      std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin (); it2 != satGeoUserPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != nullptr);
          Ptr<SatGeoUserPhy> satGeoUserPhy = satPhy->GetObject<SatGeoUserPhy> ();
          NS_ASSERT (satGeoUserPhy != nullptr);

          if (satGeoUserPhy->TraceConnectWithoutContext ("QueueSizePackets", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to QueueSizePackets trace source"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

} // end of `void DoInstallProbes ();`


} // end of namespace ns3
