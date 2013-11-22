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
 * Author: Sami Rantanen <sami.rantanennen@magister.fi>
 */

#ifndef SATELLITE_UT_PHY_H
#define SATELLITE_UT_PHY_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

namespace ns3 {


/**
 * \ingroup satellite
 *
 * The SatUtPhy models the physical layer of the satellite system (UT, GW, satellite)
 */
class SatUtPhy : public SatPhy
{
public:

  /**
   * Default constructor
   */
  SatUtPhy (void);

  SatUtPhy (Ptr<NetDevice> d, Ptr<SatChannel> txCh, Ptr<SatChannel> rxCh, uint32_t beamId);

  virtual ~SatUtPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoStart (void);
  virtual void DoDispose (void);

  /**
       * Get the SatPhyTx pointer
       * \return a pointer to the SatPhyTx instance
       */
      Ptr<SatPhyTx> GetPhyTx () const;

      /**
       * Get the SatPhyRx pointer
       * \return a pointer to the SatPhyRx instance
       */
      Ptr<SatPhyRx> GetPhyRx () const;

      /**
       * Set the SatPhyTx module
       * \param phyTx Transmitter PHY module
       */
      void SetPhyTx (Ptr<SatPhyTx> phyTx);

      /**
       * Set the SatPhyRx module
       * \param phyTx Receiver PHY module
       */
      void SetPhyRx (Ptr<SatPhyRx> phyRx);

  /**
   * Send Pdu to the PHY tx module (for initial transmissions from either UT or GW)
   * \param p packet to be sent
   * \param carrierId Carrier id for the packet transmission
   * \param duration the packet transmission duration (from MAC layer)
   */
  virtual void SendPdu (Ptr<Packet> p, uint32_t carrierId, Time duration);

  /**
   * Send Pdu to the PHY tx module (for GEO satellite switch packet forwarding)
   * \param p packet to be sent
   * \param rxParams Transmission parameters
   */
  virtual void SendPdu (Ptr<Packet> p, Ptr<SatSignalParameters> rxParams);

private:

};

}

#endif /* SATELLITE_UT_PHY_H */
