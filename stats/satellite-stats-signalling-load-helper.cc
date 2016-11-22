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
#include <ns3/mac48-address.h>
#include <ns3/satellite-net-device.h>

#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>

#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-signalling-load-helper.h"


NS_LOG_COMPONENT_DEFINE ("SatStatsSignallingLoadHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsSignallingLoadHelper);

SatStatsSignallingLoadHelper::SatStatsSignallingLoadHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsSignallingLoadHelper::~SatStatsSignallingLoadHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsSignallingLoadHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsSignallingLoadHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsSignallingLoadHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("signalling_kbps")));

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
                                         "GeneralHeading", StringValue (GetTimeHeading ("signalling_kbps")));

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
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

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
                                   "Signalling load (in kilobits per second)");
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
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    default:
      NS_FATAL_ERROR ("SatStatsSignallingLoadHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to conversion collectors.
  InstallProbes ();

} // end of `void DoInstall ();`


void
SatStatsSignallingLoadHelper::InstallProbes ()
{
  NS_LOG_FUNCTION (this);

  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


void
SatStatsSignallingLoadHelper::SignallingTxCallback (Ptr<const Packet> packet,
                                                    const Address &to)
{
  //NS_LOG_FUNCTION (this << packet->GetSize () << from);

  if (to.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding packet " << packet
                        << " (" << packet->GetSize () << " bytes)"
                        << " from statistics collection because of"
                        << " invalid sender address");
    }
  else
    {
      const Mac48Address addr = Mac48Address::ConvertFrom (to);

      if (addr.IsBroadcast ())
        {
          NS_LOG_INFO (this << " broadcast control message packet");

          // Pass the sample to every first-level collectors.
          for (CollectorMap::Iterator it = m_conversionCollectors.Begin ();
               it != m_conversionCollectors.End (); ++it)
            {
              Ptr<UnitConversionCollector> c = it->second->GetObject<UnitConversionCollector> ();
              NS_ASSERT (c != 0);
              c->TraceSinkUinteger32 (0, packet->GetSize ());
            }
        }
      else
        {
          // Determine the identifier associated with the sender address.
          std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find (addr);

          if (it == m_identifierMap.end ())
            {
              NS_LOG_WARN (this << " discarding packet " << packet
                                << " (" << packet->GetSize () << " bytes)"
                                << " from statistics collection because of"
                                << " unknown sender address " << addr);
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
    }

} // end of `void RxCallback (Ptr<const Packet>, const Address);`


void
SatStatsSignallingLoadHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
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


// FORWARD LINK ///////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdSignallingLoadHelper);

SatStatsFwdSignallingLoadHelper::SatStatsFwdSignallingLoadHelper (Ptr<const SatHelper> satHelper)
  : SatStatsSignallingLoadHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdSignallingLoadHelper::~SatStatsFwdSignallingLoadHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdSignallingLoadHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdSignallingLoadHelper")
    .SetParent<SatStatsSignallingLoadHelper> ()
  ;
  return tid;
}


void
SatStatsFwdSignallingLoadHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsFwdSignallingLoadHelper::SignallingTxCallback,
                    this);

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          NS_ASSERT ((*itDev)->GetObject<SatNetDevice> () != 0);

          if ((*itDev)->TraceConnectWithoutContext ("SignallingTx", callback))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << (*itDev)->GetIfIndex ());
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to SignallingTx trace source"
                              << " of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << (*itDev)->GetIfIndex ());
            }

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN LINK ////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnSignallingLoadHelper);

SatStatsRtnSignallingLoadHelper::SatStatsRtnSignallingLoadHelper (Ptr<const SatHelper> satHelper)
  : SatStatsSignallingLoadHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnSignallingLoadHelper::~SatStatsRtnSignallingLoadHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnSignallingLoadHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnSignallingLoadHelper")
    .SetParent<SatStatsSignallingLoadHelper> ()
  ;
  return tid;
}


void
SatStatsRtnSignallingLoadHelper::DoInstallProbes ()
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
      if (probe->ConnectByObject ("SignallingTx", dev))
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

        } // end of `if (probe->ConnectByObject ("SignallingTx", dev))`
      else
        {
          NS_FATAL_ERROR ("Error connecting to SignallingTx trace source of SatNetDevice"
                          << " at node ID " << (*it)->GetId () << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`


} // end of namespace ns3
