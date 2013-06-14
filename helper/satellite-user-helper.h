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
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  /**
   * Create a SatUserHelper to make life easier when creating Users and their connections to satellite network.
   */
  SatUserHelper ();
  virtual ~SatUserHelper () {}

  /**
     * Set an attribute value to be propagated to each CsmaNetDevice created by the
     * helper.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * Set these attributes on each ns3::SatNetDevice created
     * by SatBeamHelper::Install
     */
    void SetCsmaDeviceAttribute (std::string name, const AttributeValue &value);

    /**
     * Set an attribute value to be propagated to each Csma
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * Set these attribute on each ns3::SatChannel created
     * by SatNetDevHelper::Install
     */
    void SetCsmaChannelAttribute (std::string name, const AttributeValue &value);

  /**
  * \param network The Ipv4Address containing the initial network number to
  * use for satellite network allocation. The bits outside the network mask are not used.
  * \param mask The Ipv4Mask containing one bits in each bit position of the
  * network number.
  * \param base An optional Ipv4Address containing the initial address used for
  * IP address allocation.  Will be combined (ORed) with the network number to
  * generate the first IP address.  Defaults to 0.0.0.1.
  * \returns Nothing.
  */
  void SetUtBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

  /**
  * \param network The Ipv4Address containing the initial network number to
  * use for satellite network allocation. The bits outside the network mask are not used.
  * \param mask The Ipv4Mask containing one bits in each bit position of the
  * network number.
  * \param base An optional Ipv4Address containing the initial address used for
  * IP address allocation.  Will be combined (ORed) with the network number to
  * generate the first IP address.  Defaults to 0.0.0.1.
  * \returns Nothing.
  */
  void SetGwBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

  /**
   * \param ut a set of UT nodes
   * \param users number of users to install for every UT
   *
   * This method creates users with the requested attributes
   * to satellite network and add csma netdevice on them and csma channel between UTs/GW and their users.
   *
   */
  NodeContainer InstallUt (NodeContainer ut, uint32_t users );

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
   */
  NodeContainer InstallGw (NodeContainer gw , uint32_t users);

  /**
   * returns A container having all GW user nodes in satellite network.
   */
  NodeContainer GetGwUsers();

  /**
   * returns A container having all UT nodes in satellite network.
   */
  NodeContainer GetUtUsers();

  /**
   * returns number of GW users in satellite network.
   */
  uint32_t GetGwUserN();

  /**
   * returns number of UT users in satellite network.
   */
  uint32_t GetUtUserN();

  /**
   * Enables creation traces to be written in given file
   *
   * /param stream  stream for creation trace outputs
   * /param cb  callback to connect traces
   */
  void EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

private:

    CsmaHelper        m_csma;
    Ipv4AddressHelper m_ipv4Ut;
    Ipv4AddressHelper m_ipv4Gw;

    NodeContainer     m_gwUsers;
    NodeContainer     m_utUsers;

    /**
     * Trace callback for creation traces
     */
    TracedCallback<std::string> m_creation;

    /**
     * Install IP router to to Gateways. Creates csma link between gateways and router.
     *
     * /param gw      container having GWs
     * /param router  pointer to IP router
     */
    void InstallRouter(NodeContainer gw, Ptr<Node> router);
};

} // namespace ns3

#endif /* SAT_USER_HELPER_H */
