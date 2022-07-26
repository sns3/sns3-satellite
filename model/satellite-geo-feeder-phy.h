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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *         Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_GEO_FEEDER_PHY_H
#define SATELLITE_GEO_FEEDER_PHY_H

#include <queue>

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/address.h>

#include "satellite-phy.h"
#include "satellite-signal-parameters.h"
#include "satellite-frame-conf.h"


namespace ns3 {

class SatPhyRxCarrier;
class SatPhyRxCarrierUplink;
class SatPhyRxCarrierPerSlot;
class SatPhyRxCarrierPerFrame;
class SatPhyRxCarrierPerWindow;


/**
 * \ingroup satellite
 *
 * The SatGeoFeederPhy models the feeder link physical layer of the
 * satellite node.
 */
class SatGeoFeederPhy : public SatPhy
{
public:
  /**
   * Default constructor
   */
  SatGeoFeederPhy (void);

  SatGeoFeederPhy (SatPhy::CreateParam_t& params,
                   SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                   Ptr<SatSuperframeConf> superFrameConf,
                   SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                   SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /**
   * Destructor for SatGeoFeederPhy
   */
  virtual ~SatGeoFeederPhy ();


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoInitialize (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose (void);

  /**
   * \brief Send Pdu to the PHY tx module (for GEO satellite switch packet forwarding)
   * \param rxParams Transmission parameters
   */
  virtual void SendPduWithParams (Ptr<SatSignalParameters> rxParams);

  /**
   * \brief Receives packets from lower layer.
   *
   * \param rxParams Packet reception parameters
   * \param phyError Boolean indicating whether the packet successfully
   * received or not? Note, that this parameter is not used in the GEO satellite,
   * but exists since we are using a general interface defined in the parent
   * class.
   */
  virtual void Receive (Ptr<SatSignalParameters> rxParams, bool phyError);

  /**
   * \brief Geo Feeder specific SINR calculator.
   * Calculate SINR with Geo Feeder PHY specific parameters and given SINR.
   *
   * \param sinr Calculated link specific SINR
   * \return SINR with IM interference
   */
  virtual double CalculateSinr (double sinr);

private:

  /**
   * Send a packet from the queue. Used only in REGENERATION_PHY mode.
   */
  void SendFromQueue ();

  /**
   * Notify a packet has finished being sent. Used only in REGENERATION_PHY mode.
   */
  void EndTx ();

  /**
   * Configured external noise power.
   */
  double m_extNoisePowerDensityDbwHz;

  /**
   * Configured Intermodulation interference in dB.
   */
  double m_imInterferenceCOverIDb;

  /**
   * Intermodulation interference in linear.
   */
  double m_imInterferenceCOverI;

  /**
   * Fixed amplification gain used in RTN link at the satellite.
   */
  double m_fixedAmplificationGainDb;

  /**
   * Regeneration mode on forward link.
   */
  SatEnums::RegenerationMode_t m_forwardLinkRegenerationMode;

  /**
   * Regeneration mode on return link.
   */
  SatEnums::RegenerationMode_t m_returnLinkRegenerationMode;

  /**
   * Simple FIFO queue to avoid collisions on TX in case of REGENERATION_PHY.
   */
  std::queue<Ptr<SatSignalParameters>> m_queue;

  /**
   * Maximum size of FIFO m_queue in bursts.
   */
  uint32_t m_queueSizeMax;

  /**
   * Indicates if a packet is already being sent.
   */
  bool m_isSending;
};

}

#endif /* SATELLITE_GEO_FEEDER_PHY_H */
