/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/queue.h"
#include "ns3/string.h"
#include "ns3/type-id.h"
#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/singleton.h"
#include "../model/satellite-position-allocator.h"
#include "../model/satellite-rtn-link-time.h"
#include "satellite-helper.h"
#include "../model/satellite-log.h"

NS_LOG_COMPONENT_DEFINE ("SatHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatHelper);

TypeId
SatHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatHelper")
      .SetParent<Object> ()
      .AddConstructor<SatHelper> ()
      .AddAttribute ("UtCount", "The count of created UTs in beam (full or user-defined scenario)",
                      UintegerValue (3),
                      MakeUintegerAccessor (&SatHelper::m_utsInBeam),
                      MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("GwUsers", "The number of created GW users (full or user-defined scenario)",
                      UintegerValue (5),
                      MakeUintegerAccessor (&SatHelper::m_gwUsers),
                      MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("UtUsers", "The number of created UT users per UT (full or user-defined scenario)",
                      UintegerValue (3),
                      MakeUintegerAccessor (&SatHelper::m_utUsers),
                      MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("BeamNetworkAddress",
                     "Initial network number to use "
                     "during allocation of satellite devices "
                     "(255.255.255.0 will be used as the network mask)",
                     Ipv4AddressValue ("10.1.1.0"),
                     MakeIpv4AddressAccessor (&SatHelper::SetBeamNetworkAddress,
                                              &SatHelper::GetBeamNetworkAddress),
                     MakeIpv4AddressChecker ())
      .AddAttribute ("GwNetworkAddress",
                     "Initial network number to use "
                     "during allocation of GW, router, and GW users "
                     "(255.255.255.0 will be used as the network mask)",
                     Ipv4AddressValue ("10.2.1.0"),
                     MakeIpv4AddressAccessor (&SatHelper::SetGwNetworkAddress,
                                              &SatHelper::GetGwNetworkAddress),
                     MakeIpv4AddressChecker ())
      .AddAttribute ("UtNetworkAddress",
                     "Initial network number to use "
                     "during allocation of UT and UT users "
                     "(255.255.255.0 will be used as the network mask)",
                     Ipv4AddressValue ("10.3.1.0"),
                     MakeIpv4AddressAccessor (&SatHelper::SetUtNetworkAddress,
                                              &SatHelper::GetUtNetworkAddress),
                     MakeIpv4AddressChecker ())
      .AddTraceSource ("Creation", "Creation traces",
                        MakeTraceSourceAccessor (&SatHelper::m_creationDetailsTrace))
      .AddTraceSource ("CreationSummary", "Creation summary traces",
                        MakeTraceSourceAccessor (&SatHelper::m_creationSummaryTrace))

    ;
    return tid;
}

TypeId
SatHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatHelper::SatHelper ()
: m_hasBeamNetworkSet (false),
  m_hasGwNetworkSet (false),
  m_hasUtNetworkSet (false),
  m_scenarioCreated (false),
  m_detailedCreationTraces (false),
  m_utsInBeam (0),
  m_gwUsers (0),
  m_utUsers (0)
{
  NS_LOG_FUNCTION (this);

  // Do nothing here
  NS_ASSERT (true);
}

SatHelper::SatHelper (std::string scenarioName)
 : m_hasBeamNetworkSet (false),
   m_hasGwNetworkSet (false),
   m_hasUtNetworkSet (false),
   m_scenarioCreated (false),
   m_detailedCreationTraces (false)
{
  NS_LOG_FUNCTION (this);

  // uncomment next line, if attributes are needed already in construction phase
  //ObjectBase::ConstructSelf(AttributeConstructionList ());

  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_GENERIC, "", "Logging for generic messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_INFO, "", "Logging for info messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", "Logging for warning messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_ERROR, "", "Logging for error messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_CUSTOM, "_customTag", "Logging for custom messages started");

  std::string path = "src/satellite/data/";

  std::string satConf = scenarioName + "Conf.txt";
  std::string gwPos = scenarioName + "GwPos.txt";
  std::string satPos = scenarioName + "GeoPos.txt";
  std::string wfConf = "dvbRcs2Waveforms.txt";

  m_satConf = CreateObject<SatConf> ();

  m_satConf->Initialize (path, satConf, gwPos, satPos, wfConf);

  // Create antenna gain patterns
  m_antennaGainPatterns = CreateObject<SatAntennaGainPatternContainer> ();

  // create Geo Satellite node, set mobility to it
  Ptr<Node> geoSatNode = CreateObject<Node>();
  SetGeoSatMobility(geoSatNode);

  m_beamHelper = CreateObject<SatBeamHelper> (geoSatNode,
                                              MakeCallback (&SatConf::GetCarrierBandwidthHz, m_satConf),
                                              m_satConf->GetRtnLinkCarrierCount(),
                                              m_satConf->GetFwdLinkCarrierCount(),
                                              m_satConf->GetSuperframeSeq());

  Ptr<SatRtnLinkTime> rtnTime = Singleton<SatRtnLinkTime>::Get ();
  rtnTime->Initialize (m_satConf->GetSuperframeSeq ());

  SatBeamHelper::CarrierFreqConverter converterCb = MakeCallback (&SatConf::GetCarrierFrequencyHz, m_satConf);
  m_beamHelper->SetAttribute ("CarrierFrequencyConverter", CallbackValue (converterCb) );

  m_userHelper = CreateObject<SatUserHelper> ();

  if ( m_detailedCreationTraces )
    {
      EnableDetailedCreationTraces();
    }

  // Set the antenna patterns to beam helper
  m_beamHelper->SetAntennaGainPatterns (m_antennaGainPatterns);
}

void
SatHelper::SetBeamUserInfo(BeamUserInfoMap_t infos)
{
  NS_LOG_FUNCTION (this);

  m_beamUserInfos = infos;
}

void
SatHelper::SetBeamUserInfo(uint32_t beamId, SatBeamUserInfo info)
{
  NS_LOG_FUNCTION (this);

  std::pair<BeamUserInfoMap_t::iterator, bool> result = m_beamUserInfos.insert(std::make_pair(beamId, info));
  NS_ASSERT(result.second == true);
}

void SatHelper::CreateScenario(PreDefinedScenario_t scenario)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT(m_scenarioCreated == false);

  switch(scenario)
  {
    case SIMPLE:
      CreateSimpleScenario();
      break;

    case LARGER:
      CreateLargerScenario();
      break;

    case FULL:
      CreateFullScenario();
      break;

    case USER_DEFINED:
      CreateUserDefinedScenario();
      break;

    default:
      break;
  }

}

void SatHelper::EnableCreationTraces(std::string filename, bool details)
{
  NS_LOG_FUNCTION (this);

  AsciiTraceHelper asciiTraceHelper;
  std::string outputFile = "creation.log";

  if (!filename.empty())
    {
      outputFile = filename;
    }

  m_creationTraceStream = asciiTraceHelper.CreateFileStream (outputFile);
  m_utTraceStream = asciiTraceHelper.CreateFileStream ("ut-pos-" + outputFile);

  TraceConnectWithoutContext("CreationSummary", MakeCallback (&SatHelper::CreationSummarySink, this));

  m_detailedCreationTraces = details;
}

void
SatHelper::EnablePacketTrace ()
{
  m_beamHelper->EnablePacketTrace ();
}


void SatHelper::EnableDetailedCreationTraces()
{
  NS_LOG_FUNCTION (this);

  CallbackBase creationCb = MakeBoundCallback (&SatHelper::CreationDetailsSink, m_creationTraceStream);
  TraceConnect("Creation", "SatHelper", creationCb);

  m_userHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
  m_beamHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
}

Ipv4Address
SatHelper::GetUserAddress(Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
  return ipv4->GetAddress (1, 0).GetLocal(); // Get Ipv4InterfaceAddress of interface csma interface.
}

NodeContainer
SatHelper::GetUtUsers () const
{
  NS_LOG_FUNCTION (this);

  return m_userHelper->GetUtUsers ();
}

NodeContainer
SatHelper::GetGwUsers () const
{
  NS_LOG_FUNCTION (this);

  return m_userHelper->GetGwUsers ();
}

Ptr<SatBeamHelper>
SatHelper::GetBeamHelper () const
{
  NS_LOG_FUNCTION (this);
  return m_beamHelper;
}

Ptr<SatUserHelper>
SatHelper::GetUserHelper () const
{
  NS_LOG_FUNCTION (this);
  return m_userHelper;
}

void
SatHelper::CreateSimpleScenario()
{
  NS_LOG_FUNCTION (this);

  SatBeamUserInfo beamInfo = SatBeamUserInfo(1,1);
  BeamUserInfoMap_t beamUserInfos;
  beamUserInfos[8] = beamInfo;

  DoCreateScenario(beamUserInfos, 1);

  m_creationSummaryTrace("*** Simple Scenario Creation Summary ***");
}

void
SatHelper::CreateLargerScenario()
{
  NS_LOG_FUNCTION (this);

  // install one user for UTs in beams 12 and 22
  SatBeamUserInfo beamInfo = SatBeamUserInfo(1,1);
  BeamUserInfoMap_t beamUserInfos;

  beamUserInfos[12] = beamInfo;
  beamUserInfos[22] = beamInfo;

  // install two users for UT1 and one for UT2 in beam 3
  beamInfo.SetUtUserCount(0,2);
  beamInfo.AppendUt(1);

  beamUserInfos[3] = beamInfo;

  DoCreateScenario(beamUserInfos, 1);

  m_creationSummaryTrace("*** Larger Scenario Creation Summary ***");
}

void
SatHelper::CreateFullScenario()
{
  NS_LOG_FUNCTION (this);

  uint32_t beamCount =  m_satConf->GetBeamCount();
  BeamUserInfoMap_t beamUserInfos;

  for ( uint32_t i = 1; i < (beamCount + 1); i ++ )
    {
      BeamUserInfoMap_t::iterator beamInfo = m_beamUserInfos.find(i);
      SatBeamUserInfo info;

      if ( beamInfo != m_beamUserInfos.end())
        {
          info = beamInfo->second;
        }
      else
        {
          info = SatBeamUserInfo(m_utsInBeam, this->m_utUsers );
        }

      beamUserInfos[i] = info;
    }

  DoCreateScenario(beamUserInfos, m_gwUsers);

  m_creationSummaryTrace("*** Full Scenario Creation Summary ***");
}
void
SatHelper::CreateUserDefinedScenario()
{
  NS_LOG_FUNCTION (this);

  // create as user wants
  DoCreateScenario(m_beamUserInfos, m_gwUsers);

  m_creationSummaryTrace("*** User Defined Scenario Creation Summary ***");
}

void
SatHelper::DoCreateScenario(BeamUserInfoMap_t beamInfos, uint32_t gwUsers)
{
  NS_LOG_FUNCTION (this);

  InternetStackHelper internet;

  // create all possible GW nodes, set mobility to them and install to interner
  NodeContainer gwNodes;
  gwNodes.Create(m_satConf->GetGwCount());
  SetGwMobility(gwNodes);
  internet.Install(gwNodes);

  for ( BeamUserInfoMap_t::iterator info = beamInfos.begin(); info != beamInfos.end(); info++)
    {
      // create UTs of the beam, set mobility to them and install to internet
      NodeContainer uts;
      uts.Create(info->second.GetUtCount());
      SetUtMobility(uts, info->first);
      internet.Install(uts);

      for ( uint32_t i = 0; i < info->second.GetUtCount(); i++ )
        {
          // create and install needed users
          m_userHelper->InstallUt(uts.Get(i), info->second.GetUtUserCount(i));
        }

      std::vector<uint32_t> conf = m_satConf->GetBeamConfiguration(info->first);

      // gw index starts from 1 and we have stored them starting from 0
      Ptr<Node> gwNode = gwNodes.Get(conf[SatConf::GW_ID_INDEX]-1);
      m_beamHelper->Install(uts, gwNode, conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);
    }

  m_userHelper->InstallGw(m_beamHelper->GetGwNodes(), gwUsers);
}

void
SatHelper::SetGwMobility(NodeContainer gwNodes)
{
  NS_LOG_FUNCTION (this);

  MobilityHelper mobility;

  Ptr<SatListPositionAllocator> gwPosAllocator = CreateObject<SatListPositionAllocator> ();

  for (uint32_t i = 0; i < gwNodes.GetN(); i++)
    {
      // GW id start from 1
      gwPosAllocator->Add(m_satConf->GetGwPosition(i + 1));
    }

  mobility.SetPositionAllocator (gwPosAllocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (gwNodes);

  InstallMobilityObserver (gwNodes);
}

void
SatHelper::SetUtMobility(NodeContainer uts, uint32_t beamId)
{
  NS_LOG_FUNCTION (this);

  MobilityHelper mobility;

  // Create new position allocator
  Ptr<SatSpotBeamPositionAllocator> allocator = CreateObject<SatSpotBeamPositionAllocator> (beamId, m_antennaGainPatterns, m_satConf->GetGeoSatPosition());

  Ptr<UniformRandomVariable> altRnd = CreateObject<UniformRandomVariable> ();
  altRnd->SetAttribute ("Min", DoubleValue (0.0));
  altRnd->SetAttribute ("Max", DoubleValue (500.0));
  allocator->SetAltitude (altRnd);

  mobility.SetPositionAllocator (allocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (uts);

  InstallMobilityObserver (uts);
}

void
SatHelper::SetGeoSatMobility(Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);

  MobilityHelper mobility;

  Ptr<SatListPositionAllocator> geoSatPosAllocator = CreateObject<SatListPositionAllocator> ();
  geoSatPosAllocator->Add(m_satConf->GetGeoSatPosition());

  mobility.SetPositionAllocator (geoSatPosAllocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (node);
}

void
SatHelper::InstallMobilityObserver (NodeContainer nodes) const
{
  NS_LOG_FUNCTION (this);

  for ( NodeContainer::Iterator i = nodes.Begin();  i != nodes.End(); i++ )
    {

      Ptr<SatMobilityObserver> observer = (*i)->GetObject<SatMobilityObserver> ();

      if (observer == 0)
        {
          Ptr<SatMobilityModel> ownMobility = (*i)->GetObject<SatMobilityModel> ();
          Ptr<SatMobilityModel> satMobility = m_beamHelper->GetGeoSatNode()->GetObject<SatMobilityModel> ();

          NS_ASSERT (ownMobility != NULL);
          NS_ASSERT (satMobility != NULL);

          observer = CreateObject<SatMobilityObserver> (ownMobility, satMobility);

          (*i)->AggregateObject (observer);
        }
    }
}

bool
SatHelper::SetBeamNetworkAddress (Ipv4Address addr)
{
  NS_LOG_FUNCTION (this << addr);

  if (m_hasGwNetworkSet && (addr == m_gwNetworkAddress))
    {
      NS_FATAL_ERROR ("Network number " << addr << " has been used in GW network");
      return false;
    }

  if (m_hasUtNetworkSet && (addr == m_utNetworkAddress))
    {
      NS_FATAL_ERROR ("Network number " << addr << " has been used in UT network");
      return false;
    }

  m_beamHelper->SetBaseAddress (addr, "255.255.255.0");
  m_beamNetworkAddress = addr;
  m_hasBeamNetworkSet = true;
  return true;
}

Ipv4Address
SatHelper::GetBeamNetworkAddress () const
{
  return m_beamNetworkAddress;
}

bool
SatHelper::SetGwNetworkAddress (Ipv4Address addr)
{
  NS_LOG_FUNCTION (this << addr);

  if (m_hasBeamNetworkSet && (addr == m_beamNetworkAddress))
    {
      NS_FATAL_ERROR ("Network number " << addr << " has been used in beam network");
      return false;
    }

  if (m_hasUtNetworkSet && (addr == m_utNetworkAddress))
    {
      NS_FATAL_ERROR ("Network number " << addr << " has been used in UT network");
      return false;
    }

  m_userHelper->SetGwBaseAddress (addr, "255.255.255.0");
  m_gwNetworkAddress = addr;
  m_hasGwNetworkSet = true;
  return true;
}

Ipv4Address
SatHelper::GetGwNetworkAddress () const
{
  return m_gwNetworkAddress;
}

bool
SatHelper::SetUtNetworkAddress (Ipv4Address addr)
{
  NS_LOG_FUNCTION (this << addr);

  if (m_hasBeamNetworkSet && (addr == m_beamNetworkAddress))
    {
      NS_FATAL_ERROR ("Network number " << addr << " has been used in beam network");
      return false;
    }

  if (m_hasGwNetworkSet && (addr == m_gwNetworkAddress))
    {
      NS_FATAL_ERROR ("Network number " << addr << " has been used in GW network");
      return false;
    }

  m_userHelper->SetUtBaseAddress (addr, "255.255.255.0");
  m_utNetworkAddress = addr;
  m_hasUtNetworkSet = true;
  return true;
}

Ipv4Address
SatHelper::GetUtNetworkAddress () const
{
  return m_utNetworkAddress;
}

void
SatHelper::CreationDetailsSink(Ptr<OutputStreamWrapper> stream, std::string context, std::string info)
{
  *stream->GetStream () << context << ", " << info << std::endl;
}

void
SatHelper::CreationSummarySink(std::string title)
{
  *m_creationTraceStream->GetStream () << CreateCreationSummary (title);
  *m_utTraceStream->GetStream() << m_beamHelper->GetUtInfo ();
}

std::string
SatHelper::CreateCreationSummary(std::string title)
{
  std::ostringstream oss;

  oss << std::endl << std::endl << title << std::endl << std::endl;
  oss << "--- User Info ---" << std::endl << std::endl;
  oss << "Created GW users: " << m_userHelper->GetGwUserCount() << ", ";
  oss << "Created UT users: " << m_userHelper->GetUtUserCount() << std::endl << std::endl;
  oss << m_userHelper->GetRouterInfo () << std::endl << std:: endl;
  oss << m_beamHelper->GetBeamInfo () << std::endl;

  return oss.str();
}

void
SatHelper::DoDispose()
{
}

} // namespace ns3


