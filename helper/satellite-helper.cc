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
#include "ns3/sat-net-dev-helper.h"
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
      break;

    case Full:
      break;

    default:
      break;
  }

}


NodeContainer
SatHelper::Install (NodeContainer c)
{
  NS_ASSERT (c.GetN () == 2);
  return Install (c.Get (0), c.Get (1));
}

NodeContainer
SatHelper::Install (Ptr<Node> a, Ptr<Node> b)
{
   // creation here


  return NodeContainer::GetGlobal();
}

NodeContainer
SatHelper::Install (Ptr<Node> a, std::string bName)
{
  Ptr<Node> b = Names::Find<Node> (bName);
  return Install (a, b);
}

NodeContainer
SatHelper::Install (std::string aName, Ptr<Node> b)
{
  Ptr<Node> a = Names::Find<Node> (aName);
  return Install (a, b);
}

NodeContainer
SatHelper::Install (std::string aName, std::string bName)
{
  Ptr<Node> a = Names::Find<Node> (aName);
  Ptr<Node> b = Names::Find<Node> (bName);
  return Install (a, b);
}

Ipv4Address
SatHelper::GetUserAddress(uint32_t i)
{
  return m_userAddresses.GetAddress(i);
}

Ptr<Node>
SatHelper::GetUser(uint32_t i)
{
  return m_users.Get(i);
}

void
SatHelper::CreateSimpleScenario()
{
    m_users.Create (2);
    Ptr<Node> UT = CreateObject<Node> ();
    Ptr<Node> SB = CreateObject<Node> ();
    Ptr<Node> GW = CreateObject<Node> ();
    NodeContainer all = NodeContainer (UT, SB, GW);

    NodeContainer N0UT = NodeContainer (m_users.Get (0), UT);
    NodeContainer UTSat = NodeContainer (UT, SB);
    NodeContainer SatGW = NodeContainer (SB, GW);
    NodeContainer GWN1 = NodeContainer (GW, m_users.Get (1));

    InternetStackHelper internet;
    internet.Install (all);
    internet.Install (m_users);

    // We create the channels and net devices first without any IP addressing information
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
    NetDeviceContainer d1 = csma.Install (N0UT);
    NetDeviceContainer d4 = csma.Install (GWN1);

    SatNetDevHelper sndh;
    NetDeviceContainer d2 = sndh.Install (UTSat);
    NetDeviceContainer d3 = sndh.Install (SatGW);

    // Now, we add IP addresses.
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i0i1 = ipv4.Assign (d1);

    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    ipv4.Assign (d2);

    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    ipv4.Assign (d3);

    ipv4.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i3i4 = ipv4.Assign (d4);

    m_userAddresses.Add(i3i4.Get(1));
    m_userAddresses.Add(i0i1.Get(0));

    // Get IPv4 protocol implementations
    Ptr<Ipv4> ipv4N0 = m_users.Get (0)->GetObject<Ipv4> ();
    Ptr<Ipv4> ipv4UT = UT->GetObject<Ipv4> ();
    Ptr<Ipv4> ipv4SB = SB->GetObject<Ipv4> ();
    Ptr<Ipv4> ipv4GW = GW->GetObject<Ipv4> ();
    Ptr<Ipv4> ipv4N1 = m_users.Get (1)->GetObject<Ipv4> ();

    // Add IPv4 static routing. Note, currently you need to know
    // the netdevice indeces to attach the route to.
    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    // N0: Default route towards satellite network
    // IfIndex 1 = CSMA
    Ptr<Ipv4StaticRouting> srN0 = ipv4RoutingHelper.GetStaticRouting (ipv4N0);
    srN0->SetDefaultRoute (Ipv4Address ("10.1.1.2"), 1);

    // UT: Default route towards satellite network
    // IfIndex 1 = CSMA
    // IfIndex 2 = Sat
    Ptr<Ipv4StaticRouting> srUT = ipv4RoutingHelper.GetStaticRouting (ipv4UT);
    srUT->SetDefaultRoute (Ipv4Address ("10.1.2.2"), 2);

    // SB: Default route towards GW
    // IfIndex 1 = Sat -> UT
    // IfIndex 2 = Sat -> GW
    Ptr<Ipv4StaticRouting> srSB = ipv4RoutingHelper.GetStaticRouting (ipv4SB);
    srSB->SetDefaultRoute (Ipv4Address ("10.1.3.2"), 2);
    srSB->AddNetworkRouteTo (Ipv4Address ("10.1.1.0"), Ipv4Mask("255.255.255.0"), 1);

    // GW: Default route towards Internet
    // IfIndex 1 = Sat
    // IfIndex 2 = CSMA
    Ptr<Ipv4StaticRouting> srGW = ipv4RoutingHelper.GetStaticRouting (ipv4GW);
    srGW->SetDefaultRoute (Ipv4Address ("10.1.4.2"), 2);
    srGW->AddNetworkRouteTo (Ipv4Address ("10.1.1.0"), Ipv4Mask("255.255.255.0"), 1);
    srGW->AddNetworkRouteTo (Ipv4Address ("10.1.2.0"), Ipv4Mask("255.255.255.0"), 1);

    // N1: Default route towards satellite network
    // IfIndex 1 = CSMA
    Ptr<Ipv4StaticRouting> srN1 = ipv4RoutingHelper.GetStaticRouting (ipv4N1);
    srN1->SetDefaultRoute (Ipv4Address ("10.1.4.1"), 1);
}

} // namespace ns3


