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
#include <ns3/satellite-env-variables.h>
#include <ns3/singleton.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/node-container.h>
#include <ns3/collector-map.h>
#include <ns3/data-collection-object.h>
#include <ns3/log.h>
#include <ns3/type-id.h>
#include <ns3/object-factory.h>
#include <ns3/string.h>
#include <ns3/enum.h>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SatStatsHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsHelper);

std::string // static
SatStatsHelper::GetIdentifierTypeName (SatStatsHelper::IdentifierType_t identifierType)
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


TypeId // static
SatStatsHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsHelper")
    .SetParent<Object> ()
    .AddAttribute ("Name",
                   "String to be prepended on every output file name.",
                   StringValue ("stat"),
                   MakeStringAccessor (&SatStatsHelper::SetName,
                                       &SatStatsHelper::GetName),
                   MakeStringChecker ())
    .AddAttribute ("IdentifierType",
                   "Determines how the statistics are categorized.",
                   EnumValue (SatStatsHelper::IDENTIFIER_GLOBAL),
                   MakeEnumAccessor (&SatStatsHelper::SetIdentifierType,
                                     &SatStatsHelper::GetIdentifierType),
                   MakeEnumChecker (SatStatsHelper::IDENTIFIER_GLOBAL,  "GLOBAL",
                                    SatStatsHelper::IDENTIFIER_GW,      "GW",
                                    SatStatsHelper::IDENTIFIER_BEAM,    "BEAM",
                                    SatStatsHelper::IDENTIFIER_UT,      "UT",
                                    SatStatsHelper::IDENTIFIER_UT_USER, "UT_USER"))
    .AddAttribute ("OutputType",
                   "Determines the type and format of the output.",
                   EnumValue (SatStatsHelper::OUTPUT_SCATTER_FILE),
                   MakeEnumAccessor (&SatStatsHelper::SetOutputType,
                                     &SatStatsHelper::GetOutputType),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
  ;
  return tid;
}


