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
 * \brief SatRequestManager analyzes periodically or on-a-need-basis UT's buffer status for
 * different RC indices (= queues), and sends Capacity Requests to NCC according to need and
 * lower layer service configuration. Request manager supports currently RBDC, AVBDC and VBDC
 * request classes. However, RBDC and VBDC may not be configured in the same simulation for
 * the same RC index.
 */
class SatRequestManager : public Object
{
public:

  /**
   * Default constructor.
   */
  SatRequestManager ();

  /**
   * Destructor for SatRequestManager
   */
  virtual ~SatRequestManager ();

  void Initialize (Ptr<SatLowerLayerServiceConf> llsConf, Time superFrameDuration);


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * Callback to fetch queue statistics
   */
  typedef Callback<SatQueue::QueueStats_t, bool> QueueCallback;

  /**
   * \brief Control message sending callback
   * \param Ptr<SatControlMessage> The message to be sent
   * \param Address Packet destination address
   * \return boolean indicating whether the send was successful
   */
  typedef Callback<bool, Ptr<SatControlMessage>, const Address& > SendCtrlCallback;

  /**
   * \brief Callback to check whether control msg transmission is possible
   * \return boolean indicating whether control msg transmission is possible
   */
  typedef Callback<bool> CtrlMsgTxPossibleCallback;

  /**
   * Container for the pending RBDC requests
   */
  typedef std::vector<std::deque<std::pair<Time, uint32_t> > > PendingRbdcRequestsContainer_t;

  /**
   * \brief Receive a queue event
   * \param event Queue event from SatQueue
   * \param rcIndex RC identifier of the queue
   */
  void ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex);

  /**
   * \brief Add a callback to fetch queue statistics
   * \param rcIndex RC identifier
   * \param cb Callback
   */
  void AddQueueCallback (uint8_t rcIndex, SatRequestManager::QueueCallback cb);

  /**
   * \brief Set the control message sending callback.
   * \param cb callback to send control messages.
   */
  void SetCtrlMsgCallback (SatRequestManager::SendCtrlCallback cb);

  /**
   * \brief Set the callback to check the possibility of sending a
   * control message.
   * \param cb callback to check whether ctrl message sending is possible.
   */
  void SetCtrlMsgTxPossibleCallback (SatRequestManager::CtrlMsgTxPossibleCallback cb);

  /**
   * \brief Set the GW address needed for CR transmission.
   * \param address GW MAC address
   */
  void SetGwAddress (Mac48Address address);

  /**
   * \brief Set the node info of this UT
   * \param nodeInfo Node information pointer
   */
  void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Update C/N0 information from lower layer.
   *
   * The SatUtMac receives C/N0 information of packet receptions from GW
   * to update this information to serving GW periodically.
   *
   * \param beamId The id of the beam where C/N0 is from.
   * \param utId The id (address) of the UT.
   * \param gwId The id of the GW.
   * \param cno Value of the C/N0.
   */
  void CnoUpdated (uint32_t beamId, Address utId, Address gwId, double cno);

  /**
   * \brief Sat UT MAC informs that certain amount of resources have been received
   * in TBTP.
   * \param rcIndex RC index
   * \param bytes Amount of bytes assigned to this UT in TBTP.
   */
  void AssignedDaResources (uint8_t rcIndex, uint32_t bytes);

  /**
   * \brief Callback signature for `CrTrace` trace source.
   * \param time the current simulation time.
   * \param address the MAC address of the current node.
   * \param message the CR message to be transmitted.
   */
  typedef void (*CapacityRequestTraceCallback)
    (Time time, Mac48Address address, Ptr<SatCrMessage> message);

  /**
   * \brief Callback signature for `CrLogTrace` trace source.
   * \param traceLog A string containing the following information:
   *                    - the current simulation time (in seconds),
   *                    - node ID,
   *                    - RC index,
   *                    - type of capacity allocation category, e.g., RBDC,
   *                      VBDC, or AVBDC,
   *                    - request size (in kbps for RBDC or in bytes for VBDC),
   *                      and
   *                    - queue size (in bytes).
   */
  typedef void (*CapacityRequestTraceLogCallback)
    (std::string traceLog);

  /**
   * \brief Callback signature for `RbdcTrace` trace source.
   * \param requestSize amount of requested size via RBDC (in kbps).
   */
  typedef void (*RbdcTraceCallback)
    (uint32_t requestSize);

  /**
   * \brief Callback signature for `VbdcTrace` trace source.
   * \param requestSize amount of requested size via VBDC (in bytes).
   */
  typedef void (*VbdcTraceCallback)
    (uint32_t requestSize);

  /**
   * \brief Callback signature for `AvbdcTrace` trace source.
   * \param requestSize amount of requested size via AVBDC (in bytes).
   */
  typedef void (*AvbdcTraceCallback)
    (uint32_t requestSize);

