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

#ifndef SATELLITE_UT_LLC_H
#define SATELLITE_UT_LLC_H

#include <ns3/ptr.h>
#include <ns3/satellite-llc.h>
#include <ns3/satellite-queue.h>

namespace ns3 {

class Packet;
class Address;
class Mac48Address;
class SatRequestManager;
class SatSchedulingObject;
class SatNodeInfo;

/**
 * \ingroup satellite
 * \brief SatUtLlc holds the UT implementation of LLC layer. SatUtLlc is inherited from
 * SatLlc base class and implements the needed changes from the base class related to
 * UT LLC packet transmissions and receptions.
 */
class SatUtLlc : public SatLlc
{
public:

  /**
   * Derived form Object
   */
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
    * \param utAddr MAC address of the UT with Tx opportunity
    * \param bytes Size of the Tx opportunity
    * \param rcIndex RC index
    * \param &bytesLeft Bytes left after TxOpportunity
    * \param &nextMinTxO Minimum TxO after this TxO
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t rcIndex, uint32_t &bytesLeft, uint32_t &nextMinTxO);

  /**
   * \brief Called from higher layer (SatNetDevice) to enque packet to LLC
   *
   * \param packet packet sent from above down to SatMac
   * \param dest Destination MAC address of the packet
   * \param flowId Flow identifier
   * \return Boolean indicating whether the enque operation succeeded
   */
  virtual bool Enque (Ptr<Packet> packet, Address dest, uint8_t flowId);

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
   * \param maxPacketSizeBytes Maximum packet size threshold in Bytes
   * \return uint32_t Number of packets
   */
  virtual uint32_t GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const;

  /**
   * \brief Set the node info
   * \param nodeInfo containing node specific information
   */
  virtual void SetNodeInfo (Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Get the number of (new) bytes at LLC queue for a certain UT. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ).
   * \param utAddress the MAC address that identifies a particular UT node.
   * \return Number of bytes currently queued in the encapsulator(s)
   *         associated with the UT.
   */
  virtual uint32_t GetNBytesInQueue (Mac48Address utAddress) const;

  /**
   * \brief Get the number of (new) packets at LLC queues for a certain UT. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ).
   * \param utAddress the MAC address that identifies a particular UT node.
   * \return Number of packets currently queued in the encapsulator(s)
   *         associated with the UT.
   */
  virtual uint32_t GetNPacketsInQueue (Mac48Address utAddress) const;

  /**
   * \param cb callback to send control messages.
   */
  void SetMacQueueEventCallback (SatQueue::QueueEventCallback cb);

protected:

  /**
   * Dispose of SatUtLLc
   */
  void DoDispose ();

  /**
   * \brief Virtual method to create a new encapsulator 'on-a-need-basis' dynamically.
   * \param key Encapsulator key class
   */
  virtual void CreateEncap (Ptr<EncapKey> key);

  /**
   * \brief Virtual method to create a new decapsulator 'on-a-need-basis' dynamically.
   * \param key Encapsulator key class
   */
  virtual void CreateDecap (Ptr<EncapKey> key);

  /**
   * \brief Create and fill the scheduling objects based on LLC layer information.
   * Scheduling objects may be used at the MAC layer to assist in scheduling.
   * \param output reference to an output vector that will be filled with
   *               pointer to scheduling objects
   */
  virtual void GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const;

private:
  /**
   * Request manager handling the capacity requests
   */
  Ptr<SatRequestManager> m_requestManager;

  /**
   * Callback to send queue events to e.g. MAC layer. Note, that this
   * is not actually used by the LLC but the encapsulators. It is just
   * stored here.
  */
  SatQueue::QueueEventCallback m_macQueueEventCb;

};

} // namespace ns3


#endif /* SATELLITE_UT_LLC_H */
