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
#include "satellite-phy-rx-carrier-conf.h"
#include "satellite-enums.h"
#include "satellite-typedefs.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * Satellite channel implementation. Satellite channel is responsible of
 * passing through packets from the transmitter to the receiver. One SatChannel
 * is mapped into a frequency band, which means that there may be several nodes
 * (UTs, GWs, satellite receivers) attached to the same channel as a transmitter
 * or a receiver. There may be also several spot-beams using the same SatChannel,
 * i.e. the ones which are using the same frequency band. This enables the per-
 * packet interference calculation at the receiver side. Receiver checks all
 * incoming packets whether they are intended to be received or whether the packet
 * is causing interference with a certain received power.
 *
 * The main tasks of the SatChannel are:
 * - Pass through packets to a set of receivers
 * - Calculate the propagation delay for the packet based on propagation delay model
 * - Calculate the received signal power for the packet based on free-space loss model
 *   and fading (Markov/Loo)
 * - Handle the fading input/output trace functionality
 *
 */

class SatChannel : public Channel
{
public:

  /**
   * Default constructor.
   */
  SatChannel ();

  /**
   * Destructor for SatChannel
   */
  virtual ~SatChannel ();

  /**
   * ONLY_DEST_NODE = only the receivers to which this transmission is intended to shall receive the packet
   * ONLY_DEST_BEAM = only the receivers within the proper spot-beam shall receive the packet
   * ALL_BEAMS = all receivers in the channel shall receive the packet
   */
  enum SatChannelFwdMode_e
  {
    ONLY_DEST_NODE,
    ONLY_DEST_BEAM,
    ALL_BEAMS
  };

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Define type PhyRxContainer
   */
  typedef std::vector<Ptr<SatPhyRx> > PhyRxContainer;

  /**
   * \brief
   * \param channelType     The type of channel
   * \param freqId          The id of the carrier
   * \param carrierId       The id of the carrier
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
   * \return Ptr to the propagation delay model to be used.
   */
  virtual Ptr<PropagationDelayModel> GetPropagationDelayModel ();

  /**
   * \brief Set the type of the channel.
   * \param chType Type of the channel.
   */
  virtual void SetChannelType (SatEnums::ChannelType_t chType);

  /**
   * \brief Set the frequency id of the channel.
   * \param freqId The frequency id of the channel.
   */
  virtual void SetFrequencyId (uint32_t freqId);

  /**
   * \brief Set the frequency converter callback.
   *
   * \param converter The frequency converter callback.
   */
  virtual void SetFrequencyConverter (CarrierFreqConverter converter);

  /**
   * \brief Set the bandwidth converter callback.
   *
   * \param converter The bandwidth converter callback.
   */
  virtual void SetBandwidthConverter (SatTypedefs::CarrierBandwidthConverter_t converter);

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
   * \brief This method is used to attach the receiver entity SatPhyRx instance to a
   * SatChannel instance, so that the SatPhyRx can receive packets sent on that channel.
   * \param phyRx the SatPhyRx instance to be added to the channel as a receiver.
   */
  virtual void AddRx (Ptr<SatPhyRx> phyRx);

  /**
   * \brief This method is used to remove a SatPhyRx instance from a
   * SatChannel instance, e.g. due to a spot-beam handover
   * \param phyRx the SatPhyRx instance to be removed from the channel.
   */
  virtual void RemoveRx (Ptr<SatPhyRx> phyRx);

  /**
   * \return Number of receivers in the channel
   */
  virtual std::size_t GetNDevices (void) const;

  /**
   * \brief Get a device for a certain receiver index
   * \param i Index
   * \return A netdevice attached to this channel
   */
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

private:
  /**
   * Forwarding mode of the SatChannel:
   * SINGLE_RX = only the proper receiver of the packet shall receive the packet
   * MULTI_RX = all receivers in the channel shall receive the packet
   */
  SatChannelFwdMode_e m_fwdMode;

  /**
   * \brief Container of SatPhyRx instances attached to the channel
   */
  PhyRxContainer m_phyRxContainer;

  /**
   * \brief Type of the channel
   */
  SatEnums::ChannelType_t m_channelType;

  /**
   * \brief Frequency converter callback.
   */
  CarrierFreqConverter m_carrierFreqConverter;

  /**
   * \brief Bandwidth converter callback.
   */
  SatTypedefs::CarrierBandwidthConverter_t m_carrierBandwidthConverter;

  /**
   * \brief Frequency id of the channel
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
   * \brief Defines the mode used for Rx power calculation
   */
  SatEnums::RxPowerCalculationMode_t m_rxPowerCalculationMode;

  /**
   * \brief Defines whether Rx power output tracing is in use or not
   */
  bool m_enableRxPowerOutputTrace;

  /**
   * \brief Defines whether fading output tracing is in use or not
   */
  bool m_enableFadingOutputTrace;

  /**
   * \brief Defines whether external fading input tracing is in use or not
   */
  bool m_enableExternalFadingInputTrace;

  /**
   * Dispose SatChannel.
   */
  virtual void DoDispose ();

  /**
   * \brief Used internally to schedule the StartRx method call after the propagation delay.
   * \param rxParams Parameters of the signal being received
   * \param phyRx The receiver SatPhyRx entity
   */
  void ScheduleRx (Ptr<SatSignalParameters> txParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Used internally to start the packet reception of at the phyRx.
   *
   * \param rxParams Parameters of the signal being received
   * \param phyRx The receiver SatPhyRx entity
   */
  void StartRx (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Function for Rx power output trace
   * \param rxParams Rx parameters
   * \param phyRx The receiver SatPhyRx entity
   */
  void DoRxPowerOutputTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Function for Rx power input trace
   * \param rxParams Rx parameters
   * \param phyRx The receiver SatPhyRx entity
   */
  void DoRxPowerInputTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Function for fading output trace
   * \param rxParams Rx parameters
   * \param phyRx The receiver SatPhyRx entity
   * \param fadingValue fading value
   */
  void DoFadingOutputTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx, double fadingValue);

  /**
   * \brief Function for calculating the Rx power
   * \param rxParams Rx parameters
   * \param phyRx The receiver SatPhyRx entity
   */
  void DoRxPowerCalculation (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Function for getting the external source fading value
   * \param rxParams Rx parameters
   * \param phyRx The receiver SatPhyRx entity
   * \return fading value
   */
  double GetExternalFadingTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx);

  /**
   * \brief Function for getting the source MAC address from Rx parameters
   * \param rxParams Rx parameters
   * \return source MAC address
   */
  Mac48Address GetSourceAddress (Ptr<SatSignalParameters> rxParams);

};

}

#endif /* SATELLITE_CHANNEL_H */




