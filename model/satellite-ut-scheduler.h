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

#ifndef SATELLITE_UT_SCHEDULER_H_
#define SATELLITE_UT_SCHEDULER_H_

#include <vector>
#include <ns3/object.h>
#include <ns3/callback.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-frame-conf.h>

namespace ns3 {


class Packet;
class SatNodeInfo;
class SatLowerLayerServiceConf;
class SatSchedulingObject;

/**
 * \ingroup satellite
 *
 * \brief Sort metric which sorts a vector available RC indices based on "unallocated load".
 * Unallocated load is a the amount of bytes scheduled for UT which was not indicated
 * by NCC scheduler in TBTP. The UT scheduler tries to obey the scheduling decisions made
 * by NCC, and otherwise it tries to be byte-wise fair.
 * TODO: There possibly would need to be some forgetting factor or sliding
 * window not to remember too old samples.
 */
class SortByMetric
{
public:

  /**
   * Sort a vector available RC indices based on unallocated load.
   * \param m Vector to be sorted.
   */
  SortByMetric (const std::vector<uint32_t> &m)
    : m_cont (m)
  {

  }

  /**
   * Operator overload for function call
   * \param p1 Index of value in vector
   * \param p2 Index of value in vector
   * \return True, if value at first index was smaller than value at second index, otherwise false.
   */
  bool operator() (uint8_t p1, uint8_t p2)
  {
    return m_cont.at (p1) < m_cont.at (p2);
  }
private:
  const std::vector<uint32_t> &m_cont;
};

/**
 * \ingroup satellite
 *
 * The SatUtScheduler is responsible of getting a packet of proper size from higher
 * protocol layer. Two callbacks to LLC layer have been configured:
 * - TxOpportunity callback
 * - Scheduling requests callback
 *
 */
class SatUtScheduler : public Object
{
public:
  /**
   * Default constructor
   */
  SatUtScheduler ();

  /**
   * Used constructor
   * \param lls Lower layer service conf
   */
  SatUtScheduler (Ptr<SatLowerLayerServiceConf> lls);

  /**
   * Destructor
   */
  virtual ~SatUtScheduler ();

  /**
   * Enum describing the wanted scheduler policy.
   * STRICT = UT scheduler schedules only from the given RC index
   * LOOSE = UT scheduler may schedule also from other RC indices if needed
   */
  typedef enum
  {
    STRICT = 0,
    LOOSE = 1
  } SatCompliancePolicy_t;

  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);
  /**
   * Derived from Object
   */
  virtual TypeId GetInstanceTypeId (void) const;
  /**
   * Dispose of SatUtScheduler
   */
  virtual void DoDispose (void);

  /**
   * Callback to get scheduling contexts from upper layer
   * \param vector of scheduling contexts
   */
  typedef Callback<void, std::vector< Ptr<SatSchedulingObject> > &> SchedContextCallback;

  /**
   * Callback to notify upper layer about Tx opportunity.
   * \param   uint32_t payload size in bytes
   * \param   Mac48Address address
   * \param   uint8_t RC index
   * \param   uint32_t& Bytes left
   * \param   uint32_t& Next min TxO
   * \return  packet Packet to be transmitted to PHY
   */
  typedef Callback< Ptr<Packet>, uint32_t, Mac48Address, uint8_t, uint32_t&, uint32_t&> TxOpportunityCallback;

  /**
   * Byte counter container
   * \param uint8_t RC index
   * \param uint32_t Byte counter
   */
  typedef std::vector<uint32_t> ByteCounterContainer_t;

  /**
   * \brief Method to set Tx opportunity callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   *
   */
  void SetSchedContextCallback (SatUtScheduler::SchedContextCallback cb);

  /**
   * \brief Method to set Tx opportunity callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetTxOpportunityCallback (SatUtScheduler::TxOpportunityCallback cb);

  /**
   * \brief UT scheduling is responsible of selecting with which RC index to
   * use when requesting packets from higher layer. If RC index is set,
   * then it just utilizes it.
   * \param   packets Vector of packets to be sent in a time slot
   * \param   payloadBytes Maximum payload of a time slot
   * \param   type Time slot type
   * \param   rcIndex RC index
   * \param   policy Compliance policy of the scheduling process
   * \return  Ptr<Packet> Packet fetched from higher layer
   */
  void DoScheduling (std::vector<Ptr<Packet> > &packets, uint32_t payloadBytes, SatTimeSlotConf::SatTimeSlotType_t type, uint8_t rcIndex, SatCompliancePolicy_t policy);

  /**
   * \brief Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

private:
  /**
   * \brief Do scheduling for a given RC index
   * \param packets       Reference to a vector of packets to be sent
   * \param payloadBytes  Payload bytes available for this time slot
   * \param rcIndex       RC index to be scheduled
   * \return              Scheduled bytes
   */
  uint32_t DoSchedulingForRcIndex (std::vector<Ptr<Packet> > &packets, uint32_t &payloadBytes, uint8_t rcIndex);

  /**
   * \brief Get a prioritized order of the available RC indices for
   * LOOSE policy UT scheduling.
   * \return Vector of RC indices
   */
  std::vector<uint8_t> GetPrioritizedRcIndexOrder ();

  /**
   * The scheduling context getter callback.
   */
  SatUtScheduler::SchedContextCallback m_schedContextCallback;

  /**
   * Callback to notify the txOpportunity to upper layer
   * Returns a packet
   * Attributes: payload in bytes
   */
  SatUtScheduler::TxOpportunityCallback m_txOpportunityCallback;

  /**
   * The configured lower layer service configuration for this UT MAC.
   */
  Ptr<SatLowerLayerServiceConf> m_llsConf;

  /**
   * Strictly prioritize the control message scheduling regardless of the
   * time slot information given from MAC layer.
   */
  bool m_prioritizeControl;

  /**
   * Frame PDU header size. Frame PDU
   */
  uint32_t m_framePduHeaderSizeInBytes;

  /**
   * Node information
   */
  Ptr<SatNodeInfo> m_nodeInfo;

  /**
   * Byte counters for RC indices. The counters are updated, when UT has
   * decided to schedule some other RC index than the given one. Reason
   * for this might be that the given RC index queue is empty or does not
   * contain enough bytes.
   */
  ByteCounterContainer_t m_utScheduledByteCounters;

  /**
   * Available RC indices for scheduling
   */
  std::vector<uint8_t> m_rcIndices;

};


} // namespace



#endif /* SATELLITE_UT_SCHEDULER_H_ */
