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
#include "ns3/address.h"
#include "satellite-phy-rx-carrier-conf.h"
#include "satellite-antenna-gain-pattern.h"
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
   * \param  the packet received
   * \param  the id of the beam where packet is from
   */
  typedef Callback<void, Ptr<Packet>, Ptr<SatSignalParameters> > ReceiveCallback;

  /**
   * \param The id of the beam.
   * \param The id (address) of the source
   * \param C/N0 value
   */
  typedef Callback<void, uint32_t, Address, double> CnoCallback;

  /**
   * Default constructor
   */
  SatPhy (void);

  SatPhy ( Ptr<NetDevice> d, Ptr<SatChannel> txCh, Ptr<SatChannel> rxCh, uint32_t beamId);

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
   * Set the transmit antenna gain pattern.
   */
  void SetTxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  /**
   * Set the transmit antenna gain pattern.
   */
  void SetRxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  /**
   * \param carrierConf Carrier configuration class
   */
  void ConfigureRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf);

  /**
   * \brief Set fading container
   * \param fadingContainer fading container
   */
  void SetRxFadingContainer (Ptr<SatBaseFading> fadingContainer);

  /**
   * \brief Set fading container
   * \param fadingContainer fading container
   */
  void SetTxFadingContainer (Ptr<SatBaseFading> fadingContainer);

  /**
   * Set the device address owning this object
   * \param ownAddress address of the device owning this object
   */
   void SetAddress (Mac48Address ownAddress);

  /**
   * Get the SatPhyTx pointer
   * \return a pointer to the SatPhyTx instance
   */
  virtual Ptr<SatPhyTx> GetPhyTx () const;

  /**
   * Get the SatPhyRx pointer
   * \return a pointer to the SatPhyRx instance
   */
  virtual Ptr<SatPhyRx> GetPhyRx () const;

  /**
   * Set the SatPhyTx module
   * \param phyTx Transmitter PHY module
   */
  virtual void SetPhyTx (Ptr<SatPhyTx> phyTx);

  /**
   * Set the SatPhyRx module
   * \param phyTx Receiver PHY module
   */
  virtual void SetPhyRx (Ptr<SatPhyRx> phyRx);

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
   * \param rxParams Packet reception parameters
   */
  void Receive (Ptr<SatSignalParameters> rxParams);

  /**
   *
   * \param beamId Beam id of C/N0 is received
   * \param source Id (address) of the source (sender)
   * \param cno Value of the C/N0
   */
  void CnoInfo (uint32_t beamId, Address source, double cno);

private:

  Ptr<SatPhyTx> m_phyTx;
  Ptr<SatPhyRx> m_phyRx;
  uint32_t m_beamId;

  /**
     * The upper layer package receive callback.
     */
    SatPhy::ReceiveCallback m_rxCallback;

    /**
     * The C/N0 info callback
     */
    SatPhy::CnoCallback m_cnoCallback;

public:

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
   * \brief Default fading value
   */
  double m_defaultFadingValue;
};


}

#endif /* SATELLITE_PHY_H */
