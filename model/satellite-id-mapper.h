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

#include "satellite-enums.h"
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for ID -mapper
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

  /** ATTACH TO MAPS */

  /**
   * \brief Attach MAC address to the Trace ID maps and give it a running trace ID
   * \param mac MAC address
   */
  void AttachMacToTraceId (Address mac);

  /**
   * \brief Attach MAC address to the UT ID maps and give it a running UT ID
   * \param mac MAC address
   */
  void AttachMacToUtId (Address mac);

  /**
   * \brief Attach MAC address to the beam ID maps
   * \param mac MAC address
   * \param beamID beam ID
   */
  void AttachMacToBeamId (Address mac, uint32_t beamId);

  /**
   * \brief Attach MAC address to the GW ID maps
   * \param mac MAC address
   * \param gwID GW ID
   */
  void AttachMacToGwId (Address mac, uint32_t gwId);

  /** ID GETTERS */

  /**
   * \brief Function for getting the trace ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return Trace ID
   */
  int32_t GetTraceIdWithMac (Address mac);

  /**
   * \brief Function for getting the UT ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return UT ID
   */
  int32_t GetUtIdWithMac (Address mac);

  /**
   * \brief Function for getting the beam ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return beam ID
   */
  int32_t GetBeamIdWithMac (Address mac);

  /**
   * \brief Function for getting the GW ID with MAC. Returns -1 if the MAC is not in the map
   * \param mac MAC address
   * \return GW ID
   */
  int32_t GetGwIdWithMac (Address mac);

  /**
   * \brief Function for printing out the trace map
   */
  void PrintTraceMap ();

  /**
   * \brief Function for getting the IDs related to a MAC address in an info string
   * \param mac MAC address
   * \return info string
   */
  std::string GetMacInfo (Address mac);

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
   * \brief Map for MAC to trace ID conversion
   */
  std::map <Address, uint32_t> m_macToTraceIdMap;

  /**
   * \brief Map for MAC to UT ID conversion
   */
  std::map <Address, uint32_t> m_macToUtIdMap;

  /**
   * \brief Map for MAC to beam ID conversion
   */
  std::map <Address, uint32_t> m_macToBeamIdMap;

  /**
   * \brief Map for MAC to GW ID conversion
   */
  std::map <Address, uint32_t> m_macToGwIdMap;

  /**
   * \brief Is map printing enabled or not
   */
  bool m_enableMapPrint;
};

} // namespace ns3

#endif /* SATELLITE_ID_MAPPER_H */
