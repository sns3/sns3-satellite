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
    NETWORK_TYPE_SAT_SIMPLE, NETWORK_TYPE_CSMA
  };

  /**
   * Define UT user container
   */
  typedef std::map<Ptr<Node>, NodeContainer>    UtUsersContainer_t;

  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  TypeId GetInstanceTypeId (void) const;
  /**
   * Create a SatUserHelper to make life easier when creating Users and their connections to satellite network.
   */
  SatUserHelper ();
  virtual ~SatUserHelper ()
  {
  }

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
  void SetUtBaseAddress (const Ipv4Address& network, const Ipv4Mask& mask, Ipv4Address base = "0.0.0.1");

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
  void SetGwBaseAddress (const Ipv4Address& network, const Ipv4Mask& mask, Ipv4Address base = "0.0.0.1");

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
  NodeContainer InstallGw (NodeContainer gw, uint32_t users);

  /**
   * \return A container having all GW user nodes in satellite network.
   */
  NodeContainer GetGwUsers () const;

  /**
   * Check if node is GW user or not.
   *
   * \param node Pointer to node checked if it is GW user or not
   * \return true when requested node is GW user node, false in other case.
   */
  bool IsGwUser (Ptr<Node> node ) const;

  /**
   * \return A container having all UT user nodes in satellite network.
   */
  NodeContainer GetUtUsers () const;

  /**
   * \param ut  Pointer to UT node, which user nodes are requested.
   * \return A container having UT specific user nodes in satellite network.
   */
  NodeContainer GetUtUsers (Ptr<Node> ut) const;

  /**
   * \return number of GW users in satellite network.
   */
  uint32_t GetGwUserCount () const;

  /**
   * \return number of all UT users in satellite network.
   */
  uint32_t GetUtUserCount () const;

  /**
   * \param ut Pointer to UT node, which user node count is requested.
   * \return number of UT specific users in satellite network.
   */
  uint32_t GetUtUserCount (Ptr<Node> ut) const;

  /**
   * \param utUserNode Pointer to the UT user node
   * \return a pointer to the UT node which serves the specified UT user node,
   *         or zero if the UT user node is invalid
   */
  Ptr<Node> GetUtNode (Ptr<Node> utUserNode) const;

  /**
   * Enables creation traces to be written in given file
   *
   * \param stream  stream for creation trace outputs
   * \param cb  callback to connect traces
   */
  void EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

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
   * \param c node container having UT and its users
   * \return container of the installed net devices
   */
  NetDeviceContainer InstallSubscriberNetwork (const NodeContainer &c ) const;

  /**
   * Install network between GW and Router (or users) or Router and its users.
   *
   * \param c node container having UT and its users
   * \return container of the installed net devices
   */
  NetDeviceContainer InstallBackboneNetwork (const NodeContainer &c ) const;

  /**
   * Install satellite simple network.
   *
   * \param c node container having UT and its users
   * \return container of the installed net devices
   */
  NetDeviceContainer InstallSatSimpleNetwork (const NodeContainer &c ) const;

  /**
   * Install IP router to to Gateways. Creates csma link between gateways and router.
   *
   * \param gw      container having GWs
   * \param router  pointer to IP router
   */
  void InstallRouter (NodeContainer gw, Ptr<Node> router);

  CsmaHelper        m_csma;
  Ipv4AddressHelper m_ipv4Ut;
  Ipv4AddressHelper m_ipv4Gw;

  NodeContainer       m_gwUsers;
  UtUsersContainer_t  m_utUsers;
  NodeContainer       m_allUtUsers;

  NetworkType       m_backboneNetworkType;
  NetworkType       m_subscriberNetworkType;

  Ptr<Node>         m_router;

  /**
   * \brief Container of UT users and their corresponding UT.
   *
   * The data structure is a map which each key is a pointer to the UT user
   * node. The corresponding value is a pointer to the UT node which serves the
   * corresponding UT user node.
   *
   * The member is redundant, in a sense that #m_utUsers also stores the same
   * set of pointers to UT users. This approach is taken to preserve the
   * ordering of the node pointers in #m_utUsers, which is difficult to be
   * replicated as a map.
   */
  std::map<Ptr<Node>, Ptr<Node> > m_utMap;

  /**
   * Trace callback for creation traces
   */
  TracedCallback<std::string> m_creationTrace;

};

} // namespace ns3

#endif /* SAT_USER_HELPER_H */
