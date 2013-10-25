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

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"

#include "satellite-signal-parameters.h"

namespace ns3 {

class SatPhyTx;
class SatPhyRx;
class SatChannel;
class SatMac;

/**
 * \ingroup satellite
 *
 * The SatPhy models the physical layer of the satellite system (UT, GW, satellite)
 */
class SatPhy : public Object
{
public:

  /**
   * \param packet the packet received
   * \param  the id of the beam where packet is from
   */
  typedef Callback<void, Ptr<Packet>, Ptr<SatSignalParameters> > ReceiveCallback;

  /**
   * Default constructor
   */
  SatPhy (void);

  SatPhy (Ptr<SatPhyTx> phyTx, Ptr<SatPhyRx> phyRx, uint32_t beamId, SatPhy::ReceiveCallback cb);

  virtual ~SatPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoStart (void);
  virtual void DoDispose (void);

  /**
   * Initialize phy.
   */
  void Initialize();

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
   * Set the Tx satellite channel
   * \param c the Tx channel
   */
  Ptr<SatChannel> GetTxChannel ();

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

  /**
   * Set the beamId this PHY is connected with
   *
   * \param beamId Satellite beam id
   */
  void SetBeamId (uint32_t beamId);

  /**
   * Receives packets from lower layer (phyRx)
   *
   * @param rxParams Packet reception parameters
   */
  void Receive (Ptr<SatSignalParameters> rxParams);

private:

  Ptr<SatPhyTx> m_phyTx;
  Ptr<SatPhyRx> m_phyRx;
  uint32_t m_beamId;

  /**
   * Calculated EIRP without gain in Watts
   */
  double m_eirpWoGain_W;

  /**
   * Configured maximum RX antenna gain in Dbs
   */
  double m_rxMaxAntennaGain_db;

  /**
   * Configured maximum TX antenna gain in Dbs
   */
  double m_txMaxAntennaGain_db;

  /**
   * Configured maximum TX power in DbWs
   */
  double m_txMaxPower_dbW;

  /**
   * Configured output loss in Dbs
   */
  double m_txOutputLoss_db;

  /**
   * Configured pointing loss in Dbs
   */
  double m_txPointingLoss_db;

  /**
   * Configured OBO loss in Dbs
   */
  double m_txOboLoss_db;

  /**
   * Configured TX antenna loss in Dbs
   */
  double m_txAntennaLoss_db;

  /**
   * Configured RX antenna loss in Dbs
   */
  double m_rxAntennaLoss_db;

  /**
   * The upper layer package receive callback.
   */
  SatPhy::ReceiveCallback m_rxCallback;
};


}

#endif /* SATELLITE_PHY_H */