void
SatStatsHelper::Install ()
{
  NS_LOG_FUNCTION (this);

  if (m_outputType == SatStatsHelper::OUTPUT_NONE)
    {
      NS_LOG_WARN (this << " Skipping statistics installation"
                        << " because OUTPUT_NONE output type is selected.");
    }
  else
    {
      DoInstall (); // this method is supposed to be implemented by the child class
      m_isInstalled = true;
    }
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
  NS_LOG_FUNCTION (this << GetIdentifierTypeName (identifierType));

  if (m_isInstalled && (m_identifierType != identifierType))
    {
      NS_LOG_WARN (this << " cannot modify the current identifier type"
                        << " (" << GetIdentifierTypeName (m_identifierType) << ")"
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
  NS_LOG_FUNCTION (this << GetOutputTypeName (outputType));

  if (m_isInstalled && (m_outputType != outputType))
    {
      NS_LOG_WARN (this << " cannot modify the current output type"
                        << " (" << GetIdentifierTypeName (m_identifierType) << ")"
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


bool
SatStatsHelper::IsInstalled () const
{
  return m_isInstalled;
}


Ptr<const SatHelper>
SatStatsHelper::GetSatHelper () const
{
  return m_satHelper;
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
SatStatsHelper::CreateCollectorPerIdentifier (CollectorMap &collectorMap) const
{
  NS_LOG_FUNCTION (this);
  uint32_t n = 0;

  switch (GetIdentifierType ())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      {
        collectorMap.SetAttribute ("Name", StringValue ("0"));
        collectorMap.Create (0);
        n++;
        break;
      }

    case SatStatsHelper::IDENTIFIER_GW:
      {
        NodeContainer gws = m_satHelper->GetBeamHelper ()->GetGwNodes ();
        for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
          {
            const uint32_t gwId = GetGwId (*it);
            std::ostringstream name;
            name << gwId;
            collectorMap.SetAttribute ("Name", StringValue (name.str ()));
            collectorMap.Create (gwId);
            n++;
          }
        break;
      }

    case SatStatsHelper::IDENTIFIER_BEAM:
      {
        std::list<uint32_t> beams = m_satHelper->GetBeamHelper ()->GetBeams ();
        for (std::list<uint32_t>::const_iterator it = beams.begin ();
             it != beams.end (); ++it)
          {
            const uint32_t beamId = (*it);
            std::ostringstream name;
            name << beamId;
            collectorMap.SetAttribute ("Name", StringValue (name.str ()));
            collectorMap.Create (beamId);
            n++;
          }
        break;
      }

    case SatStatsHelper::IDENTIFIER_UT:
      {
        NodeContainer uts = m_satHelper->GetBeamHelper ()->GetUtNodes ();
        for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
          {
            const uint32_t utId = GetUtId (*it);
            std::ostringstream name;
            name << utId;
            collectorMap.SetAttribute ("Name", StringValue (name.str ()));
            collectorMap.Create (utId);
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
            const uint32_t utUserId = GetUtUserId (*it);
            std::ostringstream name;
            name << utUserId;
            collectorMap.SetAttribute ("Name", StringValue (name.str ()));
            collectorMap.Create (utUserId);
            n++;
          }
        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }

  NS_LOG_INFO (this << " created " << n << " instance(s)"
                    << " of " << collectorMap.GetType ().GetName ()
                    << " for " << GetIdentifierTypeName (GetIdentifierType ()));

  return n;

} // end of `uint32_t CreateCollectorPerIdentifier (CollectorMap &);`


std::string
SatStatsHelper::GetOutputPath () const
{
  return Singleton<SatEnvVariables>::Get ()->GetOutputPath ();
}


std::string
SatStatsHelper::GetOutputFileName () const
{
  return GetOutputPath () + "/" + GetName ();
}


std::string
SatStatsHelper::GetIdentifierHeading (std::string dataLabel) const
{
  switch (GetIdentifierType ())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return "% global " + dataLabel;

    case SatStatsHelper::IDENTIFIER_GW:
      return "% gw_id " + dataLabel;

    case SatStatsHelper::IDENTIFIER_BEAM:
      return "% beam_id " + dataLabel;

    case SatStatsHelper::IDENTIFIER_UT:
      return "% ut_id " + dataLabel;

    case SatStatsHelper::IDENTIFIER_UT_USER:
      return "% ut_user_id " + dataLabel;

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid identifier type");
      break;
    }
  return "";
}


std::string
SatStatsHelper::GetTimeHeading (std::string dataLabel) const
{
  return "% time_sec " + dataLabel;
}


std::string
SatStatsHelper::GetDistributionHeading (std::string dataLabel) const
{
  return "% " + dataLabel + " freq";
}


// IDENTIFIER RELATED METHODS /////////////////////////////////////////////////

uint32_t
SatStatsHelper::GetUtUserId (Ptr<Node> utUserNode) const
{
  uint32_t ret = 0;
  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
  const Address addr = satIdMapper->GetUtUserMacWithNode (utUserNode);

  if (addr.IsInvalid ())
    {
      NS_LOG_WARN (this << " Node " << utUserNode->GetId ()
                        << " does not have any valid Mac48Address");
    }
  else
    {
      const int32_t utUserId = satIdMapper->GetUtUserIdWithMac (addr);

      if (utUserId < 0)
        {
          NS_LOG_WARN (this << " Node " << utUserNode->GetId ()
                            << " is not found in the global list of UT users");
        }
      else
        {
          ret = utUserId;
        }
    }

  return ret;
}


uint32_t
SatStatsHelper::GetUtId (Ptr<Node> utNode) const
{
  uint32_t ret = 0;
  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
  const Address addr = satIdMapper->GetUtMacWithNode (utNode);

  if (addr.IsInvalid ())
    {
      NS_LOG_WARN (this << " Node " << utNode->GetId ()
                        << " does not have any valid Mac48Address");
    }
  else
    {
      const int32_t utId = satIdMapper->GetUtIdWithMac (addr);

      if (utId < 0)
        {
          NS_LOG_WARN (this << " Node " << utNode->GetId ()
                            << " is not found in the global list of UTs");
        }
      else
        {
          ret = utId;
        }
    }

  return ret;
}


uint32_t
SatStatsHelper::GetGwId (Ptr<Node> gwNode) const
{
  uint32_t ret = 0;
  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
  const Address addr = satIdMapper->GetGwMacWithNode (gwNode);

  if (addr.IsInvalid ())
    {
      NS_LOG_WARN (this << " Node " << gwNode->GetId ()
                        << " does not have any valid Mac48Address");
    }
  else
    {
      const int32_t gwId = satIdMapper->GetGwIdWithMac (addr);

      if (gwId < 0)
        {
          NS_LOG_WARN (this << " Node " << gwNode->GetId ()
                            << " is not found in the global list of GWs");
        }
      else
        {
          ret = gwId;
        }
    }

  return ret;
}


uint32_t
SatStatsHelper::GetIdentifierForUtUser (Ptr<Node> utUserNode) const
{
  uint32_t ret = 0;

  switch (m_identifierType)
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      ret = 0;
      break;

    case SatStatsHelper::IDENTIFIER_GW:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);

        if (utNode == 0)
          {
            NS_LOG_WARN (this << " UT user node " << utUserNode->GetId ()
                              << " is not attached to any UT node");
          }
        else
          {
            const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
            const Address utMac = satIdMapper->GetUtMacWithNode (utNode);

            if (!utMac.IsInvalid ())
              {
                const int32_t beamId = satIdMapper->GetBeamIdWithMac (utMac);
                NS_ASSERT_MSG (beamId != -1,
                               "UT user node " << utUserNode->GetId ()
                                               << " is not attached to any beam");
                const uint32_t gwId = m_satHelper->GetBeamHelper ()->GetGwId (beamId);
                NS_ASSERT_MSG (gwId != 0,
                               "UT user node " << utUserNode->GetId ()
                                               << " is not attached to any GW");
                ret = gwId;
              }
          }

        break;
      }

    case SatStatsHelper::IDENTIFIER_BEAM:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);

        if (utNode == 0)
          {
            NS_LOG_WARN (this << " UT user node " << utUserNode->GetId ()
                              << " is not attached to any UT node");
          }
        else
          {
            const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
            const Address utMac = satIdMapper->GetUtMacWithNode (utNode);

            if (!utMac.IsInvalid ())
              {
                const int32_t beamId = satIdMapper->GetBeamIdWithMac (utMac);
                NS_ASSERT_MSG (beamId != -1,
                               "UT user node " << utUserNode->GetId ()
                                               << " is not attached to any beam");
                ret = beamId;
              }
          }

        break;
      }

    case SatStatsHelper::IDENTIFIER_UT:
      {
        Ptr<Node> utNode = m_satHelper->GetUserHelper ()->GetUtNode (utUserNode);

        if (utNode == 0)
          {
            NS_LOG_WARN (this << " UT user node " << utUserNode->GetId ()
                              << " is not attached to any UT node");
          }
        else
          {
            ret = GetUtId (utNode);
          }

        break;
      }

    case SatStatsHelper::IDENTIFIER_UT_USER:
      ret = GetUtUserId (utUserNode);
      break;

    default:
      NS_LOG_WARN (this << " Identifier type "
                        << GetIdentifierTypeName (m_identifierType)
                        << " is not valid for a UT user."
                        << " Assigning identifier 0 to this UT user.");
      break;
    }

  return ret;
}


