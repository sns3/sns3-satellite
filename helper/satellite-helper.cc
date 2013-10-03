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
#include "../model/satellite-position-allocator.h"
#include "satellite-helper.h"

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
      .AddTraceSource ("Creation", "Creation traces",
                        MakeTraceSourceAccessor (&SatHelper::m_creation))
      .AddTraceSource ("CreationSummary", "Creation summary traces",
                        MakeTraceSourceAccessor (&SatHelper::m_creationSummary))

    ;
    return tid;
}

TypeId
SatHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatHelper::SatHelper ()
{
  // Do nothing here
  NS_ASSERT (true);
}

SatHelper::SatHelper (std::string scenarioName)
 : m_scenarioCreated(false),
   m_detailedCreationTraces(false)
{
  // uncomment next line, if attributes are needed already in construction phase
  //ObjectBase::ConstructSelf(AttributeConstructionList ());

  std::string path = "src/satellite/data/";

  std::string satConf = scenarioName + "Conf.txt";
  std::string gwPos = scenarioName + "GwPos.txt";
  std::string satPos = scenarioName + "GeoPos.txt";

  m_satConf = CreateObject<SatConf> ();

  m_satConf->Initialize (path, satConf, gwPos, satPos);

  // Create antenna gain patterns
  m_antennaGainPatterns = CreateObject<SatAntennaGainPatternContainer> ();
}

void
SatHelper::SetBeamUserInfo(std::map<uint32_t, SatBeamUserInfo> info)
{
  m_beamInfo = info;
}

void
SatHelper::SetBeamUserInfo(uint32_t beamId, SatBeamUserInfo info)
{
  std::pair<BeamMap::iterator, bool> result = m_beamInfo.insert(std::make_pair(beamId, info));
  NS_ASSERT(result.second == true);
}

void SatHelper::CreateScenario(PreDefinedScenario scenario)
{
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
  AsciiTraceHelper asciiTraceHelper;
  std::string outputFile = "creation.log";

  if (!filename.empty())
    {
      outputFile = filename;
    }

  m_creationTraceStream = asciiTraceHelper.CreateFileStream (outputFile);
  m_utPosTraceStream = asciiTraceHelper.CreateFileStream ("ut-pos-" + outputFile);

  TraceConnectWithoutContext("CreationSummary", MakeCallback (&SatHelper::CreationSummarySink, this));

  m_detailedCreationTraces = details;
}

void SatHelper::EnableDetailedCreationTraces()
{
  CallbackBase creationCb = MakeBoundCallback (&SatHelper::CreationDetailsSink, m_creationTraceStream);
  TraceConnect("Creation", "SatHelper", creationCb);

  m_userHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
  m_beamHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
}

Ipv4Address
SatHelper::GetUserAddress(Ptr<Node> node)
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
  return ipv4->GetAddress (1, 0).GetLocal(); // Get Ipv4InterfaceAddress of interface csma interface.
}

NodeContainer
SatHelper::GetUtUsers()
{
  return m_userHelper->GetUtUsers();
}

NodeContainer
SatHelper::GetGwUsers()
{
  return m_userHelper->GetGwUsers();
}

void
SatHelper::CreateSimpleScenario()
{
  SatBeamUserInfo beamInfo = SatBeamUserInfo(1,1);
  BeamMap beamMap;
  beamMap[8] = beamInfo;

  CreateScenario(beamMap, 1);

  m_creationSummary("*** Simple Scenario Creation Summary ***");
}

void
SatHelper::CreateLargerScenario()
{
  // install one user for UTs in beams 12 and 22
  SatBeamUserInfo beamInfo = SatBeamUserInfo(1,1);
  BeamMap beamMap;

  beamMap[12] = beamInfo;
  beamMap[22] = beamInfo;

  // install two users for UT1 and one for UT2 in beam 3
  beamInfo.SetUtUserCount(0,2);
  beamInfo.AddUt(1);

  beamMap[3] = beamInfo;

  CreateScenario(beamMap, 1);

  m_creationSummary("*** Larger Scenario Creation Summary ***");
}

void
SatHelper::CreateFullScenario()
{
  uint32_t beamCount =  m_satConf->GetBeamCount();
  BeamMap beamMap;

  for ( uint32_t i = 1; i < (beamCount + 1); i ++ )
    {
      BeamMap::iterator beamInfo = m_beamInfo.find(i);
      SatBeamUserInfo info;

      if ( beamInfo != m_beamInfo.end())
        {
          info = beamInfo->second;
        }
      else
        {
          info = SatBeamUserInfo(m_utsInBeam, this->m_utUsers );
        }

      beamMap[i] = info;
    }

  CreateScenario(beamMap, m_gwUsers);

  m_creationSummary("*** Full Scenario Creation Summary ***");
}
void
SatHelper::CreateUserDefinedScenario()
{
  // create as user wants
  CreateScenario(m_beamInfo, m_gwUsers);

  m_creationSummary("*** User Defined Scenario Creation Summary ***");
}

