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

#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/queue.h"
#include "ns3/string.h"
#include "ns3/type-id.h"
#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"

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
                     UintegerValue (DEFAULT_UTS_IN_BEAM),
                     MakeUintegerAccessor (&SatHelper::m_utsInBeam),
                     MakeUintegerChecker<uint32_t> (MIN_UTS_IN_BEAM))
      .AddAttribute ("GwUsers", "The number of created GW users (full or user-defined scenario)",
                     UintegerValue (DEFAULT_GW_USERS),
                     MakeUintegerAccessor (&SatHelper::m_gwUsers),
                     MakeUintegerChecker<uint32_t> (MIN_GW_USERS))
      .AddAttribute ("UtUsers", "The number of created UT users per UT (full or user-defined scenario)",
                     UintegerValue (DEFAULT_UT_USERS),
                     MakeUintegerAccessor (&SatHelper::m_utUsers),
                     MakeUintegerChecker<uint32_t> (MIN_UT_USERS))
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
 :m_scenarioCreated(false)
{
  // uncomment next line, if attributes are needed already in construction phase
  //ObjectBase::ConstructSelf(AttributeConstructionList ());
}

void SatHelper::CreateScenario(PreDefinedScenario scenario)
{
  NS_ASSERT(m_scenarioCreated == false);

  m_beamHelper = CreateObject<SatBeamHelper>();
  m_userHelper = CreateObject<SatUserHelper>();

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

  TraceConnectWithoutContext("CreationSummary", MakeCallback (&SatHelper::CreationSummarySink, this));

  if ( details )
    {
      CallbackBase creationCb = MakeBoundCallback (&SatHelper::CreationDetailsSink, m_creationTraceStream);
      TraceConnect("Creation", "SatHelper", creationCb);

      m_userHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
      m_beamHelper->EnableCreationTraces(m_creationTraceStream, creationCb);
    }
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
  Ptr<Node> ut = CreateObject<Node> ();
  InternetStackHelper internet;
  internet.Install(ut);

  m_userHelper->SetGwBaseAddress("10.2.1.0", "255.255.255.0");
  m_userHelper->SetUtBaseAddress("10.3.1.0", "255.255.255.0");
  m_userHelper->SetCsmaChannelAttribute ("DataRate", DataRateValue (5000000));
  m_userHelper->SetCsmaChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  m_userHelper->InstallUt(ut, 1);

  m_beamHelper->SetBaseAddress("10.1.1.0", "255.255.255.0");
  std::vector<uint32_t> conf = m_satConf.GetBeamConfiguration(8);

  m_beamHelper->Install(ut, conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);

  m_userHelper->InstallGw(m_beamHelper->GetGwNodes(), 1);

  m_creationSummary("*** Simple Scenario Creation Summary ***");
}

void
SatHelper::CreateLargerScenario()
{
  NodeContainer ut1;
  ut1.Create(1);

  NodeContainer uts;
  uts.Create(3);

  InternetStackHelper internet;
  internet.Install(ut1);
  internet.Install(uts);

  // set address base for GW user networks
  m_userHelper->SetGwBaseAddress("10.2.1.0", "255.255.255.0");

  // set address base for UT user networks
  m_userHelper->SetUtBaseAddress("10.3.1.0", "255.255.255.0");

  // set Csma channel attributes
  m_userHelper->SetCsmaChannelAttribute ("DataRate", DataRateValue (5000000));
  m_userHelper->SetCsmaChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // install two users for UT1 and one for UT2, UT3 and UT4
  m_userHelper->InstallUt(ut1, 2);
  m_userHelper->InstallUt(uts, 1);

  // set address base for satellite network
  m_beamHelper->SetBaseAddress("10.1.1.0", "255.255.255.0");

  // initialize beam 1 UT container with UT1 and UT2
  NodeContainer beam1Uts;
  beam1Uts.Add(ut1);
  beam1Uts.Add(uts.Get(0));

  // install UT1 and UT2 beam 3
  std::vector<uint32_t> conf = m_satConf.GetBeamConfiguration(3);
  m_beamHelper->Install(beam1Uts, conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);

  // install UT3 to beam 12
  conf = m_satConf.GetBeamConfiguration(12);
  m_beamHelper->Install(uts.Get(1), conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);

  // installUT4 to beam 22
  conf = m_satConf.GetBeamConfiguration(22);
  m_beamHelper->Install(uts.Get(2), conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);

  // finally install GWs to satellite network
  m_userHelper->InstallGw(m_beamHelper->GetGwNodes(), 1);

  m_creationSummary("*** Larger Scenario Creation Summary ***");
}

void
SatHelper::CreateFullScenario()
{
  NodeContainer uts;
  uint32_t beamCount =  m_satConf.GetBeamCount();
  uts.Create(beamCount * m_utsInBeam);

  InternetStackHelper internet;
  internet.Install(uts);

  // set address base for GW user networks
  m_userHelper->SetGwBaseAddress("10.2.1.0", "255.255.255.0");

  // set address base for UT user networks
  m_userHelper->SetUtBaseAddress("10.3.1.0", "255.255.255.0");

  // set Csma channel attributes
  m_userHelper->SetCsmaChannelAttribute ("DataRate", DataRateValue (5000000));
  m_userHelper->SetCsmaChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // install user(s) for every UTs
  m_userHelper->InstallUt(uts, m_utUsers);

  // set address base for satellite network
  m_beamHelper->SetBaseAddress("10.1.1.0", "255.255.255.0");

  // install UTs to satellite network
  for ( uint32_t i = 0; i < beamCount; i ++ )
    {
      NodeContainer ut;

      for (uint32_t j= 0; j < m_utsInBeam; j++)
        {
          ut.Add(uts.Get(i * m_utsInBeam + j));
        }

      std::vector<uint32_t> conf = m_satConf.GetBeamConfiguration(i + 1);
      m_beamHelper->Install(ut, conf[SatConf::GW_ID_INDEX], conf[SatConf::BEAM_ID_INDEX], conf[SatConf::U_FREQ_ID_INDEX], conf[SatConf::F_FREQ_ID_INDEX]);
    }

  // finally install GWs to satellite network
  m_userHelper->InstallGw(m_beamHelper->GetGwNodes(), m_gwUsers);

  m_creationSummary("*** Full Scenario Creation Summary ***");
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
}

std::string
SatHelper::CreateCreationSummary(std::string title)
{
  std::ostringstream oss;

  oss << std::endl << std::endl << title << std::endl << std::endl;
  oss << "--- User Info ---" << std::endl << std::endl;
  oss << "Created GW users: " << m_userHelper->GetGwUserN() << ", ";
  oss << "Created UT users: " << m_userHelper->GetUtUserN() << std::endl << std::endl;
  oss << m_beamHelper->GetBeamInfo() << std::endl;

  return oss.str();
}

} // namespace ns3


