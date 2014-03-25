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
#include <ns3/simulator.h>
#include <ns3/fatal-error.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>

#include <ns3/node-container.h>
#include <ns3/mac48-address.h>
#include <ns3/net-device.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-llc.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-phy-rx.h>

#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>

#include <ns3/data-collection-object.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/gnuplot-aggregator.h>

#include "satellite-stats-queue-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsQueueHelper");


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

std::string // static
SatStatsQueueHelper::GetUnitTypeName (SatStatsQueueHelper::UnitType_t unitType)
{
  switch (unitType)
    {
    case SatStatsQueueHelper::UNIT_BYTES:
      return "UNIT_BYTES";
    case SatStatsQueueHelper::UNIT_NUMBER_OF_PACKETS:
      return "UNIT_NUMBER_OF_PACKETS";
    default:
      NS_FATAL_ERROR ("SatStatsQueueHelper - Invalid unit type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsQueueHelper - Invalid unit type");
  return "";
}


SatStatsQueueHelper::SatStatsQueueHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
    m_pollInterval (MilliSeconds (10)),
    m_unitType (SatStatsQueueHelper::UNIT_BYTES),
    m_shortLabel (""),
    m_longLabel (""),
    m_distributionMinValue (std::numeric_limits<double>::max ()),
    m_distributionMaxValue (-std::numeric_limits<double>::max ()),
    m_distributionBinLength (-1.0)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsQueueHelper::~SatStatsQueueHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsQueueHelper::SetUnitType (SatStatsQueueHelper::UnitType_t unitType)
{
  NS_LOG_FUNCTION (this << GetUnitTypeName (unitType));
  m_unitType = unitType;

  // Update presentation-based attributes.
  if (unitType == SatStatsQueueHelper::UNIT_BYTES)
    {
      m_shortLabel = "size_bytes";
      m_longLabel = "Queue size (in bytes)";
      m_distributionMinValue = 0.0;
      m_distributionMaxValue = 10000.0;
      m_distributionBinLength = 100.0;
    }
  else if (unitType == SatStatsQueueHelper::UNIT_NUMBER_OF_PACKETS)
    {
      m_shortLabel = "num_packets";
      m_longLabel = "Queue size (in number of packets)";
      m_distributionMinValue = 0.0;
      m_distributionMaxValue = 100.0;
      m_distributionBinLength = 2.0;
    }
  else
    {
      NS_FATAL_ERROR ("SatStatsQueueHelper - Invalid unit type");
    }
}


SatStatsQueueHelper::UnitType_t
SatStatsQueueHelper::GetUnitType () const
{
  return m_unitType;
}


//void
//SatStatsQueueHelper::SetShortLabel (std::string shortLabel)
//{
//  NS_LOG_FUNCTION (this << shortLabel);
//
//  if (IsInstalled () && (m_shortLabel != shortLabel))
//    {
//      NS_LOG_WARN (this << " cannot modify the current short label"
//                        << " (" << m_shortLabel << ")"
//                        << " because this instance have already been installed");
//    }
//  else
//    {
//      m_shortLabel = shortLabel;
//    }
//}
//
//
//std::string
//SatStatsQueueHelper::GetShortLabel () const
//{
//  return m_shortLabel;
//}
//
//
//void
//SatStatsQueueHelper::SetLongLabel (std::string longLabel)
//{
//  NS_LOG_FUNCTION (this << longLabel);
//
//  if (IsInstalled () && (m_longLabel != longLabel))
//    {
//      NS_LOG_WARN (this << " cannot modify the current long label"
//                        << " (" << m_longLabel << ")"
//                        << " because this instance have already been installed");
//    }
//  else
//    {
//      m_longLabel = longLabel;
//    }
//}
//
//
//std::string
//SatStatsQueueHelper::GetLongLabel () const
//{
//  return m_longLabel;
//}


void
SatStatsQueueHelper::SetPollInterval (Time pollInterval)
{
  NS_LOG_FUNCTION (this << pollInterval.GetSeconds ());
  m_pollInterval = pollInterval;
}


Time
SatStatsQueueHelper::GetPollInterval () const
{
  return m_pollInterval;
}


void
SatStatsQueueHelper::DoInstall ()
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
                                         "OutputFileName", StringValue (GetName () + ".txt"),
                                         "MultiFileMode", BooleanValue (false));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_UINTEGER));
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
        const std::string heading = "% time_sec " + m_shortLabel;
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue (heading));

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
        const std::string heading = "% " + m_shortLabel + " freq";
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetName ()),
                                         "GeneralHeading", StringValue (heading));

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
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (m_distributionMinValue));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (m_distributionMaxValue));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (m_distributionBinLength));
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
        plotAggregator->SetLegend ("Time (in seconds)", m_longLabel);
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
        plotAggregator->SetLegend (m_longLabel, "Frequency");
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
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (m_distributionMinValue));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (m_distributionMaxValue));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (m_distributionBinLength));
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
      NS_FATAL_ERROR ("SatStatsRtnQueueBytesHelper - Invalid output type");
      break;
    }

  // Identify the list of source of queue events.
  EnlistSource ();

  // Schedule the first polling session.
  Simulator::Schedule (m_pollInterval, &SatStatsQueueHelper::Poll, this);

} // end of `void DoInstall ();`


