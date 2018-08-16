/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/ipv4-interface.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-routing-table-entry.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/csma-helper.h>
#include <ns3/singleton.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-arp-cache.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/satellite-simple-net-device.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-mobility-observer.h>
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
    .AddAttribute ("BackboneNetworkType",
                   "Network used between GW and Router, and between Router and Users in operator network",
                   EnumValue (SatUserHelper::NETWORK_TYPE_SAT_SIMPLE),
                   MakeEnumAccessor (&SatUserHelper::m_backboneNetworkType),
                   MakeEnumChecker (SatUserHelper::NETWORK_TYPE_SAT_SIMPLE, "SatSimple",
                                    SatUserHelper::NETWORK_TYPE_CSMA, "Csma"))
    .AddAttribute ("SubscriberNetworkType",
                   "Network used between UTs and Users in subscriber network",
                   EnumValue (SatUserHelper::NETWORK_TYPE_CSMA),
                   MakeEnumAccessor (&SatUserHelper::m_subscriberNetworkType),
                   MakeEnumChecker (SatUserHelper::NETWORK_TYPE_SAT_SIMPLE, "SatSimple",
                                    SatUserHelper::NETWORK_TYPE_CSMA, "Csma"))
    .AddAttribute ("PropagationDelayGetter",
                   "Callback to retrieve propagation delay models from beam IDs",
                   CallbackValue (),
                   MakeCallbackAccessor (&SatUserHelper::m_propagationDelayCallback),
                   MakeCallbackChecker ())
    .AddTraceSource ("Creation", "Creation traces",
                     MakeTraceSourceAccessor (&SatUserHelper::m_creationTrace),
                     "ns3::SatTypedefs::CreationCallback")
  ;
  return tid;
}

TypeId
SatUserHelper::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatUserHelper::SatUserHelper ()
  : m_backboneNetworkType (SatUserHelper::NETWORK_TYPE_SAT_SIMPLE),
  m_subscriberNetworkType (SatUserHelper::NETWORK_TYPE_CSMA),
  m_router (0)
{
  NS_LOG_FUNCTION (this);
}

SatUserHelper::~SatUserHelper ()
{
  NS_LOG_FUNCTION (this);
}

void
SatUserHelper::SetCsmaQueue (std::string type,
                             std::string name1, const AttributeValue &value1,
                             std::string name2, const AttributeValue &value2,
                             std::string name3, const AttributeValue &value3,
                             std::string name4, const AttributeValue &value4)
{
  NS_LOG_FUNCTION (this << type);

  m_csma.SetQueue (type, name1, value1, name2, value2,
                   name3, value3, name4, value4);
}

void
SatUserHelper::SetCsmaDeviceAttribute (std::string name,
                                       const AttributeValue &value)
{
  NS_LOG_FUNCTION (this);
  m_csma.SetDeviceAttribute (name, value);
}

void
SatUserHelper::SetCsmaChannelAttribute (std::string name,
                                        const AttributeValue &value)
{
  NS_LOG_FUNCTION (this);
  m_csma.SetChannelAttribute (name, value);
}

void
SatUserHelper::SetUtBaseAddress (const Ipv4Address& network, const Ipv4Mask& mask, const Ipv4Address address)
{
  NS_LOG_FUNCTION (this);

  m_ipv4Ut.SetBase (network, mask, address);
}

void
SatUserHelper::SetGwBaseAddress (const Ipv4Address& network, const Ipv4Mask& mask, const Ipv4Address address)
{
  NS_LOG_FUNCTION (this);

  m_ipv4Gw.SetBase (network, mask, address);
}

void
SatUserHelper::SetBeamBaseAddress (const Ipv4Address& network, const Ipv4Mask& mask, const Ipv4Address address)
{
  NS_LOG_FUNCTION (this);

  m_ipv4Beam.SetBase (network, mask, address);
}

NodeContainer
SatUserHelper::InstallUt (NodeContainer ut, uint32_t userCount )
{
  NS_LOG_FUNCTION (this << userCount);

  NodeContainer createdUsers;

  // create users and csma links between UTs and users and add IP routes
  for (NodeContainer::Iterator i = ut.Begin (); i != ut.End (); i++)
    {
      createdUsers.Add (InstallUt (*i, userCount));
    }

  return createdUsers;
}

