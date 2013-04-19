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

#ifndef SATELLITE_PHY_H
#define SATELLITE_PHY_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>


namespace ns3 {

class SatPhyTx;
class SatPhyRx;
class SatChannel;


/**
 * \ingroup satellite
 *
 * The SatPhy models the physical layer of the satellite system (UT, GW, satellite)
 */
class SatPhy : public Object
{
public:

  /**
   * Default constructor
   */
  SatPhy ();

  virtual ~SatPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoStart (void);
  virtual void DoDispose (void);

   /**
    * Get the SatPhyTx pointer
    * @return a pointer to the SatPhyTx instance
    */
   Ptr<SatPhyTx> GetPhyTx ();

   /**
    * Get the SatPhyRx pointer
    * @return a pointer to the SatPhyRx instance
    */
   Ptr<SatPhyRx> GetPhyRx ();

   /**
    * Set the SatPhyTx module
    * @param phyTx Transmitter PHY module
    */
   void SetPhyTx (Ptr<SatPhyTx> phyTx);

   /**
    * Set the SatPhyRx module
    * @param phyTx Receiver PHY module
    */
   void SetPhyRx (Ptr<SatPhyRx> phyRx);

   /**
    * Set the Tx satellite channel
    * @param c the Tx channel
    */
   Ptr<SatChannel> GetTxChannel ();

  /**
   * Set the Tx power
   * @param pw the transmission power in dBm
   */
  void SetTxPower (double pow);

  /**
   * Get the Tx power
   * @return the transmission power in dBm
   */
  double GetTxPower () const;

  /**
   * Send Pdu to the PHY tx module
   * @param p packet to be sent
   * @param duration the packet transmission duration (from MAC layer)
   */
  virtual void SendPdu (Ptr<Packet> p, Time duration);

  /**
   * Receive PHY PDU
   * @param p Packet to be received
   */
  void ReceivePdu (Ptr<Packet> p);

  /**
   * Set the beamId this PHY is connected with
   *
   * @param beamId Satellite beam id
   */
  void SetBeamId (uint16_t beamId);
  
private:

  Ptr<SatPhyTx> m_phyTx;
  Ptr<SatPhyRx> m_phyRx;
  uint16_t m_beamId;
  double m_txPower;
};


}

#endif /* SATELLITE_PHY_H */
