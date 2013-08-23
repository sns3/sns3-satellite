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


namespace ns3 {

class SatChannel;

/**
 * \ingroup satellite
 *
 * The SatellitePhyTx models the physical layer transmitter of satellite system
 *
 */
class SatPhyTx : public Object
{

public:
  SatPhyTx ();
  virtual ~SatPhyTx ();

  /**
   *  PHY states
   */
  enum State
  {
    IDLE, TX
  };

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  void SetChannel (Ptr<SatChannel> c);
  Ptr<SatChannel> GetChannel ();

  void SetMobility (Ptr<MobilityModel> m);
  Ptr<MobilityModel> GetMobility ();
  
  /**
   * Get the SatPhy pointer
   * @return a pointer to the SatPhy instance
   */
  Ptr<SatPhy> GetPhy ();

  /**
  * Set the SatPhy module
  * @param phy PHY module
  */
  void SetPhy (Ptr<SatPhy> phy);

  /**
  * Start packet transmission to the channel.
  * \param p Packet to be transmitted
  * \param txParams Transmission parameters for a packet
  */
  void StartTx (Ptr<Packet> p, Ptr<SatSignalParameters> txParams);
  
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

  State m_state;
  uint32_t m_beamId;
};


}

#endif /* SATELLITE_PHY_TX_H */
