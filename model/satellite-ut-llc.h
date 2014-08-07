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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_UT_LLC_H_
#define SATELLITE_UT_LLC_H_

#include "satellite-request-manager.h"
#include "satellite-llc.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatUtLlc holds the UT implementation of LLC layer. SatUtLlc is inherited from
 * SatLlc base class and implements the needed changes from the base class related to
 * UT LLC packet transmissions and receptions.
 */
class SatUtLlc : public SatLlc
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatUtLlc
   */
  SatUtLlc ();

  /**
   * Destroy a SatUtLlc
   *
   * This is the destructor for the SatLlc.
   */
  virtual ~SatUtLlc ();

  /**
    *  \brief Called from lower layer (MAC) to inform a Tx
    *  opportunity of certain amount of bytes
    *
    * \param macAddr Mac address of the UT with Tx opportunity
    * \param bytes Size of the Tx opportunity
    * \param rcIndex RC index
    * \param &bytesLeft Bytes left after TxOpportunity
    * \param &nextMinTxO Minimum TxO after this TxO
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint8_t rcIndex, uint32_t &bytesLeft, uint32_t &nextMinTxO);

  /**
   * \brief Set a request manager for UT's LLC instance.
   * \param rm Ptr to request manager
   */
  void SetRequestManager (Ptr<SatRequestManager> rm);

  /**
   * \brief Getter for the request manager
   * \return Pointer to SatRequestManager
   */
  Ptr<SatRequestManager> GetRequestManager () const;

  /**
   * \brief Set queue statistics callbacks for each UT packet queue. Callbacks are
   * passed on to request manager.
   */
  void SetQueueStatisticsCallbacks ();

  /**
   * \brief Method checks how many packets are smaller or equal in size than the
   * maximum packets size threshold specified as an argument. Note, that each
   * queue is gone through from the front up until there is first packet larger
   * than threshold.
   * \param maxPacketSize Maximum packet size threshold in Bytes
   * \return uint32_t Number of packets
   */
  virtual uint32_t GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const;

  /**
   * \brief Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

protected:

  void DoDispose ();

private:

  /**
   * Request manager handling the capacity requests
   */
  Ptr<SatRequestManager> m_requestManager;

};

} // namespace ns3


#endif /* SATELLITE_UT_LLC_H_ */
