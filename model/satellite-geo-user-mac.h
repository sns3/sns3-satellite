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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_GEO_USER_MAC_H
#define SATELLITE_GEO_USER_MAC_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/address.h>

#include "satellite-phy.h"
#include "satellite-mac.h"
#include "satellite-geo-user-llc.h"
#include "satellite-signal-parameters.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatGeoUserMac models the user link MAC layer of the
 * satellite node.
 */
class SatGeoUserMac : public SatMac
{
public:
  /**
   * Default constructor
   */
  SatGeoUserMac (void);

  /**
   * Construct a SatGeoUserMac
   *
   * This is the constructor for the SatGeoUserMac
   *
   * \param beamId ID of beam for UT
   * \param forwardLinkRegenerationMode Forward link regeneration mode
   * \param returnLinkRegenerationMode Return link regeneration mode
   */
  SatGeoUserMac (uint32_t beamId,
                 SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                 SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /**
   * Destructor for SatGeoUserMac
   */
  virtual ~SatGeoUserMac ();


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
   * \brief Add new packets to the LLC queue.
   * \param packets Packets to be sent.
   * \param rxParams The parameters associated to these packets.
   */
  virtual void EnquePackets (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams);

  /**
   * Receive packet from lower layer.
   *
   * \param packets Pointers to packets received.
   * \param rxParams The parameters associated to these packets.
   */
  void Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams);

  typedef Callback<void, Ptr<SatSignalParameters>> TransmitUserCallback;

  void SetTransmitUserCallback (TransmitUserCallback cb);

  typedef Callback<void, SatPhy::PacketContainer_t, Ptr<SatSignalParameters>> ReceiveUserCallback;

  void SetReceiveUserCallback (ReceiveUserCallback cb);

protected:
  /**
   * \brief Send packets to lower layer by using a callback
   * \param packets Packets to be sent.
   * \param carrierId ID of the carrier used for transmission.
   * \param duration Duration of the physical layer transmission.
   * \param txInfo Additional parameterization for burst transmission.
   */
  virtual void SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo);

  /**
   * \brief Invoke the `Rx` trace source for each received packet.
   * \param packets Container of the pointers to the packets received.
   */
  virtual void RxTraces (SatPhy::PacketContainer_t packets);

  /**
   * \brief Get the link TX direction. Must be implemented by child clases.
   * \return The link TX direction
   */
  virtual SatEnums::SatLinkDir_t GetSatLinkTxDir ();

  /**
   * \brief Get the link RX direction. Must be implemented by child clases.
   * \return The link RX direction
   */
  virtual SatEnums::SatLinkDir_t GetSatLinkRxDir ();

private:

  /**
   * ID of beam for UT
   */
  uint32_t m_beamId;

  TransmitUserCallback m_txUserCallback;
  ReceiveUserCallback m_rxUserCallback;

  Ptr<SatGeoUserLlc> m_llc;

};

}

#endif /* SATELLITE_GEO_USER_MAC_H */
