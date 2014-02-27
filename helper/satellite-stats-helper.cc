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

#include "satellite-stats-helper.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/singleton.h>
#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
#include <ns3/packet-data-rate-collector.h>
#include <ns3/gnuplot.h>
#include <ns3/gnuplot-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsHelper");


namespace ns3 {


// SATELLITE STATS HELPER /////////////////////////////////////////////////////

std::string // static
SatStatsHelper::GetIdentiferTypeName (IdentifierType_t identifierType)
{
  switch (identifierType)
    {
    case IDENTIFIER_GLOBAL:
      return "IDENTIFIER_GLOBAL";
    case IDENTIFIER_UT_USER:
      return "IDENTIFIER_UT_USER";
    case IDENTIFIER_UT:
      return "IDENTIFIER_UT";
    case IDENTIFIER_BEAM:
      return "IDENTIFIER_BEAM";
    case IDENTIFIER_GW:
      return "IDENTIFIER_GW";
    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
  return "";
}


std::string // static
SatStatsHelper::GetOutputTypeName (OutputType_t outputType)
{
  switch (outputType)
    {
    case OUTPUT_NONE:
      return "OUTPUT_NONE";
    case OUTPUT_SCALAR_FILE:
      return "OUTPUT_SCALAR_FILE";
    case OUTPUT_TRACE_FILE:
      return "OUTPUT_TRACE_FILE";
    case OUTPUT_PDF_FILE:
      return "OUTPUT_PDF_FILE";
    case OUTPUT_CDF_FILE:
      return "OUTPUT_CDF_FILE";
    case OUTPUT_SCALAR_PLOT:
      return "OUTPUT_SCALAR_PLOT";
    case OUTPUT_TRACE_PLOT:
      return "OUTPUT_TRACE_PLOT";
    case OUTPUT_PDF_PLOT:
      return "OUTPUT_PDF_PLOT";
    case OUTPUT_CDF_PLOT:
      return "OUTPUT_CDF_PLOT";
    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
  return "";
}


SatStatsHelper::SatStatsHelper ()
  : m_name ("stat"),
    m_identifierType (IDENTIFIER_GLOBAL),
    m_outputType (OUTPUT_TRACE_PLOT)
{
  NS_LOG_FUNCTION (this);
}


SatStatsHelper::~SatStatsHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsHelper::SetName (std::string name)
{
  NS_LOG_FUNCTION (this << name);

  // convert all spaces and slashes in the name to underscores
  for (size_t pos = name.find_first_of (" /");
       pos != std::string::npos;
       pos = name.find_first_of (" /", pos + 1, 1))
    {
      name[pos] = '_';
    }

  m_name = name;
}


std::string
SatStatsHelper::GetName () const
{
  return m_name;
}


void
SatStatsHelper::SetIdentifierType (IdentifierType_t identifierType)
{
  NS_LOG_FUNCTION (this << GetIdentiferTypeName (identifierType));

  if ((m_collectors.size () > 0) && (m_identifierType != identifierType))
    {
      NS_LOG_WARN (this << " cannot modify the current identifier type"
                        << " (" << GetIdentiferTypeName (m_identifierType) << ")"
                        << " if collectors have been previously created");
    }
  else
    {
      m_identifierType = identifierType;
    }
}


SatStatsHelper::IdentifierType_t
SatStatsHelper::GetIdentifierType () const
{
  return m_identifierType;
}


void
SatStatsHelper::SetOutputType (OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << GetOutputTypeName (outputType));
  m_outputType = outputType;
}


SatStatsHelper::OutputType_t
SatStatsHelper::GetOutputType () const
{
  return m_outputType;
}


uint32_t // static
SatStatsHelper::GetUtUserId (Ptr<Node> utUser)
{
  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();

  const Address addr = satIdMapper->GetUtUserMacWithNode (utUser);
  NS_ASSERT_MSG (Mac48Address::IsMatchingType (addr),
                 "Node " << utUser->GetId ()
                         << " does not have any valid Mac48Address");

  const int32_t utUserId = satIdMapper->GetUtUserIdWithMac (addr);
  NS_ASSERT_MSG (utUserId != -1,
                 "Node " << utUser->GetId ()
                         << " is not found in the global list of UT users");

  return utUserId;
}


// SATELLITE STATS FWD THROUGHPUT HELPER //////////////////////////////////////

SatStatsFwdThroughputHelper::SatStatsFwdThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


SatStatsFwdThroughputHelper::~SatStatsFwdThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdThroughputHelper::Connect (Ptr<const SatHelper> satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);

  NodeContainer utUsers = satHelper->GetUtUsers ();

  // Create a collector for each identifier
  switch (GetIdentifierType ())
    {
    case IDENTIFIER_GLOBAL:
      break;

    case IDENTIFIER_UT_USER:
      for (NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); ++it)
        {
          const int32_t utUserId = SatStatsHelper::GetUtUserId (*it);
          std::ostringstream collectorName;
          collectorName << "ut-user-" << utUserId;
          Ptr<PacketDataRateCollector> collector = CreateObject<PacketDataRateCollector> ();
          collector->SetName (collectorName.str ());
          m_collectors[utUserId] = collector;
        }
      break;

    case IDENTIFIER_UT:
      break;

    case IDENTIFIER_BEAM:
      break;

    case IDENTIFIER_GW:
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  // Create a probe for each UT user's application inside the container
  for (NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); ++it)
    {
      const int32_t utUserId = SatStatsHelper::GetUtUserId (*it);

      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          // Create and connect the probe to the application
          Ptr<Application> app = (*it)->GetApplication (i);
          Ptr<ApplicationPacketProbe> probe = CreateObject<ApplicationPacketProbe> ();
          std::ostringstream probeName;
          probeName << utUserId << "-" << i;
          probe->SetName (probeName.str ());
          const bool ret1 = probe->ConnectByObject ("Rx", app);

          if (ret1)
            {
              NS_LOG_INFO (this << " connected probe " << probeName.str ()
                                << " to application " << i
                                << " of node " << (*it)->GetId ()
                                << " (UT user " << utUserId << ")");
              m_probes.push_back (probe);
            }
          else
            {
              NS_LOG_WARN (this << " unable to connect probe " << probeName.str ()
                                << " to application " << i
                                << " of node " << (*it)->GetId ()
                                << " (UT user " << utUserId << ")");
            }

          uint32_t identifier;
          switch (GetIdentifierType ())
            {
            case IDENTIFIER_GLOBAL:
              identifier = 0;
              break;
            case IDENTIFIER_UT_USER:
              identifier = utUserId;
              break;
            case IDENTIFIER_UT:
              identifier = 0;
              break;
            case IDENTIFIER_BEAM:
              identifier = 0;
              break;
            case IDENTIFIER_GW:
              identifier = 0;
              break;
            default:
              NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
              break;
            }

          // Connect the probe to the right collector
          Ptr<PacketDataRateCollector> collector
            = m_collectors[identifier]->GetObject<PacketDataRateCollector> ();
          NS_ASSERT (collector != 0);
          const bool ret2 = probe->TraceConnectWithoutContext ("OutputBytes",
                                                               MakeCallback (&PacketDataRateCollector::TraceSink,
                                                                             collector));
          NS_ASSERT_MSG (ret2,
                         "Failed to connect probe " << probeName.str ()
                           << " to collector " << identifier);

        } // end of `for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)`

    } // end of `for (NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); ++it)`

  // Create an aggregator and connect it to the collectors
  switch (GetOutputType ())
    {
    case OUTPUT_NONE:
      m_aggregator = 0;
      break;

    case OUTPUT_SCALAR_FILE:
      m_aggregator = 0;
      break;

    case OUTPUT_TRACE_FILE:
      m_aggregator = 0;
      break;

    case OUTPUT_PDF_FILE:
      m_aggregator = 0;
      break;

    case OUTPUT_CDF_FILE:
      m_aggregator = 0;
      break;

    case OUTPUT_SCALAR_PLOT:
      {
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("UT Users ID",
                                   "Received throughput (in kbps)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::IMPULSES);

        for (std::map<uint32_t, Ptr<DataCollectionObject> >::const_iterator it = m_collectors.begin ();
          it != m_collectors.end (); ++it)
          {
            Ptr<PacketDataRateCollector> collector = it->second->GetObject<PacketDataRateCollector> ();
            NS_ASSERT (collector != 0);
            collector->SetOutputInterval (MilliSeconds (0));
            const std::string context = collector->GetName ();
            plotAggregator->Add2dDataset (context, context);
            const bool ret = collector->TraceConnect ("OutputTimeKbits",
                                                      context,
                                                      MakeCallback (&GnuplotAggregator::Write2d,
                                                                    plotAggregator));
            NS_ASSERT_MSG (ret,
                           "Failed to connect collector " << context
                                                          << " to aggregator");
          }

        m_aggregator = plotAggregator;
        break;
      }

    case OUTPUT_TRACE_PLOT:
      {
        Ptr<GnuplotAggregator> plotAggregator = CreateObject<GnuplotAggregator> (GetName ());
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Received throughput (in kbps)");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        for (std::map<uint32_t, Ptr<DataCollectionObject> >::const_iterator it = m_collectors.begin ();
          it != m_collectors.end (); ++it)
          {
            Ptr<PacketDataRateCollector> collector = it->second->GetObject<PacketDataRateCollector> ();
            NS_ASSERT (collector != 0);
            collector->SetOutputInterval (Seconds (1.0));
            const std::string context = collector->GetName ();
            plotAggregator->Add2dDataset (context, context);
            const bool ret = collector->TraceConnect ("OutputTimeKbits",
                                                      context,
                                                      MakeCallback (&GnuplotAggregator::Write2d,
                                                                    plotAggregator));
            NS_ASSERT_MSG (ret,
                           "Failed to connect collector " << context
                                                          << " to aggregator");
          }

        m_aggregator = plotAggregator;
        break;
      }

    case OUTPUT_PDF_PLOT:
      m_aggregator = 0;
      break;

    case OUTPUT_CDF_PLOT:
      m_aggregator = 0;
      break;

    default:
      NS_FATAL_ERROR("SatStatsHelper - Invalid output type");
      break;
    }

} // end of `void Connect (Ptr<const SatHelper> satHelper);`



