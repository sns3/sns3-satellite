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
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/csma-helper.h"
#include "satellite-user-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatUserHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUserHelper);

TypeId
SatUserHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatUserHelper")
      .SetParent<Object> ()
      .AddConstructor<SatUserHelper> ()
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatUserHelper::m_creation))
    ;
    return tid;
}

TypeId
SatUserHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

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
SatUserHelper::InstallUt (NodeContainer ut, uint32_t userCount )
{
  InternetStackHelper internet;

  // create users and csma links between UTs and users and add IP routes
  for (NodeContainer::Iterator i = ut.Begin (); i != ut.End (); i++)
    {
      NodeContainer users;
      users.Create(userCount);
      NodeContainer utUsers = NodeContainer((*i), users);

      internet.Install (users);

      NetDeviceContainer nd = m_csma.Install (utUsers);
      Ipv4InterfaceContainer addresses = m_ipv4Ut.Assign (nd);
      Ipv4StaticRoutingHelper ipv4RoutingHelper;

      // Set default route for users toward satellite (UTs address).
      for (NodeContainer::Iterator j = users.Begin (); j != users.End (); j++)
        {
          // Get IPv4 protocol implementations
          Ptr<Ipv4> ipv4 = (*j)->GetObject<Ipv4> ();
          Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (ipv4);
          routing->SetDefaultRoute (addresses.GetAddress(0), 1);
          NS_LOG_INFO ("SatUserHelper::InstallUt, User default route: " << addresses.GetAddress(0) );
        }

      m_utUsers.Add(users);

      m_ipv4Ut.NewNetwork();
    }

  return m_utUsers;
  }

NodeContainer
SatUserHelper::InstallGw (NodeContainer gw, uint32_t userCount )
{
  InternetStackHelper internet;
  Ptr<Node> router;

  if (gw.GetN() == 1)
    {
      router = gw.Get(0);
    }
  else
    {
      router = CreateObject<Node>();
      internet.Install(router);
      InstallRouter(gw, router);
    }

  // create users and csma links between Router and users and add IP routes
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
  NS_LOG_INFO ("SatUserHelper::InstallGw, Router default route: " << addresses.GetAddress(1) );

  // Set default route toward router (GW) for users
  for (NodeContainer::Iterator i = users.Begin (); i != users.End (); i++)
    {
      // Get IPv4 protocol implementations
      Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();
      Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (ipv4);
      routing->SetDefaultRoute (addresses.GetAddress(0), 1);
      NS_LOG_INFO ("SatUserHelper::InstallGw, User default route: " << addresses.GetAddress(0));
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

uint32_t
SatUserHelper::GetGwUserN()
{
  return m_gwUsers.GetN();
}

uint32_t
SatUserHelper::GetUtUserN()
{
  return m_utUsers.GetN();
}

void
SatUserHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  TraceConnect("Creation", "SatUserHelper", cb);
}

void
SatUserHelper::InstallRouter(NodeContainer gw, Ptr<Node> router)
{
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
    NS_LOG_INFO ("SatUserHelper::InstallRouter  GW default route: " << addresses.GetAddress(1) );

    for (uint32_t  routeIndex = 0; routeIndex < routingGw->GetNRoutes(); routeIndex++)
      {
        // Get IPv4 protocol implementations
        Ptr<Ipv4> ipv4Router = router->GetObject<Ipv4> ();
        uint32_t lastRouterIf = ipv4Router->GetNInterfaces() - 1;
        Ptr<Ipv4StaticRouting> routingRouter = ipv4RoutingHelper.GetStaticRouting (ipv4Router);

        Ipv4RoutingTableEntry route = routingGw->GetRoute(routeIndex);
        uint32_t interface = route.GetInterface();

        // set only routes for interfaces created earlier (and not for local delivery index 0)
        if ((interface != 0) && (interface != lastGwIf))
          {
            routingRouter->AddNetworkRouteTo (route.GetDest(), route.GetDestNetworkMask(), addresses.GetAddress(0), lastRouterIf);
            NS_LOG_INFO ("SatUserHelper::InstallRouter, Router network route:" << route.GetDest()
                         << ", " << route.GetDestNetworkMask() << ", " << addresses.GetAddress(0));
          }
      }

    m_ipv4Gw.NewNetwork();
  }
}

} // namespace ns3
