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
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/callback.h>

#include <ns3/packet.h>
#include <ns3/node-container.h>
#include <ns3/application.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-phy.h>

#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>

#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-throughput-helper.h"


NS_LOG_COMPONENT_DEFINE ("SatStatsThroughputHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsThroughputHelper);

SatStatsThroughputHelper::SatStatsThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
    m_averagingMode (false)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsThroughputHelper::~SatStatsThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsThroughputHelper")
    .SetParent<SatStatsHelper> ()
    .AddAttribute ("AveragingMode",
                   "If true, all samples will be averaged before passed to aggregator. "
                   "Only affects histogram, PDF, and CDF output types.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatStatsThroughputHelper::SetAveragingMode,
                                        &SatStatsThroughputHelper::GetAveragingMode),
                   MakeBooleanChecker ())
  ;
  return tid;
}


void
SatStatsThroughputHelper::SetAveragingMode (bool averagingMode)
{
  NS_LOG_FUNCTION (this << averagingMode);
  m_averagingMode = averagingMode;
}


bool
SatStatsThroughputHelper::GetAveragingMode () const
{
  return m_averagingMode;
}


void
SatStatsThroughputHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("throughput_kbps")));

        // Setup second-level collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);

        // Setup first-level collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &ScalarCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetTimeHeading ("throughput_kbps")));

        // Setup second-level collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator ("OutputString",
                                                  m_aggregator,
                                                  &MultiFileAggregator::AddContextHeading);

        // Setup first-level collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      {
        if (!m_averagingMode)
          {
            NS_FATAL_ERROR ("This statistics require AveragingMode to be enabled");
          }

        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "MultiFileMode", BooleanValue (false),
                                         "EnableContextPrinting", BooleanValue (false),
                                         "GeneralHeading", StringValue (GetDistributionHeading ("throughput_kbps")));
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

        // Setup second-level collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        Callback<void, double> callback
          = MakeCallback (&DistributionCollector::TraceSinkDouble1,
                          m_averagingCollector);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            it->second->TraceConnectWithoutContext ("Output", callback);
          }

        // Setup first-level collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &ScalarCollector::TraceSinkDouble);
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
                                   "Received throughput (in kilobits per second)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        // Setup second-level collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MagisterGnuplotAggregator::Write2d);

        // Setup first-level collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      {
        if (!m_averagingMode)
          {
            NS_FATAL_ERROR ("This statistics require AveragingMode to be enabled");
          }

        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                         "OutputPath", StringValue (GetOutputPath ()),
                                         "OutputFileName", StringValue (GetName ()));
        Ptr<MagisterGnuplotAggregator> plotAggregator
          = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
        NS_ASSERT (plotAggregator != 0);
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Received throughput (in kilobits per second)",
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

        // Setup second-level collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        Callback<void, double> callback
          = MakeCallback (&DistributionCollector::TraceSinkDouble1,
                          m_averagingCollector);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            it->second->TraceConnectWithoutContext ("Output", callback);
          }

        // Setup first-level collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &ScalarCollector::TraceSinkDouble);
        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsThroughputHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to conversion collectors.
  InstallProbes ();

} // end of `void DoInstall ();`


void
SatStatsThroughputHelper::InstallProbes ()
{
  NS_LOG_FUNCTION (this);

  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


void
SatStatsThroughputHelper::RxCallback (Ptr<const Packet> packet,
                                      const Address &from)
{
  //NS_LOG_FUNCTION (this << packet->GetSize () << from);

  if (from.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding packet " << packet
                        << " (" << packet->GetSize () << " bytes)"
                        << " from statistics collection because of"
                        << " invalid sender address");
    }
  else
    {
      // Determine the identifier associated with the sender address.
      std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find (from);

      if (it == m_identifierMap.end ())
        {
          NS_LOG_WARN (this << " discarding packet " << packet
                            << " (" << packet->GetSize () << " bytes)"
                            << " from statistics collection because of"
                            << " unknown sender address " << from);
        }
      else
        {
          // Find the first-level collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_conversionCollectors.Get (it->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it->second);
          Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
          NS_ASSERT (c != 0);

          // Pass the sample to the collector.
          c->TraceSinkUinteger32 (0, packet->GetSize ());
        }
    }

} // end of `void RxCallback (Ptr<const Packet>, const Address);`


