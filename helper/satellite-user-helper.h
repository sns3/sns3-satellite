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

#ifndef SAT_USER_HELPER_H
#define SAT_USER_HELPER_H

#include <string>
#include <set>
#include <map>
#include <stdint.h>

#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class NetDevice;
class Node;

/**
 * \brief Build a set of user nodes and links channels between user nodes and satellite nodes.
 *
 */
class SatUserHelper : public Object
{
public:
  /**
   * Network types in user networks (subscriber or backbone)
   */
  enum NetworkType
   {
     NETWORK_TYPE_IDEAL, NETWORK_TYPE_CSMA
   };

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  /**
   * Create a SatUserHelper to make life easier when creating Users and their connections to satellite network.
   */
  SatUserHelper ();
  virtual ~SatUserHelper () {}

  /**
   * \brief Set the type and the attribute values to be associated with each
   * Queue object in each CsmaNetDevice created by the helper.
   *
   * \param type the type of queue
   * \param name1 the name of the attribute to set on the queue
   * \param value1 the value of the attribute to set on the queue
   * \param name2 the name of the attribute to set on the queue
   * \param value2 the value of the attribute to set on the queue
   * \param name3 the name of the attribute to set on the queue
   * \param value3 the value of the attribute to set on the queue
   * \param name4 the name of the attribute to set on the queue
   * \param value4 the value of the attribute to set on the queue
   *
   * Set these attributes on each ns3::Queue created
   * by SatUserHelper::Install().
   */
  void SetCsmaQueue (std::string type,
                     std::string name1 = "",
                     const AttributeValue &value1 = EmptyAttributeValue (),
                     std::string name2 = "",
                     const AttributeValue &value2 = EmptyAttributeValue (),
                     std::string name3 = "",
                     const AttributeValue &value3 = EmptyAttributeValue (),
                     std::string name4 = "",
                     const AttributeValue &value4 = EmptyAttributeValue ());

  /**
   * \brief Set an attribute value to be propagated to each CsmaNetDevice
   * object created by the helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::CsmaNetDevice created
   * by SatUserHelper::Install().
   */
  void SetCsmaDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Set an attribute value to be propagated to each CsmaChannel object
   * created by the helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::CsmaChannel created
   * by SatUserHelper::Install().
   */
  void SetCsmaChannelAttribute (std::string name, const AttributeValue &value);

  /**
  * \param network The Ipv4Address containing the initial network number to
  * use for satellite network allocation. The bits outside the network mask are not used.
  *
  * \param mask The Ipv4Mask containing one bits in each bit position of the
  * network number.
  *
  * \param base An optional Ipv4Address containing the initial address used for
  * IP address allocation.  Will be combined (ORed) with the network number to
  * generate the first IP address.  Defaults to 0.0.0.1.
  */
  void SetUtBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

  /**
  * \param network The Ipv4Address containing the initial network number to
  * use for satellite network allocation. The bits outside the network mask are not used.
  *
  * \param mask The Ipv4Mask containing one bits in each bit position of the
  * network number.
  *
  * \param base An optional Ipv4Address containing the initial address used for
  * IP address allocation.  Will be combined (ORed) with the network number to
  * generate the first IP address.  Defaults to 0.0.0.1.
  */
  void SetGwBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

  /**
   * \param ut a set of UT nodes
   * \param users number of users to install for every UT
   *
   * This method creates users with the requested attributes
   * to satellite network and add csma netdevice on them and csma channel between UTs/GW and their users.
   *
   * \return  node container of created users for the UT.
   *
   */
  NodeContainer InstallUt (NodeContainer ut, uint32_t users );

  /**
   * \param ut a UT node
   * \param users number of users to install for the UT
   *
   * This method creates users with the requested attributes
   * to satellite network and add csma netdevice on them and csma channel between UT/GW and their users.
   *
   * \return  node container of created users for the UTs.
   *
   */
  NodeContainer InstallUt (Ptr<Node> ut, uint32_t users );

  /**
   * \param gw a set of GW nodes
   * \param users number of users to install for GWs. If gw has more than one GWs then
   * IP router is added between GWs and users.
   *
   * This method creates users with the requested attributes
   * to satellite network and add csma net device on them and csma channel between
   * GW and their users. In case of more than one GW channel is created between GW and router and
   * between router and users.
   *
   * \return  node container of created users for the GWs (and router).
   *
   */
  NodeContainer InstallGw (NodeContainer gw , uint32_t users);

  /**
   * \return A container having all GW user nodes in satellite network.
   */
  NodeContainer GetGwUsers () const;

  /**
   * \return A container having all UT nodes in satellite network.
   */
  NodeContainer GetUtUsers () const;

  /**
   * \return A pointer to the GW router node in satellite network.
   */
  NodeContainer GetGwRouter () const;

  /**
   * \return number of GW users in satellite network.
   */
  uint32_t GetGwUserCount () const;

  /**
   * \return number of UT users in satellite network.
   */
  uint32_t GetUtUserCount () const;

  /**
   * Enables creation traces to be written in given file
   *
   * \param stream  stream for creation trace outputs
   * \param cb  callback to connect traces
   */
  void EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

  /**
   * Get router information.
   *
   * \return Information of router used between GWs and users.
   */
  std::string GetRouterInfo () const;

  /**
   * \return pointer to the router.
   */
  Ptr<Node> GetRouter () const;

private:

  /**
   * Install network between UT and its users
   *
   * \param c nodecontainer having UT and its users
   * \return container of the installed net devices
   */
  NetDeviceContainer InstallSubscriberNetwork (const NodeContainer &c ) const;

  /**
   * Install network between GW and Router (or users) or Router and its users.
   *
   * \param c nodecontainer having UT and its users
   * \return container of the installed net devices
   */
  NetDeviceContainer InstallBackboneNetwork (const NodeContainer &c ) const;

  /**
   * Install ideal network.
   *
   * \param c nodecontainer having UT and its users
   * \return container of the installed net devices
   */
  NetDeviceContainer InstallIdealNetwork (const NodeContainer &c ) const;

  /**
   * Install IP router to to Gateways. Creates csma link between gateways and router.
   *
   * \param gw      container having GWs
   * \param router  pointer to IP router
   */
  void InstallRouter(NodeContainer gw, Ptr<Node> router);

  CsmaHelper        m_csma;
  Ipv4AddressHelper m_ipv4Ut;
  Ipv4AddressHelper m_ipv4Gw;

  NodeContainer     m_gwUsers;
  NodeContainer     m_utUsers;

  NetworkType       m_backboneNetworkType;
  NetworkType       m_subscriberNetworkType;

  Ptr<Node>         m_router;

  /**
   * Trace callback for creation traces
   */
  TracedCallback<std::string> m_creationTrace;

};

} // namespace ns3

#endif /* SAT_USER_HELPER_H */
