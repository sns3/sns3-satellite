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
#include <ns3/satellite-mac.h>
#include <ns3/satellite-phy.h>

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
#include <ns3/gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-delay-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsDelayHelper");


namespace ns3 {

SatStatsDelayHelper::SatStatsDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsDelayHelper::~SatStatsDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsDelayHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue ("% time_sec delay_sec"));

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
                                         "GeneralHeading", StringValue ("% delay_sec freq"));

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
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (0.0));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (1.0));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (0.02));
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
                                   "Packet delay (in seconds)");
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
        plotAggregator->SetLegend ("Packet delay (in seconds)",
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
        m_terminalCollectors.SetAttribute ("MinValue", DoubleValue (0.0));
        m_terminalCollectors.SetAttribute ("MaxValue", DoubleValue (1.0));
        m_terminalCollectors.SetAttribute ("BinLength", DoubleValue (0.02));
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
      NS_FATAL_ERROR ("SatStatsDelayHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to the collectors.
  InstallProbes ();

} // end of `void DoInstall ();`


void
SatStatsDelayHelper::InstallProbes ()
{
  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


void
SatStatsDelayHelper::RxDelayCallback (Time delay, const Address &from)
{
  //NS_LOG_FUNCTION (this << delay.GetSeconds () << from);

  if (from.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding a packet delay of " << delay.GetSeconds ()
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
          NS_LOG_WARN (this << " discarding a packet delay of " << delay.GetSeconds ()
                            << " from statistics collection because of"
                            << " unknown sender address " << addr);
        }
      else
        {
          // Find the collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (it->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it->second);

          switch (GetOutputType ())
            {
            case OUTPUT_SCALAR_FILE:
            case OUTPUT_SCALAR_PLOT:
              {
                Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, delay.GetSeconds ());
                break;
              }

            case OUTPUT_SCATTER_FILE:
            case OUTPUT_SCATTER_PLOT:
              {
                Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, delay.GetSeconds ());
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
                c->TraceSinkDouble (0.0, delay.GetSeconds ());
                break;
              }

            default:
              NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
              break;

            } // end of `switch (GetOutputType ())`

        } // end of `if (it == m_identifierMap.end ())`

    } // end of else of `if (from.IsInvalid ())`

} // end of `void RxDelayCallback (Time, const Address);`


void
SatStatsDelayHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
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

SatStatsFwdAppDelayHelper::SatStatsFwdAppDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdAppDelayHelper::~SatStatsFwdAppDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdAppDelayHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();

  for (NodeContainer::Iterator it = utUsers.Begin(); it != utUsers.End (); ++it)
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
          Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
          probe->SetName (probeName.str ());

          // Connect the object to the probe.
          if (probe->ConnectByObject ("RxDelay", (*it)->GetApplication (i)))
            {
              // Connect the probe to the right collector.
              bool ret = false;
              switch (GetOutputType ())
                {
                case SatStatsHelper::OUTPUT_SCALAR_FILE:
                case SatStatsHelper::OUTPUT_SCALAR_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &ScalarCollector::TraceSinkDouble);
                  break;

                case SatStatsHelper::OUTPUT_SCATTER_FILE:
                case SatStatsHelper::OUTPUT_SCATTER_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &UnitConversionCollector::TraceSinkDouble);
                  break;

                case OUTPUT_HISTOGRAM_FILE:
                case OUTPUT_HISTOGRAM_PLOT:
                case OUTPUT_PDF_FILE:
                case OUTPUT_PDF_PLOT:
                case OUTPUT_CDF_FILE:
                case OUTPUT_CDF_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &DistributionCollector::TraceSinkDouble);
                  break;

                default:
                  NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
                  break;
                }

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
            }
          else
            {
              /*
               * We're being tolerant here by only logging a warning, because
               * not every kind of Application is equipped with the expected
               * RxDelay trace source.
               */
              NS_LOG_WARN (this << " unable to connect probe " << probeName
                                << " with node ID " << (*it)->GetId ()
                                << " application #" << i);
            }

        } // end of `for (i = 0; i < (*it)->GetNApplications (); i++)`

    } // end of `for (it = utUsers.Begin(); it != utUsers.End (); ++it)`

} // end of `void DoInstallProbes ();`


