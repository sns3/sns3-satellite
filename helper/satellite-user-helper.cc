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

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"

#include "ns3/trace-helper.h"
#include "satellite-user-helper.h"
#include "satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatUserHelper");

namespace ns3 {

SatUserHelper::SatUserHelper ()
{

}

void 
SatUserHelper::SetCsmaDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_csma.SetDeviceAttribute(n1,v1);
}

void
SatUserHelper::SetCsmaChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_csma.SetChannelAttribute(n1,v1);
}

void SatUserHelper::SetUtBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, const Ipv4Address address)
{
  m_ipv4Ut.SetBase(network, mask, address);
}

void SatUserHelper::SetGwBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, const Ipv4Address address)
{
  m_ipv4Gw.SetBase(network, mask, address);
}

NodeContainer
SatUserHelper::InstallUt (NodeContainer ut, uint16_t userCount )
{
  InternetStackHelper internet;

  // We create the channels and net devices

  for (NodeContainer::Iterator i = ut.Begin (); i != ut.End (); i++)
    {
      NodeContainer users;
      users.Create(userCount);
      NodeContainer utUsers = NodeContainer((*i), users);

      internet.Install (users);

      NetDeviceContainer nd = m_csma.Install (utUsers);
      Ipv4InterfaceContainer addresses = m_ipv4Ut.Assign (nd);
      Ipv4StaticRoutingHelper ipv4RoutingHelper;

      // Set default route toward satellite (UTs address).
      for (NodeContainer::Iterator i = users.Begin (); i != users.End (); i++)
        {
          // Get IPv4 protocol implementations
          Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();
          Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (ipv4);
          routing->SetDefaultRoute (addresses.GetAddress(0), 1);
        }

      m_utUsers.Add(users);

      m_ipv4Ut.NewNetwork();
    }

  return m_utUsers;
  }

NodeContainer
SatUserHelper::InstallGw (NodeContainer gw, uint16_t userCount )
{
  InternetStackHelper internet;
  Ptr<Node> router;
  // We create the channels and net devices

  if (gw.GetN() == 1)
    {
      router = gw.Get(0);
    }
  else
    {
      router = Create<Node>();
      internet.Install(router);

      for (NodeContainer::Iterator i = gw.Begin (); i != gw.End (); i++)
        {
          NodeContainer gwRouter = NodeContainer((*i), router);

          NetDeviceContainer nd = m_csma.Install (gwRouter);
          Ipv4InterfaceContainer addresses = m_ipv4Gw.Assign (nd);
          Ipv4StaticRoutingHelper ipv4RoutingHelper;

          // Get IPv4 protocol implementations
          Ptr<Ipv4> ipv4Gw = (*i)->GetObject<Ipv4> ();
          uint32_t lastGwIf = ipv4Gw->GetNInterfaces() - 1;
          Ptr<Ipv4StaticRouting> routingGw = ipv4RoutingHelper.GetStaticRouting (ipv4Gw);
          routingGw->SetDefaultRoute (addresses.GetAddress(1), lastGwIf);

          // Get IPv4 protocol implementations
          Ptr<Ipv4> ipv4Router = (*i)->GetObject<Ipv4> ();
          uint32_t lastRouterIf = ipv4Router->GetNInterfaces() - 1;
          Ptr<Ipv4StaticRouting> routingRouter = ipv4RoutingHelper.GetStaticRouting (ipv4Router);
          Ipv4Mask mask = ipv4Router->GetAddress(lastRouterIf, 0).GetMask();
          routingRouter->AddNetworkRouteTo (addresses.GetAddress(0).CombineMask(mask), mask, lastRouterIf);

          m_ipv4Gw.NewNetwork();
        }
    }

  NodeContainer users;
  users.Create(userCount);
  NodeContainer routerUsers = NodeContainer(router, users);

  internet.Install (users);

  NetDeviceContainer nd = m_csma.Install (routerUsers);
  Ipv4InterfaceContainer addresses = m_ipv4Gw.Assign (nd);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  Ptr<Ipv4> ipv4Router = router->GetObject<Ipv4> ();
  uint32_t lastRouterIf = ipv4Router->GetNInterfaces() - 1;
  Ptr<Ipv4StaticRouting> routingRouter = ipv4RoutingHelper.GetStaticRouting (ipv4Router);
  routingRouter->SetDefaultRoute(addresses.GetAddress(1), lastRouterIf);

  // Set default route toward router (GW) for users
  for (NodeContainer::Iterator i = users.Begin (); i != users.End (); i++)
    {
      // Get IPv4 protocol implementations
      Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();
      Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (ipv4);
      routing->SetDefaultRoute (addresses.GetAddress(0), 1);
    }

   m_gwUsers.Add(users);
   m_ipv4Gw.NewNetwork();

  return m_gwUsers;
}

NodeContainer
SatUserHelper::GetGwUsers()
{
  return m_gwUsers;
}

NodeContainer
SatUserHelper::GetUtUsers()
{
  return m_utUsers;
}

} // namespace ns3