NodeContainer
SatUserHelper::InstallUt (Ptr<Node> ut, uint32_t userCount )
{
  NS_LOG_FUNCTION (this << userCount);

  if ( userCount == 0 )
    {
      NS_FATAL_ERROR ("User count is zero!!!");
    }

  InternetStackHelper internet;

  NodeContainer users;
  users.Create (userCount);
  NodeContainer utUsers = NodeContainer (ut, users);

  internet.Install (users);

  NetDeviceContainer nd = InstallSubscriberNetwork (utUsers);
  Ipv4InterfaceContainer addresses = m_ipv4Ut.Assign (nd);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  for (NodeContainer::Iterator i = users.Begin (); i != users.End (); i++)
    {
      // Add the user and the UT as a new entry to the UT map
      std::pair<std::map<Ptr<Node>, Ptr<Node> >::iterator, bool> ret
        = m_utMap.insert (std::make_pair (*i, ut));
      NS_ASSERT (ret.second);

      // Add the user's MAC address to the global mapper
      NS_ASSERT_MSG ((*i)->GetNDevices () == 2,
                     "Failed to get the device to subscriber network in UT user node " << (*i)->GetId ());
      // assuming that #0 is for loopback device and #1 is for subscriber network device
      Ptr<NetDevice> dev = (*i)->GetDevice (1);
      Singleton<SatIdMapper>::Get ()->AttachMacToUtUserId (dev->GetAddress ());

      // Get IPv4 protocol implementations
      Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();

      // Set default route for users toward satellite (UTs address)
      Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (ipv4);
      routing->SetDefaultRoute (addresses.GetAddress (0), 1);
      NS_LOG_INFO ("User default route: " << addresses.GetAddress (0) );
    }

  m_ipv4Ut.NewNetwork ();

  std::pair<UtUsersContainer_t::const_iterator, bool> result = m_utUsers.insert ( std::make_pair (ut, users) );

  if (result.second == false)
    {
      NS_FATAL_ERROR ("UT is already installed.");
    }

  m_allUtUsers.Add (users);

  return users;
}

NodeContainer
SatUserHelper::InstallGw (NodeContainer gw, uint32_t userCount )
{
  NS_LOG_FUNCTION (this << userCount);

  InternetStackHelper internet;

  if ( m_router == NULL )
    {
      m_router = CreateObject<Node> ();
      internet.Install (m_router);
      InstallRouter (gw, m_router);
    }

  // create users and csma links between Router and users and add IP routes
  NodeContainer users;
  users.Create (userCount);
  NodeContainer routerUsers = NodeContainer (m_router, users);

  internet.Install (users);

  NetDeviceContainer nd = InstallBackboneNetwork (routerUsers);
  Ipv4InterfaceContainer addresses = m_ipv4Gw.Assign (nd);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  Ptr<Ipv4> ipv4Router = m_router->GetObject<Ipv4> ();
  uint32_t lastRouterIf = ipv4Router->GetNInterfaces () - 1;
  Ptr<Ipv4StaticRouting> routingRouter = ipv4RoutingHelper.GetStaticRouting (ipv4Router);
  routingRouter->SetDefaultRoute (addresses.GetAddress (1), lastRouterIf);
  NS_LOG_INFO ("Router default route: " << addresses.GetAddress (1) );

  for (NodeContainer::Iterator i = users.Begin (); i != users.End (); i++)
    {
      // Add the user's MAC address to the global mapper
      NS_ASSERT_MSG ((*i)->GetNDevices () == 2,
                     "Failed to get the device to backbone network in GW user node " << (*i)->GetId ());
      // assuming that #0 is for loopback device and #1 is for backbone network device
      Ptr<NetDevice> dev = (*i)->GetDevice (1);
      Singleton<SatIdMapper>::Get ()->AttachMacToGwUserId (dev->GetAddress ());

      // Get IPv4 protocol implementations
      Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();

      // Set default route toward router (GW) for users
      Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (ipv4);
      routing->SetDefaultRoute (addresses.GetAddress (0), 1);
      NS_LOG_INFO ("User default route: " << addresses.GetAddress (0));
    }

  m_gwUsers.Add (users);
  m_ipv4Gw.NewNetwork ();

  return m_gwUsers;
}

