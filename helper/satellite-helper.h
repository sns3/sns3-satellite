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
#ifndef __SATELLITE_HELPER_H__
#define __SATELLITE_HELPER_H__

#include "ns3/satellite.h"
#include <string>

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/deprecated.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/csma-helper.h"
#include "ns3/satellite-ut-helper.h"
#include "ns3/trace-helper.h"

namespace ns3 {

class Queue;
class NetDevice;
class Node;
class Propagation;
class Ipv4Address;

/**
 * \brief Build a set of SatNetDevice objects
 *
 */
class SatHelper
{
public:
  /**
       * \enum values for pre-defined scenarios to be used by helper when building
               satellite network topology base.

       *  - Simple: Simple scenario used as base.
       *  - Larger: Larger scenario used as base.
       *  - Full:   Full scenario used as base.
       */
    enum PREDEFINED_SCENARIO
    {
      None,
      Simple,
      Larger,
      Full
    };

  /**
   * \brief Create a base SatHelper for creating customized Satellite topologies.
   */
  SatHelper ();

  /**
   * \brief Create a pre-defined SatHelper to make life easier when creating Satellite topologies.
   */
  SatHelper (PREDEFINED_SCENARIO scenario);
  virtual ~SatHelper () {}

  /**
   * \param c a set of nodes
   *
   * This method creates a ns3::SatChannel with the
   * attributes configured by SatHelper::SetChannelAttribute,
   * then, for each node in the input container, we create a
   * ns3::SatNetDevice with the requested attributes,
   * a queue for this ns3::NetDevice, and associate the resulting
   * ns3::NetDevice with the ns3::Node and ns3::SatChannel.
   */
  NodeContainer Install (NodeContainer c);

  /**
   * \param a first node
   * \param b second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NodeContainer Install (Ptr<Node> a, Ptr<Node> b);

  /**
   * \param a first node
   * \param bName name of second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NodeContainer Install (Ptr<Node> a, std::string bName);

  /**
   * \param aName Name of first node
   * \param b second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NodeContainer Install (std::string aName, Ptr<Node> b);

  /**
   * \param aNode Name of first node
   * \param bNode Name of second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NodeContainer Install (std::string aNode, std::string bNode);

  /**
   * \param i Index of the user.
   *
   * Returns address of the user.
   */
  Ipv4Address GetUserAddress(uint32_t i);

  /**
   * \param i Index of the user.
   *
   * Returns pointer to user node.
   */
  Ptr<Node>  GetUser(uint32_t i);

private:

  /**
   * Creates satellite objects according to simple scenario.
   */
  void CreateSimpleScenario();

  /**
   * Container having users.
   */
  NodeContainer m_users;

  /**
   * Container having user addresses.
   */
  Ipv4InterfaceContainer m_userAddresses;

};

} // namespace ns3


#endif /* __SATELLITE_HELPER_H__ */

