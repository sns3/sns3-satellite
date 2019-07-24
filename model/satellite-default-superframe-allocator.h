/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Joaquin Muguerza <jmuguerza@viveris.fr>
 */

#ifndef SAT_DEFAULT_SUPERFRAME_ALLOCATOR_H
#define SAT_DEFAULT_SUPERFRAME_ALLOCATOR_H

#include "ns3/simple-ref-count.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "ns3/satellite-frame-conf.h"
#include "satellite-control-message.h"
#include "satellite-frame-allocator.h"
#include "satellite-enums.h"
#include "satellite-superframe-allocator.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief helper class for Satellite Beam Scheduler.
 *
 * SatDefaultSuperframeAllocator class is used by SatBeamScheduler to maintain information
 * of the pre-allocated symbols per Capacity Category (CC) in frame.
 * It also knows physical constrains of the frames.
 *
 * SatDefaultSuperframeAllocator is created and used by SatBeamScheduler.
 *
 */
class SatDefaultSuperframeAllocator : public SatSuperframeAllocator
{
public:
  /**
   * \brief Construct SatDefaultSuperframeAllocator
   * \param superFrameConf Super frame configuration
   */
  SatDefaultSuperframeAllocator (Ptr<SatSuperframeConf> superFrameConf);

  /**
   * Destruct SatDefaultSuperframeAllocator
   */
  ~SatDefaultSuperframeAllocator ();


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
   * \brief Reserve minimum rate from the allocator. This method is called to perform CAC functionality.
   *
   * \param minimumRateBytes Minimum rate based bytes needed to reserve
   * \param controlSlotsEnabled Flag indicating if control slot generation is enabled
   */
  void ReserveMinimumRate (uint32_t minimumRateBytes, bool controlSlotsEnabled);

  /**
   * \brief Preallocate symbols for given to UTs in superframe.
   * Pre-allocation is done in fairly manner between UTs and RCs.
   */
  void PreAllocateSymbols (SatFrameAllocator::SatFrameAllocContainer_t& allocReqs);

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
  void GenerateTimeSlots (SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer, uint32_t maxSizeInBytes, SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer,
                          TracedCallback<uint32_t> waveformTrace, TracedCallback<uint32_t, uint32_t> utLoadTrace, TracedCallback<uint32_t, double> loadTrace);

private:
  /**
   * Container for SatFrameInfo items.
   */
  typedef std::vector< Ptr<SatFrameAllocator> > FrameAllocatorContainer_t;

  /**
   * Container for the supported SatFrameAllocator (frames).
   */
  typedef std::map<Ptr<SatFrameAllocator>, uint32_t> SupportedFramesMap_t;

  // Frame info container.
  FrameAllocatorContainer_t    m_frameAllocators;

  // target load for the frame
  double  m_targetLoad;

  // flag telling if FCA (free capacity allocation) is on
  bool  m_fcaEnabled;

  // minimum carrier payload in bytes
  uint32_t  m_minCarrierPayloadInBytes;

  // minimum rate based bytes left can been guaranteed by frame allocator
  uint32_t  m_minimumRateBasedBytesLeft;

  // The flag telling if time slot generation is done per RC based symbols
  // instead of UT based symbols
  bool m_rcBasedAllocationEnabled;

  // the most robust
  uint32_t m_mostRobustSlotPayloadInBytes;

  /**
   *  Allocate given request according to type.
   *
   * \param ccLevel CC level of the request
   * \param allocReq Requested bytes
   * \param frames Information of the possibles frames to allocate.
   * \return
   */
  bool AllocateBasedOnCc (SatFrameAllocator::CcLevel_t ccLevel, SatFrameAllocator::SatFrameAllocReq * allocReq, const SupportedFramesMap_t &frames);

  /**
   * Allocate a request to a frame.
   *
   * \param allocReq  Allocation request parameters for RC/CCs
   * \return true when allocation is successful, false otherwise
   */
  bool AllocateToFrame (SatFrameAllocator::SatFrameAllocReq * allocReq);

  /**
   * Remove allocations from all frames maintained by frame allocator.
   */
  void RemoveAllocations ();
};

} // namespace ns3

#endif /* SAT_DEFAULT_SUPERFRAME_ALLOCATOR_H */
