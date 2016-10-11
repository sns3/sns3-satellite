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

#ifndef SATELLITE_PHY_TX_H
#define SATELLITE_PHY_TX_H

#include <map>

#include "ns3/mobility-model.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/satellite-net-device.h"
#include "ns3/mac48-address.h"
#include "satellite-signal-parameters.h"
#include "satellite-antenna-gain-pattern.h"
#include "satellite-mobility-model.h"
#include "satellite-base-fading.h"

namespace ns3 {

class SatChannel;

/**
 * \ingroup satellite
 *
 * \brief The SatellitePhyTx models the physical layer transmitter of satellite system
 *
 */
class SatPhyTx : public Object
{

public:
  typedef enum
  {
    NORMAL,
    TRANSPARENT
  } SatPhyTxMode_t;

  /**
   * Default constructor.
   */
  SatPhyTx ();

  /**
   * Destructor for SatPhyTx
   */
  virtual ~SatPhyTx ();

  /**
   *  PHY states
   */
  enum State
  {
    IDLE, TX
  };


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  void SetChannel (Ptr<SatChannel> c);
  Ptr<SatChannel> GetChannel ();

  void SetMobility (Ptr<MobilityModel> m);
  Ptr<MobilityModel> GetMobility ();

  /*
   * Set the transmit antenna gain pattern.
   * \param agp antenna gain pattern
   */
  void SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  /**
   * Set the maximum Antenna gain in Db
   * \param gain_db maximum antenna gain in Dbs
   */
  void SetMaxAntennaGain_Db (double gain_db);

  /**
   * Get antenna gain based on position
   * or in case that antenna pattern is not configured, maximum configured gain is return
   *
   * \param mobility  Mobility used to get gain from antenna pattern
   * \return antenna gain
   */
  double GetAntennaGain (Ptr<MobilityModel> mobility);

  /**
   * \brief Function for setting the default fading value
   * \param fadingValue default fading value
   */
  void SetDefaultFadingValue (double fadingValue);

  /**
   * \brief Get fading value
   * \param macAddress MAC address
   * \param channelType channel type
   * \return
   */
  double GetFadingValue (Address macAddress, SatEnums::ChannelType_t channelType);

  /**
   * \brief Set fading container
   * \param fadingContainer fading container
   */
  void SetFadingContainer (Ptr<SatBaseFading> fadingContainer);

  /**
  * Start packet transmission to the channel.
  * \param txParams Transmission parameters for a packet
  */
  void StartTx (Ptr<SatSignalParameters> txParams);

  /**
   * Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

private:
  void ChangeState (State newState);
  void EndTx ();

  Ptr<MobilityModel> m_mobility;
  Ptr<SatChannel> m_channel;

  /*
   * Transmit antenna gain pattern
   */
  Ptr<SatAntennaGainPattern> m_antennaGainPattern;

  /**
   * Configured maximum antenna gain in linear
   */
  double m_maxAntennaGain;

  State m_state;
  uint32_t m_beamId;
  SatPhyTxMode_t m_txMode;

  /**
   * \brief Fading container for fading model
   */
  Ptr<SatBaseFading> m_fadingContainer;

  /**
   * \brief Default fading value
   */
  double m_defaultFadingValue;
};


}

#endif /* SATELLITE_PHY_TX_H */
