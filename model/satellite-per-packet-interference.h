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

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Packet by packet interference. Interference is calculated packet
 */
class SatPerPacketInterference : public SatInterference
{
public:
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  SatPerPacketInterference ();
  ~SatPerPacketInterference ();

private:
  /**
   * Adds interference power to interference object.
   *
   * \param rxDuration Duration of the receiving.
   * \param rxPower Receiving power.
   *
   * \return the pointer to interference event as a reference of the addition
   */
  virtual Ptr<SatInterference::Event> DoAdd (Time rxDuration, double rxPower);

  /**
   * Calculates interference power for the given reference
   * Sets final power at end time to finalPower.
   *
   * \param event Reference event which for interference is calculated.
   *
   * \return Final calculated power value at end of receiving
   */
  virtual double DoCalculate (Ptr<SatInterference::Event> event);

  /**
   * Resets current interference.
   */
  virtual void DoReset (void);

  /**
   * Notifies that RX is started by a receiver.
   *
   * \param event Interference reference event of receiver
   */
  virtual void DoNotifyRxStart (Ptr<SatInterference::Event> event);

  /**
   * Notifies that RX is ended by a receiver.
   *
   * \param event Interference reference event of receiver
   */
  virtual void DoNotifyRxEnd (Ptr<SatInterference::Event> event);

  typedef std::pair <uint32_t, double > InterferenceChange;
  typedef std::multimap <Time, InterferenceChange > InterferenceChanges;

  SatPerPacketInterference (const SatPerPacketInterference &o);
  SatPerPacketInterference &operator = (const SatPerPacketInterference &o);

  // interference change list
  InterferenceChanges m_changes;

  // notified interference event IDs
  std::set <uint32_t> m_events;

  // first power value for interference
  // sum of negative values in list m_changes, which positive value is not in list
  double m_firstPower;

  // flag to indicate that at least one receiving is on
  bool m_rxing;

  // event id for Events
  uint32_t m_nextEventId;
};

} // namespace ns3

#endif /* SATELLITE_PER_PACKET_INTERFERENCE_H */
