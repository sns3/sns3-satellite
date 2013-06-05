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

#include "satellite-helper.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/string.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/satellite-user-helper.h"
#include "ns3/satellite-beam-helper.h"
#include "ns3/satellite-ut-helper.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

NS_LOG_COMPONENT_DEFINE ("SatHelper");

namespace ns3 {

SatHelper::SatHelper ()
{
}

SatHelper::SatHelper (PREDEFINED_SCENARIO scenario)
{
  switch(scenario)
  {
    case Simple:
      CreateSimpleScenario();
      break;

    case Larger:
      CreateLargerScenario();
      break;

    case Full:
      break;

    default:
      break;
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
  return m_userHelper.GetUtUsers();
}

NodeContainer
SatHelper::GetGwUsers()
{
  return m_userHelper.GetGwUsers();
}

void
SatHelper::CreateSimpleScenario()
{
  Ptr<Node> UT = CreateObject<Node> ();
  InternetStackHelper internet;
  internet.Install(UT);

  m_userHelper.SetGwBaseAddress("10.2.1.0", "255.255.255.0");
  m_userHelper.SetUtBaseAddress("10.3.1.0", "255.255.255.0");
  m_userHelper.SetCsmaChannelAttribute ("DataRate", DataRateValue (5000000));
  m_userHelper.SetCsmaChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  m_userHelper.InstallUt(UT, 1);

  SatBeamHelper beamHelper;

  beamHelper.SetBaseAddress("10.1.1.0", "255.255.255.0");
  std::vector<uint32_t> conf = satConf.GetBeamConfiguration(8);

  beamHelper.Install(UT, conf[2], conf[0], conf[1], conf[3]);

  m_userHelper.InstallGw(beamHelper.GetGwNodes(), 1);
}

void
SatHelper::CreateLargerScenario()
{
  NodeContainer Ut1;
  Ut1.Create(1);

  NodeContainer Uts;
  Uts.Create(3);

  InternetStackHelper internet;
  internet.Install(Ut1);
  internet.Install(Uts);

  // set address base for GW user networks
  m_userHelper.SetGwBaseAddress("10.2.1.0", "255.255.255.0");

  // set address base for UT user networks
  m_userHelper.SetUtBaseAddress("10.3.1.0", "255.255.255.0");

  // set Csma channel attributes
  m_userHelper.SetCsmaChannelAttribute ("DataRate", DataRateValue (5000000));
  m_userHelper.SetCsmaChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // install 2 two user for UT1 and one for UT2, UT3 and UT4
  m_userHelper.InstallUt(Ut1, 2);
  m_userHelper.InstallUt(Uts, 1);

  SatBeamHelper beamHelper;

  // set address base for satellite network
  beamHelper.SetBaseAddress("10.1.1.0", "255.255.255.0");

  // initialize beam 1 UT container with UT1 and UT2
  NodeContainer beam1Uts;
  beam1Uts.Add(Ut1);
  beam1Uts.Add(Uts.Get(0));

  // install UT1 and UT2 beam 3
  std::vector<uint32_t> conf = satConf.GetBeamConfiguration(3);
  beamHelper.Install(beam1Uts, conf[2], conf[0], conf[1], conf[3]);

  // install UT3 to beam 11
  conf = satConf.GetBeamConfiguration(11);
  beamHelper.Install(Uts.Get(1), conf[2], conf[0], conf[1], conf[3]);

  // installUT4 to beam 22
  conf = satConf.GetBeamConfiguration(22);
  beamHelper.Install(Uts.Get(2), conf[2], conf[0], conf[1], conf[3]);

  // finally install GWs to satellite network
  m_userHelper.InstallGw(beamHelper.GetGwNodes(), 1);
}


} // namespace ns3


