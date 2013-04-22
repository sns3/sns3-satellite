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


#ifndef SATELLITE_CHANNEL_H
#define SATELLITE_CHANNEL_H

#include "ns3/satellite-signal-parameters.h"
#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/channel.h>
#include "ns3/traced-callback.h"

namespace ns3 {

class PropagationDelayModel;
class SatPhyRx;

/**
 * \ingroup satellite
 *
 * Satellite channel implementation
 *
 */

class SatChannel : public Channel
{
public:

  SatChannel ();
  virtual ~SatChannel ();
  static TypeId GetTypeId (void);

  typedef std::vector<Ptr<SatPhyRx> > PhyList;

  /**
   * Set the  propagation delay model to be used in the SatChannel
   * \param delay Ptr to the propagation delay model to be used.
   */
  virtual void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);

  /**
   * Used by attached SatPhyTx instances to transmit signals to the channel
   * \param params the parameters of the signals being transmitted
   */
  virtual void StartTx (Ptr<SatSignalParameters> params);

  /**
   * This method is used to attach the receiver entity SatPhyRx instance to a
   * SatChannel instance, so that the SatPhyRx can receive packets sent on that channel.
   * \param phyRx the SatPhyRx instance to be added to the channel as
   * a receiver.
   */
  virtual void AddRx (Ptr<SatPhyRx> phyRx);

  /**
   * This method is used to remove a SatPhyRx instance from a
   * SatChannel instance, e.g. due to a spot-beam handover
   * \param phy the SatPhyRx instance to be removed from the channel.
   */
  virtual void RemoveRx (Ptr<SatPhyRx> phyRx);

  // inherited from Channel
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:
  virtual void DoDispose ();

  /**
   * Used internally to schedule the reception start after the propagation delay.
   *
   * \param rxParams Parameters of the signal being received
   * \param phyRx The receiver SatPhyRx entity
   */
  void StartRx (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * list of SatPhyRx instances attached to
   * the channel
   */
  PhyList m_phyList;

  /**
   * propagation delay model to be used with this channel
   */
  Ptr<PropagationDelayModel> m_propagationDelay;

  /**
     * The trace source for the packet transmission animation events that the
     * device can fire.
     * Arguments to the callback are the packet, transmitting
     * net device, receiving net device, transmission time and
     * packet receipt time.
     *
     * @see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, // Packet being transmitted
                   Ptr<NetDevice>,    // Transmitting NetDevice
                   Ptr<NetDevice>,    // Receiving NetDevice
                   Time,              // Amount of time to transmit the pkt
                   Time               // Last bit receive time (relative to now)
                   > m_txrxPointToPoint;

};


}

#endif /* SATELLITE_CHANNEL_H */