// FORWARD LINK DEVICE-LEVEL /////////////////////////////////////////////////////

SatStatsFwdDevDelayHelper::SatStatsFwdDevDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdDevDelayHelper::~SatStatsFwdDevDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdDevDelayHelper::DoInstallProbes ()
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
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

      /*
       * Assuming that device #0 is for loopback device, device #1 is for
       * subscriber network device, and device #2 is for satellite beam device.
       */
      NS_ASSERT ((*it)->GetNDevices () >= 3);
      Ptr<NetDevice> dev = (*it)->GetDevice (2);
      NS_ASSERT (dev->GetObject<SatNetDevice> () != 0);

      // Connect the object to the probe.
      if (probe->ConnectByObject ("RxDelay", dev))
        {
          // Connect the probe to the right collector.
          bool ret = false;
          switch (GetOutputType ())
            {
            case SatStatsHelper::OUTPUT_SCALAR_FILE:
            case SatStatsHelper::OUTPUT_SCALAR_PLOT:
              ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                           "OutputSeconds",
                                                           identifier,
                                                           &ScalarCollector::TraceSinkDouble);
              break;

            case SatStatsHelper::OUTPUT_SCATTER_FILE:
            case SatStatsHelper::OUTPUT_SCATTER_PLOT:
              ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                           "OutputSeconds",
                                                           identifier,
                                                           &UnitConversionCollector::TraceSinkDouble);
              break;

            case OUTPUT_HISTOGRAM_FILE:
            case OUTPUT_HISTOGRAM_PLOT:
            case OUTPUT_PDF_FILE:
            case OUTPUT_PDF_PLOT:
            case OUTPUT_CDF_FILE:
            case OUTPUT_CDF_PLOT:
              ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                           "OutputSeconds",
                                                           identifier,
                                                           &DistributionCollector::TraceSinkDouble);
              break;

            default:
              NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
              break;
            }

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

        } // end of `if (probe->ConnectByObject ("RxDelay", dev))`
      else
        {
          NS_FATAL_ERROR ("Error connecting to RxDelay trace source of SatNetDevice"
                          << " at node ID " << (*it)->GetId () << " device #2");
        }

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`


// FORWARD LINK MAC-LEVEL /////////////////////////////////////////////////////

SatStatsFwdMacDelayHelper::SatStatsFwdMacDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdMacDelayHelper::~SatStatsFwdMacDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdMacDelayHelper::DoInstallProbes ()
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
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

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
          Ptr<SatMac> satMac = satDev->GetMac ();
          NS_ASSERT (satMac != 0);

          // Connect the object to the probe.
          if (probe->ConnectByObject ("RxDelay", satMac))
            {
              // Connect the probe to the right collector.
              bool ret = false;
              switch (GetOutputType ())
                {
                case SatStatsHelper::OUTPUT_SCALAR_FILE:
                case SatStatsHelper::OUTPUT_SCALAR_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &ScalarCollector::TraceSinkDouble);
                  break;

                case SatStatsHelper::OUTPUT_SCATTER_FILE:
                case SatStatsHelper::OUTPUT_SCATTER_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &UnitConversionCollector::TraceSinkDouble);
                  break;

                case OUTPUT_HISTOGRAM_FILE:
                case OUTPUT_HISTOGRAM_PLOT:
                case OUTPUT_PDF_FILE:
                case OUTPUT_PDF_PLOT:
                case OUTPUT_CDF_FILE:
                case OUTPUT_CDF_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &DistributionCollector::TraceSinkDouble);
                  break;

                default:
                  NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
                  break;
                }

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

            } // end of `if (probe->ConnectByObject ("RxDelay", satMac))`
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxDelay trace source of satMac"
                              << " at node ID " << (*it)->GetId () << " device #2");
            }

        } // end of else of `if (satDev == 0)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`


// FORWARD LINK PHY-LEVEL /////////////////////////////////////////////////////

SatStatsFwdPhyDelayHelper::SatStatsFwdPhyDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdPhyDelayHelper::~SatStatsFwdPhyDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdPhyDelayHelper::DoInstallProbes ()
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
      Ptr<ApplicationDelayProbe> probe = CreateObject<ApplicationDelayProbe> ();
      probe->SetName (probeName.str ());

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
          Ptr<SatPhy> satPhy = satDev->GetPhy ();
          NS_ASSERT (satPhy != 0);

          // Connect the object to the probe.
          if (probe->ConnectByObject ("RxDelay", satPhy))
            {
              // Connect the probe to the right collector.
              bool ret = false;
              switch (GetOutputType ())
                {
                case SatStatsHelper::OUTPUT_SCALAR_FILE:
                case SatStatsHelper::OUTPUT_SCALAR_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &ScalarCollector::TraceSinkDouble);
                  break;

                case SatStatsHelper::OUTPUT_SCATTER_FILE:
                case SatStatsHelper::OUTPUT_SCATTER_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &UnitConversionCollector::TraceSinkDouble);
                  break;

                case OUTPUT_HISTOGRAM_FILE:
                case OUTPUT_HISTOGRAM_PLOT:
                case OUTPUT_PDF_FILE:
                case OUTPUT_PDF_PLOT:
                case OUTPUT_CDF_FILE:
                case OUTPUT_CDF_PLOT:
                  ret = m_terminalCollectors.ConnectWithProbe (probe->GetObject<Probe> (),
                                                               "OutputSeconds",
                                                               identifier,
                                                               &DistributionCollector::TraceSinkDouble);
                  break;

                default:
                  NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
                  break;
                }

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

            } // end of `if (probe->ConnectByObject ("RxDelay", satPhy))`
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxDelay trace source of SatPhy"
                              << " at node ID " << (*it)->GetId () << " device #2");
            }

        } // end of else of `if (satDev == 0)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`


// RETURN LINK APPLICATION-LEVEL //////////////////////////////////////////////

SatStatsRtnAppDelayHelper::SatStatsRtnAppDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnAppDelayHelper::~SatStatsRtnAppDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnAppDelayHelper::DoInstallProbes ()
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
  Callback<void, Time, const Address &> callback
    = MakeCallback (&SatStatsRtnAppDelayHelper::Ipv4Callback, this);

  for (NodeContainer::Iterator it = gwUsers.Begin ();
       it != gwUsers.End (); ++it)
    {
      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          Ptr<Application> app = (*it)->GetApplication (i);

          if (app->TraceConnectWithoutContext ("RxDelay", callback))
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
SatStatsRtnAppDelayHelper::Ipv4Callback (Time delay, const Address &from)
{
  //NS_LOG_FUNCTION (this << Time.GetSeconds () << from);

  if (InetSocketAddress::IsMatchingType (from))
    {
      // Determine the identifier associated with the sender address.
      const Address ipv4Addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
      std::map<const Address, uint32_t>::const_iterator it1 = m_identifierMap.find (ipv4Addr);

      if (it1 == m_identifierMap.end ())
        {
          NS_LOG_WARN (this << " discarding a packet delay of " << delay.GetSeconds ()
                            << " from statistics collection because of"
                            << " unknown sender IPV4 address " << ipv4Addr);
        }
      else
        {
          // Find the collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (it1->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it1->second);

          switch (GetOutputType ())
            {
            case OUTPUT_SCALAR_FILE:
            case OUTPUT_SCALAR_PLOT:
              {
                Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, delay.GetSeconds ());
                break;
              }

            case OUTPUT_SCATTER_FILE:
            case OUTPUT_SCATTER_PLOT:
              {
                Ptr<UnitConversionCollector> c = collector->GetObject<UnitConversionCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkDouble (0.0, delay.GetSeconds ());
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
                c->TraceSinkDouble (0.0, delay.GetSeconds ());
                break;
              }

            default:
              NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
              break;

            } // end of `switch (GetOutputType ())`

        } // end of `else of if (it1 == m_identifierMap.end ())`

    } // end of `if (InetSocketAddress::IsMatchingType (from))`
  else
    {
      NS_LOG_WARN (this << " discarding a packet delay of " << delay.GetSeconds ()
                        << " from statistics collection"
                        << " because it comes from sender " << from
                        << " without valid InetSocketAddress");
    }

} // end of `void Ipv4Callback (Time, const Address);`


void
SatStatsRtnAppDelayHelper::SaveIpv4AddressAndIdentifier (Ptr<Node> utUserNode)
{
  NS_LOG_FUNCTION (this << utUserNode->GetId ());

  Ptr<Ipv4> ipv4 = utUserNode->GetObject<Ipv4> ();

  if (ipv4 == 0)
    {
      NS_LOG_LOGIC (this << " Node " << utUserNode->GetId ()
                         << " does not support IPv4 protocol");
    }
  else if (ipv4->GetNInterfaces () >= 2)
    {
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
  else
    {
      NS_LOG_WARN (this << " Node " << utUserNode->GetId ()
                        << " is not a valid UT user");
    }
}


// RETURN LINK DEVICE-LEVEL //////////////////////////////////////////////////////

SatStatsRtnDevDelayHelper::SatStatsRtnDevDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnDevDelayHelper::~SatStatsRtnDevDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnDevDelayHelper::DoInstallProbes ()
{
  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, Time, const Address &> callback
    = MakeCallback (&SatStatsRtnDevDelayHelper::RxDelayCallback, this);

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
          if ((dev->GetObject<SatNetDevice> () != 0)
              && (dev->TraceConnectWithoutContext ("RxDelay", callback)))
            {
              NS_LOG_INFO (this << " successfully connected with node ID "
                                << (*it)->GetId ()
                                << " device #" << i);
            }
          else
            {
              NS_FATAL_ERROR ("Error connecting to RxDelay trace source of SatNetDevice"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << i);
            }

        } // end of `for (uint32_t i = 1; i <= (*it)->GetNDevices ()-2; i++)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN LINK MAC-LEVEL //////////////////////////////////////////////////////