NodeContainer
SatUserHelper::GetGwUsers () const
{
  NS_LOG_FUNCTION (this);

  return m_gwUsers;
}

bool
SatUserHelper::IsGwUser (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this);

  bool isGwUser = false;

  for ( NodeContainer::Iterator it = m_gwUsers.Begin (); ( (it != m_gwUsers.End ()) && !isGwUser ); it++)
    {
      if ( *it == node )
        {
          isGwUser = true;
        }
    }

  return isGwUser;
}


NodeContainer
SatUserHelper::GetUtUsers () const
{
  NS_LOG_FUNCTION (this);

  return m_allUtUsers;
}

NodeContainer
SatUserHelper::GetUtUsers (Ptr<Node> ut) const
{
  NS_LOG_FUNCTION (this);

  UtUsersContainer_t::const_iterator it = m_utUsers.find (ut);

  if ( it == m_utUsers.end ())
    {
      NS_FATAL_ERROR ("UT which users are requested in not installed!!!");
    }

  return it->second;
}

uint32_t
SatUserHelper::GetGwUserCount () const
{
  NS_LOG_FUNCTION (this);

  return m_gwUsers.GetN ();
}

uint32_t
SatUserHelper::GetUtUserCount () const
{
  NS_LOG_FUNCTION (this);

  return m_allUtUsers.GetN ();
}

uint32_t
SatUserHelper::GetUtUserCount (Ptr<Node> ut) const
{
  NS_LOG_FUNCTION (this);

  UtUsersContainer_t::const_iterator it = m_utUsers.find (ut);

  if ( it == m_utUsers.end ())
    {
      NS_FATAL_ERROR ("UT which user count is requested in not installed!!!");
    }

  return it->second.GetN ();
}

Ptr<Node>
SatUserHelper::GetUtNode (Ptr<Node> utUserNode) const
{
  std::map<Ptr<Node>, Ptr<Node> >::const_iterator it
    = m_utMap.find (utUserNode);

  if (it == m_utMap.end ())
    {
      return 0;
    }
  else
    {
      return it->second;
    }
}

void
SatUserHelper::EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect ("Creation", "SatUserHelper", cb);
}

void
SatUserHelper::InstallRouter (NodeContainer gw, Ptr<Node> router)
{
  NS_LOG_FUNCTION (this);

  for (NodeContainer::Iterator i = gw.Begin (); i != gw.End (); i++)
    {
      NodeContainer gwRouter = NodeContainer ((*i), router);

      NetDeviceContainer nd = InstallBackboneNetwork (gwRouter);
      Ipv4InterfaceContainer addresses = m_ipv4Gw.Assign (nd);
      Ipv4StaticRoutingHelper ipv4RoutingHelper;

      // Get IPv4 protocol implementations
      Ptr<Ipv4> ipv4Gw = (*i)->GetObject<Ipv4> ();
      uint32_t lastGwIf = ipv4Gw->GetNInterfaces () - 1;
      Ptr<Ipv4StaticRouting> routingGw = ipv4RoutingHelper.GetStaticRouting (ipv4Gw);
      routingGw->SetDefaultRoute (addresses.GetAddress (1), lastGwIf);
      NS_LOG_INFO ("GW default route: " << addresses.GetAddress (1) );

      for (uint32_t  routeIndex = 0; routeIndex < routingGw->GetNRoutes (); routeIndex++)
        {
          // Get IPv4 protocol implementations
          Ptr<Ipv4> ipv4Router = router->GetObject<Ipv4> ();
          uint32_t lastRouterIf = ipv4Router->GetNInterfaces () - 1;
          Ptr<Ipv4StaticRouting> routingRouter = ipv4RoutingHelper.GetStaticRouting (ipv4Router);

          Ipv4RoutingTableEntry route = routingGw->GetRoute (routeIndex);
          uint32_t interface = route.GetInterface ();

          // set only routes for interfaces created earlier (and not for local delivery index 0)
          if ((interface != 0) && (interface != lastGwIf))
            {
              routingRouter->AddNetworkRouteTo (route.GetDest (), route.GetDestNetworkMask (), addresses.GetAddress (0), lastRouterIf);
              NS_LOG_INFO ("Router network route:" << route.GetDest () <<
                           ", " << route.GetDestNetworkMask () <<
                           ", " << addresses.GetAddress (0));
            }
        }

      m_ipv4Gw.NewNetwork ();
    }
}

