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
   * Actual construct a SatDamaEntry.
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
   * Get CRA based bytes with given duration.
   *
   * \param durationInSeconds of the transmission in seconds
   * \return Rate based bytes with given duration.
   */
  uint32_t GetCraBasedBytes (double durationInSeconds) const;

  /**
   * Get RBDC based bytes with given duration.
   *
   * \param durationInSeconds of the transmission in seconds
   * \return Rate based bytes with given duration.
   */
  uint32_t GetRbdcBasedBytes (double durationInSeconds) const;

  /**
   * Get VBDC based bytes.
   *
   * \return Volume based bytes with given duration.
   */
  uint32_t GetVbdcBasedBytes () const;

  /**
   * Get current value of the dynamic rate requested.
   *
   * \param index Index of RC, which dynamic rate is requested.
   * \return Current value of the dynamic rate requested in kbps.
   */
  uint16_t GetDynamicRateInKbps (uint8_t index) const;

  /**
   * Update dynamic rate request of a RC.
   *
   * \param index Index of RC, which dynamic rate is updated.
   * \param rateInKbps Dynamic rate value [kbps] for update
   */
  void UpdateDynamicRateInKbps (uint8_t index, uint16_t rateInKbps);

  /**
   * Get current value of the volume backlog requested.
   *
   * \param index Index of RC, which volume backlog  is requested.
   * \return Current value of the volume backlog requested in bytes.
   */
  uint8_t GetVolumeBacklogInBytes (uint8_t index) const;

  /**
   * Update volume backlog request a RC.
   *
   * \param index Index of RC, which volume backlog is updated.
   * \param volumeInBytes Volume backlog value [bytes] for update
   */
  void UpdateVolumeBacklogInBytes (uint8_t index, uint32_t volumeInBytes);

  /**
   * Set volume backlog request a RC.
   *
   * \param index Index of RC, which volume backlog is updated.
   * \param volumeInBytes Volume backlog value [bytes] to set
   */
  void SetVolumeBacklogInBytes (uint8_t index, uint32_t volumeInBytes);

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
  uint8_t                         m_dynamicRatePersistence;
  uint8_t                         m_volumeBacklogPersistence;
  Ptr<SatLowerLayerServiceConf>   m_llsConf;
  std::vector<uint16_t>           m_dynamicRateRequestedInKbps;
  std::vector<uint32_t>           m_volumeBacklogRequestedInBytes;
};

} // namespace ns3

#endif /* SATELLITE_DAMA_ENTRY_H */
