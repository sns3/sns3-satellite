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

#ifndef SATELLITE_PER_PACKET_INTERFERENCE_H
#define SATELLITE_PER_PACKET_INTERFERENCE_H

#include <map>
#include <set>
#include "satellite-interference.h"
#include "satellite-interference-output-trace-container.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Packet by packet interference. Interference is calculated
 * separately for each packet by listening to all transmissions within
 * the same SatChannel.
 */
class SatPerPacketInterference : public SatInterference
{
public:
  /**
   *
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   *
   * \return
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   *
   */
  SatPerPacketInterference ();

  /**
   *
   * \param channelType
   * \param rxBandwidthHz Receiver bandwidth in Hertz
   */
  SatPerPacketInterference (SatEnums::ChannelType_t channelType, double rxBandwidthHz);

  /**
   *
   */
  ~SatPerPacketInterference ();

  /**
   *
   */
  void DoDispose ();

  /**
   *
   * \param rxBandwidth
   */
  void SetRxBandwidth (double rxBandwidth);

protected:
  /**
   * Calculates interference power for the given reference
   * Sets final power at end time to finalPower.
   *
   * \param event Reference event which for interference is calculated.
   *
   * \return Final calculated power value at end of receiving
   */
  virtual std::vector< std::pair<double, double> > DoCalculate (Ptr<SatInterference::InterferenceChangeEvent> event);

  /**
   * Helper function meant for subclasses to override.
   * Called during DoCalculate when the start of the event whose
   * interferences are being calculated has been reached.
   */
  virtual void onOwnStartReached (double ifPowerW);

  /**
   * Helper function meant for subclasses to override.
   * Called during DoCalculate after the start of the event whose
   * interferences are being calculated has been reached.
   *
   * Update the current interference power by adding the power of
   * the event being processed proportional to the remaining time
   * until the end of the event whose interferences are being calculated.
   *
   * \param timeRatio ratio of time compared to an event duration of the
   * distance between the event being processed and the end of the event
   * whose interferences are being calculated.
   * \param interferenceValue the interference value of the event being
   * processed.
   * \param ifPowerW the current value of the interference for the event
   * whose interferences are being calculated; which will be updated.
   */
  virtual void onInterferentEvent (long double timeRatio, double interferenceValue, double& ifPowerW);

private:
  /**
   * Adds interference power to interference object.
   *
   * \param rxDuration Duration of the receiving.
   * \param rxPower Receiving power.
   *
   * \return the pointer to interference event as a reference of the addition
   */
  virtual Ptr<SatInterference::InterferenceChangeEvent> DoAdd (Time rxDuration, double rxPower, Address rxAddress);

  /**
   * Resets current interference.
   */
  virtual void DoReset (void);

  /**
   * Notifies that RX is started by a receiver.
   *
   * \param event Interference reference event of receiver
   */
  virtual void DoNotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event);

  /**
   * Notifies that RX is ended by a receiver.
   *
   * \param event Interference reference event of receiver
   */
  virtual void DoNotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event);

  /**
   *
   */
  typedef std::pair <uint32_t, long double > InterferenceChange;

  /**
   *
   */
  typedef std::multimap <Time, InterferenceChange > InterferenceChanges;

  /**
   *
   * \param o
   */
  SatPerPacketInterference (const SatPerPacketInterference &o);

  /**
   *
   * \param o
   * \return
   */
  SatPerPacketInterference &operator = (const SatPerPacketInterference &o);

  /**
   * \brief interference change list
   */
  InterferenceChanges m_interferenceChanges;

  /**
   * \brief notified interference event IDs
   */
  std::set <uint32_t> m_rxEventIds;

  /**
   * \brief Residual power value for interference.
   * Sum of negative values in list m_changes, which positive value is not in list.
   */
  long double m_residualPowerW;

  /**
   * \brief flag to indicate that at least one receiving is on
   */
  bool m_rxing;

  /**
   * \brief event id for Events
   */
  uint32_t m_nextEventId;

  /**
   *
   */
  bool m_enableTraceOutput;

  /**
   *
   */
  SatEnums::ChannelType_t m_channelType;

  /**
   * \brief RX Bandwidth in Hz
   */
  double m_rxBandwidth_Hz;

};

} // namespace ns3

#endif /* SATELLITE_PER_PACKET_INTERFERENCE_H */
