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

#include <ns3/mobility-model.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/satellite-net-device.h>
#include "ns3/satellite-signal-parameters.h"


namespace ns3 {

class SatChannel;

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

  /**
   *  PHY states
   */
  enum State
  {
    IDLE, RX
  };

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
   * Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

  /** 
   * Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint16_t beamId);


private:
  void ChangeState (State newState);
  void EndRxData ();
  
  Ptr<MobilityModel> m_mobility;
  Ptr<NetDevice> m_device;
  Ptr<Packet> m_packet;

  State m_state;
  Time m_firstRxStart;
  Time m_firstRxDuration;
  uint16_t m_beamId;
  Ptr<SatPhy> m_phy;
};


}

#endif /* SATELLITE_PHY_RX_H */