NetDeviceContainer
SatUserHelper::InstallSubscriberNetwork (const NodeContainer &c ) const
{
  NS_LOG_FUNCTION (this);

  NetDeviceContainer devs;

  switch (m_subscriberNetworkType)
    {
    case NETWORK_TYPE_SAT_SIMPLE:
      devs =  InstallSatSimpleNetwork (c);
      break;

    case NETWORK_TYPE_CSMA:
      devs = m_csma.Install (c);
      break;

    default:
      NS_ASSERT (false);
      break;
    }

  return devs;
}

NetDeviceContainer
SatUserHelper::InstallBackboneNetwork (const NodeContainer &c ) const
{
  NS_LOG_FUNCTION (this);

  NetDeviceContainer devs;

  switch (m_backboneNetworkType)
    {
    case NETWORK_TYPE_SAT_SIMPLE:
      devs =  InstallSatSimpleNetwork (c);
      break;

    case NETWORK_TYPE_CSMA:
      devs = m_csma.Install (c);
      break;

    default:
      NS_ASSERT (false);
      break;
    }

  return devs;
}

NetDeviceContainer
SatUserHelper::InstallSatSimpleNetwork (const NodeContainer &c ) const
{
  NS_LOG_FUNCTION (this);

  NetDeviceContainer devs;
  Ptr<SatSimpleChannel> channel = CreateObject<SatSimpleChannel> ();

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<SatSimpleNetDevice> device = CreateObject<SatSimpleNetDevice> ();
      device->SetAddress (Mac48Address::Allocate ());
      (*i)->AddDevice (device);
      device->SetChannel (channel);
      devs.Add (device);
    }

  return devs;
}

std::string
SatUserHelper::GetRouterInfo () const
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;

  Address devAddress;
  Ptr<Ipv4> ipv4 = m_router->GetObject<Ipv4> (); // Get Ipv4 instance of the node

  std::vector<Ipv4Address> IPAddressVector;
  std::vector<std::string> devNameVector;
  std::vector<Address> devAddressVector;

  oss << "---  Router info  ---" << std::endl << std::endl;

  for ( uint32_t j = 0; j < m_router->GetNDevices (); j++)
    {
      Ptr<NetDevice> device = m_router->GetDevice (j);

      oss << device->GetInstanceTypeId ().GetName () << " ";
      oss << device->GetAddress () << " ";
      oss << ipv4->GetAddress (j, 0).GetLocal () << " ";
    }

  return oss.str ();
}

Ptr<Node>
SatUserHelper::GetRouter () const
{
  NS_LOG_FUNCTION (this);

  return m_router;
}

