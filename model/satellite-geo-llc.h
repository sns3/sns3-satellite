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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_GEO_LLC_H_
#define SATELLITE_GEO_LLC_H_

#include "satellite-base-encapsulator.h"
#include "satellite-llc.h"
#include "satellite-node-info.h"


namespace ns3 {

/**
 * \ingroup satellite TODO
 */
class SatGeoLlc : public SatLlc
{
public:
  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Construct a SatGeoLlc
   */
  SatGeoLlc ();

  /**
   * Destroy a SatGeoLlc
   *
   * This is the destructor for the SatGeoLlc.
   */
  virtual ~SatGeoLlc ();

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
    * \brief Called from lower layer (MAC) to inform a Tx
    * opportunity of certain amount of bytes. Note, that this
    * method is not to be used in this class, but the real
    * implementation is located in inherited classes.
    *
    * \param bytes Size of the Tx opportunity
    * \param utAddr MAC address of the UT with the Tx opportunity
    * \param flowId Flow identifier
    * \param &bytesLeft Bytes left after TxOpportunity
    * \param &nextMinTxO Minimum TxO after this TxO
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t flowId, uint32_t &bytesLeft, uint32_t &nextMinTxO);

  /**
   * \brief Receive HL PDU from encapsulator/decapsulator entity
   *
   * \param packet Pointer to packet received.
   * \param source MAC address of the source
   * \param dest MAC address of the destination
   */
  virtual void ReceiveHigherLayerPdu (Ptr<Packet> packet, Mac48Address source, Mac48Address dest);

  /**
   * \brief Create and fill the scheduling objects based on LLC layer information.
   * Scheduling objects may be used at the MAC layer to assist in scheduling.
   * \param output reference to an output vector that will be filled with
   *               pointer to scheduling objects
   */
  virtual void GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const;

  /**
   * \brief Get the number of (new) bytes at LLC queue for a certain UT. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ). This is a pure
   * virtual method to be implemented to inherited classes.
   * \param utAddress the MAC address that identifies a particular UT node.
   * \return Number of bytes currently queued in the encapsulator(s)
   *         associated with the UT.
   */
  virtual uint32_t GetNBytesInQueue (Mac48Address utAddress) const;

  /**
    \brief Get the number of (new) packets at LLC queues for a certain UT. Method
   * checks only the SatQueue for packets, thus it does not count possible
   * packets buffered at the encapsulator (e.g. in case of ARQ). This is a pure
   * virtual method to be implemented to inherited classes.
   * \param utAddress the MAC address that identifies a particular UT node.
   * \return Number of packets currently queued in the encapsulator(s)
   *         associated with the UT.
   */
  virtual uint32_t GetNPacketsInQueue (Mac48Address utAddress) const;

  /**
   * \brief Receive callback used for sending packet to netdevice layer.
    * \param packet the packet received
    */
  typedef Callback<void, Ptr<Packet>, const Address& > ReceiveSatelliteCallback;

  /**
   * \brief Set Receive callback to forward packet to upper layer on satellite
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetReceiveSatelliteCallback (SatGeoLlc::ReceiveSatelliteCallback cb);

protected:
  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Virtual method to create a new encapsulator 'on-a-need-basis' dynamically.
   * Method is implemented in the inherited class which knows which type of encapsulator to create.
   * This is a pure virtual method to be implemented to inherited classes.
   * \param key Encapsulator key class
   */
  virtual void CreateEncap (Ptr<EncapKey> key);

  /**
   * \brief Virtual method to create a new decapsulator 'on-a-need-basis' dynamically.
   * Method is implemented in the inherited class which knows which type of decapsulator to create.
   * This is a pure virtual method to be implemented to inherited classes.
   * \param key Encapsulator key class
   */
  virtual void CreateDecap (Ptr<EncapKey> key);

  /**
   * The upper layer package receive callback.
   */
  ReceiveSatelliteCallback m_rxSatelliteCallback;

};

} // namespace ns3


#endif /* SATELLITE_GEO_LLC_H_ */