void
SatStatsQueueHelper::EnlistSource ()
{
  // The method below is supposed to be implemented by the child class.
  DoEnlistSource ();
}


void
SatStatsQueueHelper::Poll ()
{
  // The method below is supposed to be implemented by the child class.
  DoPoll ();

  // Schedule the next polling session.
  Simulator::Schedule (m_pollInterval, &SatStatsQueueHelper::Poll, this);
}


void
SatStatsQueueHelper::PushToCollector (uint32_t identifier, uint32_t value)
{
  // Find the collector with the right identifier.
  Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (identifier);
  NS_ASSERT_MSG (collector != 0,
                 "Unable to find collector with identifier " << identifier);

  switch (GetOutputType ())
    {
    case OUTPUT_SCALAR_FILE:
    case OUTPUT_SCALAR_PLOT:
      {
        Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkUinteger32 (0, value);
        break;
      }

    case OUTPUT_SCATTER_FILE:
    case OUTPUT_SCATTER_PLOT:
      {
        Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkUinteger32 (0, value);
        break;
      }

    case OUTPUT_HISTOGRAM_FILE:
    case OUTPUT_HISTOGRAM_PLOT:
    case OUTPUT_PDF_FILE:
    case OUTPUT_PDF_PLOT:
    case OUTPUT_CDF_FILE:
    case OUTPUT_CDF_PLOT:
      {
        Ptr<DistributionCollector> c = collector->GetObject<DistributionCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkUinteger32 (0, value);
        break;
      }

    default:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    } // end of `switch (GetOutputType ())`

} // end of `void PushToCollector (uint32_t, uint32_t)`


// FORWARD LINK ///////////////////////////////////////////////////////////////

SatStatsFwdQueueHelper::SatStatsFwdQueueHelper (Ptr<const SatHelper> satHelper)
  : SatStatsQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdQueueHelper::~SatStatsFwdQueueHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdQueueHelper::DoEnlistSource ()
{
  NS_LOG_FUNCTION (this);

  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  for (NodeContainer::Iterator it1 = gws.Begin(); it1 != gws.End (); ++it1)
    {
      NS_LOG_DEBUG (this << " Node ID " << (*it1)->GetId ()
                         << " has " << (*it1)->GetNDevices () << " devices");
      /*
       * Assuming that device #0 is for loopback device, device #(N-1) is for
       * backbone network device, and devices #1 until #(N-2) are for satellite
       * beam device.
       */
      for (uint32_t i = 1; i <= (*it1)->GetNDevices ()-2; i++)
        {
          Ptr<NetDevice> dev = (*it1)->GetDevice (i);
          Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();

          if (satDev == 0)
            {
              NS_LOG_WARN (this << " Node " << (*it1)->GetId ()
                                << " is not a valid GW");
            }
          else
            {
              // Get the beam ID of this device.
              Ptr<SatPhy> satPhy = satDev->GetPhy ();
              NS_ASSERT (satPhy != 0);
              Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
              NS_ASSERT (satPhyRx != 0);
              const uint32_t beamId = satPhyRx->GetBeamId ();
              NS_LOG_DEBUG (this << " enlisting UT from beam ID " << beamId);

              // Go through the UTs of this beam.
              ListOfUt_t listOfUt;
              NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes (beamId);
              for (NodeContainer::Iterator it2 = uts.Begin();
                   it2 != uts.End (); ++it2)
                {
                  const Address addr = satIdMapper->GetUtMacWithNode (*it2);
                  const Mac48Address mac48Addr = Mac48Address::ConvertFrom (addr);

                  if (addr.IsInvalid ())
                    {
                      NS_LOG_WARN (this << " Node " << (*it2)->GetId ()
                                        << " is not a valid UT");
                    }
                  else
                    {
                      const uint32_t identifier = GetIdentifierForUt (*it2);
                      listOfUt.push_back (std::make_pair (mac48Addr, identifier));
                    }
                }

              // Add an entry to the LLC list.
              Ptr<SatLlc> satLlc = satDev->GetLlc ();
              NS_ASSERT (satLlc != 0);
              m_llc.push_back (std::make_pair (satLlc, listOfUt));

            } // end of else of `if (satDev == 0)`

        } // end of `for (uint32_t i = 1; i <= (*it)->GetNDevices ()-2; i++)`

    } // end of `for (it1 = gws.Begin(); it1 != gws.End (); ++it1)`

} // end of `void DoInstall ();`


