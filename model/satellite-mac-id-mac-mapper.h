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
#ifndef SATELLITE_MAC_ID_MAC_MAPPER_H
#define SATELLITE_MAC_ID_MAC_MAPPER_H

#include "satellite-enums.h"
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for MAC to ID to MAC -mapper
 */
class SatMacIdMacMapper : public Object
{
public:

  /**
   * \brief Constructor
   */
  SatMacIdMacMapper ();

  /**
   * \brief Destructor
   */
  ~SatMacIdMacMapper ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

  /**
   * \brief Add the vector containing the values to container matching the key
   * \param mac MAC address
   */
  void AddMacToMapper (Address mac);

  /**
   * \brief
   * \param mac
   * \return
   */
  uint32_t GetId (Address mac);

  /**
   * \brief
   * \param
   * \return
   */
  Address GetMac (uint32_t id);

private:

  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

  /**
   * \brief
   */
  void AddBroadcastMac ();

  /**
   * \brief
   */
  uint32_t m_index;

  /**
   * \brief Map for MAC to ID conversion
   */
  std::map <Address, uint32_t> m_macToIdMap;

  /**
   * \brief Map for ID to MAC conversion
   */
  std::map <uint32_t, Address> m_idToMacMap;
};

} // namespace ns3

#endif /* SATELLITE_MAC_ID_MAC_MAPPER_H */