// SATELLITE STATS HELPER CONTAINER ///////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsHelperContainer);


SatStatsHelperContainer::SatStatsHelperContainer (Ptr<const SatHelper> satHelper)
  : m_satHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsHelperContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsHelperContainer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsHelperContainer")
    .SetParent<Object> ()
    .AddAttribute ("Name",
                   "String to be prepended on every output file name",
                   StringValue ("stat"),
                   MakeStringAccessor (&SatStatsHelperContainer::SetName,
                                       &SatStatsHelperContainer::GetName),
                   MakeStringChecker ())
    .AddAttribute ("PerUtUserFwdThroughputScalar",
                   "Enable the scalar output of "
                   "per UT user forward link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtUserFwdThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE, "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_TRACE_FILE,  "TRACE_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,    "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,    "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT, "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_TRACE_PLOT,  "TRACE_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,    "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,    "CDF_PLOT"))
  ;
  return tid;
}


void
SatStatsHelperContainer::SetName (std::string name)
{
  NS_LOG_FUNCTION (this << name);

  // convert all spaces and slashes in the name to underscores
  for (size_t pos = name.find_first_of (" /");
       pos != std::string::npos;
       pos = name.find_first_of (" /", pos + 1, 1))
    {
      name[pos] = '_';
    }

  m_name = name;
}


std::string
SatStatsHelperContainer::GetName () const
{
  return m_name;
}


void
SatStatsHelperContainer::AddPerUtUserFwdThroughput (
  SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> ();
      stat->SetName (m_name + "-per-ut-user-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Connect (m_satHelper);
      m_stats.push_back (stat);
    }
}

/*
void
SatStatsHelperContainer::AddPerBeamFwdThroughput (
  SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> ();
      stat->SetName (m_name + "-per-ut-user-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Connect (m_satHelper);
      m_stats.push_back (stat);
    }
}
*/

std::string // static
SatStatsHelperContainer::GetOutputTypeSuffix (SatStatsHelper::OutputType_t outputType)
{
  switch (outputType)
    {
    case SatStatsHelper::OUTPUT_NONE:
      return "";

    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      return "-scalar";

    case SatStatsHelper::OUTPUT_TRACE_FILE:
    case SatStatsHelper::OUTPUT_TRACE_PLOT:
      return "-trace";

    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
      return "-pdf";

    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      return "-cdf";

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
  return "";
}


} // end of namespace ns3