private:
  typedef std::map<uint8_t, QueueCallback> CallbackContainer_t;

  /**
   * \brief Periodically check the buffer status and whether
   * a new CR is needed to be sent.
   */
  void DoPeriodicalEvaluation ();

  /**
   * \brief Do evaluation of the buffer status and decide whether or not
   * to send CRs.
   */
  void DoEvaluation ();

  /**
   * \brief Do RBDC calculation for a RC
   * \param rc Request class index
   * \param stats Queue statistics
   * \return uint32_t Requested bytes
   */
  uint32_t DoRbdc (uint8_t rc, const SatQueue::QueueStats_t &stats);

  /**
   * \brief Do VBDC calculation for a RC
   * \param rc Request class index
   * \param stats Queue statistics
   * \param &vbdcBytes Reference to vbdcBytes
   * \return SatCapacityAllocationCategory_t Capacity allocation category
   */
  SatEnums::SatCapacityAllocationCategory_t DoVbdc (uint8_t rc, const SatQueue::QueueStats_t &stats, uint32_t &rcVbdcBytes);

  /**
   * \brief Calculate the needed VBDC bytes for a RC
   * \param rc Request class index
   * \param stats Queue statistics
   * \return Requested VBDC kilobytes
   */
  uint32_t GetVbdcBytes (uint8_t rc, const SatQueue::QueueStats_t &stats);

  /**
   * \brief Calculate the needed AVBDC bytes for a RC
   * \param rc Request class index
   * \param stats Queue statistics
   * \return Requested AVBDC kilobytes
   */
  uint32_t GetAvbdcBytes (uint8_t rc, const SatQueue::QueueStats_t &stats);

  /**
   * Check whether VBDC volume backlog persistence shall expire and
   * whether UT should update request by AVBDC due pending requests.
   */
  void CheckForVolumeBacklogPersistence ();

  /**
   * \brief Calculate the pending RBDC requests related to a specific RC.
   * \param rc Request class index
   * \return Pending sum in kbps or Bytes
   */
  uint32_t GetPendingRbdcSumKbps (uint8_t rc);

  /**
   * \brief Update the pending RBDC counters with new request information
   * \param rc Request class index
   * \param kbps Requested value in kbps or Bytes
   */
  void UpdatePendingRbdcCounters (uint8_t rc, uint32_t kbps);

  /**
   * Clean-up the pending RBDC container from old samples.
   * \param rc Request class index
   */
  void RemoveOldEntriesFromPendingRbdcContainer (uint8_t rc);

  /**
   * Update pending VBDC counters for all RCs
   */
  void UpdatePendingVbdcCounters ();

  /**
   * \brief Update the pending VBDC counters with new request information
   * \param rc Request class index
   */
  void UpdatePendingVbdcCounters (uint8_t rc);

  /**
   * \brief Send the capacity request control msg via txCallback to
   * SatNetDevice
   * \param crMsg Created capacity request
   */
  void SendCapacityRequest (Ptr<SatCrMessage> crMsg);

  /**
   * \brief Send the C/N0 report message via txCallback to SatNetDevice.
   */
  void SendCnoReport ();

  /**
   * \brief Reset the assigned resources counter
   */
  void ResetAssignedResources ();

  /**
   * \brief Reset RC index counters
   * \param rc RC index
   */
  void Reset (uint8_t rc);

  /**
   * The RBDC value is signalled with 8 bits, which means that to be able to signal
   * larger than 256 values, we need to use quantization and coding to convert the
   * raw values into defined discrete values.
   * \param index RC index
   * \param reqRbdcKbps Raw RBDC request
   * \return uint16_t Quantized RBDC value
   */
  uint16_t GetQuantizedRbdcValue (uint8_t index, uint16_t reqRbdcKbps) const;

  /**
   * The RBDC value is signalled with 8 bits, which means that to be able to signal
   * larger than 256 values, we need to use quantization and coding to convert the
   * raw values into defined discrete values.
   * \param index RC index
   * \param reqVbdcBytes Raw VBDC request in Bytes
   * \return uint16_t Quantized VBDC value
   */
  uint16_t GetQuantizedVbdcValue (uint8_t index, uint16_t reqVbdcBytes) const;


  /**
   * The queue enque/deque rate getter callback
   */
  CallbackContainer_t m_queueCallbacks;

  /**
   * Callback to send control messages.
  */
  SendCtrlCallback m_ctrlCallback;

  /**
   * Callback to check from MAC if a control msg may be
   * transmitted in the near future.
   */
  CtrlMsgTxPossibleCallback m_ctrlMsgTxPossibleCallback;

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
   * Interval to send C/N0 report.
   */
  Time m_cnoReportInterval;

  /**
   * Event id for the C/NO report.
   */
  EventId m_cnoReportEvent;

  /**
   * Gain value K for the RBDC calculation
   */
  double m_gainValueK;

  /**
   * Round trip time estimate. Used to estimate the amount of
   * capacity requests on the air.
   */
  Time m_rttEstimate;

  /**
   * Over-estimation factor used for estimating a bit more resources
   * than there are in the buffers. The purpose is to compensate the
   * RLE and FPDU header overhead.
   */
  double m_overEstimationFactor;

  /**
   * Enable on demand / ad hoc CR evaluation.
   */
  bool m_enableOnDemandEvaluation;

  /**
   * Key = RC index
   * Value -> Key   = Time when the request was sent
   * Value -> Value = Requested bitrate or bytes
   */
  PendingRbdcRequestsContainer_t m_pendingRbdcRequestsKbps;

  /**
   * Pending VBDC counter for each RC index
   */
  std::vector<uint32_t> m_pendingVbdcBytes;

  /**
   * Time when CR evaluation was previously done
   */
  std::vector<Time> m_previousEvaluationTime;

  /**
   * Node information
   */
  Ptr<SatNodeInfo> m_nodeInfo;

  /**
   * Dedicated assignments received within the previous superframe
   */
  std::vector<uint32_t> m_assignedDaResourcesBytes;

  /**
   * Time when the last CR including VBDC request was sent
   */
  Time m_lastVbdcCrSent;

  /**
   * Superframe duration used for updating the volume backlog persistence
   */
  Time m_superFrameDuration;

  /**
   * Flag indicating that UT should send a forced AVBDC request, since
   * the volume backlog persistence shall expire and UT still has pending
   * requested bytes.
   */
  bool m_forcedAvbdcUpdate;

  uint32_t m_numValues;
  static const uint32_t m_rbdcScalingFactors [4];
  static const uint32_t m_vbdcScalingFactors [4];

  /**
   * Trace callback used for CR tracing.
   */
  TracedCallback< Time, Mac48Address, Ptr<SatCrMessage> > m_crTrace;

  /**
   * Trace callback used for CR tracing.
   */
  TracedCallback<std::string> m_crTraceLog;

  /**
   * Traced callbacks for all sent RBDC and VBDC capacity requests.
   * Note, that the RC indices are not identified! Thus, if you have
   * two RC indices using the same CAC, the requests may be mixed up
   * at the receiving side.
   */
  TracedCallback< uint32_t> m_rbdcTrace;
  TracedCallback< uint32_t> m_vbdcTrace;
  TracedCallback< uint32_t> m_aVbdcTrace;
};

} // namespace

#endif /* SATELLITE_REQUEST_MANAGER_H_ */