void
SatStatsThroughputHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
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


// FORWARD LINK APPLICATION-LEVEL /////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdAppThroughputHelper);

SatStatsFwdAppThroughputHelper::SatStatsFwdAppThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdAppThroughputHelper::~SatStatsFwdAppThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdAppThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdAppThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsFwdAppThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();

  for (NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); ++it)
    {
      const int32_t utUserId = GetUtUserId (*it);
      NS_ASSERT_MSG (utUserId > 0,
                     "Node " << (*it)->GetId () << " is not a valid UT user");
      const uint32_t identifier = GetIdentifierForUtUser (*it);

      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          // Create the probe.
          std::ostringstream probeName;
          probeName << utUserId << "-" << i;
          Ptr<ApplicationPacketProbe> probe = CreateObject<ApplicationPacketProbe> ();
          probe->SetName (probeName.str ());

          // Connect the object to the probe.
          if (probe->ConnectByObject ("Rx", (*it)->GetApplication (i)))
            {
              // Connect the probe to the right collector.
              if (m_conversionCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                           "OutputBytes",
                                                           identifier,
                                                           &UnitConversionCollector::TraceSinkUinteger32))
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
              /*
               * We're being tolerant here by only logging a warning, because
               * not every kind of Application is equipped with the expected
               * Rx trace source.
               */
              NS_LOG_WARN (this << " unable to connect probe " << probeName.str ()
                                << " with node ID " << (*it)->GetId ()
                                << " application #" << i);
            }

        } // end of `for (i = 0; i < (*it)->GetNApplications (); i++)`

    } // end of `for (it = utUsers.Begin(); it != utUsers.End (); ++it)`

} // end of `void DoInstallProbes ();`


// FORWARD LINK DEVICE-LEVEL //////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdDevThroughputHelper);

SatStatsFwdDevThroughputHelper::SatStatsFwdDevThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdDevThroughputHelper::~SatStatsFwdDevThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdDevThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdDevThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsFwdDevThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);
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
      Ptr<ApplicationPacketProbe> probe = CreateObject<ApplicationPacketProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("Rx", dev))
        {
          // Connect the probe to the right collector.
          if (m_conversionCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                       "OutputBytes",
                                                       identifier,
                                                       &UnitConversionCollector::TraceSinkUinteger32))
            {
              NS_LOG_INFO (this << " created probe " << probeName.str ()
                                << ", connected to collector " << identifier);
              m_probes.push_back (probe->GetObject<Probe> ());

              // Enable statistics-related tags and trace sources on the device.
              dev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_LOG_WARN (this << " unable to connect probe " << probeName.str ()
                                << " to collector " << identifier);
            }

        } // end of `if (probe->ConnectByObject ("Rx", dev))`
      else
        {
          NS_FATAL_ERROR ("Error connecting to Rx trace source of SatNetDevice"
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
          NS_ASSERT ((*itDev)->GetObject<SatNetDevice> () != 0);
          (*itDev)->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
        }
    }

} // end of `void DoInstallProbes ();`


// FORWARD LINK MAC-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdMacThroughputHelper);

