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

#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "satellite-scheduling-object.h"
#include "satellite-node-info.h"

namespace ns3 {

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

  virtual ~SatUtScheduler ();

  typedef enum {
    STRICT = 0,
    LOOSE = 1
  } SatCompliancePolicy_t;

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose (void);

  /**
   * Callback to get scheduling contexts from upper layer
   * \param vector of scheduling contexts
   */
  typedef Callback<std::vector< Ptr<SatSchedulingObject> > > SchedContextCallback;

  /**
   * Callback to notify upper layer about Tx opportunity.
   * \param uint32_t payload size in bytes
   * \param Mac48Address address
   * \param uint8_t RC index
   * \return packet Packet to be transmitted to PHY
   */
  typedef Callback< Ptr<Packet>, uint32_t, Mac48Address, uint8_t> TxOpportunityCallback;

  /**
   * Method to set Tx opportunity callback.
    * \param cb callback to invoke whenever a packet has been received and must
    *        be forwarded to the higher layers.
    *
    */
  void SetSchedContextCallback (SatUtScheduler::SchedContextCallback cb);

  /**
   * Method to set Tx opportunity callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetTxOpportunityCallback (SatUtScheduler::TxOpportunityCallback cb);

  /**
   * UT scheduling is responsible of selecting with which RC index to
   * use when requesting packets from higher layer. If RC index is set,
   * then it just utilizes it.
   * \param payloadBytes
   * \param rcIndex RC index as int
   * \param level Compliance level of the scheduling process
   * \return Ptr<Packet> Packet fetched from higher layer
   */
  Ptr<Packet> DoScheduling (uint32_t payloadBytes, int rcIndex = -1, SatCompliancePolicy_t level = LOOSE);

  /**
   * Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

private:

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
   * Node information
   */
  Ptr<SatNodeInfo> m_nodeInfo;
};


} // namespace



#endif /* SATELLITE_UT_SCHEDULER_H_ */
