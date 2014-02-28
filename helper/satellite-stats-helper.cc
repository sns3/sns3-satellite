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
#include <ns3/type-id.h>
#include <ns3/object-factory.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-beam-helper.h>
#include <ns3/satellite-user-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/singleton.h>
#include <ns3/data-collection-object.h>
#include <ns3/probe.h>
#include <ns3/application-packet-probe.h>
#include <ns3/packet-data-rate-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/scatter-collector.h>
#include <ns3/multi-file-aggregator.h>
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
    case IDENTIFIER_GW:
      return "IDENTIFIER_GW";
    case IDENTIFIER_BEAM:
      return "IDENTIFIER_BEAM";
    case IDENTIFIER_UT:
      return "IDENTIFIER_UT";
    case IDENTIFIER_UT_USER:
      return "IDENTIFIER_UT_USER";
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
    case OUTPUT_SCATTER_FILE:
      return "OUTPUT_SCATTER_FILE";
    case OUTPUT_HISTOGRAM_FILE:
      return "OUTPUT_HISTOGRAM_FILE";
    case OUTPUT_PDF_FILE:
      return "OUTPUT_PDF_FILE";
    case OUTPUT_CDF_FILE:
      return "OUTPUT_CDF_FILE";
    case OUTPUT_SCALAR_PLOT:
      return "OUTPUT_SCALAR_PLOT";
    case OUTPUT_SCATTER_PLOT:
      return "OUTPUT_SCATTER_PLOT";
    case OUTPUT_HISTOGRAM_PLOT:
      return "OUTPUT_HISTOGRAM_PLOT";
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