void
SatStatsFwdQueueHelper::DoPoll ()
{
  //NS_LOG_FUNCTION (this);

  // Go through the LLC list.
  std::list<std::pair<Ptr<SatLlc>, ListOfUt_t> >::const_iterator it1;
  for (it1 = m_llc.begin (); it1 != m_llc.end (); ++it1)
    {
      for (ListOfUt_t::const_iterator it2 = it1->second.begin ();
           it2 != it1->second.end (); ++it2)
        {
          const Mac48Address addr = it2->first;
          const uint32_t identifier = it2->second;
          if (GetUnitType () == SatStatsQueueHelper::UNIT_BYTES)
            {
              PushToCollector (identifier,
                               it1->first->GetNBytesInQueue (addr));
            }
          else
            {
              NS_ASSERT (GetUnitType () == SatStatsQueueHelper::UNIT_NUMBER_OF_PACKETS);
              PushToCollector (identifier,
                               it1->first->GetNPacketsInQueue (addr));
            }
        }
    }
}


// FORWARD LINK IN BYTES //////////////////////////////////////////////////////

SatStatsFwdQueueBytesHelper::SatStatsFwdQueueBytesHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFwdQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsQueueHelper::UNIT_BYTES);
}


SatStatsFwdQueueBytesHelper::~SatStatsFwdQueueBytesHelper ()
{
  NS_LOG_FUNCTION (this);
}


// FORWARD LINK IN PACKETS ////////////////////////////////////////////////////

SatStatsFwdQueuePacketsHelper::SatStatsFwdQueuePacketsHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFwdQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsQueueHelper::UNIT_NUMBER_OF_PACKETS);
}


SatStatsFwdQueuePacketsHelper::~SatStatsFwdQueuePacketsHelper ()
{
  NS_LOG_FUNCTION (this);
}


// RETURN LINK ////////////////////////////////////////////////////////////////

SatStatsRtnQueueHelper::SatStatsRtnQueueHelper (Ptr<const SatHelper> satHelper)
  : SatStatsQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnQueueHelper::~SatStatsRtnQueueHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnQueueHelper::DoEnlistSource ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin(); it != uts.End (); ++it)
    {
      const uint32_t identifier = GetIdentifierForUt (*it);

      /*
       * Assuming that device #0 is for loopback device, device #1 is for
       * subscriber network device, and device #2 is for satellite beam device.
       */
      NS_ASSERT ((*it)->GetNDevices () >= 3);
      Ptr<NetDevice> dev = (*it)->GetDevice (2);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();

      if (satDev == 0)
        {
          NS_LOG_WARN (this << " Node " << (*it)->GetId ()
                            << " is not a valid UT");
        }
      else
        {
          Ptr<SatLlc> satLlc = satDev->GetLlc ();
          NS_ASSERT (satLlc != 0);
          m_llc.push_back (std::make_pair (satLlc, identifier));
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstall ();`


void
SatStatsRtnQueueHelper::DoPoll ()
{
  //NS_LOG_FUNCTION (this);

  // Go through the LLC list.
  std::list<std::pair<Ptr<SatLlc>, uint32_t> >::const_iterator it;
  for (it = m_llc.begin (); it != m_llc.end (); ++it)
    {
      if (GetUnitType () == SatStatsQueueHelper::UNIT_BYTES)
        {
          PushToCollector (it->second, it->first->GetNBytesInQueue ());
        }
      else
        {
          NS_ASSERT (GetUnitType () == SatStatsQueueHelper::UNIT_NUMBER_OF_PACKETS);
          PushToCollector (it->second, it->first->GetNPacketsInQueue ());
        }
    }
}

// RETURN LINK IN BYTES ///////////////////////////////////////////////////////

SatStatsRtnQueueBytesHelper::SatStatsRtnQueueBytesHelper (Ptr<const SatHelper> satHelper)
  : SatStatsRtnQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsQueueHelper::UNIT_BYTES);
}


SatStatsRtnQueueBytesHelper::~SatStatsRtnQueueBytesHelper ()
{
  NS_LOG_FUNCTION (this);
}


// RETURN LINK IN PACKETS /////////////////////////////////////////////////////

SatStatsRtnQueuePacketsHelper::SatStatsRtnQueuePacketsHelper (Ptr<const SatHelper> satHelper)
  : SatStatsRtnQueueHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetUnitType (SatStatsQueueHelper::UNIT_NUMBER_OF_PACKETS);
}


SatStatsRtnQueuePacketsHelper::~SatStatsRtnQueuePacketsHelper ()
{
  NS_LOG_FUNCTION (this);
}


} // end of namespace ns3
