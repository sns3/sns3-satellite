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
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 *              Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef LORAWAN_MAC_END_DEVICE_CLASS_A_H
#define LORAWAN_MAC_END_DEVICE_CLASS_A_H

#include <ns3/lorawan-mac.h>                // Packet
#include <ns3/lorawan-mac-end-device.h>     // LorawanMacEndDevice
#include <ns3/lora-frame-header.h>          // RxParamSetupReq
#include <ns3/lora-device-address.h>

namespace ns3 {

/**
 * Class representing the MAC layer of a Class A LoRaWAN device.
 */
class LorawanMacEndDeviceClassA : public LorawanMacEndDevice
{
public:
  static TypeId GetTypeId (void);

  LorawanMacEndDeviceClassA ();
  LorawanMacEndDeviceClassA (uint32_t beamId, Ptr<SatSuperframeSeq> seq);
  virtual ~LorawanMacEndDeviceClassA ();

  /////////////////////
  // Sending methods //
  /////////////////////

  /**
  * Add headers and send a packet with the sending function of the physical layer.
  *
  * \param packet the packet to send
  */
  virtual void SendToPhy (Ptr<Packet> packet);

  //////////////////////////
  //  Receiving methods   //
  //////////////////////////

  /**
   * Receive a packet.
   *
   * This method is typically registered as a callback in the underlying PHY
   * layer so that it's called when a packet is going up the stack.
   *
   * \param packet the received packet.
   */
  virtual void Receive (Ptr<Packet> packet);

  virtual void FailedReception (Ptr<Packet const> packet);

  /**
   * Perform the actions that are required after a packet send.
   *
   * This function handles opening of the first receive window.
   */
  virtual void TxFinished ();

  /**
   *  Set the random access model.
   *
   * \param randomAccessModel The model to use.
   */
  void SetRaModel (SatEnums::RandomAccessModel_t randomAccessModel);

  /**
   * Perform operations needed to open the first receive window.
   */
  void OpenFirstReceiveWindow (void);

  /**
   * Perform operations needed to open the second receive window.
   */
  void OpenSecondReceiveWindow (void);

  /**
   * Perform operations needed to close the first receive window.
   */
  void CloseFirstReceiveWindow (void);

  /**
   * Perform operations needed to close the second receive window.
   */
  void CloseSecondReceiveWindow (void);

  /////////////////////////
  // Getters and Setters //
  /////////////////////////

  /**
   * Find the minimum waiting time before the next possible transmission based
   * on End Device's Class Type.
   *
   * \param waitingTime The minimum waiting time that has to be respected,
   * irrespective of the class (e.g., because of duty cycle limitations).
   */
  virtual Time GetNextClassTransmissionDelay (Time waitingTime);

  /**
   * Get the Data Rate that will be used in the first receive window.
   *
   * \return The Data Rate
   */
  uint8_t GetFirstReceiveWindowDataRate (void);

  /**
   * Set the Data Rate to be used in the second receive window.
   *
   * \param dataRate The Data Rate.
   */
  void SetSecondReceiveWindowDataRate (uint8_t dataRate);

  /**
   * Get the Data Rate that will be used in the second receive window.
   *
   * \return The Data Rate
   */
  uint8_t GetSecondReceiveWindowDataRate (void);

  /**
   * Set the frequency that will be used for the second receive window.
   *
   * \param frequencyMHz the Frequency.
   */
  void SetSecondReceiveWindowFrequency (double frequencyMHz);

  /**
   * Get the frequency that is used for the second receive window.
   *
   * @return The frequency, in MHz
   */
  double GetSecondReceiveWindowFrequency (void);

  /////////////////////////
  // MAC command methods //
  /////////////////////////

  /**
   * Perform the actions that need to be taken when receiving a RxParamSetupReq
   * command based on the Device's Class Type.
   *
   * \param rxParamSetupReq The Parameter Setup Request, which contains:
   *                            - The offset to set.
   *                            - The data rate to use for the second receive window.
   *                            - The frequency to use for the second receive window.
   */
  virtual void OnRxClassParamSetupReq (Ptr<RxParamSetupReq> rxParamSetupReq);

private:

  /**
   * Used superframe sequence for the return link
   */
  Ptr<SatSuperframeSeq> m_superframeSeq;

  /**
   * Packet type used on RTN channel
   */
  SatEnums::PacketType_t m_packetType;

  /**
   * Time to wait between end of message transmission and opening of first reception window
   */
  Time m_firstWindowDelay;

  /**
   * Time to wait between end of message transmission and opening of second reception window
   */
  Time m_secondWindowDelay;

  /**
   * Duration of first reception window
   */
  Time m_firstWindowDuration;

  /**
   * Duration of second reception window
   */
  Time m_secondWindowDuration;

  /**
   * The event of the closing the first receive window.
   *
   * This Event will be canceled if there's a successful reception of a packet.
   */
  EventId m_closeFirstWindow;

  /**
   * The event of the closing the second receive window.
   *
   * This Event will be canceled if there's a successful reception of a packet.
   */
  EventId m_closeSecondWindow;

  /**
   * The event of the second receive window opening.
   *
   * This Event is used to cancel the second window in case the first one is
   * successful.
   */
  EventId m_secondReceiveWindow;

  /**
   * The frequency to listen on for the second receive window.
   */
  double m_secondReceiveWindowFrequency;

  /**
   * The Data Rate to listen for during the second downlink transmission.
   */
  uint8_t m_secondReceiveWindowDataRate;

  /**
   * The RX1DROffset parameter value
   */
  uint8_t m_rx1DrOffset;

}; /* LorawanMacEndDeviceClassA */
} /* namespace ns3 */
#endif /* LORAWAN_MAC_END_DEVICE_CLASS_A_H */
