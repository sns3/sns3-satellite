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
#include <ns3/object-vector.h>
#include <ns3/callback.h>

#include <ns3/node-container.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-rx-carrier.h>

#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>

#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/satellite-sinr-probe.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-composite-sinr-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsCompositeSinrHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsCompositeSinrHelper);

SatStatsCompositeSinrHelper::SatStatsCompositeSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
    m_minValue (0.0),
    m_maxValue (0.0),
    m_binLength (0.0)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsCompositeSinrHelper::~SatStatsCompositeSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsCompositeSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsCompositeSinrHelper")
    .SetParent<SatStatsHelper> ()
    .AddAttribute ("MinValue",
                   "Configure the MinValue attribute of the histogram, PDF, CDF output "
                   "(in dB).",
                   DoubleValue (-5.0),
                   MakeDoubleAccessor (&SatStatsCompositeSinrHelper::SetMinValue,
                                       &SatStatsCompositeSinrHelper::GetMinValue),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxValue",
                   "Configure the MaxValue attribute of the histogram, PDF, CDF output "
                   "(in dB).",
                   DoubleValue (20.0),
                   MakeDoubleAccessor (&SatStatsCompositeSinrHelper::SetMaxValue,
                                       &SatStatsCompositeSinrHelper::GetMaxValue),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BinLength",
                   "Configure the BinLength attribute of the histogram, PDF, CDF output "
                   "(in dB).",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&SatStatsCompositeSinrHelper::SetBinLength,
                                       &SatStatsCompositeSinrHelper::GetBinLength),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}


void
SatStatsCompositeSinrHelper::SetMinValue (double minValue)
{
  NS_LOG_FUNCTION (this << minValue);
  m_minValue = minValue;
}


double
SatStatsCompositeSinrHelper::GetMinValue () const
{
  return m_minValue;
}


void
SatStatsCompositeSinrHelper::SetMaxValue (double maxValue)
{
  NS_LOG_FUNCTION (this << maxValue);
  m_maxValue = maxValue;
}


double
SatStatsCompositeSinrHelper::GetMaxValue () const
{
  return m_maxValue;
}


void
SatStatsCompositeSinrHelper::SetBinLength (double binLength)
{
  NS_LOG_FUNCTION (this << binLength);
  m_binLength = binLength;
}


double
SatStatsCompositeSinrHelper::GetBinLength () const
{
  return m_binLength;
}


void
SatStatsCompositeSinrHelper::DoInstall ()
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
                                         "OutputFileName", StringValue (GetName ()),
                                         "MultiFileMode", BooleanValue (false),
                                         "EnableContextPrinting", BooleanValue (true),
                                         "GeneralHeading", StringValue ("% identifier sinr_db"));

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
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue ("% time_sec sinr_db"));

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
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue ("% sinr_db freq"));

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
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (m_minValue));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (m_maxValue));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (m_binLength));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator ("OutputString",
                                                  m_aggregator,
                                                  &MultiFileAggregator::AddContextHeading);
        break;
      }

    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      /// \todo Add support for boxes in Gnuplot.
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        // Setup aggregator.
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "SINR (in dB)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
        m_aggregator = plotAggregator;

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
                                                  &GnuplotAggregator::Write2d);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      {
        // Setup aggregator.
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("SINR (in dB)",
                                   "Frequency");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);
        m_aggregator = plotAggregator;

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
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (m_minValue));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (m_maxValue));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (m_binLength));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &GnuplotAggregator::Write2d);
        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsCompositeSinrHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to the collectors.
  InstallProbes ();

} // end of `void DoInstall ();`


void
SatStatsCompositeSinrHelper::InstallProbes ()
{
  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


// FORWARD LINK ///////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdCompositeSinrHelper);

SatStatsFwdCompositeSinrHelper::SatStatsFwdCompositeSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsCompositeSinrHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdCompositeSinrHelper::~SatStatsFwdCompositeSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdCompositeSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdCompositeSinrHelper")
    .SetParent<SatStatsCompositeSinrHelper> ()
  ;
  return tid;
}


