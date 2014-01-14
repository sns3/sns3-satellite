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
 */

class SatPhyRxCarrier : public Object
{
public:

  /**
   * \brief
   * \param carrierId
   * \param carrierConf
   */
  SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf);

  /**
   * \brief
   */
  virtual ~SatPhyRxCarrier ();

  /**
   * \brief PHY states
   */
  enum State
  {
    IDLE, RX
  };

  /**
   * \brief
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   * \brief
   */
  virtual void DoDispose ();

  /**
  * \brief Set the SatPhy module
  * \param phy PHY module
  */
  void SetPhy (Ptr<SatPhy> phy);

  /**
   * \brief Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

  /**
   * \brief Set the own device MAC address
   * \param ownAddress address of the device owning this object
   */
  void SetAddress (Mac48Address ownAddress);

  /**
   * \brief Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

  /**
   * \brief
   * \param cb
   */
  void SetReceiveCb(SatPhyRx::ReceiveCallback cb);

  /**
   * \brief
   * \param cb
   */
  void SetCnoCb(SatPhyRx::CnoCallback cb);

private:

  /**
   * \brief Boltzmann Constant
   */
  static const double BoltzmannConstant = 1.3806488e-23;

  /**
   * \brief Function for composite SINR output tracing
   * \param cSinr composite SINR
   */
  void DoCompositeSinrOutputTrace (double cSinr);

  /**
   * \brief
   * \param newState
   */
  void ChangeState (State newState);

  /**
   * \brief Function for checking the SINR against the link results
   * \param cSinr composite SINR
   * \return result of the check
   */
  bool CheckAgainstLinkResults (double cSinr);

  /**
   * \brief
   */
  void EndRxData ();

  /**
   * \brief
   * \param rxPowerW
   * \param iPowerW
   * \return
   */
  double CalculateSinr(double rxPowerW, double iPowerW);

  /**
   * \brief
   * \param sinr1
   * \param sinr2
   * \return
   */
  double CalculateCompositeSinr(double sinr1, double sinr2);

  /**
   * \brief
   */
  State m_state;

  /**
   * \brief
   */
  Ptr<SatSignalParameters> m_rxParams;

  /**
   * \brief
   */
  uint32_t m_beamId;

  /**
   * \brief
   */
  uint32_t m_carrierId;

  /**
   * \brief
   * Interference model:
   * - Constant
   * - Per-packet
   * - Traced
   */
  Ptr<SatInterference> m_satInterference;

  /**
   * \brief Interference event
   */
  Ptr<SatInterference::Event> m_interferenceEvent;

  /**
   * \brief Link results used for error modeling
   */
  Ptr<SatLinkResults> m_linkResults;

  /**
   * \brief RX noise temperature in K.
   */
  double m_rxTemperatureK;

  /**
   * \brief RX Bandwidth in Hz
   */
  double m_rxBandwidthHz;

  /**
   * \brief External noise power  system RX noise
   */
  double m_rxExtNoisePowerW;

  /**
   * \brief RX noise
   */
  double m_rxNoisePowerW;

  /**
   * \brief RX Adjacent channel interference
   */
  double m_rxAciIfPowerW;

  /**
   * \brief The upper layer package receive callback.
   */
  SatPhyRx::ReceiveCallback m_rxCallback;

  /**
   * \brief The upper layer C/N0 receive callback.
   */
  SatPhy::CnoCallback m_cnoCallback;

  /**
   * \brief Address of the device owning this object.
   */
  Mac48Address m_ownAddress;

  /**
   * \brief Destination address of the packet in m_rxParams.
   */
  Mac48Address m_destAddress;

  /**
   * \brief Source address of the packet in m_rxParams.
   */
  Mac48Address m_sourceAddress;

  /**
   * \brief Receiving mode.
   */
  SatPhyRxCarrierConf::RxMode m_rxMode;

  /**
   * \brief Error model.
   */
  SatPhyRxCarrierConf::ErrorModel m_errorModel;

  /**
   * \brief Channel type.
   */
  SatEnums::ChannelType_t m_channelType;

  /**
   * \brief Callback to calculate SINR.
   */
  SatPhyRxCarrierConf::SinrCalculatorCallback m_sinrCalculate;

  /**
   * \brief Error rate for constant error model
   */
  double m_constantErrorRate;

  /**
   * \brief The trace source fired for added interference
   *
   * \see class CallBackTraceSource
   */
  TracedCallback< Ptr<SatSignalParameters>, // RX signalling parameters
                  Mac48Address,             // receiver address
                  Mac48Address,             // packet destination address
                  double,                   // interference power
                  double                    // composite SINR
                  >
     m_packetTrace;

  /**
   * \brief Rx start time
   */
  Time m_startRxTime;

  /**
   * \brief Amount of bits to contain a byte
   */
  uint32_t m_bitsToContainByte;

  /**
   * \brief Enable composite SINR output tracing
   */
  bool m_enableCompositeSinrOutputTrace;

  /**
   * \brief A random variable for packet reception
   */
    Ptr<UniformRandomVariable> m_uniformVariable;
};

}

#endif /* SATELLITE_PHY_RX_CARRIER_H */
