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


#ifndef SATELLITE_PHY_RX_CARRIER_H
#define SATELLITE_PHY_RX_CARRIER_H

#include "ns3/object.h"

#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel.h"
#include "satellite-interference.h"
#include "satellite-phy-rx-carrier-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatPhyRxCarrier models the physical layer receiver of satellite system. There
 * are one SatPhyRxCarrier receiver for each carrier in both forward and return links.
 *
 */

class SatPhyRxCarrier : public Object
{
public:

  SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf);
  virtual ~SatPhyRxCarrier ();

  /**
   *  PHY states
   */
  enum State
  {
    IDLE, RX
  };

  // inherited from Object
  static TypeId GetTypeId (void);

  virtual void DoDispose ();

  /**
  * Set the SatPhy module
  * @param phy PHY module
  */

  void SetPhy (Ptr<SatPhy> phy);

  /**
   * Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

  /**
   * Set the own device MAC address
   * \param ownAddress address of the device owning this object
   */
  void SetAddress (Mac48Address ownAddress);

  /**
   * Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   *
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);
  void SetCb(SatPhyRx::ReceiveCallback cb);

private:
  static const double BoltzmannConstant = 1.3806488e-23;

  void ChangeState (State newState);
  void EndRxData ();

  double CalculateSinr(double rxPower_W, double iPower_W);
  double CalculateCompositeSinr(double sinr1_W, double sinr2_W);

  State m_state;
  Ptr<SatSignalParameters> m_rxParams;
  uint32_t m_beamId;
  uint32_t m_carrierId;

  /*
   * Interference model:
   * - Constant
   * - Per-packet
   * - Traced
   */
  Ptr<SatInterference> m_satInterference;

  /*
   * Interference event
   */
  Ptr<SatInterference::Event> m_interferenceEvent;

  /*
   * Link results used for error modeling
   */
  Ptr<SatLinkResults> m_linkResults;

  /*
   * RX noise temperature in Kelvins
   */
  double m_rxTemperature_K;

  /*
   * RX Bandwidth in Hz
   */
  double m_rxBandwidth_Hz;

  /*
   * Other system RX noise
   */
  double m_rxOtherSysNoise_W;

  /*
   * RX noise
   */
  double m_rxNoise_W;

  /**
    * The upper layer package receive callback.
    */
  SatPhyRx::ReceiveCallback m_rxCallback;

  /**
   * Address of the device owning this object.
   */
  Mac48Address m_ownAddress;

  /**
   * Destination address of the packet in m_rxParams.
   */
  Mac48Address m_destAddress;

  /**
   * Receiving mode.
   */
  SatPhyRxCarrierConf::RxMode m_rxMode;

  /**
   * The trace source fired for added interferencies
   *
   * \see class CallBackTraceSource
   */

  TracedCallback< SatChannel::ChannelType,  // Type of the RX channel (link)
                  Mac48Address,             // receiver address
                  Mac48Address,             // packet destination address
                  uint32_t,                 // destination beam
                  double,                   // interference power
                  double,                   // TX power (C)
                  double,                   // SINR
                  double                    // composite SINR
                  >
     m_packetTrace;
};

}

#endif /* SATELLITE_PHY_RX_CARRIER_H */