void
SatHelper::CreateScenario(BeamMap beamInfo, uint32_t gwUsers)
{
  InternetStackHelper internet;

  // create Geo Satellite node, set mobility to it
  Ptr<Node> geoSatNode = CreateObject<Node>();
  SetGeoSatMobility(geoSatNode);

  m_beamHelper = CreateObject<SatBeamHelper> (geoSatNode,
                                              MakeCallback (&SatConf::GetCarrierBandwidth, m_satConf),
                                              m_satConf->GetRtnLinkCarrierCount(),
                                              m_satConf->GetFwdLinkCarrierCount());

  SatBeamHelper::CarrierFreqConverter converterCb = MakeCallback (&SatConf::GetCarrierFrequency, m_satConf);
  m_beamHelper->SetAttribute ("CarrierFrequencyConverter", CallbackValue (converterCb) );

  m_userHelper = CreateObject<SatUserHelper> ();

  if ( m_detailedCreationTraces )
    {
      EnableDetailedCreationTraces();
    }

  // set address base for GW user networks
  m_userHelper->SetGwBaseAddress("10.2.1.0", "255.255.255.0");

  // set address base for UT user networks
  m_userHelper->SetUtBaseAddress("10.3.1.0", "255.255.255.0");

  // set address base for satellite network
  m_beamHelper->SetBaseAddress("10.1.1.0", "255.255.255.0");

  // set Csma channel attributes
  m_userHelper->SetCsmaChannelAttribute ("DataRate", DataRateValue (5000000));
  m_userHelper->SetCsmaChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // Set the antenna patterns to beam helper
  m_beamHelper->SetAntennaGainPatterns (m_antennaGainPatterns);

  // create all possible GW nodes, set mobility to them and install to interner
  NodeContainer gwNodes;
  gwNodes.Create(m_satConf->GetGwCount());
  SetGwMobility(gwNodes);
  internet.Install(gwNodes);

  for ( BeamMap::iterator info = beamInfo.begin(); info != beamInfo.end(); info++)
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
}

void
SatHelper::SetUtMobility(NodeContainer uts, uint32_t beamId)
{
  MobilityHelper mobility;

  // Create new position allocator
  Ptr<SatSpotBeamPositionAllocator> allocator = CreateObject<SatSpotBeamPositionAllocator> (beamId, m_antennaGainPatterns);

  Ptr<UniformRandomVariable> altRnd = CreateObject<UniformRandomVariable> ();
  altRnd->SetAttribute ("Min", DoubleValue (0.0));
  altRnd->SetAttribute ("Max", DoubleValue (500.0));
  allocator->SetAltitude (altRnd);

  mobility.SetPositionAllocator (allocator);

  for ( NodeContainer::Iterator i = uts.Begin();  i != uts.End(); i++ )
    {
      mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
      mobility.Install (uts);
    }
}

void
SatHelper::SetGeoSatMobility(Ptr<Node> node)
{
  MobilityHelper mobility;

  Ptr<SatListPositionAllocator> geoSatPosAllocator = CreateObject<SatListPositionAllocator> ();
  geoSatPosAllocator->Add(m_satConf->GetGeoSatPosition());

  mobility.SetPositionAllocator (geoSatPosAllocator);
  mobility.SetMobilityModel ("ns3::SatConstantPositionMobilityModel");
  mobility.Install (node);
}

void
SatHelper::CreationDetailsSink(Ptr<OutputStreamWrapper> stream, std::string context, std::string info)
{
  *stream->GetStream () << context << ", " << info << std::endl;
}

void
SatHelper::CreationSummarySink(std::string title)
{
  *m_creationTraceStream->GetStream () << CreateCreationSummary(title);
  *m_utPosTraceStream->GetStream() << m_beamHelper->GetUtPositionInfo(false);
}

std::string
SatHelper::CreateCreationSummary(std::string title)
{
  std::ostringstream oss;

  oss << std::endl << std::endl << title << std::endl << std::endl;
  oss << "--- User Info ---" << std::endl << std::endl;
  oss << "Created GW users: " << m_userHelper->GetGwUserCount() << ", ";
  oss << "Created UT users: " << m_userHelper->GetUtUserCount() << std::endl << std::endl;
  oss << m_beamHelper->GetBeamInfo() << std::endl;

  return oss.str();
}

} // namespace ns3


