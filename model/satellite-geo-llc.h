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

protected:
  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * Simple FIFO encapsulator to store packets.
   */
  Ptr<SatBaseEncapsulator> m_encap;

};

} // namespace ns3


#endif /* SATELLITE_GEO_LLC_H_ */
