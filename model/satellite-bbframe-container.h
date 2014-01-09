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


#ifndef SATELLITE_BBFRAME_CONTAINER_H_
#define SATELLITE_BBFRAME_CONTAINER_H_

#include <vector>
#include "ns3/simple-ref-count.h"
#include "satellite-bbframe.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief BB Frame container class.
 * Holds information of BB Frames for forward link scheduling.
 *
 */
class SatBbFrameContainer :  public SimpleRefCount<SatBbFrameContainer>
{
public:
  /**
   * Default constructor for SatBbFrameContainer not supported.
   */
  SatBbFrameContainer ();

  /**
   * Actual constructor for SatBbFrameContainer.
   * \param priorityClassCount Count of priority classes.
   */
  SatBbFrameContainer (uint32_t priorityClassCount);

  /**
   * Destructor
   */
  virtual ~SatBbFrameContainer ();

  /**
   * Add data to transmit buffer of this BB Frame info
   *
   * \param priorityClass The priority class of the frame to add.
   * \param frame Frame to insert to container
   */
  void AddFrame (uint32_t priorityClass, Ptr<SatBbFrame> frame);

  /**
   * Get next frame from container to transmit.
   * \return BB Frame
   */
  Ptr<SatBbFrame> GetNextFrame ();

  /**
   * Get total transmission duration of the frames in container.
   * \return Total transmission duration of the frames.
   */
  Time GetTotalDuration () const;

private:
  typedef std::multimap<uint32_t, Ptr<SatBbFrame> > FrameContainer_t;

  uint32_t m_priorityClassCount;
  FrameContainer_t m_container;

  Time m_totalDuration;
};


} // namespace ns3


#endif /* SATELLITE_BBFRAME_CONTAINER_H_ */
