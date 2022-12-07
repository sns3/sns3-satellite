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
 * Inspired and adapted from Hypatia: https://github.com/snkas/hypatia
 *
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_ISL_ARBITER_UNICAST_HELPER_H
#define SATELLITE_ISL_ARBITER_UNICAST_HELPER_H

#include <ns3/satellite-isl-arbiter-unicast.h>

namespace ns3 {

class SatIslArbiterUnicastHelper : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Default constructor. Must not be used
   */
  SatIslArbiterUnicastHelper ();

  /**
   * Constructor
   * 
   * \param geoNodes List of all satellite nodes
   * \param isls List of all ISLs
   */
  SatIslArbiterUnicastHelper (NodeContainer geoNodes, std::vector <std::pair <uint32_t, uint32_t>> isls);

  /**
   * Install arbiter on all satellite nodes
   */
  void InstallArbiters ();

  /**
   * Update arbiter on all satellite nodes
   */
  void UpdateArbiters ();

private:
  /**
   * Compute routing tables for all satellite nodes
   */
  std::vector<std::map<uint32_t, uint32_t>> CalculateGlobalState ();

  NodeContainer                                m_geoNodes;  // List of all satellite nodes
  std::vector <std::pair <uint32_t, uint32_t>> m_isls;      // List of all ISLs
};

} // namespace ns3

#endif /* SATELLITE_ISL_ARBITER_UNICAST_HELPER_H */