void
SatStatsFwdCompositeSinrHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();

  for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
    {
      const int32_t utId = GetUtId (*it);
      NS_ASSERT_MSG (utId > 0,
                     "Node " << (*it)->GetId () << " is not a valid UT");
      const uint32_t identifier = GetIdentifierForUt (*it);

      // Create the probe.
      std::ostringstream probeName;
      probeName << utId;
      Ptr<SatSinrProbe> probe = CreateObject<SatSinrProbe> ();
      probe->SetName (probeName.str ());

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);
      Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
      NS_ASSERT (satPhyRx != 0);
      ObjectVectorValue carriers;
      satPhyRx->GetAttribute ("RxCarrierList", carriers);
      NS_LOG_DEBUG (this << " Node ID " << (*it)->GetId ()
                         << " device #" << dev->GetIfIndex ()
                         << " has " << carriers.GetN () << " RX carriers");

      for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
           itCarrier != carriers.End (); ++itCarrier)
        {
          // Connect the object to the probe.
          if (probe->ConnectByObject ("Sinr", itCarrier->second))
            {
              // Connect the probe to the right collector.
              bool ret = false;
              switch (GetOutputType ())
                {
                case SatStatsHelper::OUTPUT_SCALAR_FILE:
                case SatStatsHelper::OUTPUT_SCALAR_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSinr",
                                                               identifier,
                                                               &ScalarCollector::TraceSinkDouble);
                  break;

                case SatStatsHelper::OUTPUT_SCATTER_FILE:
                case SatStatsHelper::OUTPUT_SCATTER_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
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
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSinr",
                                                               identifier,
                                                               &DistributionCollector::TraceSinkDouble);
                  break;

                default:
                  NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
                  break;

                } // end of `switch (GetOutputType ())`

              if (ret)
                {
                  NS_LOG_INFO (this << " created probe " << probeName
                                    << ", connected to collector " << identifier);
                  m_probes.push_back (probe->GetObject<Probe> ());
                }
              else
                {
                  NS_LOG_WARN (this << " unable to connect probe " << probeName
                                    << " to collector " << identifier);
                }

            } // end of `if (probe->ConnectByObject ("Sinr", itCarrier->second))`
          else
            {
              NS_FATAL_ERROR ("Error connecting to Sinr trace source"
                              << " of SatPhyRxCarrier"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << dev->GetIfIndex ()
                              << " RX carrier #" << itCarrier->first);
            }

        } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`


// RETURN LINK ////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnCompositeSinrHelper);

SatStatsRtnCompositeSinrHelper::SatStatsRtnCompositeSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsCompositeSinrHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnCompositeSinrHelper::~SatStatsRtnCompositeSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnCompositeSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnCompositeSinrHelper")
    .SetParent<SatStatsCompositeSinrHelper> ()
  ;
  return tid;
}


void
SatStatsRtnCompositeSinrHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      // Create a map of UT addresses and identifiers.
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, double, const Address &> callback
    = MakeCallback (&SatStatsRtnCompositeSinrHelper::SinrCallback, this);

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
          Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
          NS_ASSERT (satPhyRx != 0);
          ObjectVectorValue carriers;
          satPhyRx->GetAttribute ("RxCarrierList", carriers);
          NS_LOG_DEBUG (this << " Node ID " << (*it)->GetId ()
                             << " device #" << (*itDev)->GetIfIndex ()
                             << " has " << carriers.GetN () << " RX carriers");

          for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
               itCarrier != carriers.End (); ++itCarrier)
            {
              if (itCarrier->second->TraceConnectWithoutContext ("Sinr",
                                                                 callback))
                {
                  NS_LOG_INFO (this << " successfully connected with node ID "
                                    << (*it)->GetId ()
                                    << " device #" << (*itDev)->GetIfIndex ()
                                    << " RX carrier #" << itCarrier->first);
                }
              else
                {
                  NS_FATAL_ERROR ("Error connecting to Sinr trace source"
                                  << " of SatPhyRxCarrier"
                                  << " at node ID " << (*it)->GetId ()
                                  << " device #" << (*itDev)->GetIfIndex ()
                                  << " RX carrier #" << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


void
SatStatsRtnCompositeSinrHelper::SinrCallback (double sinrDb, const Address &from)
{
  //NS_LOG_FUNCTION (this << sinrDb << from);

  if (from.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding a SINR trace of " << sinrDb << " dB"
                        << " from statistics collection because of"
                        << " invalid sender address");
    }
  else
    {
      // Determine the identifier associated with the sender address.
      std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find (from);

      if (it == m_identifierMap.end ())
        {
          NS_LOG_WARN (this << " discarding a SINR trace of " << sinrDb << " dB"
                            << " from statistics collection because of"
                            << " unknown sender address " << from);
        }
      else
        {
          // Find the collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (it->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it->second);

          switch (GetOutputType ())
            {
            case SatStatsHelper::OUTPUT_SCALAR_FILE:
            case SatStatsHelper::OUTPUT_SCALAR_PLOT:
              {
                Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, sinrDb);
                break;
              }

            case SatStatsHelper::OUTPUT_SCATTER_FILE:
            case SatStatsHelper::OUTPUT_SCATTER_PLOT:
              {
                Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, sinrDb);
                break;
              }

            case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
            case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
            case SatStatsHelper::OUTPUT_PDF_FILE:
            case SatStatsHelper::OUTPUT_PDF_PLOT:
            case SatStatsHelper::OUTPUT_CDF_FILE:
            case SatStatsHelper::OUTPUT_CDF_PLOT:
              {
                Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, sinrDb);
                break;
              }

            default:
              NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
              break;

            } // end of `switch (GetOutputType ())`

        } // end of `if (it == m_identifierMap.end ())`

    } // end of else of `if (from.IsInvalid ())`

} // end of `void SinrCallback (double, const Address &);`


void
SatStatsRtnCompositeSinrHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
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
