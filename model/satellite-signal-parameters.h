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

#ifndef SATELLITE_SIGNAL_PARAMETERS_H
#define SATELLITE_SIGNAL_PARAMETERS_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "satellite-enums.h"

namespace ns3 {

class SatPhyTx;

/**
* \ingroup satellite
* \brief Actual physical layer transmission container. SatSignalParameters is passed
* through the SatChannel from the transmitter to the receiver. It includes e.g. the packet
* container (BBFrame in FWD link, FPDU in RTN link) as well as all the transmission related
* information (MODCODs, frequency, tx power, etc.).
*/
class SatSignalParameters : public Object
{
public:
  /**
   * \brief Struct for storing the packet specific Tx information
   */
  typedef struct
  {
    SatEnums::PacketType_t packetType;
    SatEnums::SatModcod_t modCod;
    uint32_t fecBlockSizeInBytes;
    SatEnums::SatBbFrameType_t frameType;
    uint32_t waveformId;
    uint32_t crdsaUniquePacketId;
  } txInfo_s;

  /**
   * Buffer for transmissions. Buffer just holds data as pointer to packets.
   * Real length of buffer is simulated by duration of the PDU transmission.
   */
  typedef std::vector< Ptr<Packet> > PacketsInBurst_t;

  /**
   * default constructor
   */
  SatSignalParameters ();

  /**
   * copy constructor
   */
  SatSignalParameters (const SatSignalParameters& p);

  Ptr<SatSignalParameters> Copy ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * The packets being transmitted with this signal i.e.
   * this is transmit buffer including packet pointers.
   */

  PacketsInBurst_t m_packetsInBurst;

  /**
   * The beam for the packet transmission
   */
  uint32_t m_beamId;

  /**
   * The carrier for the packet transmission
   */
  uint32_t m_carrierId;

  /**
   * The carrier center frequency for the packet transmission
   */
  double m_carrierFreq_hz;

  /**
   * The duration of the packet transmission.
   *
   */
  Time m_duration;

  /**
   * The TX power in Watts. Equivalent Isotropically Radiated Power (EIRP).
   *
   */
  double m_txPower_W;

  /**
   * The RX power in Watts.
   *
   */
  double m_rxPower_W;

  /**
   * The SatPhyTx instance that is making the transmission
   */
  Ptr<SatPhyTx> m_phyTx;

  /**
   * Calculated SINR.
   */
  double m_sinr;

  /**
   * The SatChannel instance received the packet.
   */
  SatEnums::ChannelType_t m_channelType;

  /**
   * Transmission information including packet type, modcod and waveform ID
   */
  txInfo_s m_txInfo;

  /**
   * The RX power in the satellite in Watts.
   *
   */
  double m_rxPowerInSatellite_W;

  /**
   * Interference power (I)
   */
  double m_ifPower_W;

  /**
   * Interference power in the satellite (I)
   */
  double m_ifPowerInSatellite_W;

  /**
   * Rx noise power in satellite
   */
  double m_rxNoisePowerInSatellite_W;

  /**
   * ACI interference power in satellite
   */
  double m_rxAciIfPowerInSatellite_W;

  /**
   * RX external noise power in satellite
   */
  double m_rxExtNoisePowerInSatellite_W;

  /**
   * Callback for SINR calculation
   */
  Callback<double, double> m_sinrCalculate;
};


}  // namespace ns3


#endif /* SATELLITE_SIGNAL_PARAMETERS_H */
