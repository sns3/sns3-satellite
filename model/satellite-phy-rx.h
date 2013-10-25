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

#ifndef SATELLITE_PHY_RX_H
#define SATELLITE_PHY_RX_H

#include "ns3/mobility-model.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"

#include "satellite-net-device.h"
#include "satellite-signal-parameters.h"
#include "satellite-antenna-gain-pattern.h"
#include "satellite-mobility-model.h"

namespace ns3 {

class SatChannel;
class SatPhyRxCarrier;
class SatPhyRxCarrierConf;

/**
 * \ingroup satellite
 *
 * The SatPhyRx models the physical layer receiver of satellite system
 *
 */
class SatPhyRx : public Object
{

public:
  SatPhyRx ();
  virtual ~SatPhyRx ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  void SetChannel (Ptr<SatChannel> c);

  void SetVirtualChannel (Ptr<SatChannel> c);
  Ptr<SatChannel> GetVirtualChannel ();

  void SetMobility (Ptr<MobilityModel> m);
  Ptr<MobilityModel> GetMobility ();

  /*
   * Set the receive antenna gain pattern.
   */
  void SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  void SetDevice (Ptr<NetDevice> d);
  Ptr<NetDevice> GetDevice ();

  /**
   * Set the maximum Antenna gain in Db
   * /param gain_Db maximum antenna gain in Dbs
   */
  void SetMaxAntennaGain_Db (double gain_Db);

  /**
   * Get antenna gain based on position
   * or in case that antenna pattern is not configured, maximum configured gain is return
   *
   * /param mobility  Mobility used to get gain from antenna pattern
   */
  double GetAntennaGain (Ptr<MobilityModel> mobility);

  /**
   * Set the Antenna loss in Db
   * /param loss_Db antenna loss in Dbs
   */
  void SetAntennaLoss_Db (double loss_Db);

  /**
   * Get configures RX losses, currently only antenna loss used.
   *
   * /return  Antenna loss in linear
   */
  double GetLosses ();

  /** 
   * Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

  /**
   * \param carrierConf Carrier configuration class
   */
  void ConfigurePhyRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf);

  /**
   * Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

  /**
   * \param packet the packet received
   */
  typedef Callback<void,Ptr<SatSignalParameters> > ReceiveCallback;

  /**
   * Set the upper layer receive callback
   * \param cb receive callback funtion pointer
   */
   void SetReceiveCallback (SatPhyRx::ReceiveCallback cb);

  /**
   * Set the device address owning this object
   * \param ownAddress address of the device owning this object
   */
   void SetAddress (Mac48Address ownAddress);

private:

  Ptr<MobilityModel> m_mobility;
  Ptr<NetDevice> m_device;

  /*
   * Receive antenna gain pattern
   */
  Ptr<SatAntennaGainPattern> m_antennaGainPattern;

  /**
   * Configured maximum antenna gain in linear
   */
  double m_maxAntennaGain;

  /**
   * Configured antenna loss in linear
   */
  double m_antennaLoss;

  // A SatPhyRxCarrier object for receiving packets from each carrier
  std::vector< Ptr<SatPhyRxCarrier> > m_rxCarriers;
};


}

#endif /* SATELLITE_PHY_RX_H */
