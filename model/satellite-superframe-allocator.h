/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 CNES
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
 * Author: Joaquin Muguerza <jmuguerza@viveris.fr>
 */

#ifndef SAT_SUPERFRAME_ALLOCATOR_H
#define SAT_SUPERFRAME_ALLOCATOR_H

#include <ns3/simple-ref-count.h>
#include <ns3/address.h>
#include <ns3/traced-callback.h>

#include "satellite-frame-conf.h"
#include "satellite-control-message.h"
#include "satellite-frame-allocator.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief helper class for Satellite Beam Scheduler.
 *
 * SatSuperframeAllocator class is used by SatBeamScheduler to maintain information
 * of the pre-allocated symbols per Capacity Category (CC) in frame.
 * It also knows physical constrains of the frames.
 *
 * SatSuperframeAllocator is created and used by SatBeamScheduler.
 *
 */
class SatSuperframeAllocator : public Object
{
public:
  /**
   * \brief Construct SatSuperframeAllocator
   * \param superFrameConf Super frame configuration
   */
  SatSuperframeAllocator (Ptr<SatSuperframeConf> superFrameConf);

  /**
   * Destruct SatSuperframeAllocator
   */
  virtual ~SatSuperframeAllocator ();


  /**
   * derived from object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Get super frame duration.
   *
   *  \return Super frame duration.
   **/
  inline Time GetSuperframeDuration () const
  {
    return m_superframeConf->GetDuration ();
  }

  /**
   * \brief Reserve minimum rate from the allocator. This method is called to perform CAC functionality.
   *
   * \param minimumRateBytes Minimum rate based bytes needed to reserve
   * \param controlSlotsEnabled Flag indicating if control slot generation is enabled
   */
  virtual void ReserveMinimumRate (uint32_t minimumRateBytes, bool controlSlotsEnabled) = 0;

  /**
   * \brief Release minimum rate from the allocator. This method is called when a UT leaves the beam using this allocator.
   *
   * \param minimumRateBytes Minimum rate based bytes needed to reserve
   * \param controlSlotsEnabled Flag indicating if control slot generation is enabled
   */
  virtual void ReleaseMinimumRate (uint32_t minimumRateBytes, bool controlSlotsEnabled) = 0;

  /**
   * \brief Preallocate symbols for given to UTs in superframe.
   * Pre-allocation is done in fairly manner between UTs and RCs.
   */
  virtual void PreAllocateSymbols (SatFrameAllocator::SatFrameAllocContainer_t& allocReqs) = 0;

  /**
   * \brief Generate time slots in TBTP(s) for the UT/RC.
   *
   * \param tbtpContainer TBTP message container to add/fill TBTPs.
   * \param maxSizeInBytes Maximum size for a TBTP message.
   * \param utAllocContainer Reference to UT allocation container to fill in info of the allocation
   * \param waveformTrace Wave form trace callback
   * \param utLoadTrace UT load per the frame trace callback
   * \param loadTrace Load per the frame trace callback
   */
  virtual void GenerateTimeSlots (SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer, uint32_t maxSizeInBytes, SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer,
                          TracedCallback<uint32_t> waveformTrace, TracedCallback<uint32_t, uint32_t> utLoadTrace, TracedCallback<uint32_t, double> loadTrace) = 0;

protected:
  // super frame  configuration
  Ptr<SatSuperframeConf>  m_superframeConf;
};

} // namespace ns3

#endif /* SAT_SUPERFRAME_ALLOCATOR_H */
