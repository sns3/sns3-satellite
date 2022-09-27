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
#include <ns3/nstime.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/callback.h>

#include <ns3/node-container.h>
#include <ns3/application.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-phy.h>

#include <ns3/satellite-time-tag.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>

#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-delay-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>
#include <ns3/traffic-time-tag.h>

#include <sstream>
#include "satellite-stats-link-delay-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsLinkDelayHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsLinkDelayHelper);

SatStatsLinkDelayHelper::SatStatsLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
  m_averagingMode (false)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsLinkDelayHelper::~SatStatsLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsLinkDelayHelper")
    .SetParent<SatStatsHelper> ()
    .AddAttribute ("AveragingMode",
                   "If true, all samples will be averaged before passed to aggregator. "
                   "Only affects histogram, PDF, and CDF output types.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatStatsLinkDelayHelper::SetAveragingMode,
                                        &SatStatsLinkDelayHelper::GetAveragingMode),
                   MakeBooleanChecker ())
  ;
  return tid;
}


void
SatStatsLinkDelayHelper::SetAveragingMode (bool averagingMode)
{
  NS_LOG_FUNCTION (this << averagingMode);
  m_averagingMode = averagingMode;
}


bool
SatStatsLinkDelayHelper::GetAveragingMode () const
{
  return m_averagingMode;
}


void
SatStatsLinkDelayHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("delay_sec")));

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
                                         "GeneralHeading", StringValue (GetTimeHeading ("delay_sec")));

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
                                             "GeneralHeading", StringValue (GetDistributionHeading ("delay_sec")));
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
                                             "GeneralHeading", StringValue (GetDistributionHeading ("delay_sec")));

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
                                   "Packet delay (in seconds)");
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
            plotAggregator->SetLegend ("Packet delay (in seconds)",
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
            plotAggregator->SetLegend ("Packet delay (in seconds)",
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
SatStatsLinkDelayHelper::InstallProbes ()
{
  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


void
SatStatsLinkDelayHelper::RxLinkDelayCallback (const Time &delay, const Address &from)
{
  //NS_LOG_FUNCTION (this << delay.GetSeconds () << from);

  if (from.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding a packet delay of " << delay.GetSeconds ()
                        << " from statistics collection because of"
                        << " invalid sender address");
    }
  else if (Mac48Address::ConvertFrom (from).IsBroadcast ())
    {
      for (std::pair<const Address, uint32_t> item : m_identifierMap)
        {
          PassSampleToCollector (delay, item.second);
        }
    }
  else
    {
      // Determine the identifier associated with the sender address.
      std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find (from);

      if (it != m_identifierMap.end ())
        {
          PassSampleToCollector (delay, it->second);
        }
      else
        {
          NS_LOG_WARN (this << " discarding a packet delay of " << delay.GetSeconds ()
                            << " from statistics collection because of"
                            << " unknown sender address " << from);
        }
    }
}


void
SatStatsLinkDelayHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
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
SatStatsLinkDelayHelper::ConnectProbeToCollector (Ptr<Probe> probe,
                                              uint32_t identifier)
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
SatStatsLinkDelayHelper::PassSampleToCollector (const Time &delay, uint32_t identifier)
{
  //NS_LOG_FUNCTION (this << delay.GetSeconds () << identifier);

  Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (identifier);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to find collector with identifier " << identifier);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkDouble (0.0, delay.GetSeconds ());
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkDouble (0.0, delay.GetSeconds ());
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
          c->TraceSinkDouble (0.0, delay.GetSeconds ());
        }
      else
        {
          Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector> ();
          NS_ASSERT (c != 0);
          c->TraceSinkDouble (0.0, delay.GetSeconds ());
        }
      break;

    default:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    } // end of `switch (GetOutputType ())`

} // end of `void PassSampleToCollector (Time, uint32_t)`


// FORWARD FEEDER LINK MAC-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdFeederMacLinkDelayHelper);

