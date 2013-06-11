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


#ifndef SATELLITE_PHY_RX_CARRIER_H
#define SATELLITE_PHY_RX_CARRIER_H

#include "ns3/object.h"

#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatPhyRxCarrier models the physical layer receiver of satellite system. There
 * are one SatPhyRxCarrier receiver for each carrier in both forward and return links.
 *
 */

class SatPhyRxCarrier : public Object
{
public:
  SatPhyRxCarrier (uint16_t carrierId);
  virtual ~SatPhyRxCarrier ();

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
   * Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);


private:
  void ChangeState (State newState);
  void EndRxData ();

  Ptr<SatPhy> m_phy;

  State m_state;
  Ptr<SatSignalParameters> m_rxParams;
  uint16_t m_beamId;
  uint16_t m_carrierId;

};

}

#endif /* SATELLITE_PHY_RX_CARRIER_H */
