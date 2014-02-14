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

#include "ns3/simple-ref-count.h"
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
class SatDamaEntry : public SimpleRefCount<SatDamaEntry>
{
public:
  /**
   * Default construct a SatDamaEntry.
   *
   * This version of the constructor shall not used.
   *
   */
  SatDamaEntry ();

  /**
   * Default construct a SatDamaEntry.
   *
   * This version of the constructor shall not used.
   *
   */
  SatDamaEntry (Ptr<SatLowerLayerServiceConf> llsConf);

  /**
   * Destroy a SatDamaEntry
   *
   * This is the destructor for the SatDamaEntry.
   */
  ~SatDamaEntry ();

  /**
   * Get current value of the dynamic rate persistence.
   *
   * \return Current value of the dynamic rate persistence.
   */
  inline uint32_t GetDynamicRatePersistence () const { return m_dynamicRatePersistence;}

  /**
   * Get current value of the volume backlog persistence.
   *
   * \return Current value of the volume backlog persistence.
   */
  inline uint32_t GetVolumeBacklogPersistence () const { return m_volumeBacklogPersistence;}

  /**
   * Reset dynamic rate persistence to the value given in lower layer service configuration.
   */
  void ResetDynamicRatePersistence ();

  /**
   * Decrement dynamic rate persistence.
   */
  void DecrementDynamicRatePersistence ();

  /**
   * Reset volume backlog persistence to the value given in lower layer service configuration.
   */
  void ResetVolumeBacklogPersistence ();

  /**
   * Decrement volume backlog persistence.
   */
  void DecrementVolumeBacklogPersistence ();

private:

  uint32_t                      m_dynamicRatePersistence;
  uint32_t                      m_volumeBacklogPersistence;
  Ptr<SatLowerLayerServiceConf> m_llsConf;
  std::vector<double>           m_dynamicRateRequestedInKbps;
  std::vector<uint32_t>         m_volumeRequestedBytes;
};

} // namespace ns3

#endif /* SATELLITE_DAMA_ENTRY_H */
