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


#ifndef SATELLITE_SCHEDULING_OBJECT_H_
#define SATELLITE_SCHEDULING_OBJECT_H_

#include "ns3/simple-ref-count.h"
#include "ns3/nstime.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatSchedulingObject is on object which passes the LLC/encapsulator
 * layer related information for MAC layer to be used by the scheduler (either
 * at GW by the forward link scheduler or at the UT by the RTN link UT scheduler).
 * One scheduling object holds information from one single LLC encapsulator (MAC
 * address, flow identifier). Scheduler may analyze the data and decide how to
 * divide the MAC Tx opportunity bytes between UTs (MAC address) and flows (flow id).
 */
class SatSchedulingObject :  public SimpleRefCount<SatSchedulingObject>
{
public:
  /**
   * Default constructor
   */
  SatSchedulingObject ();

  /**
   * Default constructor
   * \param addr MAC address of an UT
   * \param bytes Amount of bytes at an encapsulator
   * \param minTxOpportunity Minimum size of the Tx opportunity to be
   *        able to create a packet.
   * \param holDelay Head of line queuing delay
   * \param flowId Flow identifier
   */
  SatSchedulingObject (Mac48Address addr, uint32_t bytes, uint32_t minTxOpportunity, Time holDelay, uint8_t flowId);

  /**
   * Destructor
   */
  virtual ~SatSchedulingObject ();

  /**
   * \brief Get the MAC address of this object
   * \return Mac48Address MAC address
   */
  Mac48Address GetMacAddress () const;

  /**
   * \brief Get buffered bytes of this object
   * \return Buffered bytes
   */
  uint32_t GetBufferedBytes () const;

  /**
   * \brief Get minimum Tx opportunity in bytes
   * \return Minimum Tx opportunity in bytes
   */
  uint32_t GetMinTxOpportunityInBytes () const;

  /**
   * \brief Get the flow identifier of the scheduling object.
   * This may be used as priority as well.
   * \return Flow identifier
   */
  uint8_t GetFlowId () const;

  /**
   * \brief Get HOL delay of the object.
   * \return HOL delay of the object in Time.
   */
  Time GetHolDelay () const;

private:
  Mac48Address m_macAddress;
  uint32_t m_bufferedBytes;
  uint32_t m_minTxOpportunity;
  Time m_holDelay;
  uint8_t m_flowId;
};



} // namespace ns3


#endif /* SATELLITE_SCHEDULING_OBJECT_H_ */
