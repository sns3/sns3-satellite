/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#ifndef SATELLITE_DAMA_ENTRY_H
#define SATELLITE_DAMA_ENTRY_H

#include <map>

#include "satellite-lower-layer-service.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief class for module SatDamaEntry.
 *
 * This SatDamaEntry class holds information of a satellite DAMA entry.
 * It's is created and used by NCC.
 *
 */
class SatDamaEntry
{
public:
  /**
   * Construct a SatDamaEntry
   *
   * This is the constructor for the SatDamaEntry
   *
   */
  SatDamaEntry ();

  /**
   * Destroy a SatDamaEntry
   *
   * This is the destructor for the SatDamaEntry.
   */
  ~SatDamaEntry ();

private:

  /**
   * UT identifier (MAC address)
   */
  Address   m_macAddress;
  uint32_t  m_dynamicRatePersistence;
  uint32_t  m_volumeBacklogPersistence;
  uint32_t  m_defaultControlRandomizationInterval;
  Ptr<SatLowerLayerServiceConf> llsConf;
};

} // namespace ns3

#endif /* SATELLITE_DAMA_ENTRY_H */