SatStatsHelper::SatStatsHelper (Ptr<const SatHelper> satHelper)
  : m_name ("stat"),
    m_identifierType (IDENTIFIER_GLOBAL),
    m_outputType (OUTPUT_SCATTER_FILE),
    m_isInstalled (false),
    m_satHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsHelper::~SatStatsHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsHelper::Install ()
{
  NS_LOG_FUNCTION (this);
  DoInstall (); // this method is supposed to be implemented by the child class
  m_isInstalled = true;
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

  if (m_isInstalled && (m_identifierType != identifierType))
    {
      NS_LOG_WARN (this << " cannot modify the current identifier type"
                        << " (" << GetIdentiferTypeName (m_identifierType) << ")"
                        << " because this instance have already been installed");
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
  if (m_isInstalled && (m_outputType != outputType))
    {
      NS_LOG_WARN (this << " cannot modify the current output type"
                        << " (" << GetIdentiferTypeName (m_identifierType) << ")"
                        << " because this instance have already been installed");
    }
  else
    {
      m_outputType = outputType;
    }
}


SatStatsHelper::OutputType_t
SatStatsHelper::GetOutputType () const
{
  return m_outputType;
}


Ptr<const SatHelper>
SatStatsHelper::GetSatHelper () const
{
  return m_satHelper;
}


Ptr<DataCollectionObject>
SatStatsHelper::GetAggregator () const
{
  return m_aggregator;
}


uint32_t // static
SatStatsHelper::GetUtId (Ptr<Node> ut)
{
  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();

  const Address addr = satIdMapper->GetUtMacWithNode (ut);
  NS_ASSERT_MSG (Mac48Address::IsMatchingType (addr),
                 "Node " << ut->GetId ()
                         << " does not have any valid Mac48Address");

  const int32_t utId = satIdMapper->GetUtIdWithMac (addr);
  NS_ASSERT_MSG (utId != -1,
                 "Node " << ut->GetId ()
                         << " is not found in the global list of UTs");

  return utId;
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


void
SatStatsHelper::CreateAggregator ()
{
  NS_LOG_FUNCTION (this);

  switch (m_outputType)
  {
    case OUTPUT_NONE:
      NS_LOG_WARN (this << " output type is not set yet; no aggregator is installed.");
      m_aggregator = 0;
      break;

    case OUTPUT_SCALAR_FILE:
    case OUTPUT_SCATTER_FILE:
    case OUTPUT_HISTOGRAM_FILE:
    case OUTPUT_PDF_FILE:
    case OUTPUT_CDF_FILE:
      {
        const std::string fileName = GetName ();
        Ptr<MultiFileAggregator> multiFile = CreateObject<MultiFileAggregator> (fileName);
        m_aggregator = multiFile;
        break;
      }

    case OUTPUT_SCALAR_PLOT:
    case OUTPUT_SCATTER_PLOT:
    case OUTPUT_HISTOGRAM_PLOT:
    case OUTPUT_PDF_PLOT:
    case OUTPUT_CDF_PLOT:
      m_aggregator = 0;  // TODO
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
  }
}


uint32_t
SatStatsHelper::CreateTerminalCollectors (CollectorMap_t &collectorMap) const
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (collectorMap.size () == 0,
                 "The provided collector map is not empty");

  // Create the collectors according to the output type and connect them to the aggregator.
  switch (m_outputType)
  {
    case OUTPUT_NONE:
      NS_LOG_WARN (this << " output type is not set yet; no collector is installed.");
      return 0;

    case OUTPUT_SCALAR_FILE:
    case OUTPUT_SCALAR_PLOT:
      return 0;  // TODO
      break;

    case OUTPUT_SCATTER_FILE:
    case OUTPUT_SCATTER_PLOT:
      {
        const uint32_t n = CreateCollectors ("ns3::ScatterCollector",
                                             collectorMap);
        NS_LOG_INFO (this << " created " << n << " terminal ScatterCollector(s)"
                          << " for " << GetIdentiferTypeName (GetIdentifierType())
                          << " identifier");
        NS_ASSERT (collectorMap.size () == n);
        return n;
        break;
      }

    case OUTPUT_HISTOGRAM_FILE:
    case OUTPUT_HISTOGRAM_PLOT:
      return 0;  // TODO
      break;

    case OUTPUT_PDF_FILE:
    case OUTPUT_PDF_PLOT:
      return 0;  // TODO
      break;

    case OUTPUT_CDF_FILE:
    case OUTPUT_CDF_PLOT:
      return 0;  // TODO
      break;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
  }
}


uint32_t
SatStatsHelper::CreateCollectors (std::string collectorTypeId,
                                  SatStatsHelper::CollectorMap_t &collectorMap) const
{
  NS_LOG_FUNCTION (this << collectorTypeId);

  uint32_t n = 0;
  TypeId tid = TypeId::LookupByName (collectorTypeId);
  ObjectFactory factory;
  factory.SetTypeId (tid);

  switch (GetIdentifierType ())
    {
    case IDENTIFIER_GLOBAL:
      {
        factory.Set ("Name", StringValue ("global"));
        collectorMap[0] = factory.Create ()->GetObject<DataCollectionObject> ();
        n++;
        break;
      }

    case IDENTIFIER_GW:
      // TODO
      break;

    case IDENTIFIER_BEAM:
      {
        std::list<uint32_t> beams = m_satHelper->GetBeamHelper ()->GetBeams ();
        for (std::list<uint32_t>::const_iterator it = beams.begin ();
             it != beams.end (); ++it)
          {
            const uint32_t beamId = (*it);
            std::ostringstream name;
            name << "beam-" << beamId;
            factory.Set ("Name", StringValue (name.str ()));
            collectorMap[beamId] = factory.Create ()->GetObject<DataCollectionObject> (tid);
            n++;
          }
        break;
      }

    case IDENTIFIER_UT:
      {
        NodeContainer uts = m_satHelper->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
          {
            const uint32_t utId = SatStatsHelper::GetUtId (*it);
            std::ostringstream name;
            name << "ut-" << utId;
            factory.Set ("Name", StringValue (name.str ()));
            collectorMap[utId] = factory.Create ()->GetObject<DataCollectionObject> (tid);
            n++;
          }
        break;
      }

    case IDENTIFIER_UT_USER:
      {
        NodeContainer utUsers = m_satHelper->GetUtUsers ();
        for (NodeContainer::Iterator it = utUsers.Begin ();
             it != utUsers.End (); ++it)
          {
            const uint32_t utUserId = SatStatsHelper::GetUtUserId (*it);
            std::ostringstream name;
            name << "ut-user-" << utUserId;
            factory.Set ("Name", StringValue (name.str ()));
            collectorMap[utUserId] = factory.Create ()->GetObject<DataCollectionObject> (tid);
            n++;
          }
        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  NS_LOG_INFO (this << " created " << n << " instance(s)"
                    << " of " << collectorTypeId
                    << " for " << GetIdentiferTypeName (GetIdentifierType ()));

  return n;

} // end of `CreateCollectors`


template<typename R, typename C, typename P1, typename P2>
Ptr<Probe>
SatStatsHelper::InstallProbe (Ptr<Object> object,
                              std::string objectTypeId,
                              std::string objectTraceSourceName,
                              std::string probeName,
                              std::string probeTypeId,
                              std::string probeTraceSourceName,
                              uint32_t    identifier,
                              SatStatsHelper::CollectorMap_t &collectorMap,
                              R (C::*collectorTraceSink) (P1, P2)) const
{
  NS_LOG_FUNCTION (this << object << objectTypeId << objectTraceSourceName
                        << probeName << probeTypeId << probeTraceSourceName
                        << identifier);

  // Confirm that the object has the right type and the specified trace source.
  TypeId objectTid = TypeId::LookupByName (objectTypeId);
  NS_ASSERT (object->GetObject<Object> (objectTid));
  //NS_ASSERT (objectTid.LookupTraceSourceByName (objectTraceSourceName) != 0);

  // Create the probe.
  TypeId probeTid = TypeId::LookupByName (probeTypeId);
  NS_ASSERT (probeTid.LookupTraceSourceByName (probeTraceSourceName) != 0);
  ObjectFactory factory;
  factory.SetTypeId (probeTid);
  factory.Set ("Name", StringValue (probeName));
  Ptr<Probe> probe = factory.Create ()->GetObject<Probe> (probeTid);
  NS_LOG_INFO (this << " created probe " << probeName);

  // Connect the probe to the object.
  if (probe->ConnectByObject (objectTraceSourceName, object))
    {
      NS_LOG_INFO (this << " probe " << probeName << " is connected with "
                        << objectTypeId << "::" << objectTraceSourceName
                        << " (" << object << ")");

      // Connect the probe to the right collector.
      SatStatsHelper::CollectorMap_t::iterator it = collectorMap.find (identifier);
      NS_ASSERT_MSG (it != collectorMap.end (),
                     "Unable to find collector with identifier " << identifier);
      Ptr<C> collector = it->second->GetObject<C> ();
      NS_ASSERT (collector != 0);

      if (probe->TraceConnectWithoutContext (probeTraceSourceName,
                                             MakeCallback (collectorTraceSink,
                                                           collector)))
        {
          NS_LOG_INFO (this << " probe " << probeName << " is connected with"
                            << " collector " << collector->GetName ());
          return probe;
        }
      else
        {
          NS_LOG_WARN (this << " unable to connect probe " << probeName
                            << " to collector " << collector->GetName ());
          return 0;
        }
    }
  else
    {
      NS_LOG_WARN (this << " unable to connect probe " << probeName << " to "
                        << objectTypeId << "::" << objectTraceSourceName
                        << " (" << object << ")");
      return 0;
    }

} // end of `InstallProbe`


template<typename R, typename C, typename P1, typename P2>
bool
SatStatsHelper::ConnectCollectorToCollector (SatStatsHelper::CollectorMap_t &sourceCollectorMap,
                                             std::string sourceCollectorTypeId,
                                             std::string traceSourceName,
                                             SatStatsHelper::CollectorMap_t &targetCollectorMap,
                                             std::string targetCollectorTypeId,
                                             R (C::*traceSink) (P1, P2)) const
{
  NS_ASSERT (sourceCollectorMap.size () == targetCollectorMap.size ());

  for (SatStatsHelper::CollectorMap_t::iterator it1 = sourceCollectorMap.begin ();
       it1 != sourceCollectorMap.end (); ++it1)
    {
      const uint32_t identifier = it1->first;
      SatStatsHelper::CollectorMap_t::iterator it2 = targetCollectorMap.find (identifier);
      NS_ASSERT_MSG (it2 != targetCollectorMap.end (),
                     "Unable to find target collector with identifier " << identifier);

      if (!ConnectCollectorToCollector (it1->second,
                                        sourceCollectorTypeId,
                                        traceSourceName,
                                        it2->second,
                                        targetCollectorTypeId,
                                        traceSink))
        {
          return false;
        }
    }

  return true;
}


template<typename R, typename C, typename P1, typename P2>
bool
SatStatsHelper::ConnectCollectorToCollector (SatStatsHelper::CollectorMap_t &sourceCollectorMap,
                                             std::string sourceCollectorTypeId,
                                             std::string traceSourceName,
                                             Ptr<DataCollectionObject> targetCollector,
                                             std::string targetCollectorTypeId,
                                             R (C::*traceSink) (P1, P2)) const
{
  for (SatStatsHelper::CollectorMap_t::iterator it = sourceCollectorMap.begin ();
       it != sourceCollectorMap.end (); ++it)
    {
      if (!ConnectCollectorToCollector (it->second,
                                        sourceCollectorTypeId,
                                        traceSourceName,
                                        targetCollector,
                                        targetCollectorTypeId,
                                        traceSink))
        {
          return false;
        }
    }

  return true;
}


template<typename R, typename C, typename P1, typename P2>
bool
SatStatsHelper::ConnectCollectorToCollector (Ptr<DataCollectionObject> sourceCollector,
                                             std::string sourceCollectorTypeId,
                                             std::string traceSourceName,
                                             SatStatsHelper::CollectorMap_t &targetCollectorMap,
                                             std::string targetCollectorTypeId,
                                             R (C::*traceSink) (P1, P2)) const
{
  for (SatStatsHelper::CollectorMap_t::iterator it = targetCollectorMap.begin ();
       it != targetCollectorMap.end (); ++it)
    {
      if (!ConnectCollectorToCollector (sourceCollector,
                                        sourceCollectorTypeId,
                                        traceSourceName,
                                        it->second,
                                        targetCollectorTypeId,
                                        traceSink))
        {
          return false;
        }
    }

  return true;
}


template<typename R, typename C, typename P1, typename P2>
bool
SatStatsHelper::ConnectCollectorToCollector (Ptr<DataCollectionObject> sourceCollector,
                                             std::string sourceCollectorTypeId,
                                             std::string traceSourceName,
                                             Ptr<DataCollectionObject> targetCollector,
                                             std::string targetCollectorTypeId,
                                             R (C::*traceSink) (P1, P2)) const
{
  NS_LOG_FUNCTION (this << sourceCollector->GetName () << traceSourceName
                        << targetCollector->GetName ());

  // Confirm that the source has the right type and the specified trace source.
  TypeId sourceTid = TypeId::LookupByName (sourceCollectorTypeId);
  NS_ASSERT (sourceCollector->GetObject<DataCollectionObject> (sourceTid));
  NS_ASSERT (sourceTid.LookupTraceSourceByName (traceSourceName) != 0);

  // Confirm that the target has the right type.
  //TypeId targetTid = TypeId::LookupByName (targetCollectorTypeId);
  //NS_ASSERT (targetCollector->GetObject<DataCollectionObject> (targetTid));
  Ptr<C> target = targetCollector->GetObject<C> ();

  return sourceCollector->TraceConnectWithoutContext (traceSourceName,
                                                      MakeCallback (traceSink,
                                                                    target));
}


uint32_t
SatStatsHelper::GetUtUserIdentifier (Ptr<Node> utUserNode) const
{
  switch (m_identifierType)
    {
    case IDENTIFIER_GLOBAL:
      return 0;

    case IDENTIFIER_GW:
      return 0; // TODO

    case IDENTIFIER_BEAM:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);
        NS_ASSERT_MSG (utNode != 0,
                       "UT user node " << utUserNode
                                       << " is not attached to any UT node");
        const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
        const Address utMac = satIdMapper->GetUtMacWithNode (utNode);
        return satIdMapper->GetBeamIdWithMac (utMac);
      }

    case IDENTIFIER_UT:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);
        NS_ASSERT_MSG (utNode != 0,
                       "UT user node " << utUserNode
                                       << " is not attached to any UT node");
        return GetUtId (utNode);
      }

    case IDENTIFIER_UT_USER:
      return GetUtUserId (utUserNode);

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  return 0;
}


