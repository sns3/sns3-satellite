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
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "../model/satellite-position-allocator.h"
#include "../model/satellite-rtn-link-time.h"
#include "satellite-helper.h"
#include "../model/satellite-log.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

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
      .AddAttribute ("ScenarioCreationTraceFileName",
                     "File name for the scenario creation trace output",
                     StringValue ("CreationTraceScenario"),
                     MakeStringAccessor (&SatHelper::m_scenarioCreationFileName),
                     MakeStringChecker ())
      .AddAttribute ("UtCreationTraceFileName",
                     "File name for the UT creation trace output",
                     StringValue ("CreationTraceUt"),
                     MakeStringAccessor (&SatHelper::m_utCreationFileName),
                     MakeStringChecker ())
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

  std::string satConf = scenarioName + "Conf.txt";
  std::string gwPos = scenarioName + "GwPos.txt";
  std::string satPos = scenarioName + "GeoPos.txt";
  std::string wfConf = "dvbRcs2Waveforms.txt";

  Singleton<SatEnvVariables>::Get ()->Initialize ();

  m_satConf = CreateObject<SatConf> ();

  m_satConf->Initialize (satConf, gwPos, satPos, wfConf);

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

void SatHelper::CreatePredefinedScenario (PreDefinedScenario_t scenario)
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

    default:
      NS_FATAL_ERROR ("Not supported predefined scenario.");
      break;
  }
}

