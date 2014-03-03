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
#include <ns3/satellite-helper.h>
#include <ns3/satellite-beam-helper.h>
#include <ns3/satellite-user-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/singleton.h>
#include <ns3/scatter-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsHelper");


namespace ns3 {


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


} // end of namespace ns3