SatStatsFwdFeederMacLinkDelayHelper::SatStatsFwdFeederMacLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdFeederMacLinkDelayHelper::~SatStatsFwdFeederMacLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdFeederMacLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdFeederMacLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsFwdFeederMacLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());
  Callback<void, const Time &, const Address &> callback
    = MakeCallback (&SatStatsFwdFeederMacLinkDelayHelper::RxLinkDelayCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatMac> > satGeoFeederMacs = satGeoDev->GetFeederMac ();
      Ptr<SatMac> satMac;
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin (); it2 != satGeoFeederMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));

          // Connect the object to the probe.
          if (satMac->TraceConnectWithoutContext ("RxLinkDelay", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());

              // Enable statistics-related tags and trace sources on the device.
              satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }
        }
      std::map<uint32_t, Ptr<SatMac> > satGeoUserMacs = satGeoDev->GetUserMac ();
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin (); it2 != satGeoUserMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();

  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatMac> satMac = satDev->GetMac ();
      NS_ASSERT (satMac != 0);

      satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
      satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

  // Enable statistics-related tags on the transmitting device.
  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatMac> satMac = satDev->GetMac ();
          NS_ASSERT (satMac != 0);

          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    }


} // end of `void DoInstallProbes ();`


// FORWARD USER LINK MAC-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdUserMacLinkDelayHelper);

SatStatsFwdUserMacLinkDelayHelper::SatStatsFwdUserMacLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdUserMacLinkDelayHelper::~SatStatsFwdUserMacLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdUserMacLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdUserMacLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsFwdUserMacLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatMac> > satGeoFeederMacs = satGeoDev->GetFeederMac ();
      Ptr<SatMac> satMac;
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin (); it2 != satGeoFeederMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      std::map<uint32_t, Ptr<SatMac> > satGeoUserMacs = satGeoDev->GetUserMac ();
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin (); it2 != satGeoUserMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();

  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      const int32_t utId = GetUtId (*it);
      NS_ASSERT_MSG (utId > 0,
                     "Node " << (*it)->GetId () << " is not a valid UT");
      const uint32_t identifier = GetIdentifierForUt (*it);

      // Create the probe.
      std::ostringstream probeName;
      probeName << utId;
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatMac> satMac = satDev->GetMac ();
      NS_ASSERT (satMac != 0);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("RxLinkDelay", satMac)
          && ConnectProbeToCollector (probe, identifier))
        {
          m_probes.push_back (probe->GetObject<Probe> ());

          // Enable statistics-related tags and trace sources on the device.
          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      else
        {
          NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatMac"
                          << " at node ID " << (*it)->GetId () << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

  // Enable statistics-related tags on the transmitting device.
  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatMac> satMac = satDev->GetMac ();
          NS_ASSERT (satMac != 0);

          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    }

} // end of `void DoInstallProbes ();`


// FORWARD FEEDER LINK PHY-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdFeederPhyLinkDelayHelper);

SatStatsFwdFeederPhyLinkDelayHelper::SatStatsFwdFeederPhyLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdFeederPhyLinkDelayHelper::~SatStatsFwdFeederPhyLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdFeederPhyLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdFeederPhyLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsFwdFeederPhyLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());
  Callback<void, const Time &, const Address &> callback
    = MakeCallback (&SatStatsFwdFeederPhyLinkDelayHelper::RxLinkDelayCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatPhy> > satGeoFeederPhys = satGeoDev->GetFeederPhy ();
      Ptr<SatPhy> satPhy;
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin (); it2 != satGeoFeederPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));

          // Connect the object to the probe.
          if (satPhy->TraceConnectWithoutContext ("RxLinkDelay", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());

              // Enable statistics-related tags and trace sources on the device.
              satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }
        }
      std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin (); it2 != satGeoUserPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();

  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);

      satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
      satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

  // Enable statistics-related tags on the transmitting device.
  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatPhy> satPhy = satDev->GetPhy ();
          NS_ASSERT (satPhy != 0);

          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    }

} // end of `void DoInstallProbes ();`


