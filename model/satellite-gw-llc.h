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

#ifndef SATELLITE_GW_LLC_H_
#define SATELLITE_GW_LLC_H_

#include "ns3/ptr.h"
#include "satellite-llc.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatGwLlc holds the GW implementation of LLC layer. Inherited from SatLlc.
 */
class SatGwLlc : public SatLlc
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatGwLlc
   */
  SatGwLlc ();

  /**
   * Destroy a SatGwLlc
   *
   * This is the destructor for the SatGwLlc.
   */
  virtual ~SatGwLlc ();

  /**
    *  Called from lower layer (MAC) to inform a tx
    *  opportunity of certain amount of bytes
    *
    * \param macAddr Mac address of the UT with tx opportunity
    * \param bytes Size of the Tx opportunity
    * \param flowId Flow identifier
    * \param &bytesLeft Bytes left after TxOpportunity
    * \return Pointer to packet to be transmitted
    */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint8_t flowId, uint32_t &bytesLeft);

  /**
   * Is control encapsulator already created. Only one control encapsulator
   * for flow id 0 and broadcast address is needed.
   * \return bool Flag indicating whether control encapsulator exists
   */
  virtual bool ControlEncapsulatorCreated () const;

protected:
  /**
   * \brief
   */
  void DoDispose ();

private:


};

} // namespace ns3


#endif /* SATELLITE_GW_LLC_H_ */
