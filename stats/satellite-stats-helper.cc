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
SatStatsHelper::GetIdentiferTypeName (SatStatsHelper::IdentifierType_t identifierType)
{
  switch (identifierType)
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return "IDENTIFIER_GLOBAL";
    case SatStatsHelper::IDENTIFIER_GW:
      return "IDENTIFIER_GW";
    case SatStatsHelper::IDENTIFIER_BEAM:
      return "IDENTIFIER_BEAM";
    case SatStatsHelper::IDENTIFIER_UT:
      return "IDENTIFIER_UT";
    case SatStatsHelper::IDENTIFIER_UT_USER:
      return "IDENTIFIER_UT_USER";
    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
  return "";
}


std::string // static
SatStatsHelper::GetOutputTypeName (SatStatsHelper::OutputType_t outputType)
{
  switch (outputType)
    {
    case SatStatsHelper::OUTPUT_NONE:
      return "OUTPUT_NONE";
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
      return "OUTPUT_SCALAR_FILE";
    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      return "OUTPUT_SCATTER_FILE";
    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
      return "OUTPUT_HISTOGRAM_FILE";
    case SatStatsHelper::OUTPUT_PDF_FILE:
      return "OUTPUT_PDF_FILE";
    case SatStatsHelper::OUTPUT_CDF_FILE:
      return "OUTPUT_CDF_FILE";
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      return "OUTPUT_SCALAR_PLOT";
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      return "OUTPUT_SCATTER_PLOT";
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
      return "OUTPUT_HISTOGRAM_PLOT";
    case SatStatsHelper::OUTPUT_PDF_PLOT:
      return "OUTPUT_PDF_PLOT";
    case SatStatsHelper::OUTPUT_CDF_PLOT:
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
    m_identifierType (SatStatsHelper::IDENTIFIER_GLOBAL),
    m_outputType (SatStatsHelper::OUTPUT_SCATTER_FILE),
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
SatStatsHelper::SetIdentifierType (SatStatsHelper::IdentifierType_t identifierType)
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
SatStatsHelper::SetOutputType (SatStatsHelper::OutputType_t outputType)
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


Ptr<DataCollectionObject>
SatStatsHelper::CreateAggregator (std::string aggregatorTypeId,
                                  std::string n1, const AttributeValue &v1,
                                  std::string n2, const AttributeValue &v2,
                                  std::string n3, const AttributeValue &v3,
                                  std::string n4, const AttributeValue &v4,
                                  std::string n5, const AttributeValue &v5)
{
  NS_LOG_FUNCTION (this << aggregatorTypeId);

  TypeId tid = TypeId::LookupByName (aggregatorTypeId);
  ObjectFactory factory;
  factory.SetTypeId (tid);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  return factory.Create ()->GetObject<DataCollectionObject> ();
}


uint32_t
SatStatsHelper::CreateCollectors (std::string collectorTypeId,
                                  SatStatsHelper::CollectorMap_t &collectorMap,
                                  std::string n1, const AttributeValue &v1,
                                  std::string n2, const AttributeValue &v2,
                                  std::string n3, const AttributeValue &v3,
                                  std::string n4, const AttributeValue &v4,
                                  std::string n5, const AttributeValue &v5) const
{
  NS_LOG_FUNCTION (this << collectorTypeId);

  uint32_t n = 0;
  TypeId tid = TypeId::LookupByName (collectorTypeId);
  ObjectFactory factory;
  factory.SetTypeId (tid);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);

  switch (GetIdentifierType ())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      {
        factory.Set ("Name", StringValue ("global"));
        collectorMap[0] = factory.Create ()->GetObject<DataCollectionObject> ();
        n++;
        break;
      }

    case SatStatsHelper::IDENTIFIER_GW:
      // TODO
      break;

    case SatStatsHelper::IDENTIFIER_BEAM:
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

    case SatStatsHelper::IDENTIFIER_UT:
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

    case SatStatsHelper::IDENTIFIER_UT_USER:
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
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return 0;

    case SatStatsHelper::IDENTIFIER_GW:
      return 0; // TODO

    case SatStatsHelper::IDENTIFIER_BEAM:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);
        NS_ASSERT_MSG (utNode != 0,
                       "UT user node " << utUserNode
                                       << " is not attached to any UT node");
        const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
        const Address utMac = satIdMapper->GetUtMacWithNode (utNode);
        return satIdMapper->GetBeamIdWithMac (utMac);
      }

    case SatStatsHelper::IDENTIFIER_UT:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);
        NS_ASSERT_MSG (utNode != 0,
                       "UT user node " << utUserNode
                                       << " is not attached to any UT node");
        return GetUtId (utNode);
      }

    case SatStatsHelper::IDENTIFIER_UT_USER:
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
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return 0;

    case SatStatsHelper::IDENTIFIER_GW:
      return 0; // TODO

    case SatStatsHelper::IDENTIFIER_BEAM:
      {
        const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
        const Address utMac = satIdMapper->GetUtMacWithNode (utNode);
        return satIdMapper->GetBeamIdWithMac (utMac);
      }

    case SatStatsHelper::IDENTIFIER_UT:
      {
        return GetUtId (utNode);
      }

    case SatStatsHelper::IDENTIFIER_UT_USER:
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
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return 0;

    case SatStatsHelper::IDENTIFIER_GW:
      return 0; // TODO

    case SatStatsHelper::IDENTIFIER_BEAM:
      return beamId;

    case SatStatsHelper::IDENTIFIER_UT:
    case SatStatsHelper::IDENTIFIER_UT_USER:
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
