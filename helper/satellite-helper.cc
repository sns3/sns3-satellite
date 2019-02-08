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
#include <ns3/satellite-typedefs.h>

NS_LOG_COMPONENT_DEFINE ("SatHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatHelper);

TypeId
SatHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatHelper")
    .SetParent<Object> ()
    .AddConstructor<SatHelper> ()
    .AddAttribute ("SatRtnConfFileName",
                   "Name of the satellite network RTN link configuration file.",
                   StringValue ("Scenario72RtnConf.txt"),
                   MakeStringAccessor (&SatHelper::m_rtnConfFileName),
                   MakeStringChecker ())
    .AddAttribute ("SatFwdConfFileName",
                   "Name of the satellite network FWD link configuration file.",
                   StringValue ("Scenario72FwdConf.txt"),
                   MakeStringAccessor (&SatHelper::m_fwdConfFileName),
                   MakeStringChecker ())
    .AddAttribute ("GwPosFileName",
                   "Name of the GW positions configuration file.",
                   StringValue ("Scenario72GwPos.txt"),
                   MakeStringAccessor (&SatHelper::m_gwPosFileName),
                   MakeStringChecker ())
    .AddAttribute ("GeoSatPosFileName",
                   "Name of the geostationary satellite position configuration file.",
                   StringValue ("Scenario72GeoPos.txt"),
                   MakeStringAccessor (&SatHelper::m_geoPosFileName),
                   MakeStringChecker ())
    .AddAttribute ("RtnLinkWaveformConfFileName",
                   "Name of the RTN link waveform configuration file.",
                   StringValue ("dvbRcs2Waveforms.txt"),
                   MakeStringAccessor (&SatHelper::m_waveformConfFileName),
                   MakeStringChecker ())
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
                   "(mask set by attribute 'BeamNetworkMask' will be used as the network mask)",
                   Ipv4AddressValue ("40.1.0.0"),
                   MakeIpv4AddressAccessor (&SatHelper::m_beamNetworkAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("BeamNetworkMask",
                   "Network mask to use during allocation of satellite devices.",
                   Ipv4MaskValue ("255.255.0.0"),
                   MakeIpv4MaskAccessor (&SatHelper::m_beamNetworkMask),
                   MakeIpv4MaskChecker ())
    .AddAttribute ("GwNetworkAddress",
                   "Initial network number to use "
                   "during allocation of GW, router, and GW users "
                   "(mask set by attribute 'GwNetworkMask' will be used as the network mask)",
                   Ipv4AddressValue ("90.1.0.0"),
                   MakeIpv4AddressAccessor (&SatHelper::m_gwNetworkAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("GwNetworkMask",
                   "Network mask to use during allocation of GW, router, and GW users.",
                   Ipv4MaskValue ("255.255.0.0"),
                   MakeIpv4MaskAccessor (&SatHelper::m_gwNetworkMask),
                   MakeIpv4MaskChecker ())
    .AddAttribute ("UtNetworkAddress",
                   "Initial network number to use "
                   "during allocation of UT and UT users "
                   "(mask set by attribute 'UtNetworkMask' will be used as the network mask)",
                   Ipv4AddressValue ("10.1.0.0"),
                   MakeIpv4AddressAccessor (&SatHelper::m_utNetworkAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("UtNetworkMask",
                   "Network mask to use during allocation of UT and UT users.",
                   Ipv4MaskValue ("255.255.0.0"),
                   MakeIpv4MaskAccessor (&SatHelper::m_utNetworkMask),
                   MakeIpv4MaskChecker ())
    .AddAttribute ("PacketTraceEnabled",
                   "Packet tracing enable status.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatHelper::m_packetTraces),
                   MakeBooleanChecker ())
    .AddAttribute ("ScenarioCreationTraceEnabled",
                   "Scenario creation trace output enable status.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatHelper::m_creationTraces),
                   MakeBooleanChecker ())
    .AddAttribute ("DetailedScenarioCreationTraceEnabled",
                   "Detailed scenario creation trace output enable status.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatHelper::m_detailedCreationTraces),
                   MakeBooleanChecker ())
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
                     MakeTraceSourceAccessor (&SatHelper::m_creationDetailsTrace),
                     "ns3::SatTypedefs::CreationCallback")
    .AddTraceSource ("CreationSummary", "Creation summary traces",
                     MakeTraceSourceAccessor (&SatHelper::m_creationSummaryTrace),
                     "ns3::SatTypedefs::CreationCallback")

  ;
  return tid;
}

TypeId
SatHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatHelper::SatHelper ()
  : m_rtnConfFileName ("Scenario72RtnConf.txt"),
    m_fwdConfFileName ("Scenario72FwdConf.txt"),
    m_gwPosFileName ("Scenario72GwPos.txt"),
    m_geoPosFileName ("Scenario72GeoPos.txt"),
    m_waveformConfFileName ("dvbRcs2Waveforms.txt"),
    m_scenarioCreated (false),
    m_creationTraces (false),
    m_detailedCreationTraces (false),
    m_packetTraces (false),
    m_utsInBeam (0),
    m_gwUsers (0),
    m_utUsers (0),
		m_utPositionsByBeam ()
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf(AttributeConstructionList ());

  Singleton<SatEnvVariables>::Get ()->Initialize ();

  m_satConf = CreateObject<SatConf> ();

  m_satConf->Initialize (m_rtnConfFileName,
                         m_fwdConfFileName,
                         m_gwPosFileName,
                         m_geoPosFileName,
                         m_waveformConfFileName);

  // Create antenna gain patterns
  m_antennaGainPatterns = CreateObject<SatAntennaGainPatternContainer> ();

  // create Geo Satellite node, set mobility to it
  Ptr<Node> geoSatNode = CreateObject<Node> ();
  SetGeoSatMobility (geoSatNode);

  m_beamHelper = CreateObject<SatBeamHelper> (geoSatNode,
                                              MakeCallback (&SatConf::GetCarrierBandwidthHz, m_satConf),
                                              m_satConf->GetRtnLinkCarrierCount (),
                                              m_satConf->GetFwdLinkCarrierCount (),
                                              m_satConf->GetSuperframeSeq ());

  Ptr<SatRtnLinkTime> rtnTime = Singleton<SatRtnLinkTime>::Get ();
  rtnTime->Initialize (m_satConf->GetSuperframeSeq ());

  SatBeamHelper::CarrierFreqConverter converterCb = MakeCallback (&SatConf::GetCarrierFrequencyHz, m_satConf);
  m_beamHelper->SetAttribute ("CarrierFrequencyConverter", CallbackValue (converterCb) );

  m_userHelper = CreateObject<SatUserHelper> ();

  // Set the antenna patterns to beam helper
  m_beamHelper->SetAntennaGainPatterns (m_antennaGainPatterns);
}

void SatHelper::CreatePredefinedScenario (PreDefinedScenario_t scenario)
{
  NS_LOG_FUNCTION (this);

  switch (scenario)
    {
    case SIMPLE:
      CreateSimpleScenario ();
      break;

    case LARGER:
      CreateLargerScenario ();
      break;

    case FULL:
      CreateFullScenario ();
      break;

    default:
      NS_FATAL_ERROR ("Not supported predefined scenario.");
      break;
    }
}

void SatHelper::EnableCreationTraces ()
{
  NS_LOG_FUNCTION (this);

  AsciiTraceHelper asciiTraceHelper;

  std::stringstream outputPathCreation;
  std::stringstream outputPathUt;
  outputPathCreation << Singleton<SatEnvVariables>::Get ()->GetOutputPath () << "/" << m_scenarioCreationFileName << ".log";
  outputPathUt << Singleton<SatEnvVariables>::Get ()->GetOutputPath () << "/" << m_utCreationFileName << ".log";

  m_creationTraceStream = asciiTraceHelper.CreateFileStream (outputPathCreation.str ());
  m_utTraceStream = asciiTraceHelper.CreateFileStream (outputPathUt.str ());

  TraceConnectWithoutContext ("CreationSummary", MakeCallback (&SatHelper::CreationSummarySink, this));

  if ( m_detailedCreationTraces )
    {
      EnableDetailedCreationTraces ();
    }
}

void
SatHelper::EnablePacketTrace ()
{
  m_beamHelper->EnablePacketTrace ();
}


void SatHelper::EnableDetailedCreationTraces ()
{
  NS_LOG_FUNCTION (this);

  CallbackBase creationCb = MakeBoundCallback (&SatHelper::CreationDetailsSink, m_creationTraceStream);
  TraceConnect ("Creation", "SatHelper", creationCb);

  m_userHelper->EnableCreationTraces (m_creationTraceStream, creationCb);
  m_beamHelper->EnableCreationTraces (m_creationTraceStream, creationCb);
}

Ipv4Address
SatHelper::GetUserAddress (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
  return ipv4->GetAddress (1, 0).GetLocal (); // Get Ipv4InterfaceAddress of interface csma interface.
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

  SatBeamUserInfo beamInfo = SatBeamUserInfo (1,1);
  BeamUserInfoMap_t beamUserInfos;
  beamUserInfos[8] = beamInfo;

  DoCreateScenario (beamUserInfos, 1);

  m_creationSummaryTrace ("*** Simple Scenario Creation Summary ***");
}

void
SatHelper::CreateLargerScenario ()
{
  NS_LOG_FUNCTION (this);

  // install one user for UTs in beams 12 and 22
  SatBeamUserInfo beamInfo = SatBeamUserInfo (1,1);
  BeamUserInfoMap_t beamUserInfos;

  beamUserInfos[12] = beamInfo;
  beamUserInfos[22] = beamInfo;

  // install two users for UT1 and one for UT2 in beam 3
  beamInfo.SetUtUserCount (0,2);
  beamInfo.AppendUt (1);

  beamUserInfos[3] = beamInfo;

  DoCreateScenario (beamUserInfos, 1);

  m_creationSummaryTrace ("*** Larger Scenario Creation Summary ***");
}

void
SatHelper::CreateFullScenario ()
{
  NS_LOG_FUNCTION (this);

  uint32_t beamCount =  m_satConf->GetBeamCount ();
  BeamUserInfoMap_t beamUserInfos;

  for ( uint32_t i = 1; i < (beamCount + 1); i++ )
    {
      BeamUserInfoMap_t::iterator beamInfo = m_beamUserInfos.find (i);
      SatBeamUserInfo info;

      if ( beamInfo != m_beamUserInfos.end ())
        {
          info = beamInfo->second;
        }
      else
        {
          info = SatBeamUserInfo (m_utsInBeam, this->m_utUsers );
        }

      beamUserInfos[i] = info;
    }

  DoCreateScenario (beamUserInfos, m_gwUsers);

  m_creationSummaryTrace ("*** Full Scenario Creation Summary ***");
}

void
SatHelper::CreateUserDefinedScenario (BeamUserInfoMap_t& infos)
{
  NS_LOG_FUNCTION (this);

  // create as user wants
  DoCreateScenario (infos, m_gwUsers);

  m_creationSummaryTrace ("*** User Defined Scenario Creation Summary ***");
}

void
SatHelper::SetCustomUtPositionAllocator (Ptr<SatListPositionAllocator> posAllocator)
{
	NS_LOG_FUNCTION (this);
	m_utPositions = posAllocator;
}

void
SatHelper::SetUtPositionAllocatorForBeam (uint32_t beamId, Ptr<SatListPositionAllocator> posAllocator)
{
	NS_LOG_FUNCTION (this << beamId);
	m_utPositionsByBeam[beamId] = posAllocator;
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
  DoCreateScenario (infos, m_gwUsers);

  m_creationSummaryTrace ("*** User Defined Scenario with List Positions Creation Summary ***");
}

void
SatHelper::DoCreateScenario (BeamUserInfoMap_t& beamInfos, uint32_t gwUsers)
{
  NS_LOG_FUNCTION (this);

  if (m_scenarioCreated)
    {
      Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", "Scenario tried to re-create with SatHelper. Creation can be done only once.");
    }
  else
    {
      SetNetworkAddresses (beamInfos, gwUsers);

      if (m_creationTraces)
        {
          EnableCreationTraces ();
        }

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
              m_userHelper->InstallUt (uts.Get (i), info->second.GetUtUserCount (i));
            }

          std::vector<uint32_t> rtnConf = m_satConf->GetBeamConfiguration (info->first, SatEnums::LD_RETURN);
          std::vector<uint32_t> fwdConf = m_satConf->GetBeamConfiguration (info->first, SatEnums::LD_FORWARD);

          /**
           * GW and beam ids are assumed to be the same for both directions
           * currently!
           */
          NS_ASSERT (rtnConf[SatConf::GW_ID_INDEX] == fwdConf[SatConf::GW_ID_INDEX]);
          NS_ASSERT (rtnConf[SatConf::BEAM_ID_INDEX] == fwdConf[SatConf::BEAM_ID_INDEX]);

          // gw index starts from 1 and we have stored them starting from 0
          Ptr<Node> gwNode = gwNodes.Get (rtnConf[SatConf::GW_ID_INDEX] - 1);
          m_beamHelper->Install (uts,
                                 gwNode,
                                 rtnConf[SatConf::GW_ID_INDEX],
                                 rtnConf[SatConf::BEAM_ID_INDEX],
                                 rtnConf[SatConf::U_FREQ_ID_INDEX],
                                 rtnConf[SatConf::F_FREQ_ID_INDEX],
                                 fwdConf[SatConf::U_FREQ_ID_INDEX],
                                 fwdConf[SatConf::F_FREQ_ID_INDEX]);
        }

      m_userHelper->InstallGw (m_beamHelper->GetGwNodes (), gwUsers);

      if (m_packetTraces)
        {
          EnablePacketTrace ();
        }

      m_scenarioCreated = true;
    }

  m_beamHelper->Init ();
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
  if (m_utPositionsByBeam.find (beamId) != m_utPositionsByBeam.end ())
    {
    	allocator = m_utPositionsByBeam[beamId];
    }
  else if ( m_utPositions != NULL )
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

  m_utPositionsByBeam.clear ();
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
          if (staticRouting->GetRoute (i).GetDestNetwork ().IsEqual (defMulticastNetwork)
              && staticRouting->GetRoute (i).GetDestNetworkMask ().IsEqual (defMulticastNetworkMask) )
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

                  if ( FindMatchingDevices (receiverNode, m_userHelper->GetRouter (), devices ) )
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

void
SatHelper::SetNetworkAddresses (BeamUserInfoMap_t& beamInfos, uint32_t gwUsers) const
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t> networkAddresses;
  std::pair<std::set<uint32_t>::const_iterator, bool> addressInsertionResult;

  // test first that configured initial addresses per configured address space
  // are not same by inserting them to set container
  networkAddresses.insert (m_beamNetworkAddress.Get ());
  addressInsertionResult = networkAddresses.insert (m_gwNetworkAddress.Get ());

  if (!addressInsertionResult.second)
    {
      NS_FATAL_ERROR ("GW network address is invalid (same as Beam network address)");
    }

  addressInsertionResult = networkAddresses.insert (m_utNetworkAddress.Get ());

  if (!addressInsertionResult.second)
    {
      NS_FATAL_ERROR ("UT network address is invalid (same as Beam or GW network address)");
    }

  // calculate values to check needed network and host address counts
  uint32_t utNetworkAddressCount = 0;   // network addresses needed in UT network
  uint32_t utHostAddressCount = 0;      // host addresses needed in UT network
  uint32_t beamHostAddressCount = 0;    // host addresses needed in Beam network
  uint32_t gwNetworkAddressCount = 1;   // network addresses needed in GW network. Initially one network needed between GW users and Router needed
  std::set<uint32_t> gwIds;             // to find out the additional network addresses needed in GW network

  for (BeamUserInfoMap_t::const_iterator it = beamInfos.begin (); it != beamInfos.end (); it++)
    {
      uint32_t beamUtCount = it->second.GetUtCount ();
      utNetworkAddressCount += beamUtCount;

      if (beamUtCount > beamHostAddressCount)
        {
          beamHostAddressCount = beamUtCount;
        }

      for (uint32_t i = 0; i < beamUtCount; i++)
        {
          if (it->second.GetUtUserCount (i) > utHostAddressCount)
            {
              utHostAddressCount = it->second.GetUtUserCount (i);
            }
        }

      // try to add GW id to container, if not existing already in the container
      // increment GW network address count
      if (gwIds.insert (m_beamHelper->GetGwId (it->first)).second)
        {
          gwNetworkAddressCount++; // one network more needed between a GW and Router
        }
    }

  // do final checking of the configured address spaces
  CheckNetwork ("Beam", m_beamNetworkAddress, m_beamNetworkMask, networkAddresses, beamInfos.size (), beamHostAddressCount);
  CheckNetwork ("GW", m_gwNetworkAddress, m_gwNetworkMask, networkAddresses, gwNetworkAddressCount, gwUsers);
  CheckNetwork ("UT", m_utNetworkAddress, m_utNetworkMask, networkAddresses, utNetworkAddressCount, utHostAddressCount);

  // set base addresses of the sub-helpers
  m_beamHelper->SetBaseAddress (m_beamNetworkAddress, m_beamNetworkMask);
  m_userHelper->SetGwBaseAddress (m_gwNetworkAddress, m_gwNetworkMask);
  m_userHelper->SetUtBaseAddress (m_utNetworkAddress, m_utNetworkMask);
}

void
SatHelper::CheckNetwork (std::string networkName,
                         const Ipv4Address& firstNetwork,
                         const Ipv4Mask& mask,
                         const std::set<uint32_t>& networkAddresses,
                         uint32_t networkCount,
                         uint32_t hostCount) const
{
  NS_LOG_FUNCTION (this);

  uint16_t addressPrefixLength = mask.GetPrefixLength ();

  // test that configured mask is valid (address prefix length is in valid range)
  if ((addressPrefixLength < MIN_ADDRESS_PREFIX_LENGTH) || (addressPrefixLength > MAX_ADDRESS_PREFIX_LENGTH))
      {
        NS_FATAL_ERROR (networkName << " network mask value out of range (0xFFFFFF70 to 0x10000000).");
      }

  // test that configured initial network number (prefix) is valid, consistent with mask
  if ((firstNetwork.Get () & mask.GetInverse ()) != 0)
    {
      NS_FATAL_ERROR (networkName << " network address and mask inconsistent.");
    }

  std::set<uint32_t>::const_iterator currentAddressIt = networkAddresses.find (firstNetwork.Get ());

  // test that network we are checking is in given container
  if (currentAddressIt != networkAddresses.end ())
    {
      // calculate network and host count based on configured initial network address and
      // mask for the network space
      uint32_t hostAddressCount = std::pow (2, (32 - addressPrefixLength)) - 2;
      uint32_t firstAddressValue = firstNetwork.Get ();
      uint32_t networkAddressCount = mask.Get () - firstAddressValue + 1; // increase subtraction result by one, to include also first address

      currentAddressIt++;

      // test in the case that checked address space is not last ('highest') in the
      // given address container that the address space doesn't overlap with other configured address spaces
      if ( (currentAddressIt != networkAddresses.end ()) &&
           (firstAddressValue + (networkCount  << (32 - addressPrefixLength))) >= *currentAddressIt)
        {
          NS_FATAL_ERROR (networkName << " network's addresses overlaps with some other network)");
        }

      // test that enough network addresses are available in address space
      if (networkCount > networkAddressCount)
        {
          NS_FATAL_ERROR ("Not enough network addresses for '" << networkName << "' network");
        }

      // test that enough host addresses are available in address space
      if (hostCount > hostAddressCount)
        {
          NS_FATAL_ERROR ("Not enough host addresses for '" << networkName << "' network");
        }
    }
  else
    {
      NS_FATAL_ERROR (networkName << "network's initial address number not among of the given addresses");
    }
}


} // namespace ns3