void
SatUserHelper::PopulateBeamRoutings (NodeContainer ut, NetDeviceContainer utNd,
                                     Ptr<Node> gw, Ptr<NetDevice> gwNd)
{
  Ipv4InterfaceContainer gwAddress = m_ipv4Beam.Assign (gwNd);
  Ipv4Address gwAddr = gwAddress.GetAddress (0);
  NS_LOG_FUNCTION (this << gw << gwNd << gwAddr);

  Ipv4InterfaceContainer utIfs = m_ipv4Beam.Assign (utNd);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4L3Protocol> ipv4Gw = gw->GetObject<Ipv4L3Protocol> ();
  Ptr<Ipv4StaticRouting> srGw = ipv4RoutingHelper.GetStaticRouting (ipv4Gw);

  // Store GW NetDevice for updating routing during handover
  m_gwDevices.insert (std::make_pair (gwNd->GetAddress (), gwNd));

  // Create an ARP entry of the default GW for the UTs in this beam
  Address macAddressGw = gwNd->GetAddress ();
  Ptr<SatArpCache> utArpCache = CreateObject<SatArpCache> ();
  utArpCache->Add (gwAddr, macAddressGw);
  NS_LOG_INFO ("UT ARP entry:  " << gwAddr << " - " << macAddressGw );

  // Add the ARP entries of all the UTs in this beam
  // - MAC address vs. IPv4 address
  Ptr<SatArpCache> gwArpCache = CreateObject<SatArpCache> ();
  for (uint32_t i = 0; i < utIfs.GetN (); ++i)
    {
      NS_ASSERT (utIfs.GetN () == utNd.GetN ());
      Ptr<NetDevice> nd = utNd.Get (i);
      Ipv4Address ipv4Addr = utIfs.GetAddress (i);
      gwArpCache->Add (ipv4Addr, nd->GetAddress ());
      NS_LOG_INFO ("GW ARP entry:  " << ipv4Addr << " - " << nd->GetAddress ());
      // Store UT NetDevice for updating routing during handover
      m_utDevices.insert (std::make_pair (nd->GetAddress (), nd));
    }

  // Set the ARP cache to the proper GW IPv4Interface (the one for satellite
  // link). ARP cache contains the entries for all UTs within this spot-beam.
  ipv4Gw->GetInterface (gwNd->GetIfIndex ())->SetArpCache (gwArpCache);
  NS_LOG_INFO ("Add ARP cache to GW " << gw->GetId () );

  uint32_t utAddressIndex = 0;

  for (NodeContainer::Iterator i = ut.Begin (); i != ut.End (); i++)
    {
      Ptr<Ipv4L3Protocol> ipv4Ut = (*i)->GetObject<Ipv4L3Protocol> ();

      uint32_t count = ipv4Ut->GetNInterfaces ();

      for (uint32_t j = 1; j < count; j++)
        {
          std::string devName = ipv4Ut->GetNetDevice (j)->GetInstanceTypeId ().GetName ();

          // If SatNetDevice interface, add default route to towards GW of the beam on UTs
          if ( devName == "ns3::SatNetDevice" )
            {
              Ptr<Ipv4StaticRouting> srUt = ipv4RoutingHelper.GetStaticRouting (ipv4Ut);
              srUt->SetDefaultRoute (gwAddr, j);
              NS_LOG_INFO ("UT default route: " << gwAddr);

              // Set the ARP cache (including the ARP entry for the default GW) to the UT
              ipv4Ut->GetInterface (j)->SetArpCache (utArpCache);
              NS_LOG_INFO ("Add the ARP cache to UT " << (*i)->GetId () );

            }
          else  // add other interface route to GW's Satellite interface
            {
              Ipv4Address address = ipv4Ut->GetAddress (j, 0).GetLocal ();
              Ipv4Mask mask = ipv4Ut->GetAddress (j, 0).GetMask ();

              srGw->AddNetworkRouteTo (address.CombineMask (mask), mask, utIfs.GetAddress (utAddressIndex), gwNd->GetIfIndex ());
              NS_LOG_INFO ("GW Network route:  " << address.CombineMask (mask) <<
                           ", " << mask << ", " << utIfs.GetAddress (utAddressIndex));
            }
        }

      utAddressIndex++;
    }

  m_ipv4Beam.NewNetwork ();
}

