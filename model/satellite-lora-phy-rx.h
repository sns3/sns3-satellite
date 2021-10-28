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

#include "ns3/satellite-lora-phy-tx.h"

namespace ns3 {

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
   * Start receiving a packet. Set a few actions then call mother class method.
   *
   * \param rxParams All the info needed.
   */
  virtual void StartRx (Ptr<SatSignalParameters> rxParams);

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

  /**
   * Switch to the RX state
   */
  void SwitchToRx ();

  /**
   * Switch to the TX state
   */
  void SwitchToTx ();

  /**
   * Set the frequency this EndDevice will listen on.
   *
   * Should a packet be transmitted on a frequency different than that the
   * EndDeviceLoraPhy is listening on, the packet will be discarded.
   *
   * \param The frequency [MHz] to listen to.
   */
  void SetFrequency (double frequencyMHz);

  /**
   * Set the Spreading Factor this EndDevice will listen for.
   *
   * The EndDeviceLoraPhy object will not be able to lock on transmissions that
   * use a different SF than the one it's listening for.
   *
   * \param sf The spreading factor to listen for.
   */
  void SetSpreadingFactor (uint8_t sf);

  State GetState ();

protected:

private:

  // The state this PHY is currently in.
  State m_state;

  // The frequency this device is listening on
  double m_frequency;

  // The Spreading Factor this device is listening for
  uint8_t m_sf;

};

} /* namespace ns3 */

#endif /* SAT_LORA_PHY_RX_H */