SatStatsRtnMacDelayHelper::SatStatsRtnMacDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnMacDelayHelper::~SatStatsRtnMacDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnMacDelayHelper::DoInstallProbes ()
{
  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, Time, const Address &> callback
    = MakeCallback (&SatStatsRtnMacDelayHelper::RxDelayCallback, this);

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
              if (satMac->TraceConnectWithoutContext ("RxDelay", callback))
                {
                  NS_LOG_INFO (this << " successfully connected with node ID "
                                    << (*it)->GetId ()
                                    << " device #" << i);
                }
              else
                {
                  NS_FATAL_ERROR ("Error connecting to RxDelay trace source of SatMac"
                                  << " at node ID " << (*it)->GetId ()
                                  << " device #" << i);
                }

            } // end of else of `if (satDev == 0)`

        } // end of `for (uint32_t i = 1; i <= (*it)->GetNDevices ()-2; i++)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


// RETURN LINK PHY-LEVEL //////////////////////////////////////////////////////

SatStatsRtnPhyDelayHelper::SatStatsRtnPhyDelayHelper (Ptr<const SatHelper> satHelper)
  : SatStatsDelayHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnPhyDelayHelper::~SatStatsRtnPhyDelayHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsRtnPhyDelayHelper::DoInstallProbes ()
{
  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, Time, const Address &> callback
    = MakeCallback (&SatStatsRtnPhyDelayHelper::RxDelayCallback, this);

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
              Ptr<SatPhy> satPhy = satDev->GetPhy ();
              NS_ASSERT (satPhy != 0);

              // Connect the object to the probe.
              if (satPhy->TraceConnectWithoutContext ("RxDelay", callback))
                {
                  NS_LOG_INFO (this << " successfully connected with node ID "
                                    << (*it)->GetId ()
                                    << " device #" << i);
                }
              else
                {
                  NS_FATAL_ERROR ("Error connecting to RxDelay trace source of SatPhy"
                                  << " at node ID " << (*it)->GetId ()
                                  << " device #" << i);
                }

            } // end of else of `if (satDev == 0)`

        } // end of `for (uint32_t i = 1; i <= (*it)->GetNDevices ()-2; i++)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstallProbes ();`


} // end of namespace ns3
