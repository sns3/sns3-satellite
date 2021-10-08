/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef SAT_LORA_PHY_RX_H
#define SAT_LORA_PHY_RX_H

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"

#include "ns3/satellite-phy-rx.h"

namespace ns3 {

/**
 * Structure to collect all parameters that are used to compute the duration of
 * a packet (excluding payload length).
 */
struct LoraTxParameters
{
  uint8_t sf = 7;     //!< Spreading Factor
  bool headerDisabled = 0;     //!< Whether to use implicit header mode
  uint8_t codingRate = 1;     //!< Code rate (obtained as 4/(codingRate+4))
  double bandwidthHz = 125000;     //!< Bandwidth in Hz
  uint32_t nPreamble = 8;     //!< Number of preamble symbols
  bool crcEnabled = 1;     //!< Whether Cyclic Redundancy Check is enabled
  bool lowDataRateOptimizationEnabled = 0;     //!< Whether Low Data Rate Optimization is enabled
};

/**
 * \ingroup lorawan
 *
 * Class adding methods linked to Lora, needed to be used in a satellite context
 */
class SatLoraPhyRx : public SatPhyRx
{
public:
  enum State
  {
    /**
     * The PHY layer is sleeping.
     * During sleep, the device is not listening for incoming messages.
     */
    SLEEP,

    /**
     * The PHY layer is in STANDBY.
     * When the PHY is in this state, it's listening to the channel, and
     * it's also ready to transmit data passed to it by the MAC layer.
     */
    STANDBY,

    /**
     * The PHY layer is sending a packet.
     * During transmission, the device cannot receive any packet or send
     * any additional packet.
     */
    TX,

    /**
     * The PHY layer is receiving a packet.
     * While the device is locked on an incoming packet, transmission is
     * not possible.
     */
    RX
  };

  // TypeId
  static TypeId GetTypeId (void);

  /**
   * Constructor and destructor
   */
  SatLoraPhyRx ();
  virtual ~SatLoraPhyRx ();

  /**
   * Start receiving a packet.
   *
   * This method is typically called by LoraChannel.
   *
   * \param packet The packet that is arriving at this PHY layer.
   * \param rxPowerDbm The power of the arriving packet (assumed to be constant
   * for the whole reception).
   * \param sf The Spreading Factor of the arriving packet.
   * \param duration The on air time of this packet.
   * \param frequencyMHz The frequency this packet is being transmitted on.
   */
  void StartReceive (Ptr<Packet> packet, double rxPowerDbm,
                             uint8_t sf, Time duration,
                             double frequencyMHz);

  /**
   * Finish reception of a packet.
   *
   * This method is scheduled by StartReceive, based on the packet duration. By
   * passing a LoraInterferenceHelper Event to this method, the class will be
   * able to identify the packet that is being received among all those that
   * were registered as interference by StartReceive.
   *
   * \param packet The received packet.
   */
  void EndReceive (Ptr<Packet> packet);

  /**
   * Instruct the PHY to send a packet according to some parameters.
   *
   * \param packet The packet to send.
   * \param txParams The desired transmission parameters.
   * \param frequencyMHz The frequency on which to transmit.
   * \param txPowerDbm The power in dBm with which to transmit the packet.
   */
  void Send (Ptr<Packet> packet, LoraTxParameters txParams,
                     double frequencyMHz, double txPowerDbm);

  /**
   * Whether this device is transmitting or not.
   *
   * \returns true if the device is currently transmitting a packet, false
   * otherwise.
   */
  bool IsTransmitting (void);

  /**
   * Whether this device is listening on the specified frequency or not.
   *
   * \param frequency The frequency to query.
   * \returns true if the device is listening on that frequency, false
   * otherwise.
   */
  bool IsOnFrequency (double frequency);

  /**
   * Switch to the STANDBY state.
   */
  void SwitchToStandby (void);

  /**
   * Switch to the SLEEP state.
   */
  void SwitchToSleep (void);

protected:

private:
  /**
   * Switch to the RX state
   */
  void SwitchToRx ();

  /**
   * Switch to the TX state
   */
  void SwitchToTx ();

  // The state this PHY is currently in.
  State m_state;

};

} /* namespace ns3 */

#endif /* SAT_LORA_PHY_RX_H */