SatStatsFwdMacThroughputHelper::SatStatsFwdMacThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdMacThroughputHelper::~SatStatsFwdMacThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdMacThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdMacThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsFwdMacThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);
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
      Ptr<ApplicationPacketProbe> probe = CreateObject<ApplicationPacketProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatMac> satMac = satDev->GetMac ();
      NS_ASSERT (satMac != 0);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("Rx", satMac))
        {
          // Connect the probe to the right collector.
          if (m_conversionCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                       "OutputBytes",
                                                       identifier,
                                                       &UnitConversionCollector::TraceSinkUinteger32))
            {
              NS_LOG_INFO (this << " created probe " << probeName.str ()
                                << ", connected to collector " << identifier);
              m_probes.push_back (probe->GetObject<Probe> ());

              // Enable statistics-related tags and trace sources on the device.
              satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
              satMac->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_LOG_WARN (this << " unable to connect probe " << probeName.str ()
                                << " to collector " << identifier);
            }

        } // end of `if (probe->ConnectByObject ("Rx", satMac))`
      else
        {
          NS_FATAL_ERROR ("Error connecting to Rx trace source of SatMac"
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


// FORWARD LINK PHY-LEVEL /////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdPhyThroughputHelper);

SatStatsFwdPhyThroughputHelper::SatStatsFwdPhyThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdPhyThroughputHelper::~SatStatsFwdPhyThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdPhyThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdPhyThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsFwdPhyThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);
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
      Ptr<ApplicationPacketProbe> probe = CreateObject<ApplicationPacketProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("Rx", satPhy))
        {
          // Connect the probe to the right collector.
          if (m_conversionCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                       "OutputBytes",
                                                       identifier,
                                                       &UnitConversionCollector::TraceSinkUinteger32))
            {
              NS_LOG_INFO (this << " created probe " << probeName.str ()
                                << ", connected to collector " << identifier);
              m_probes.push_back (probe->GetObject<Probe> ());

              // Enable statistics-related tags and trace sources on the device.
              satDev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
              satPhy->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
            }
          else
            {
              NS_LOG_WARN (this << " unable to connect probe " << probeName.str ()
                                << " to collector " << identifier);
            }

        } // end of `if (probe->ConnectByObject ("Rx", satPhy))`
      else
        {
          NS_FATAL_ERROR ("Error connecting to Rx trace source of SatPhy"
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


// RETURN LINK APPLICATION-LEVEL //////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnAppThroughputHelper);

SatStatsRtnAppThroughputHelper::SatStatsRtnAppThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnAppThroughputHelper::~SatStatsRtnAppThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnAppThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnAppThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsRtnAppThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  // Create a map of UT user addresses and identifiers.
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();
  for (NodeContainer::Iterator it = utUsers.Begin ();
       it != utUsers.End (); ++it)
    {
      SaveIpv4AddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW user node's applications.

  NodeContainer gwUsers = GetSatHelper ()->GetGwUsers ();
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsRtnAppThroughputHelper::Ipv4Callback, this);

  for (NodeContainer::Iterator it = gwUsers.Begin ();
       it != gwUsers.End (); ++it)
    {
      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          Ptr<Application> app = (*it)->GetApplication (i);

          if (app->TraceConnectWithoutContext ("Rx", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID " << (*it)->GetId ()
                                << " application #" << i);
            }
          else
            {
              /*
               * We're being tolerant here by only logging a warning, because
               * not every kind of Application is equipped with the expected
               * Rx trace source.
               */
              NS_LOG_WARN (this << " unable to connect with node ID " << (*it)->GetId ()
                                << " application #" << i);
            }
        }
    }

} // end of `void DoInstallProbes ();`


void
SatStatsRtnAppThroughputHelper::Ipv4Callback (Ptr<const Packet> packet,
                                              const Address &from)
{
  //NS_LOG_FUNCTION (this << packet->GetSize () << from);

  if (InetSocketAddress::IsMatchingType (from))
    {
      // Determine the identifier associated with the sender address.
      const Address ipv4Addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
      std::map<const Address, uint32_t>::const_iterator it1 = m_identifierMap.find (ipv4Addr);

      if (it1 == m_identifierMap.end ())
        {
          NS_LOG_WARN (this << " discarding packet " << packet
                            << " (" << packet->GetSize () << " bytes)"
                            << " from statistics collection because of"
                            << " unknown sender IPv4 address " << ipv4Addr);
        }
      else
        {
          // Find the collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_conversionCollectors.Get (it1->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it1->second);
          Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
          NS_ASSERT (c != 0);

          // Pass the sample to the collector.
          c->TraceSinkUinteger32 (0, packet->GetSize ());
        }
    }
  else
    {
      NS_LOG_WARN (this << " discarding packet " << packet
                        << " (" << packet->GetSize () << " bytes)"
                        << " from statistics collection"
                        << " because it comes from sender " << from
                        << " without valid InetSocketAddress");
    }

} // end of `void Ipv4Callback (Ptr<const Packet>, const Address);`