uint32_t
SatStatsHelper::GetUtIdentifier (Ptr<Node> utNode) const
{
  switch (m_identifierType)
    {
    case IDENTIFIER_GLOBAL:
      return 0;

    case IDENTIFIER_GW:
      return 0; // TODO

    case IDENTIFIER_BEAM:
      {
        const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
        const Address utMac = satIdMapper->GetUtMacWithNode (utNode);
        return satIdMapper->GetBeamIdWithMac (utMac);
      }

    case IDENTIFIER_UT:
      {
        return GetUtId (utNode);
      }

    case IDENTIFIER_UT_USER:
      return 0;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  return 0;
}


uint32_t
SatStatsHelper::GetBeamIdentifier (uint32_t beamId) const
{
  switch (m_identifierType)
    {
    case IDENTIFIER_GLOBAL:
      return 0;

    case IDENTIFIER_GW:
      return 0; // TODO

    case IDENTIFIER_BEAM:
      return beamId;

    case IDENTIFIER_UT:
    case IDENTIFIER_UT_USER:
      return 0;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  return 0;
}


uint32_t
SatStatsHelper::GetGwIdentifier (Ptr<Node> gwNode) const
{
  if (m_identifierType == IDENTIFIER_GW)
    {
      return 0; // TODO
    }
  else
    {
      return 0;
    }
}


// SATELLITE STATS FWD THROUGHPUT HELPER //////////////////////////////////////

SatStatsFwdThroughputHelper::SatStatsFwdThroughputHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}


SatStatsFwdThroughputHelper::~SatStatsFwdThroughputHelper ()
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsFwdThroughputHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  // Create interval rate collectors.
  CreateCollectors ("ns3::IntervalRateCollector", m_intervalRateCollectors);

  // Create a probe for each UT user's application inside the container.
  NodeContainer utUsers = GetSatHelper ()->GetUtUsers ();
  for (NodeContainer::Iterator it = utUsers.Begin (); it != utUsers.End (); ++it)
    {
      const int32_t utUserId = GetUtUserId (*it);
      const uint32_t identifier = GetUtUserIdentifier (*it);

      for (uint32_t i = 0; i < (*it)->GetNApplications (); i++)
        {
          std::ostringstream probeName;
          probeName << utUserId << "-" << i;
          Ptr<Probe> probe = InstallProbe ((*it)->GetApplication (i),
                                           "ns3::Application",
                                           "Rx",
                                           probeName.str (),
                                           "ns3::ApplicationPacketProbe",
                                           "OutputBytes",
                                           identifier,
                                           m_intervalRateCollectors,
                                           &IntervalRateCollector::TraceSinkUinteger32);
          if (probe != 0)
            {
              m_probes.push_back (probe);
            }
        }
    }

  CreateAggregator ();

  // Connect the terminal collectors to the aggregator and the interval collectors.
  for (SatStatsHelper::CollectorMap_t::iterator it = m_intervalRateCollectors.begin ();
       it != m_intervalRateCollectors.end (); ++it)
    {
      Ptr<DataCollectionObject> collector = it->second;
      NS_ASSERT (it->second != 0);
      const std::string context = collector->GetName ();

      switch (GetOutputType ())
      {
        case OUTPUT_NONE:
        case OUTPUT_SCALAR_FILE:
          break;

        case OUTPUT_SCATTER_FILE:
          {
            Ptr<MultiFileAggregator> aggregator
              = GetAggregator ()->GetObject<MultiFileAggregator> ();
            NS_ASSERT (aggregator != 0);
            bool ret = collector->TraceConnect ("OutputWithTime",
                                                context,
                                                MakeCallback (&MultiFileAggregator::Write2d,
                                                              aggregator));
            NS_ASSERT_MSG (ret,
                           "Failed to connect collector " << context
                                                          << " to aggregator");
            break;
          }

        case OUTPUT_HISTOGRAM_FILE:
        case OUTPUT_PDF_FILE:
        case OUTPUT_CDF_FILE:
        case OUTPUT_SCALAR_PLOT:
        case OUTPUT_SCATTER_PLOT:
        case OUTPUT_HISTOGRAM_PLOT:
        case OUTPUT_PDF_PLOT:
        case OUTPUT_CDF_PLOT:
          break;

        default:
          NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
          break;
      }
    }

} // end of `void DoInstall ();`


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
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
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
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Install ();
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

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      return "-scatter";

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
      return "-histogram";

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
