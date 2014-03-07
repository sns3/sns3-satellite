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

#include "satellite-stats-rtn-throughput-helper.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/node-container.h>
#include <ns3/inet-socket-address.h>
#include <ns3/application.h>
#include <ns3/ipv4.h>
#include <ns3/net-device.h>
#include <ns3/packet.h>
#include <ns3/satellite-helper.h>
#include <ns3/data-collection-object.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsRtnThroughputHelper");


namespace ns3 {

SatStatsRtnThroughputHelper::SatStatsRtnThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}


SatStatsRtnThroughputHelper::~SatStatsRtnThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnThroughputHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case OUTPUT_NONE:
      break;

    case OUTPUT_SCALAR_FILE:
      {
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName () + ".txt"),
                                         "MultiFileMode", BooleanValue (false));
        CreateCollectors ("ns3::ScalarCollector",
                          m_terminalCollectors,
                          "InputDataType", EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE),
                          "OutputType", EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        ConnectCollectorsToAggregator (m_terminalCollectors,
                                       "Output",
                                       m_aggregator,
                                       &MultiFileAggregator::Write1d);
        CreateCollectors ("ns3::UnitConversionCollector",
                          m_conversionCollectors,
                          "ConversionType", EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        ConnectCollectorToCollector (m_conversionCollectors,
                                     "Output",
                                     m_terminalCollectors,
                                     &ScalarCollector::TraceSinkDouble);
        break;
      }

    case OUTPUT_SCATTER_FILE:
      {
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()));
        CreateCollectors ("ns3::IntervalRateCollector",
                          m_terminalCollectors,
                          "InputDataType", EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        ConnectCollectorsToAggregator (m_terminalCollectors,
                                       "OutputWithTime",
                                       m_aggregator,
                                       &MultiFileAggregator::Write2d);
        CreateCollectors ("ns3::UnitConversionCollector",
                          m_conversionCollectors,
                          "ConversionType", EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        ConnectCollectorToCollector (m_conversionCollectors,
                                     "Output",
                                     m_terminalCollectors,
                                     &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case OUTPUT_HISTOGRAM_FILE:
    case OUTPUT_PDF_FILE:
    case OUTPUT_CDF_FILE:
      break;

    case OUTPUT_SCALAR_PLOT:
      // TODO: Add support for boxes in Gnuplot.
      break;

    case OUTPUT_SCATTER_PLOT:
      {
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Received throughput (in kilobits per second)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        CreateCollectors ("ns3::IntervalRateCollector",
                          m_terminalCollectors,
                          "InputDataType", EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));

        for (SatStatsHelper::CollectorMap_t::const_iterator it = m_terminalCollectors.begin ();
             it != m_terminalCollectors.end (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }

        m_aggregator = plotAggregator;
        ConnectCollectorsToAggregator (m_terminalCollectors,
                                       "OutputWithTime",
                                       m_aggregator,
                                       &GnuplotAggregator::Write2d);
        CreateCollectors ("ns3::UnitConversionCollector",
                          m_conversionCollectors,
                          "ConversionType", EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        ConnectCollectorToCollector (m_conversionCollectors,
                                     "Output",
                                     m_terminalCollectors,
                                     &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case OUTPUT_HISTOGRAM_PLOT:
    case OUTPUT_PDF_PLOT:
    case OUTPUT_CDF_PLOT:
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;

    } // end of switch (GetOutputType ())

  // Create a map of UT user addresses and identifiers.
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();
  for (NodeContainer::Iterator it = utUsers.Begin ();
       it != utUsers.End (); ++it)
    {
      SaveIpv4AddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW user node's applications.

  NodeContainer gwUsers = GetSatHelper ()->GetGwUsers ();
  // TODO: Maybe UT users should also be included.
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsRtnThroughputHelper::ApplicationPacketCallback,
                    this);

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
              NS_LOG_WARN (this << " unable to connect with node ID " << (*it)->GetId ()
                                << " application #" << i);
            }
        }
    }

} // end of `void DoInstall ();`


void
SatStatsRtnThroughputHelper::ApplicationPacketCallback (Ptr<const Packet> packet,
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
                            << " unknown sender IPV4 address " << ipv4Addr);
        }
      else
        {
          // Find the collector with the right identifier.
          SatStatsHelper::CollectorMap_t::iterator it2 = m_conversionCollectors.find (it1->second);
          NS_ASSERT_MSG (it2 != m_conversionCollectors.end (),
                         "Unable to find collector with identifier " << it1->second);
          Ptr<UnitConversionCollector> collector = it2->second->GetObject<UnitConversionCollector> ();
          NS_ASSERT (collector != 0);

          // Pass the sample to the collector.
          collector->TraceSinkUinteger32 (0, packet->GetSize ());
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

} // end of `void ApplicationPacketCallback (Ptr<const Packet>, const Address);`


void
SatStatsRtnThroughputHelper::SaveMacAddressAndIdentifier (Ptr<Node> utUserNode)
{
  NS_LOG_FUNCTION (this << utUserNode->GetId ());
  NS_ASSERT_MSG (utUserNode->GetNDevices () >= 2,
                 "Node " << utUserNode->GetId () << " is not a valid UT user");

  // Assuming that #0 is for loopback device and #1 is for subscriber network device.
  Ptr<NetDevice> dev = utUserNode->GetDevice (1);
  const Address addr = dev->GetAddress ();
  NS_ASSERT_MSG (Mac48Address::IsMatchingType (addr),
                 "Node " << utUserNode->GetId () << " device #1 "
                         << "does not have any valid Mac48Address");

  // Save the address.
  const uint32_t identifier = GetIdentifierForUtUser (utUserNode);
  m_identifierMap[addr] = identifier;
  NS_LOG_INFO (this << " associated address " << addr
                    << " with identifier " << identifier);
}


void
SatStatsRtnThroughputHelper::SaveIpv4AddressAndIdentifier (Ptr<Node> utUserNode)
{
  NS_LOG_FUNCTION (this << utUserNode->GetId ());

  Ptr<Ipv4> ipv4 = utUserNode->GetObject<Ipv4> ();
  if (ipv4 == 0)
    {
      NS_LOG_LOGIC (this << "Node " << utUserNode->GetId ()
                         << " does not support IPv4 protocol");
    }
  else
    {
      NS_ASSERT_MSG (ipv4->GetNInterfaces () >= 2,
                     "Node " << utUserNode->GetId () << " is not a valid UT user");
      const uint32_t identifier = GetIdentifierForUtUser (utUserNode);

      // Assuming that #0 is for loopback interface and #1 is for subscriber network interface.
      for (uint32_t i = 0; i < ipv4->GetNAddresses (1); i++)
        {
          const Address addr = ipv4->GetAddress (1, i).GetLocal ();
          m_identifierMap[addr] = identifier;
          NS_LOG_INFO (this << " associated address " << addr
                            << " with identifier " << identifier);
        }
    }
}


} // end of namespace ns3

