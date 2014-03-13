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

#include <deque>
#include "ns3/object.h"
#include "ns3/callback.h"
#include "satellite-queue.h"
#include "satellite-lower-layer-service.h"
#include "satellite-control-message.h"
#include "satellite-enums.h"
#include "satellite-node-info.h"

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
  virtual ~SatRequestManager ();

  void Initialize (Ptr<SatLowerLayerServiceConf> llsConf);

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual void DoDispose ();

  /**
   * Callback to fetch queue statistics
   */
  typedef Callback<SatQueue::QueueStats_t, bool> QueueCallback;

  /**
   * Control msg sending callback
   * \param msg        the message send
   * \param address    Packet destination address
   * \return bool
   */
  typedef Callback<bool, Ptr<SatControlMessage>, const Address& > SendCtrlCallback;

  /**
   * Container for the pending RBDC requests
   * Key = pair of RC index and CAC
   * Value = deque of values
   */
  typedef std::vector<std::deque<uint32_t> > PendingRbdcRequestsContainer_t;

  /**
   * Receive a queue event
   * /param event Queue event from SatQueue
   * /param rcIndex Identifier of the queue
   */
  void ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex);

  /**
   * Set a callback to fetch queue statistics
   * \param cb Callback
   */
  void AddQueueCallback (uint8_t rcIndex, SatRequestManager::QueueCallback cb);

  /**
   * \param cb callback to send control messages.
   */
  void SetCtrlMsgCallback (SatRequestManager::SendCtrlCallback cb);

  /**
   * GW address needed for CR transmission
   */
  void SetGwAddress (Mac48Address address);

  /**
   * Set the node info of this UT
   * \param nodeInfo Node information
   */
  void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * Update C/N0 information from lower layer.
   *
   * The SatUtMac receives C/N0 information of packet receptions from GW
   * to update this information to serving GW periodically.
   *
   * \param beamId  The id of the beam where C/N0 is from.
   * \param gwId  The id of the GW.
   * \param utId  The id (address) of the UT.
   * \param cno Value of the C/N0.
   */
  void CnoUpdated (uint32_t beamId, Address utId, Address gwId, double cno);

  /**
   * Sat UT MAC informs that certain amount of resources have been received
   * in TBTP.
   */
  void AssignedDaResources (uint8_t rcIndex, uint32_t bytes);

  /**
   * Resynchronize
   */
  void ReSynchronizeVbdc ();

private:

  typedef std::map<uint8_t, QueueCallback> CallbackContainer_t;

  /**
   * \brief Periodically check the buffer status and whether
   * a new CR is needed to be sent.
   */
  void DoPeriodicalEvaluation ();

  /**
   * Do evaluation of the buffer status and decide whether or not
   * to send CRs
   */
  void DoEvaluation (bool periodical);

  /**
   * Do RBDC calculation for a RC
   * \param rc Request class index
   * \param stats Queue statistics
   * \return uint32_t Requested bytes
   */
  uint32_t DoRbdc (uint8_t rc, const SatQueue::QueueStats_t stats);

  /**
   * Do VBDC calculation for a RC
   * \param rc Request class index
   * \param stats Queue statistics
   * \param &vbdcBytes Reference to vbdcBytes
   * \return SatCapacityAllocationCategory_t Capacity allocation category
   */
  SatEnums::SatCapacityAllocationCategory_t DoVbdc (uint8_t rc, const SatQueue::QueueStats_t stats, uint32_t &vbdcBytes);

  /**
   * Calculate the pending RBDC requests related to a specific RC
   * \param rc Request class index
   * \return uint32_t Pending sum in kbps or Bytes
   */
  uint32_t GetPendingRbdcSum (uint8_t rc) const;

  /**
   * Update the pending RBDC counters with new request information
   * \param rc Request class index
   * \param value Requested value in kbps or Bytes
   */
  void UpdatePendingRbdcCounters (uint8_t rc, uint32_t value);

  /**
   * Send the capacity request control msg via txCallback to
   * SatNetDevice
   */
  void SendCapacityRequest (Ptr<SatCrMessage> crMsg);

  void Reset ();

  /**
   * The queue enque/deque rate getter callback
   */
  CallbackContainer_t m_queueCallbacks;

  /**
   * Callback to send control messages.
  */
  SendCtrlCallback m_ctrlCallback;

  /**
   * GW address
   */
  Mac48Address m_gwAddress;

  /**
   * The last received C/N0 information from lower layer
   * in linear format.
   */
  double m_lastCno;

  /**
  * Lower layer services conf pointer, which holds the configurations
  * for RCs and capacity allocation categories.
  */
  Ptr<SatLowerLayerServiceConf> m_llsConf;

  /**
   * Interval to do the periodical CR evaluation
   */
  Time m_evaluationInterval;

  /**
   * Round trip time estimate. Used to estimate the amount of
   * capacity requests on the air.
   */
  Time m_rttEstimate;

  /**
   * Maximum values to take into account in estimation of capacity
   * requests on the air. Calculated based on m_rttEstimate and
   * evaluation period.
   */
  uint32_t m_maxPendingCrEntries;

  /**
   * Gain value K utilized for RBDC/VBDC calculation
   */
  double m_gainValueK;

  // Key = RC index
  // Value -> Key   = Time when the request was sent
  // Value -> Value = Requested bitrate or bytes
  PendingRbdcRequestsContainer_t m_pendingRbdcRequests;

  /**
   * Pending VBDC counter for each RC index
   */
  std::vector<uint32_t> m_pendingVbdcCounters;

  /**
   * Periodical VBDC resynchronization timer in superframes.
   */
  uint32_t m_vbdcResynchronizationTimer;
  uint32_t m_vbdcResynchronizationCount;

  /**
   * Node information
   */
  Ptr<SatNodeInfo> m_nodeInfo;

  /**
   * Dedicated assignments received within the previous superframe
   */
  std::vector<uint32_t> m_assignedDaResources;

  /**
   * Trace callback used for CR tracing:
   */
  TracedCallback< Time, Mac48Address, Ptr<SatCrMessage> > m_crTrace;

  /**
   * Traced callbacks for all sent RBDC and VBDC capacity requests.
   * Note, that the RC indices are not identified! Thus, if you have
   * two RC indices using the same CAC, the requests may be mixed up
   * at the receiving side.
   */
  TracedCallback< uint32_t> m_rbdcTrace;
  TracedCallback< uint32_t> m_vbdcTrace;

  static const uint32_t M_RBDC_QUANTIZATION_STEP_SMALL_KBPS = 2;
  static const uint32_t M_RBDC_QUANTIZATION_STEP_LARGE_KBPS = 32;
  static const uint32_t M_RBDC_QUANTIZATION_THRESHOLD_KBPS = 512;

  static const uint32_t M_VBDC_QUANTIZATION_STEP_SMALL = 1;
  static const uint32_t M_VBDC_QUANTIZATION_STEP_LARGE = 16;
  static const uint32_t M_VBDC_QUANTIZATION_THRESHOLD_KBYTES = 255;

};

} // namespace

#endif /* SATELLITE_REQUEST_MANAGER_H_ */