// FORWARD USER LINK PHY-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdUserPhyLinkDelayHelper);

SatStatsFwdUserPhyLinkDelayHelper::SatStatsFwdUserPhyLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdUserPhyLinkDelayHelper::~SatStatsFwdUserPhyLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdUserPhyLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdUserPhyLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsFwdUserPhyLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatPhy> > satGeoFeederPhys = satGeoDev->GetFeederPhy ();
      Ptr<SatPhy> satPhy;
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin (); it2 != satGeoFeederPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin (); it2 != satGeoUserPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();

  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      const int32_t utId = GetUtId (*it);
      NS_ASSERT_MSG (utId > 0,
                     "Node " << (*it)->GetId () << " is not a valid UT");
      const uint32_t identifier = GetIdentifierForUt (*it);

      // Create the probe.
      std::ostringstream probeName;
      probeName << utId;
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("RxLinkDelay", satPhy)
          && ConnectProbeToCollector (probe, identifier))
        {
          m_probes.push_back (probe->GetObject<Probe> ());

          // Enable statistics-related tags and trace sources on the device.
          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      else
        {
          NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatPhy"
                          << " at node ID " << (*it)->GetId () << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

  // Enable statistics-related tags on the transmitting device.
  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatPhy> satPhy = satDev->GetPhy ();
          NS_ASSERT (satPhy != 0);

          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    }

} // end of `void DoInstallProbes ();`


// RETURN FEEDER LINK MAC-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnFeederMacLinkDelayHelper);

SatStatsRtnFeederMacLinkDelayHelper::SatStatsRtnFeederMacLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnFeederMacLinkDelayHelper::~SatStatsRtnFeederMacLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnFeederMacLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnFeederMacLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsRtnFeederMacLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<SatMac> satMac;
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatMac> > satGeoFeederMacs = satGeoDev->GetFeederMac ();
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin (); it2 != satGeoFeederMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      std::map<uint32_t, Ptr<SatMac> > satGeoUserMacs = satGeoDev->GetUserMac ();
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin (); it2 != satGeoUserMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      // Enable statistics-related tags and trace sources on the device.
      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatMac> satMac = satDev->GetMac ();
      NS_ASSERT (satMac != 0);
      satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
      satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, const Time &, const Address &> callback
    = MakeCallback (&SatStatsRtnFeederMacLinkDelayHelper::RxLinkDelayCallback, this);

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatMac> satMac = satDev->GetMac ();
          NS_ASSERT (satMac != 0);

          // Connect the object to the probe.
          if (satMac->TraceConnectWithoutContext ("RxLinkDelay", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satDev->GetIfIndex ());

              // Enable statistics-related tags and trace sources on the device.
              satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
              satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satDev->GetIfIndex ());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN USER LINK MAC-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnUserMacLinkDelayHelper);

SatStatsRtnUserMacLinkDelayHelper::SatStatsRtnUserMacLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnUserMacLinkDelayHelper::~SatStatsRtnUserMacLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnUserMacLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnUserMacLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsRtnUserMacLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());
  Callback<void, const Time &, const Address &> callback
    = MakeCallback (&SatStatsRtnUserMacLinkDelayHelper::RxLinkDelayCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      const int32_t satId = GetSatId (*it);
      NS_ASSERT_MSG (satId > 0,
                     "Node " << (*it)->GetId () << " is not a valid SAT");

      // Create the probe.
      std::ostringstream probeName;
      probeName << satId;
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatMac> > satGeoFeederMacs = satGeoDev->GetFeederMac ();
      Ptr<SatMac> satMac;
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoFeederMacs.begin (); it2 != satGeoFeederMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      std::map<uint32_t, Ptr<SatMac> > satGeoUserMacs = satGeoDev->GetUserMac ();
      for (std::map<uint32_t, Ptr<SatMac>>::iterator it2 = satGeoUserMacs.begin (); it2 != satGeoUserMacs.end (); ++it2)
        {
          satMac = it2->second;
          NS_ASSERT (satMac != 0);
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));

          // Connect the object to the probe.
          if (satMac->TraceConnectWithoutContext ("RxLinkDelay", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      // Enable statistics-related tags and trace sources on the device.
      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatMac> satMac = satDev->GetMac ();
      NS_ASSERT (satMac != 0);
      satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
      satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatMac> satMac = satDev->GetMac ();
          NS_ASSERT (satMac != 0);

          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN FEEDER LINK PHY-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnFeederPhyLinkDelayHelper);

SatStatsRtnFeederPhyLinkDelayHelper::SatStatsRtnFeederPhyLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnFeederPhyLinkDelayHelper::~SatStatsRtnFeederPhyLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnFeederPhyLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnFeederPhyLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsRtnFeederPhyLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      Ptr<SatPhy> satPhy;
      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatPhy> > satGeoFeederPhys = satGeoDev->GetFeederPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin (); it2 != satGeoFeederPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin (); it2 != satGeoUserPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      // Enable statistics-related tags and trace sources on the device.
      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);
      satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
      satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, const Time &, const Address &> callback
    = MakeCallback (&SatStatsRtnFeederPhyLinkDelayHelper::RxLinkDelayCallback, this);

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatPhy> satPhy = satDev->GetPhy ();
          NS_ASSERT (satPhy != 0);

          // Connect the object to the probe.
          if (satPhy->TraceConnectWithoutContext ("RxLinkDelay", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satDev->GetIfIndex ());

              // Enable statistics-related tags and trace sources on the device.
              satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
              satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satDev->GetIfIndex ());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN USER LINK PHY-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnUserPhyLinkDelayHelper);

SatStatsRtnUserPhyLinkDelayHelper::SatStatsRtnUserPhyLinkDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnUserPhyLinkDelayHelper::~SatStatsRtnUserPhyLinkDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnUserPhyLinkDelayHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnUserPhyLinkDelayHelper")
    .SetParent<SatStatsLinkDelayHelper> ()
  ;
  return tid;
}


void
SatStatsRtnUserPhyLinkDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());
  Callback<void, const Time &, const Address &> callback
    = MakeCallback (&SatStatsRtnUserPhyLinkDelayHelper::RxLinkDelayCallback, this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      const int32_t satId = GetSatId (*it);
      NS_ASSERT_MSG (satId > 0,
                     "Node " << (*it)->GetId () << " is not a valid SAT");

      // Create the probe.
      std::ostringstream probeName;
      probeName << satId;
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetSatSatGeoNetDevice (*it);
      Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
      NS_ASSERT (satGeoDev != 0);
      std::map<uint32_t, Ptr<SatPhy> > satGeoFeederPhys = satGeoDev->GetFeederPhy ();
      Ptr<SatPhy> satPhy;
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoFeederPhys.begin (); it2 != satGeoFeederPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
      std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
      for (std::map<uint32_t, Ptr<SatPhy>>::iterator it2 = satGeoUserPhys.begin (); it2 != satGeoUserPhys.end (); ++it2)
        {
          satPhy = it2->second;
          NS_ASSERT (satPhy != 0);
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));

          // Connect the object to the probe.
          if (satPhy->TraceConnectWithoutContext ("RxLinkDelay", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << satGeoDev->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxLinkDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satGeoDev->GetIfIndex ());
            }
        }
    } // end of `for (it = sats.Begin(); it != sats.End (); ++it)`

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      // Enable statistics-related tags and trace sources on the device.
      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);
      satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
      satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatPhy> satPhy = satDev->GetPhy ();
          NS_ASSERT (satPhy != 0);

          satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
          satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


} // end of namespace ns3
