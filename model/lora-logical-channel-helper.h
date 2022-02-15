/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef LORA_LOGICAL_CHANNEL_HELPER_H
#define LORA_LOGICAL_CHANNEL_HELPER_H

#include <list>
#include <iterator>
#include <vector>

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"

#include "ns3/lora-sub-band.h"
#include "ns3/lora-logical-channel.h"

namespace ns3 {

/**
 * This class supports LorawanMac instances by managing a list of the logical
 * channels that the device is supposed to be using, and establishes their
 * relationship with LoraSubBands.
 *
 * This class also takes into account duty cycle limitations, by updating a list
 * of LoraSubBand objects and providing methods to query whether transmission on a
 * set channel is admissible or not.
 */
class LoraLogicalChannelHelper : public Object
{
public:
  static TypeId GetTypeId (void);

  LoraLogicalChannelHelper ();
  virtual ~LoraLogicalChannelHelper ();

  /**
   * Get the time it is necessary to wait before transmitting again, according
   * to the aggregate duty cycle timer.
   *
   * \return The aggregate waiting time.
   */
  Time GetAggregatedWaitingTime (void);

  /**
   * Get the time it is necessary to wait for before transmitting on a given
   * channel.
   *
   * \remark This function does not take into account aggregate waiting time.
   * Check on this should be performed before calling this function.
   *
   * \param channel A pointer to the channel we want to know the waiting time
   * for.
   * \return A Time instance containing the waiting time before transmission is
   * allowed on the channel.
   */
  Time GetWaitingTime (Ptr<LoraLogicalChannel> channel);

  /**
   * Register the transmission of a packet.
   *
   * \param duration The duration of the transmission event.
   * \param channel The channel the transmission was made on.
   */
  void AddEvent (Time duration, Ptr<LoraLogicalChannel> channel);

  /**
   * Get the list of LoraLogicalChannels currently registered on this helper.
   *
   * \return A list of the managed channels.
   */
  std::vector<Ptr<LoraLogicalChannel> > GetChannelList (void);

  /**
   * Get the list of LoraLogicalChannels currently registered on this helper
   * that have been enabled for Uplink transmission with the channel mask.
   *
   * \return A list of the managed channels enabled for Uplink transmission.
   */
  std::vector<Ptr<LoraLogicalChannel> > GetEnabledChannelList (void);

  /**
   * Add a new channel to the list.
   *
   * \param frequency The frequency of the channel to create.
   */
  void AddChannel (double frequency);

  /**
   * Add a new channel to the list.
   *
   * \param logicalChannel A pointer to the channel to add to the list.
   */
  void AddChannel (Ptr<LoraLogicalChannel> logicalChannel);

  /**
   * Set a new channel at a fixed index.
   *
   * \param chIndex The index of the channel to substitute.
   * \param logicalChannel A pointer to the channel to add to the list.
   */
  void SetChannel (uint8_t chIndex, Ptr<LoraLogicalChannel> logicalChannel);

  /**
   * Add a new LoraSubBand to this helper.
   *
   * \param firstFrequency The first frequency of the subband, in MHz.
   * \param lastFrequency The last frequency of the subband, in MHz.
   * \param dutyCycle The duty cycle that needs to be enforced on this subband.
   * \param maxTxPowerDbm The maximum transmission power [dBm] that can be used
   * on this LoraSubBand.
   */
  void AddLoraSubBand (double firstFrequency, double lastFrequency,
                   double dutyCycle, double maxTxPowerDbm);

  /**
   * Add a new LoraSubBand.
   *
   * \param subBand A pointer to the LoraSubBand that needs to be added.
   */
  void AddLoraSubBand (Ptr<LoraSubBand> subBand);

  /**
   * Remove a channel.
   *
   * \param channel A pointer to the channel we want to remove.
   */
  void RemoveChannel (Ptr<LoraLogicalChannel> channel);

  /**
   * Returns the maximum transmission power [dBm] that is allowed on a channel.
   *
   * \param logicalChannel The power for which to check the maximum allowed
   * transmission power.
   * \return The power in dBm.
   */
  double GetTxPowerForChannel (Ptr<LoraLogicalChannel> logicalChannel);

  /**
   * Get the LoraSubBand a channel belongs to.
   *
   * \param channel The channel whose LoraSubBand we want to get.
   * \return The LoraSubBand the channel belongs to.
   */
  Ptr<LoraSubBand> GetLoraSubBandFromChannel (Ptr<LoraLogicalChannel> channel);

  /**
   * Get the LoraSubBand a frequency belongs to.
   *
   * \param frequency The frequency we want to check.
   * \return The LoraSubBand the frequency belongs to.
   */
  Ptr<LoraSubBand> GetLoraSubBandFromFrequency (double frequency);

  /**
   * Disable the channel at a specified index.
   *
   * \param index The index of the channel to disable.
   */
  void DisableChannel (int index);

private:
  /**
   * A list of the LoraSubBands that are currently registered within this helper.
   */
  std::list<Ptr <LoraSubBand> > m_subBandList;

  /**
   * A vector of the LoraLogicalChannels that are currently registered within
   * this helper. This vector represents the node's channel mask. The first N
   * channels are the default ones for a fixed region.
   */
  std::vector<Ptr <LoraLogicalChannel> > m_channelList;

  Time m_nextAggregatedTransmissionTime; //!< The next time at which
  //!transmission will be possible
  //!according to the aggregated
  //!transmission timer

  double m_aggregatedDutyCycle; //!< The next time at which
  //!transmission will be possible
  //!according to the aggregated
  //!transmission timer
};
}

#endif /* LORA_LOGICAL_CHANNEL_HELPER_H */
