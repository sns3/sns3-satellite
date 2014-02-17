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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_REQUEST_MANAGER_H_
#define SATELLITE_REQUEST_MANAGER_H_

#include "ns3/object.h"
#include "satellite-queue.h"
#include "satellite-lower-layer-service.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatRequestManager analyzes periodically (every superframe) UT's
 * buffers' status and sends Capacity Requests to NCC.
 */
class SatRequestManager : public Object
{
public:

  SatRequestManager ();
  SatRequestManager (Ptr<SatLowerLayerServiceConf> llsConf);
  virtual ~SatRequestManager ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual void DoDispose ();

  /**
   * Callback to fetch queue statistics
   */
  typedef Callback<SatQueue::QueueStats_t, bool> QueueCallback;

  /**
   * \brief Periodically check the buffer status and whether
   * a new CR is needed to be sent.
   */
  void DoPeriodicalEvaluation ();

  /**
   * Receive a queue event
   * /param event Queue event from SatQueue
   * /param rcIndex Identifier of the queue
   */
  void ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint32_t rcIndex);

  /**
   * Set a callback to fetch queue statistics
   * \param cb Callback
   */
  void AddQueueCallback (uint8_t rcIndex, SatRequestManager::QueueCallback cb);

private:

  typedef std::map<uint8_t, QueueCallback> CallbackContainer_t;

  /**
   * Do evaluation of the buffer status and decide whether or not
   * to send CRs
   */
  void DoEvaluation (bool periodical);

  /**
   * The queue enque/deque rate getter callback
   */
  CallbackContainer_t m_queueCallbacks;

  /**
  * Lower layer services conf pointer, which holds the configurations
  * for RCs and capacity allocation categories.
  */
  Ptr<SatLowerLayerServiceConf> m_llsConf;

  /**
   * Interval to do the periodical CR evaluation
   */
  Time m_requestInterval;

  /**
   * RC index to check the queue status. Note, that is is not final
   * implementation.
   */
  uint32_t m_rcIndex;

};

} // namespace

#endif /* SATELLITE_REQUEST_MANAGER_H_ */
