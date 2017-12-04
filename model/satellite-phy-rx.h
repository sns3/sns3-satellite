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
#include "satellite-base-fading.h"
#include "ns3/satellite-frame-conf.h"

namespace ns3 {

class SatPhyRxCarrierConf;
class SatPhyRxCarrier;
class SatPhyRxCarrierPerSlot;
class SatPhyRxCarrierPerFrame;
class SatPhyRxCarrierUplink;

/**
 * \ingroup satellite
 *
 * The SatPhyRx models the physical layer receiver of satellite system
 *
 */
class SatPhyRx : public Object
{

public:

  /**
   * Default constructor.
   */
  SatPhyRx ();

  /**
   * Destructor for SatPhyRx
   */
  virtual ~SatPhyRx ();


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  void SetMobility (Ptr<MobilityModel> m);
  Ptr<MobilityModel> GetMobility ();

  /*
   * Set the receive antenna gain pattern.
   * \param agp antenna gain pattern
   */
  void SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  void SetDevice (Ptr<NetDevice> d);
  Ptr<NetDevice> GetDevice ();

  /**
   * Set the maximum Antenna gain in Db
   * \param gain_Db maximum antenna gain in Dbs
   */
  void SetMaxAntennaGain_Db (double gain_Db);

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
   * Set the Antenna loss in Db
   * \param loss_Db antenna loss in Dbs
   */
  void SetAntennaLoss_Db (double loss_Db);

  /**
   * Get configures RX losses, currently only antenna loss used.
   *
   * \return  Antenna loss in linear
   */
  double GetLosses ();

  /**
   * Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

  /**
   * \brief Get beam id of this receiver
   * \return uint32_t beam id
   */
  uint32_t GetBeamId () const;

  /**
   * \param carrierConf Carrier configuration class
   * \param superFrameConf Superframe configuration
   */
  void ConfigurePhyRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatSuperframeConf> superFrameConf);

  /**
   * Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

  /**
   * \param SatSignalParameters containing e.g. the received packet
   * \param boolean indicating whether there was a PHY error
   *
   */
  typedef Callback<void,Ptr<SatSignalParameters>,bool> ReceiveCallback;

  /**
   * \param beam Id
   * \param Id (address) of the source (sender)
   * \param Id (address) of the destination (receiver)
   * \param C/N0 value
   */
  typedef Callback<void, uint32_t, Address, Address, double > CnoCallback;

  /**
   * \param beam Id
   * \param carrier Id
   * \param allocation channel Id
   * \param average normalized offered load
   */
  typedef Callback<void, uint32_t, uint32_t, uint8_t, double > AverageNormalizedOfferedLoadCallback;

  /**
   * Set the upper layer receive callback
   * \param cb receive callback funtion pointer
   */
  void SetReceiveCallback (SatPhyRx::ReceiveCallback cb);

  /**
   * Set C/N0 receiver
   * \param cb receive callback funtion pointer
   */
  void SetCnoCallback (SatPhyRx::CnoCallback cb);

  /**
   * Set average normalized offered load callback
   * \param cb
   */
  void SetAverageNormalizedOfferedLoadCallback (SatPhyRx::AverageNormalizedOfferedLoadCallback cb);

  /**
   * \brief Get MAC address of this PHY/MAC
   * \return Mac48Address MAC address of this PHY
   */
  Mac48Address GetAddress () const;

  /**
   * \brief Set the node info class
   * \param nodeInfo Node information related to this SatPhyRx
   */
  void SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Begin frame end scheduling for processes utilizing frame length as interval
   */
  void BeginFrameEndScheduling ();

private:
  Ptr<MobilityModel> m_mobility;
  Ptr<NetDevice> m_device;

  uint32_t m_beamId;
  Mac48Address m_macAddress;

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

#endif /* SATELLITE_PHY_RX_H */