uint32_t
SatStatsHelper::GetIdentifierForUt (Ptr<Node> utNode) const
{
  uint32_t ret = 0;

  switch (m_identifierType)
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      ret = 0;
      break;

    case SatStatsHelper::IDENTIFIER_GW:
      {
        const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
        const Address utMac = satIdMapper->GetUtMacWithNode (utNode);

        if (!utMac.IsInvalid ())
          {
            const int32_t beamId = satIdMapper->GetBeamIdWithMac (utMac);
            NS_ASSERT_MSG (beamId != -1,
                           "UT node " << utNode->GetId ()
                                      << " is not attached to any beam");
            const uint32_t gwId = m_satHelper->GetBeamHelper ()->GetGwId (beamId);
            NS_ASSERT_MSG (gwId != 0,
                           "UT node " << utNode->GetId ()
                                      << " is not attached to any GW");
            ret = gwId;
          }

        break;
      }

    case SatStatsHelper::IDENTIFIER_BEAM:
      {
        const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
        const Address utMac = satIdMapper->GetUtMacWithNode (utNode);

        if (!utMac.IsInvalid ())
          {
            const int32_t beamId = satIdMapper->GetBeamIdWithMac (utMac);
            NS_ASSERT_MSG (beamId != -1,
                           "UT node " << utNode->GetId ()
                                      << " is not attached to any beam");
            ret = beamId;
          }

        break;
      }

    case SatStatsHelper::IDENTIFIER_UT:
      ret = GetUtId (utNode);
      break;

    default:
      NS_LOG_WARN (this << " Identifier type "
                        << GetIdentifierTypeName (m_identifierType)
                        << " is not valid for a UT."
                        << " Assigning identifier 0 to this UT.");
      break;
    }

  return ret;
}