void
SatUserHelper::UpdateUtRoutes (Address utAddress, Address gwAddress)
{
  NS_LOG_FUNCTION (this << utAddress << gwAddress);

  std::map<Address, Ptr<NetDevice> >::iterator gwNdIterator = m_gwDevices.find (gwAddress);
  NS_ASSERT_MSG (gwNdIterator != m_gwDevices.end (), "Unknown GW with MAC address " << gwAddress);

  Ptr<SatNetDevice> gwNd = DynamicCast<SatNetDevice> (gwNdIterator->second);
  NS_ASSERT (gwNd != NULL);
  Ipv4Address ip = gwNd->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (gwNd->GetIfIndex (), 0).GetLocal ();

  std::map<Address, Ptr<NetDevice> >::iterator utNdIterator = m_utDevices.find (utAddress);
  NS_ASSERT_MSG (utNdIterator != m_utDevices.end (), "Unknown UT with MAC address " << utAddress);

  Ptr<SatNetDevice> utNd = DynamicCast<SatNetDevice> (utNdIterator->second);
  NS_ASSERT (utNd != NULL);
  Ptr<Ipv4L3Protocol> protocol = utNd->GetNode ()->GetObject<Ipv4L3Protocol> ();
  uint32_t utIfIndex = utNdIterator->second->GetIfIndex ();

  NS_LOG_INFO ("Adding ARP cache entry to UT " << utAddress <<
               " pointing to " << ip << " through " << gwAddress);
  ArpCache::Entry *entry = protocol->GetInterface (utIfIndex)->GetArpCache ()->Add (ip);
  entry->SetMacAddress (gwAddress);
  entry->MarkPermanent ();

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (protocol);
  routing->SetDefaultRoute (ip, utIfIndex);

  uint32_t beamId = gwNd->GetMac ()->GetBeamId ();
  Ptr<PropagationDelayModel> flDelayModel = m_propagationDelayCallback (beamId, SatEnums::RETURN_FEEDER_CH);
  Ptr<PropagationDelayModel> ulDelayModel = m_propagationDelayCallback (beamId, SatEnums::RETURN_USER_CH);
  Ptr<SatMobilityObserver> observer = utNd->GetNode ()->GetObject<SatMobilityObserver> ();
  observer->ObserveTimingAdvance (ulDelayModel, flDelayModel, gwNd->GetNode ()->GetObject<SatMobilityModel> ());
}

