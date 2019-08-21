/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
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

  /**
   * \brief Set interference power based on packet fragment
   * \param ifPowerPerFragment
   */
  inline void SetInterferencePower (std::vector< std::pair<double, double> > ifPowerPerFragment)
  {
    m_ifPowerPerFragment_W = ifPowerPerFragment;
    m_ifPower_W = ComputeIfPowerFromFragments (m_ifPowerPerFragment_W);
  }

  /**
   * \brief Get interference power (I)
   */
  inline double GetInterferencePower ()
  {
    return m_ifPower_W;
  }

  /**
   * \brief Get interference power per packet fragment
   */
  inline std::vector< std::pair<double, double> > GetInterferencePowerPerFragment ()
  {
    return m_ifPowerPerFragment_W;
  }

  /**
   * \brief Set interference power in satellite based on packet fragment
   * \param ifPowerPerFragment
   */
  inline void SetInterferencePowerInSatellite (std::vector< std::pair<double, double> > ifPowerPerFragment)
  {
    m_ifPowerInSatellitePerFragment_W = ifPowerPerFragment;
    m_ifPowerInSatellite_W = ComputeIfPowerFromFragments (m_ifPowerInSatellitePerFragment_W);
  }

  /**
   * \brief Get interference power in satellite (I)
   */
  inline double GetInterferencePowerInSatellite ()
  {
    return m_ifPowerInSatellite_W;
  }

  /**
   * \brief Get interference power in satellite per packet fragment
   */
  inline std::vector< std::pair<double, double> > GetInterferencePowerInSatellitePerFragment ()
  {
    return m_ifPowerInSatellitePerFragment_W;
  }

private:
  /**
   * Interference power (I)
   */
  double m_ifPower_W;

  /**
   * Interference power in the satellite (I)
   */
  double m_ifPowerInSatellite_W;

  /**
   * Interference power (I) per packet fragment.
   *
   * A pair p represent:
   *  - p.first The percentage of time of the packet the interference is applicable
   *  - p.second The value of the interference for the given amount of time
   *
   * As an example, the following values {(0.25, x), (0.5, y), (0.25, z)}
   * represent the following interferences on the packet:
   *
   *  +---+-----+---+
   *  | x |  y  | z |
   *  +---+-----+---+
   *  0   ¼     ¾   1
   */
  std::vector< std::pair<double, double> > m_ifPowerPerFragment_W;

  /**
   * Interference power in the satellite (I) per packet fragment.
   *
   * See m_ifPowerPerFragment_W for full description
   */
  std::vector< std::pair<double, double> > m_ifPowerInSatellitePerFragment_W;

  /**
   * \brief Compute the interference power value for the whole packet
   * based off of individual values per fragment of packet.
   */
  double ComputeIfPowerFromFragments (const std::vector< std::pair<double, double> >& ifPowerPerFragment);
};


}  // namespace ns3


#endif /* SATELLITE_SIGNAL_PARAMETERS_H */
