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

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/channel.h"
#include "ns3/traced-callback.h"
#include "ns3/propagation-delay-model.h"
#include "satellite-signal-parameters.h"
#include "satellite-free-space-loss.h"
#include "satellite-phy-rx.h"
#include "satellite-enums.h"

namespace ns3 {

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

  /**
   * \brief
   */
  SatChannel ();

  /**
   * \brief
   */
  virtual ~SatChannel ();

  /**
   * \brief
   * \return
   */
  static TypeId GetTypeId (void);

  typedef std::vector<Ptr<SatPhyRx> > PhyList;

  /**
   * \brief
   * \param channelType     The type of channel
   * \param freqId          The id of the carrier
   * \param carrierId       The id of the carrier
   *
   * \return The center frequency of the carrier.
   */
  typedef Callback<double, SatEnums::ChannelType_t, uint32_t, uint32_t  > CarrierFreqConverter;

  /**
   * \brief Set the  propagation delay model to be used in the SatChannel
   * \param delay Ptr to the propagation delay model to be used.
   */
  virtual void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);

  /**
   * \brief Get the  propagation delay model to be used in the SatChannel
   * \param Ptr to the propagation delay model to be used.
   */
  virtual Ptr<PropagationDelayModel> GetPropagationDelayModel ();

  /**
   * \brief Set the type of the channel.
   *
   * \param chType Type of the channel.
   */
  virtual void SetChannelType (SatEnums::ChannelType_t chType);

  /**
   * \brief Set the frequency id of the channel.
   *
   * \param frequencyId The frequency id of the channel.
   */
  virtual void SetFrequencyId (uint32_t freqId);

  /**
   * \brief Set the frequency converter callback.
   *
   * \param converter The frequency converter callback.
   */
  virtual void SetFrequencyConverter (CarrierFreqConverter converter);

  /**
   * \brief Get the type of the channel.
   * \return Type of the channel.
   */
  virtual SatEnums::ChannelType_t GetChannelType ();

  /**
   * \brief Set the  propagation delay model to be used in the SatChannel
   * \param delay Ptr to the propagation delay model to be used.
   */
  virtual void SetFreeSpaceLoss (Ptr<SatFreeSpaceLoss> delay);

  /**
   * \brief Used by attached SatPhyTx instances to transmit signals to the channel
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

  /**
   *
   * \return
   */
  virtual uint32_t GetNDevices (void) const;

  /**
   *
   *\@param i
   *\@return
   */
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:

  virtual void DoDispose ();

  /**
   * \brief Used internally to schedule the reception start after the propagation delay.
   *
   * \param rxParams Parameters of the signal being received
   * \param phyRx The receiver SatPhyRx entity
   */
  void StartRx (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Function for Rx power output trace
   * \param rxParams Rx parameters
   */
  void DoRxPowerOutputTrace (Ptr<SatSignalParameters> rxParams);

  /**
   * \brief Function for Rx power input trace
   * \param rxParams Rx parameters
   */
  void DoRxPowerInputTrace (Ptr<SatSignalParameters> rxParams);

  /**
   * \brief Function for calculating the Rx power
   * \param rxParams Rx parameters
   * \param phyRx phy Rx
   * \param frequency_hz frequency in Hz
   */
  void DoRxPowerCalculation (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief list of SatPhyRx instances attached to the channel
   */
  PhyList m_phyList;

  /**
   * \brief Type of the channel
   */
  SatEnums::ChannelType_t m_channelType;

  /**
   * \brief Frequency coverter callback.
   */
  CarrierFreqConverter m_carrierFreqConverter;

  /**
   * \brief Freequency id of the channel
   */
  uint32_t m_freqId;

  /**
   * \brief Propagation delay model to be used with this channel
   */
  Ptr<PropagationDelayModel> m_propagationDelay;

  /**
   * \brief Free space loss model to be used with this channel.
   */
  Ptr<SatFreeSpaceLoss> m_freeSpaceLoss;

  /**
   * \brief
   */
  SatEnums::RxPowerCalculationMode_t m_rxPowerCalculationMode;

  /**
   * \brief
   */
  bool m_enableRxPowerOutputTrace;

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