void
SatUserHelper::UpdateGwRoutes (Address ut, Address oldGateway, Address newGateway)
{
  NS_LOG_FUNCTION (this << ut << oldGateway << newGateway);

  std::map<Address, Ptr<NetDevice> >::iterator utNdIterator = m_utDevices.find (ut);
  if (utNdIterator == m_utDevices.end ())
    {
      NS_FATAL_ERROR ("Unknown UT with MAC address " << ut);
    }

  std::map<Address, Ptr<NetDevice> >::iterator oldGwNdIterator = m_gwDevices.find (oldGateway);
  if (oldGwNdIterator == m_gwDevices.end ())
    {
      NS_FATAL_ERROR ("Unknown GW with MAC address " << oldGateway);
    }

  std::map<Address, Ptr<NetDevice> >::iterator newGwNdIterator = m_gwDevices.find (newGateway);
  if (newGwNdIterator == m_gwDevices.end ())
    {
      NS_FATAL_ERROR ("Unknown GW with MAC address " << newGateway);
    }

  uint32_t utIfIndex = utNdIterator->second->GetIfIndex ();
  Ptr<Ipv4L3Protocol> utProtocol = utNdIterator->second->GetNode ()->GetObject<Ipv4L3Protocol> ();
  Ipv4Address utIpAddress = utProtocol->GetAddress (utIfIndex, 0).GetLocal ();

  Ptr<Node> oldGatewayNode = oldGwNdIterator->second->GetNode ();
  uint32_t oldIfIndex = oldGwNdIterator->second->GetIfIndex ();
  Ptr<Node> newGatewayNode = newGwNdIterator->second->GetNode ();
  uint32_t newIfIndex = newGwNdIterator->second->GetIfIndex ();

  Ptr<ArpCache> arpCache;
  // Clear old ARP cache
  arpCache = oldGatewayNode->GetObject<Ipv4L3Protocol> ()->GetInterface (oldIfIndex)->GetArpCache ();
  for (ArpCache::Entry *entry : arpCache->LookupInverse (ut))
    {
      arpCache->Remove (entry);
    }
  // Add entry in new ARP cache
  arpCache = newGatewayNode->GetObject<Ipv4L3Protocol> ()->GetInterface (newIfIndex)->GetArpCache ();
  ArpCache::Entry *entry = arpCache->Add (utIpAddress);
  entry->SetMacAddress (ut);
  entry->MarkPermanent ();

  // Change routes on GW
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  if (oldGatewayNode == newGatewayNode)
    {
      // intra-GW handover
      Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (oldGatewayNode->GetObject<Ipv4L3Protocol> ());

      // purge old routes
      for (uint32_t routeIndex = routing->GetNRoutes (); routeIndex > 0; --routeIndex)
        {
          // Note: keeping routeIndex 1-off because we are using unsigned values
          if (routing->GetRoute (routeIndex - 1).GetGateway () == utIpAddress)
            {
              routing->RemoveRoute (routeIndex - 1);
            }
        }

      // add new ones
      for (uint32_t ifIndex = 1; ifIndex < utProtocol->GetNInterfaces (); ++ifIndex)
        {
          Ipv4Address address = utProtocol->GetAddress (ifIndex, 0).GetLocal ();
          Ipv4Mask mask = utProtocol->GetAddress (ifIndex, 0).GetMask ();

          if (ifIndex == utIfIndex)
            {
              mask = Ipv4Mask ("/32");
            }

          routing->AddNetworkRouteTo (address.CombineMask (mask), mask, utIpAddress, newIfIndex);
        }
    }
  else
    {
      // inter-GW handover
      Ptr<Ipv4StaticRouting> routing = ipv4RoutingHelper.GetStaticRouting (oldGatewayNode->GetObject<Ipv4L3Protocol> ());
      Ptr<Ipv4StaticRouting> routingRouter = ipv4RoutingHelper.GetStaticRouting (m_router->GetObject<Ipv4L3Protocol> ());

      // purge old routes
      for (uint32_t routeIndex = routing->GetNRoutes (); routeIndex > 0; --routeIndex)
        {
          // Note: keeping routeIndex 1-off because we are using unsigned values
          Ipv4RoutingTableEntry gwRoute = routing->GetRoute (routeIndex - 1);
          if (gwRoute.GetGateway () == utIpAddress)
            {
              routing->RemoveRoute (routeIndex - 1);
              // search for corresponding route on terrestrial router
              for (uint32_t routerIndex = 0; routerIndex < routingRouter->GetNRoutes (); ++routerIndex)
                {
                  Ipv4RoutingTableEntry route = routingRouter->GetRoute (routerIndex);
                  if (route.GetDestNetwork () == gwRoute.GetDestNetwork () && route.GetDestNetworkMask () == gwRoute.GetDestNetworkMask ())
                    {
                      routingRouter->RemoveRoute (routerIndex);
                      break;
                    }
                }
            }
        }

      // add new ones
      Ptr<Ipv4L3Protocol> gwProtocol = newGatewayNode->GetObject<Ipv4L3Protocol> ();

      // start by looking up GW IP as seen by the terrestrial router
      Ipv4Address gwAddress;
      for (uint32_t ifIndex = 1; ifIndex < gwProtocol->GetNInterfaces (); ++ifIndex)
        {
          Ptr<NetDevice> gwNd = gwProtocol->GetNetDevice (ifIndex);
          if (gwNd->GetInstanceTypeId ().GetName () != "ns3::SatNetDevice")
            {
              gwAddress = gwProtocol->GetAddress (ifIndex, 0).GetLocal ();
              break;
            }
        }

      // find interface on the terrestrial router to send messages to GW
      uint32_t routingIfIndex = routingRouter->GetNRoutes ();
      for (uint32_t routeIndex = 0; routeIndex < routingRouter->GetNRoutes (); ++routeIndex)
        {
          Ipv4RoutingTableEntry route = routingRouter->GetRoute (routeIndex);
          if (route.GetGateway () == gwAddress)
            {
              routingIfIndex = route.GetInterface ();
              break;
            }
        }

      NS_ASSERT_MSG (routingIfIndex != routingRouter->GetNRoutes (), "Couldn't find interface on the terrestrial router to the new gateway.");

      // add routes to the new GW and the terrestrial router
      routing = ipv4RoutingHelper.GetStaticRouting (gwProtocol);
      for (uint32_t ifIndex = 1; ifIndex < utProtocol->GetNInterfaces (); ++ifIndex)
        {
          Ipv4Address address = utProtocol->GetAddress (ifIndex, 0).GetLocal ();
          Ipv4Mask mask = utProtocol->GetAddress (ifIndex, 0).GetMask ();

          if (ifIndex == utIfIndex)
            {
              mask = Ipv4Mask ("/32");
            }

          routing->AddNetworkRouteTo (address.CombineMask (mask), mask, utIpAddress, newIfIndex);
          routingRouter->AddNetworkRouteTo (address.CombineMask (mask), mask, gwAddress, routingIfIndex);
        }
    }
}

} // namespace ns3
