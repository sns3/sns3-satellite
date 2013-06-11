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


namespace ns3 {

class SatChannel;
class SatPhyRxCarrier;

/**
 * \ingroup satellite
 *
 * The SatPhyRx models the physical layer receiver of satellite system
 *
 */
class SatPhyRx : public Object
{

public:
  SatPhyRx ();
  virtual ~SatPhyRx ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  void SetChannel (Ptr<SatChannel> c);

  void SetVirtualChannel (Ptr<SatChannel> c);
  Ptr<SatChannel> GetVirtualChannel ();

  void SetMobility (Ptr<MobilityModel> m);
  void SetDevice (Ptr<NetDevice> d);
  Ptr<MobilityModel> GetMobility ();
  Ptr<NetDevice> GetDevice ();

  /**
  * Set the SatPhy module
  * @param phy PHY module
  */
  void SetPhy (Ptr<SatPhy> phy);

  /** 
   * Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint16_t beamId);

  /**
   * Create a sufficient amount of SatPhyRxCarrier instances; one for
   * each carrier in either forward or return links.
   * \param maxRxCarriers Maximum supported SatPhyRxCarrier instances (carriers)
   */
  void ConfigurePhyRxCarriers (uint16_t maxRxCarriers);

  /**
   * Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

private:

  Ptr<MobilityModel> m_mobility;
  Ptr<NetDevice> m_device;

  // A SatPhyRxCarrier object for receiving packets from each carrier
  std::vector< Ptr<SatPhyRxCarrier> > m_rxCarriers;
};


}

#endif /* SATELLITE_PHY_RX_H */