uint32_t
SatStatsHelper::GetIdentifierForBeam (uint32_t beamId) const
{
  uint32_t ret = 0;

  switch (m_identifierType)
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      ret = 0;
      break;

    case SatStatsHelper::IDENTIFIER_GW:
      {
        const uint32_t gwId = m_satHelper->GetBeamHelper ()->GetGwId (beamId);
        NS_ASSERT_MSG (gwId != 0,
                       "Beam " << beamId << " is not attached to any GW");
        ret = gwId;
        break;
      }

    case SatStatsHelper::IDENTIFIER_BEAM:
      ret = beamId;
      break;

    default:
      NS_LOG_WARN (this << " Identifier type "
                        << GetIdentifierTypeName (m_identifierType)
                        << " is not valid for a beam."
                        << " Assigning identifier 0 to this beam.");
      break;
    }

  return ret;
}


uint32_t
SatStatsHelper::GetIdentifierForGw (Ptr<Node> gwNode) const
{
  if (m_identifierType == IDENTIFIER_GW)
    {
      return GetGwId (gwNode);
    }
  else if (m_identifierType == IDENTIFIER_GLOBAL)
    {
      return 0;
    }
  else
    {
      NS_LOG_WARN (this << " Identifier type "
                        << GetIdentifierTypeName (m_identifierType)
                        << " is not valid for a GW."
                        << " Assigning identifier 0 to this GW.");
      return 0;
    }
}


NetDeviceContainer // static
SatStatsHelper::GetGwSatNetDevice (Ptr<Node> gwNode)
{
  NetDeviceContainer ret;

  NS_LOG_DEBUG ("Node ID " << gwNode->GetId ()
                           << " has " << gwNode->GetNDevices () << " devices");
  /*
   * Assuming that device #0 is for loopback device, device #(N-1) is for
   * backbone network device, and devices #1 until #(N-2) are for satellite
   * beam device.
   */
  for (uint32_t i = 1; i <= gwNode->GetNDevices () - 2; i++)
    {
      Ptr<NetDevice> dev = gwNode->GetDevice (i);

      if (dev->GetObject<SatNetDevice> () == 0)
        {
          NS_FATAL_ERROR ("Node " << gwNode->GetId () << " is not a valid GW");
        }
      else
        {
          ret.Add (dev);
        }
    }

  return ret;
}


Ptr<NetDevice> // static
SatStatsHelper::GetUtSatNetDevice (Ptr<Node> utNode)
{
  /*
   * Assuming that device #0 is for loopback device, device #1 is for
   * subscriber network device, and device #2 is for satellite beam device.
   */
  NS_ASSERT (utNode->GetNDevices () >= 3);
  Ptr<NetDevice> dev = utNode->GetDevice (2);

  if (dev->GetObject<SatNetDevice> () == 0)
    {
      NS_FATAL_ERROR ("Node " << utNode->GetId () << " is not a valid UT");
    }

  return dev;
}


} // end of namespace ns3
