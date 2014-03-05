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
 * \brief SatUtLlc holds the Ut implementation of LLC layer. Inherited from SatLlc.
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
    *  Called from lower layer (MAC) to inform a tx
    *  opportunity of certain amount of bytes
    *
    * \param macAddr Mac address of the UT with tx opportunity
    * \param bytes Size of the Tx opportunity
    * \param rcIndex RC index
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint8_t rcIndex);

  /**
   * Set a request manager for UT's LLC instance.
   * @param rm Ptr to request manager
   */
  void SetRequestManager (Ptr<SatRequestManager> rm);

  /**
   * Getter for the request manager
   * \return Ptr<SatRequestManager>
   */
  Ptr<SatRequestManager> GetRequestManager () const;

  /**
   * Set queue statistics callbacks
   * from SatRequestManager
   * to SatQueue
   */
  void SetQueueStatisticsCallbacks ();

  /**
   * Method checks how many packets are smaller or equal in size than the
   * maximum packets size threshold specified as an argument. Note, that each
   * queue is gone through from the front up until there is first packet larger
   * than threshold.
   * \param maxPacketSize Maximum packet size threshold in Bytes
   * \return uint32_t Number of packets
   */
  virtual uint32_t GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const;

  /**
   * Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

protected:
  /**
   * \brief
   */
  void DoDispose ();

private:

  /**
   * Request manager handling the capacity requests
   */
  Ptr<SatRequestManager> m_requestManager;

};

} // namespace ns3


#endif /* SATELLITE_UT_LLC_H_ */