void SatHelper::EnableCreationTraces (bool details)
{
  NS_LOG_FUNCTION (this);

  AsciiTraceHelper asciiTraceHelper;

  std::stringstream outputPathCreation;
  std::stringstream outputPathUt;
  outputPathCreation << Singleton<SatEnvVariables>::Get ()->GetOutputPath () << "/" << m_scenarioCreationFileName << ".log";
  outputPathUt << Singleton<SatEnvVariables>::Get ()->GetOutputPath () << "/" << m_utCreationFileName << ".log";

  m_creationTraceStream = asciiTraceHelper.CreateFileStream (outputPathCreation.str ());
  m_utTraceStream = asciiTraceHelper.CreateFileStream (outputPathUt.str ());

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
SatHelper::CreateSimpleScenario ()
{
  NS_LOG_FUNCTION (this);

  SatBeamUserInfo beamInfo = SatBeamUserInfo(1,1);
  BeamUserInfoMap_t beamUserInfos;
  beamUserInfos[8] = beamInfo;

  DoCreateScenario(beamUserInfos, 1);

  m_creationSummaryTrace("*** Simple Scenario Creation Summary ***");
}

void
SatHelper::CreateLargerScenario ()
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
SatHelper::CreateFullScenario ()
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
SatHelper::CreateUserDefinedScenario (BeamUserInfoMap_t& infos)
{
  NS_LOG_FUNCTION (this);

  // create as user wants
  DoCreateScenario(infos, m_gwUsers);

  m_creationSummaryTrace("*** User Defined Scenario Creation Summary ***");
}

void
SatHelper::CreateUserDefinedScenarioFromListPositions (BeamUserInfoMap_t& infos, bool checkBeam)
{
  NS_LOG_FUNCTION (this);

  uint32_t positionIndex = 1;

  // construct list position allocator and fill it with position
  // configured through SatConf

  m_utPositions = CreateObject<SatListPositionAllocator> ();

  for ( BeamUserInfoMap_t::iterator it = infos.begin (); it !=  infos.end (); it++ )
    {
      for (uint32_t i = 0; i < it->second.GetUtCount (); i++ )
        {
          if (positionIndex > m_satConf->GetUtCount ())
            {
              NS_FATAL_ERROR ("Not enough positions available in SatConf for UTs!!!");
            }

          GeoCoordinate position = m_satConf->GetUtPosition (positionIndex);
          m_utPositions->Add (position);
          positionIndex++;

          // if requested, check that the given beam is the best in the configured position
          if (checkBeam)
            {
              uint32_t bestBeamId = m_antennaGainPatterns->GetBestBeamId (position);

              if ( bestBeamId != it->first )
                {
                  NS_FATAL_ERROR ("The beam: " << it->first << " is not the best beam (" << bestBeamId << ") for the position: " << position);
                }
            }
        }
    }

  // create as user wants
  DoCreateScenario(infos, m_gwUsers);

  m_creationSummaryTrace("*** User Defined Scenario Creation Summary ***");
}

void
SatHelper::DoCreateScenario (BeamUserInfoMap_t beamInfos, uint32_t gwUsers)
{
  NS_LOG_FUNCTION (this);

  InternetStackHelper internet;

  // create all possible GW nodes, set mobility to them and install to Internet
  NodeContainer gwNodes;
  gwNodes.Create (m_satConf->GetGwCount ());
  SetGwMobility (gwNodes);
  internet.Install (gwNodes);

  for ( BeamUserInfoMap_t::iterator info = beamInfos.begin (); info != beamInfos.end (); info++)
    {
      // create UTs of the beam, set mobility to them and install to Internet
      NodeContainer uts;
      uts.Create (info->second.GetUtCount ());
      SetUtMobility (uts, info->first);
      internet.Install (uts);

      for ( uint32_t i = 0; i < info->second.GetUtCount (); i++ )
        {
          // create and install needed users
          m_userHelper->InstallUt (uts.Get(i), info->second.GetUtUserCount (i));
        }

      std::vector<uint32_t> conf = m_satConf->GetBeamConfiguration (info->first);

      // gw index starts from 1 and we have stored them starting from 0
      Ptr<Node> gwNode = gwNodes.Get (conf[SatConf::GW_ID_INDEX]-1);
      m_beamHelper->Install (uts, gwNode, conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);
    }

  m_userHelper->InstallGw (m_beamHelper->GetGwNodes (), gwUsers);
}

void
SatHelper::SetGwMobility (NodeContainer gwNodes)
{
  NS_LOG_FUNCTION (this);

  MobilityHelper mobility;

  Ptr<SatListPositionAllocator> gwPosAllocator = CreateObject<SatListPositionAllocator> ();

  for (uint32_t i = 0; i < gwNodes.GetN (); i++)
    {
      // GW id start from 1
      gwPosAllocator->Add (m_satConf->GetGwPosition (i + 1));
    }

  mobility.SetPositionAllocator (gwPosAllocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (gwNodes);

  InstallMobilityObserver (gwNodes);
}

void
SatHelper::SetUtMobility (NodeContainer uts, uint32_t beamId)
{
  NS_LOG_FUNCTION (this);

  MobilityHelper mobility;

  Ptr<SatPositionAllocator> allocator;

  // if position allocator (list) for UTs is created by helper already use it,
  // in other case use the spot beam position allocator
  if ( m_utPositions != NULL )
    {
      allocator = m_utPositions;
    }
  else
    {
      // Create new position allocator
      Ptr<SatSpotBeamPositionAllocator> beamAllocator = CreateObject<SatSpotBeamPositionAllocator> (beamId, m_antennaGainPatterns, m_satConf->GetGeoSatPosition ());

      Ptr<UniformRandomVariable> altRnd = CreateObject<UniformRandomVariable> ();
      altRnd->SetAttribute ("Min", DoubleValue (0.0));
      altRnd->SetAttribute ("Max", DoubleValue (500.0));
      beamAllocator->SetAltitude (altRnd);

      allocator = beamAllocator;
    }

  mobility.SetPositionAllocator (allocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (uts);

  InstallMobilityObserver (uts);
}

void
SatHelper::SetGeoSatMobility (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);

  MobilityHelper mobility;

  Ptr<SatListPositionAllocator> geoSatPosAllocator = CreateObject<SatListPositionAllocator> ();
  geoSatPosAllocator->Add (m_satConf->GetGeoSatPosition ());

  mobility.SetPositionAllocator (geoSatPosAllocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (node);
}

void
SatHelper::InstallMobilityObserver (NodeContainer nodes) const
{
  NS_LOG_FUNCTION (this);

  for ( NodeContainer::Iterator i = nodes.Begin ();  i != nodes.End (); i++ )
    {
      Ptr<SatMobilityObserver> observer = (*i)->GetObject<SatMobilityObserver> ();

      if (observer == 0)
        {
          Ptr<SatMobilityModel> ownMobility = (*i)->GetObject<SatMobilityModel> ();
          Ptr<SatMobilityModel> satMobility = m_beamHelper->GetGeoSatNode ()->GetObject<SatMobilityModel> ();

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
  NS_LOG_FUNCTION (this);

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
  NS_LOG_FUNCTION (this);

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
  NS_LOG_FUNCTION (this);

  return m_utNetworkAddress;
}

void
SatHelper::SetMulticastGroupRoutes (Ptr<Node> source, NodeContainer receivers, Ipv4Address sourceAddress, Ipv4Address groupAddress)
{
  NS_LOG_FUNCTION (this);

  MulticastBeamInfo_t beamInfo;
  Ptr<NetDevice> routerUserOutputDev;
  Ptr<Node> sourceUtNode = m_userHelper->GetUtNode (source);

  // Construct multicast info from source UT node and receivers. In case that sourceUtNode is
  // NULL, source is some GW user. As a result is given flag indicating if traffic shall be forwarded to
  // source's own network

  if ( ConstructMulticastInfo (sourceUtNode, receivers, beamInfo, routerUserOutputDev) )
    {
      // Some multicast receiver belongs to same group with source

      // select destination node
      Ptr<Node> destNode = m_userHelper->GetRouter ();

      if ( sourceUtNode )
        {
          destNode = sourceUtNode;
        }

      // add default route for multicast group to source's network
      SetMulticastRouteToSourceNetwork (source, destNode);
    }

  // set routes outside source's network only when there are receivers
  if ( !beamInfo.empty () || (sourceUtNode && routerUserOutputDev ) )
    {
      Ptr<Node> routerNode = m_userHelper->GetRouter ();

      Ptr<NetDevice> routerInputDev = NULL;
      Ptr<NetDevice> gwOutputDev = NULL;

      // set multicast routes to satellite network utilizing beam helper
      NetDeviceContainer gwInputDevices = m_beamHelper->AddMulticastGroupRoutes (beamInfo, sourceUtNode, sourceAddress,
                                                                                 groupAddress, (bool) routerUserOutputDev, gwOutputDev );

      Ipv4StaticRoutingHelper multicast;

      // select input device in IP router
      if ( gwOutputDev )
        {
          // traffic coming from some GW to router (satellite network and source is UT)
          // find matching input device using GW output device
          routerInputDev = FindMatchingDevice (gwOutputDev, routerNode);
        }
      else if ( !sourceUtNode )
        {
          // traffic is coming form user network (some GW user)

          // find matching device using source node
          std::pair<Ptr<NetDevice>,Ptr<NetDevice> > devices;

          if ( FindMatchingDevices (source, routerNode, devices) )
            {
              routerInputDev = devices.second;
            }
         }

      NetDeviceContainer routerOutputDevices;

      if (routerUserOutputDev)
        {
          routerOutputDevices.Add (routerUserOutputDev);
        }

      for (NetDeviceContainer::Iterator it = gwInputDevices.Begin (); it != gwInputDevices.End (); it++ )
        {
          Ptr<NetDevice> matchingDevice = FindMatchingDevice ( *it, routerNode );

          if ( matchingDevice )
            {
              routerOutputDevices.Add (matchingDevice);
            }
        }

      // Add multicast route over IP router
      // Input device is getting traffic from user network (GW users) or from some GW
      // Output devices are forwarding traffic to user network (GW users) and/or GWs
      if ( routerInputDev && ( routerOutputDevices.GetN () > 0 ) )
        {
          multicast.AddMulticastRoute (routerNode, sourceAddress, groupAddress, routerInputDev, routerOutputDevices);
        }
    }
}

void
SatHelper::CreationDetailsSink (Ptr<OutputStreamWrapper> stream, std::string context, std::string info)
{
  *stream->GetStream () << context << ", " << info << std::endl;
}

void
SatHelper::CreationSummarySink (std::string title)
{
  NS_LOG_FUNCTION (this);

  *m_creationTraceStream->GetStream () << CreateCreationSummary (title);
  *m_utTraceStream->GetStream () << m_beamHelper->GetUtInfo ();
}

std::string
SatHelper::CreateCreationSummary (std::string title)
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;

  oss << std::endl << std::endl << title << std::endl << std::endl;
  oss << "--- User Info ---" << std::endl << std::endl;
  oss << "Created GW users: " << m_userHelper->GetGwUserCount () << ", ";
  oss << "Created UT users: " << m_userHelper->GetUtUserCount () << std::endl << std::endl;
  oss << m_userHelper->GetRouterInfo () << std::endl << std:: endl;
  oss << m_beamHelper->GetBeamInfo () << std::endl;

  return oss.str ();
}

void
SatHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

bool
SatHelper::FindMatchingDevices ( Ptr<Node> nodeA, Ptr<Node> nodeB, std::pair<Ptr<NetDevice>, Ptr<NetDevice> >& matchingDevices)
{
  bool found = false;

  for ( uint32_t i = 1; ( (i < nodeA->GetNDevices ()) && !found ); i++)
    {
      Ptr<NetDevice> devA = nodeA->GetDevice (i);
      Ptr<NetDevice> devB = FindMatchingDevice (devA, nodeB);

      if ( devB )
        {
          matchingDevices = std::make_pair (devA, devB);
          found = true;
        }
    }

  return found;
}

Ptr<NetDevice>
SatHelper::FindMatchingDevice ( Ptr<NetDevice> devA, Ptr<Node> nodeB )
{
  Ptr<NetDevice> matchingDevice = NULL;

  Ipv4Address addressA = devA->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (devA->GetIfIndex (), 0).GetLocal ();
  Ipv4Mask maskA = devA->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (devA->GetIfIndex (), 0).GetMask ();

  Ipv4Address netAddressA = addressA.CombineMask (maskA);

  for ( uint32_t j = 1; j < nodeB->GetNDevices (); j++)
    {
      Ipv4Address addressB = nodeB->GetObject<Ipv4L3Protocol> ()->GetAddress (j, 0).GetLocal ();
      Ipv4Mask maskB = nodeB->GetObject<Ipv4L3Protocol> ()->GetAddress (j, 0).GetMask ();

      Ipv4Address netAddressB = addressB.CombineMask (maskB);

      if ( netAddressA.IsEqual (netAddressB))
        {
          matchingDevice = nodeB->GetDevice (j);
        }
    }

  return matchingDevice;
}

void
SatHelper::SetMulticastRouteToSourceNetwork (Ptr<Node> source, Ptr<Node> dest)
{
  NS_LOG_FUNCTION (this);

  std::pair<Ptr<NetDevice>, Ptr<NetDevice> > devices;

  if ( FindMatchingDevices (source, dest, devices) )
    {
      Ipv4StaticRoutingHelper multicast;
      Ptr<Ipv4StaticRouting> staticRouting = multicast.GetStaticRouting (source->GetObject<ns3::Ipv4> ());

      // check if default multicast route already exists
      bool defaultMulticastRouteExists = false;
      Ipv4Address defMulticastNetwork = Ipv4Address ("224.0.0.0");
      Ipv4Mask defMulticastNetworkMask = Ipv4Mask ("240.0.0.0");

      for ( uint32_t i = 0; i < staticRouting->GetNRoutes (); i++ )
        {
          if (staticRouting->GetRoute (i).GetDestNetwork ().IsEqual (defMulticastNetwork) &&
              staticRouting->GetRoute (i).GetDestNetworkMask ().IsEqual (defMulticastNetworkMask) )
            {
              defaultMulticastRouteExists = true;
            }
        }

      // add default multicast route only if it does not exist already
      if ( !defaultMulticastRouteExists )
        {
          multicast.SetDefaultMulticastRoute (source, devices.first);
        }
    }
}

bool
SatHelper::ConstructMulticastInfo (Ptr<Node> sourceUtNode, NodeContainer receivers, MulticastBeamInfo_t& beamInfo, Ptr<NetDevice>& routerUserOutputDev )
{
  NS_LOG_FUNCTION (this);

  bool routeToSourceNertwork = false;

  routerUserOutputDev = NULL;

  // go through all receivers
  for (uint32_t i = 0; i < receivers.GetN (); i++ )
    {
      Ptr<Node> receiverNode = receivers.Get (i);
      Ptr<Node> utNode = m_userHelper->GetUtNode (receiverNode);

      // check if user is connected to UT or GW

      if ( utNode ) // connected to UT
        {
          uint32_t beamId = m_beamHelper->GetUtBeamId (utNode);

          if ( beamId != 0 )  // beam ID is found
            {
              if (sourceUtNode == utNode)
                {
                  // Source UT node is same than current UT node. Set flag to indicate that
                  // multicast group traffic shall be routed to source own network.
                  routeToSourceNertwork = true;
                }
              else
                {
                  // store other UT nodes beam ID and pointer to multicast group info for later routing
                  MulticastBeamInfo_t::iterator it = beamInfo.find (beamId);

                    // find or create first storage for the beam
                  if ( it == beamInfo.end () )
                    {
                      std::pair<MulticastBeamInfo_t::iterator, bool> result = beamInfo.insert (std::make_pair (beamId, MulticastBeamInfoItem_t ()));

                      if ( result.second )
                        {
                          it = result.first;
                        }
                      else
                        {
                          NS_FATAL_ERROR ("Cannot insert beam to map container");
                        }
                    }

                  // Add to UT node to beam storage (map)
                  it->second.insert (utNode);
                }
            }
          else
            {
              NS_FATAL_ERROR ("UT node's beam ID is invalid!!");
            }
        }
      else if (m_userHelper->IsGwUser (receiverNode)) // connected to GW
        {
          if ( !routerUserOutputDev )
            {
              if (sourceUtNode)
                {
                  std::pair<Ptr<NetDevice>, Ptr<NetDevice> > devices;

                  if ( FindMatchingDevices (receiverNode, m_userHelper->GetRouter(), devices ) )
                    {
                      routerUserOutputDev = devices.second;
                    }
                }
              else
                {
                  routeToSourceNertwork = true;
                }
            }
        }
      else
        {
          NS_FATAL_ERROR ("Multicast receiver node is expected to be connected UT or GW node!!!");
        }
    }

  return routeToSourceNertwork;
}

} // namespace ns3
