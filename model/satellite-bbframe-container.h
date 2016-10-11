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


#ifndef SATELLITE_BBFRAME_CONTAINER_H
#define SATELLITE_BBFRAME_CONTAINER_H

#include <vector>
#include <deque>
#include "ns3/simple-ref-count.h"
#include "satellite-bbframe.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief BB Frame container class.
 * Holds information of BB Frames for forward link scheduling.
 *
 * SatBbFrameContainer implements own queues (container) for every used MODCODs.
 * These MODCOD queues are used for non control packets (priority class different than 0).
 *
 * For control messages (priority class 0) is used only one queue with most robust MODCOD.
 *
 *
 */
class SatBbFrameContainer :  public Object
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor for SatBbFrameContainer not supported.
   */
  SatBbFrameContainer ();

  /**
   * Actual constructor for SatBbFrameContainer.
   * \param modcodsInUse MODCODs in use.
   * \param conf Pointer to BB Frame configuration.
   */
  SatBbFrameContainer (std::vector<SatEnums::SatModcod_t>& modcodsInUse, Ptr<SatBbFrameConf> conf);

  /**
   * Destructor
   */
  virtual ~SatBbFrameContainer ();

  /**
   * Add data according to given priority class and MODCOD to container.
   *
   * \param priorityClass Priority class of the data (packet) to be added
   * \param modcod MODCOD of the data (packet) to be added. MODCOD is ignored when priorityClass is 0.
   * \param data Data (packet) to be added to container
   */
  void AddData (uint32_t priorityClass, SatEnums::SatModcod_t modcod, Ptr<Packet> data);

  /**
   * Get bytes left in last frame of the queue with the given priority class and MODCOD.
   *
   * \param priorityClass Priority class of the queue requested
   * \param modcod MODOCOD of the queue requested. MODCOD is ignored when priorityClass is 0.
   * \return Bytes left in last frame in requested queue.
   */
  uint32_t GetBytesLeftInTailFrame (uint32_t priorityClass, SatEnums::SatModcod_t modcod);

  /**
   * Get maximum payload bytes of a frame with the given priority class and MODCOD.
   *
   * \param priorityClass Priority class of the frame payload requested
   * \param modcod MODOCOD of the queue requested. MODCOD is ignored when priorityClass is 0.
   * \return Maximum payload bytes.
   */
  uint32_t GetMaxFramePayloadInBytes (uint32_t priorityClass, SatEnums::SatModcod_t modcod);

  /**
   * Get maximum MODCOD with the given priority class and C/N0.
   *
   * \param priorityClass Priority class of the MODCOD requested
   * \param cno C/N0 value of the MODCOD requested
   * \return MODCOD fulfills given criteria.
   */
  SatEnums::SatModcod_t GetModcod (uint32_t priorityClass, double cno);

  /**
   * Get next frame from container to transmit.
   * \return BB Frame
   */
  Ptr<SatBbFrame> GetNextFrame ();

  void MergeBbFrames (double carrierBandwidthInHz);

  /**
   * Get total transmission duration of the frames in container.
   * \return Total transmission duration of the frames.
   */
  Time GetTotalDuration () const;

private:
  typedef std::map<SatEnums::SatModcod_t, std::deque<Ptr<SatBbFrame> > > FrameContainer_t;

  std::deque<Ptr<SatBbFrame> >  m_ctrlContainer;
  FrameContainer_t              m_container;
  Time                          m_totalDuration;
  Ptr<SatBbFrameConf>           m_bbFrameConf;
  SatEnums::SatBbFrameType_t    m_defaultBbFrameType;

  /**
   * Trace for merged BB frames.
   * \param BB frame merge to
   * \parma BB frame merge from
   */
  TracedCallback<Ptr<SatBbFrame>, Ptr<SatBbFrame> > m_bbFrameMergeTrace;

  /**
   * Create short or normal frame according to MODCOD and member #m_bbFrameUsageMode.
   * Created frame is added back of appropriate container.
   *
   * \param priorityClass Priority for created frame
   * \param modcod MODCOD for created frame
   */
  void CreateFrameToTail (uint32_t priorityClass, SatEnums::SatModcod_t modcod);
};


} // namespace ns3


#endif /* SATELLITE_BBFRAME_CONTAINER_H */
