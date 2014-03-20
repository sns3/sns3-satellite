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

#include "satellite-stats-rtn-mac-throughput-helper.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/node-container.h>
#include <ns3/packet.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>
#include <ns3/data-collection-object.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsRtnMacThroughputHelper");


namespace ns3 {

SatStatsRtnMacThroughputHelper::SatStatsRtnMacThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnMacThroughputHelper::~SatStatsRtnMacThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnMacThroughputHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case OUTPUT_NONE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case OUTPUT_SCALAR_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName () + ".txt"),
                                         "MultiFileMode", BooleanValue (false));

        // Setup terminal collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SECOND));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);

        // Setup conversion collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &ScalarCollector::TraceSinkDouble);
        break;
      }

    case OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue ("% time_sec throughput_kbps"));

        // Setup terminal collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_DOUBLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);

        // Setup conversion collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case OUTPUT_HISTOGRAM_FILE:
    case OUTPUT_PDF_FILE:
    case OUTPUT_CDF_FILE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case OUTPUT_SCALAR_PLOT:
      /// \todo Add support for boxes in Gnuplot.
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case OUTPUT_SCATTER_PLOT:
      {
        // Setup aggregator.
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Received throughput (in kilobits per second)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
        m_aggregator = plotAggregator;

        // Setup terminal collectors.
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
                                                  &GnuplotAggregator::Write2d);

        // Setup conversion collectors.
        m_conversionCollectors.SetType ("ns3::UnitConversionCollector");
        m_conversionCollectors.SetAttribute ("ConversionType",
                                             EnumValue (UnitConversionCollector::FROM_BYTES_TO_KBIT));
        CreateCollectorPerIdentifier (m_conversionCollectors);
        m_conversionCollectors.ConnectToCollector ("Output",
                                                   m_terminalCollectors,
                                                   &IntervalRateCollector::TraceSinkDouble);
        break;
      }

    case OUTPUT_HISTOGRAM_PLOT:
    case OUTPUT_PDF_PLOT:
    case OUTPUT_CDF_PLOT:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;

    } // end of switch (GetOutputType ())

  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, Ptr<const Packet>, const Address &> callback
    = MakeCallback (&SatStatsRtnMacThroughputHelper::ApplicationPacketCallback,
                    this);

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NS_LOG_DEBUG (this << " Node ID " << (*it)->GetId ()
                         << " has " << (*it)->GetNDevices () << " devices");
      /*
       * Assuming that device #0 is for loopback device, device #(N-1) is for
       * backbone network device, and devices #1 until #(N-2) are for satellite
       * beam device.
       */
      for (uint32_t i = 1; i <= (*it)->GetNDevices ()-2; i++)
        {
          Ptr<NetDevice> dev = (*it)->GetDevice (i);
          Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();

          if (satDev == 0)
            {
              NS_LOG_WARN (this << " Node " << (*it)->GetId ()
                                << " is not a valid GW");
            }
          else
            {
              Ptr<SatMac> satMac = satDev->GetMac ();
              NS_ASSERT (satMac != 0);

              // Connect the object to the probe.
              if (satMac->TraceConnectWithoutContext ("Rx", callback))
                {
                  NS_LOG_INFO (this << " successfully connected with node ID "
                                    << (*it)->GetId ()
                                    << " device #" << i);
                }
              else
                {
                  NS_LOG_WARN (this << " unable to connect with node ID "
                                    << (*it)->GetId ()
                                    << " device #" << i);
                }

            } // end of else of `if (satDev == 0)`

        } // end of `for (uint32_t i = 1; i <= (*it)->GetNDevices ()-2; i++)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstall ();`


void
SatStatsRtnMacThroughputHelper::ApplicationPacketCallback (Ptr<const Packet> packet,
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
      const Address addr = Mac48Address::ConvertFrom (from);
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
          // Find the collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_conversionCollectors.Get (it->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it->second);
          Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
          NS_ASSERT (c != 0);

          // Pass the sample to the collector.
          c->TraceSinkUinteger32 (0, packet->GetSize ());
        }
    }

} // end of `void ApplicationPacketCallback (Ptr<const Packet>, const Address);`


void
SatStatsRtnMacThroughputHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
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


} // end of namespace ns3

