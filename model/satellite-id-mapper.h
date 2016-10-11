/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_ID_MAPPER_H
#define SATELLITE_ID_MAPPER_H

#include <ns3/object.h>
#include <map>

namespace ns3 {


class Node;
class Address;

/**
 * \ingroup satellite
 *
 * \brief Class for ID-mapper. The class enables mapping of a specific
 * MAC-address to UT/GW/user/beam ID. These IDs can be obtained with
 * MAC-address by using the provided functions. It is also possible to
 * obtain the MAC-address with node.
 */
class SatIdMapper : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatIdMapper ();

  /**
   * \brief Destructor
   */
  ~SatIdMapper ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

  /* ATTACH TO MAPS */

  /**
   * \brief Attach MAC address to the Trace ID maps and give it a running
   *        trace ID (starting from 1)
   * \param mac MAC address
   * \return the trace ID assigned to the given MAC address
   */
  uint32_t AttachMacToTraceId (Address mac);

  /**
   * \brief Attach MAC address to the UT ID maps and give it a running
   *        UT ID (starting from 1)
   * \param mac MAC address
   * \return the UT ID assigned to the given MAC address
   */
  uint32_t AttachMacToUtId (Address mac);

  /**
   * \brief Attach MAC address to the UT user ID maps and give it a running
   *        UT user ID (starting from 1)
   * \param mac MAC address
   * \return the UT user ID assigned to the given MAC address
   */
  uint32_t AttachMacToUtUserId (Address mac);

  /**
   * \brief Attach MAC address to the beam ID maps
   * \param mac MAC address
   * \param beamId beam ID
   */
  void AttachMacToBeamId (Address mac, uint32_t beamId);

  /**
   * \brief Attach MAC address to the GW ID maps
   * \param mac MAC address
   * \param gwId GW ID
   */
  void AttachMacToGwId (Address mac, uint32_t gwId);

  /**
   * \brief Attach MAC address to the GW user ID maps and give it a running
   *        GW user ID (starting from 1)
   * \param mac MAC address
   * \return the GW user ID assigned to the given MAC address
   */
  uint32_t AttachMacToGwUserId (Address mac);

  /* ID GETTERS */

  /**
   * \brief Function for getting the trace ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return Trace ID
   */
  int32_t GetTraceIdWithMac (Address mac) const;

  /**
   * \brief Function for getting the UT ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return UT ID
   */
  int32_t GetUtIdWithMac (Address mac) const;

  /**
   * \brief Function for getting the UT user ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return UT user ID
   */
  int32_t GetUtUserIdWithMac (Address mac) const;

  /**
   * \brief Function for getting the beam ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return beam ID
   */
  int32_t GetBeamIdWithMac (Address mac) const;

  /**
   * \brief Function for getting the GW ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return GW ID
   */
  int32_t GetGwIdWithMac (Address mac) const;

  /**
   * \brief Function for getting the GW user ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return GW user ID
   */
  int32_t GetGwUserIdWithMac (Address mac) const;

  /* NODE GETTERS */

  /**
   * \param gwNode pointer to a GW node
   * \return the MAC address of the GW node's first satellite beam network
   *         device, or an invalid address if such device is not found.
   */
  Address GetGwMacWithNode (Ptr<Node> gwNode) const;

  /**
   * \param utNode pointer to a UT node
   * \return MAC address of the UT node's satellite beam network device, or an
   *         invalid address if such device is not found.
   */
  Address GetUtMacWithNode (Ptr<Node> utNode) const;

  /**
   * \param utUserNode pointer to a UT user node
   * \return MAC address of the UT user node's subscriber network device, or an
   *         invalid address if such device is not found.
   */
  Address GetUtUserMacWithNode (Ptr<Node> utUserNode) const;

  /* PRINT RELATED METHODS */

  /**
   * \brief Function for printing out the trace map
   */
  void PrintTraceMap () const;

  /**
   * \brief Function for getting the IDs related to a MAC address in an info string
   * \param mac MAC address
   * \return info string
   */
  std::string GetMacInfo (Address mac) const;

  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

  /**
   * \brief Function for enabling the map prints
   */
  void EnableMapPrint (bool enableMapPrint)
  {
    m_enableMapPrint = enableMapPrint;
  }

private:
  /**
   * \brief Running trace index number
   */
  uint32_t m_traceIdIndex;

  /**
   * \brief Running UT index number
   */
  uint32_t m_utIdIndex;

  /**
   * \brief Running UT user index number
   */
  uint32_t m_utUserIdIndex;

  /**
   * \brief Running GW user index number
   */
  uint32_t m_gwUserIdIndex;

  /**
   * \brief Map for MAC to trace ID conversion
   */
  std::map <Address, uint32_t> m_macToTraceIdMap;

  /**
   * \brief Map for MAC to UT ID conversion
   */
  std::map <Address, uint32_t> m_macToUtIdMap;

  /**
   * \brief Map for MAC to UT user ID conversion
   */
  std::map <Address, uint32_t> m_macToUtUserIdMap;

  /**
   * \brief Map for MAC to beam ID conversion
   */
  std::map <Address, uint32_t> m_macToBeamIdMap;

  /**
   * \brief Map for MAC to GW ID conversion
   */
  std::map <Address, uint32_t> m_macToGwIdMap;

  /**
   * \brief Map for MAC to GW user ID conversion
   */
  std::map <Address, uint32_t> m_macToGwUserIdMap;

  /**
   * \brief Is map printing enabled or not
   */
  bool m_enableMapPrint;
};

} // namespace ns3

#endif /* SATELLITE_ID_MAPPER_H */