void
SatStatsRtnAppThroughputHelper::SaveIpv4AddressAndIdentifier (Ptr<Node> utUserNode)
{
  NS_LOG_FUNCTION (this << utUserNode->GetId ());

  Ptr<Ipv4> ipv4 = utUserNode->GetObject<Ipv4> ();

  if (ipv4 == 0)
    {
      NS_LOG_INFO (this << " Node " << utUserNode->GetId ()
                         << " does not support IPv4 protocol");
    }
  else if (ipv4->GetNInterfaces () >= 2)
    {
      const uint32_t identifier = GetIdentifierForUtUser (utUserNode);

      /*
       * Assuming that #0 is for loopback interface and #1 is for subscriber
       * network interface.
       */
      for (uint32_t i = 0; i < ipv4->GetNAddresses (1); i++)
        {
          const Address addr = ipv4->GetAddress (1, i).GetLocal ();
          m_identifierMap[addr] = identifier;
          NS_LOG_INFO (this << " associated address " << addr
                            << " with identifier " << identifier);
        }
    }
  else
    {
      NS_LOG_WARN (this << " Node " << utUserNode->GetId ()
                        << " is not a valid UT user");
    }
}


// RETURN LINK DEVICE-LEVEL ///////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnDevThroughputHelper);

SatStatsRtnDevThroughputHelper::SatStatsRtnDevThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnDevThroughputHelper::~SatStatsRtnDevThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnDevThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnDevThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsRtnDevThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);

      // Enable statistics-related tags and trace sources on the device.
      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      dev->SetAttribute ("EnableStatisticsTags", BooleanValue (true));
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsRtnDevThroughputHelper::RxCallback, this);

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          NS_ASSERT ((*itDev)->GetObject<SatNetDevice> () != 0);

          if ((*itDev)->TraceConnectWithoutContext ("Rx", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << (*itDev)->GetIfIndex ());

              // Enable statistics-related tags and trace sources on the device.
              (*itDev)->SetAttribute ("EnableStatisticsTags",
                                      BooleanValue (true));
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to Rx trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << (*itDev)->GetIfIndex ());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN LINK MAC-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnMacThroughputHelper);

SatStatsRtnMacThroughputHelper::SatStatsRtnMacThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnMacThroughputHelper::~SatStatsRtnMacThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnMacThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnMacThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsRtnMacThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

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
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsRtnMacThroughputHelper::RxCallback, this);

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
          if (satMac->TraceConnectWithoutContext ("Rx", callback))
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
              NS_FATAL_ERROR ("Error connecting to Rx trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satDev->GetIfIndex ());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN LINK PHY-LEVEL //////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnPhyThroughputHelper);

SatStatsRtnPhyThroughputHelper::SatStatsRtnPhyThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsThroughputHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnPhyThroughputHelper::~SatStatsRtnPhyThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnPhyThroughputHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnPhyThroughputHelper")
    .SetParent<SatStatsThroughputHelper> ()
  ;
  return tid;
}


void
SatStatsRtnPhyThroughputHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

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
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsRtnPhyThroughputHelper::RxCallback, this);

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
          if (satPhy->TraceConnectWithoutContext ("Rx", callback))
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
              NS_FATAL_ERROR ("Error connecting to Rx trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << satDev->GetIfIndex ());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


} // end of namespace ns3
