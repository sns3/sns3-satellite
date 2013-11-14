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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_TRACED_INTERFERENCE_H
#define SATELLITE_TRACED_INTERFERENCE_H

#include "satellite-interference.h"
#include "ns3/satellite-input-fstream-double-container.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Satellite traced interference. Interference values is based on trace file.
 */
class SatTracedInterference : public SatInterference
{
public:
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  SatTracedInterference (std::string filename);
  SatTracedInterference ();
  ~SatTracedInterference ();

private:

  static const uint32_t ROW_COUNT = 3;

  static const uint32_t INTERFERENCE_VALUE_COLUMN_NUMBER = 3;

  static const uint32_t TIME_VALUE_COLUMN_NUMBER = 0;
  /**
   * Adds interference power to interference object.
   * No effect in this implementation.
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
   * Just return next value from trace input.
   * finalPower is set to returned value.
   *
   * \param event Reference event which for interference is calculated.
   *
   * \return Final power value at end of receiving
   */
  virtual double DoCalculate (Ptr<SatInterference::Event> event);

  /**
   * Resets current interference.
   */
  virtual void DoReset (void);

  /**
   * Notifies that RX is started by a receiver.
   *
   * \param event Interference reference event of receiver (ignored in this implementation)
   */
  virtual void DoNotifyRxStart (Ptr<SatInterference::Event> event);

  /**
   * Notifies that RX is ended by a receiver.
   *
   * \param event Interference reference event of receiver (ignored in this implementation)
   */
  virtual void DoNotifyRxEnd (Ptr<SatInterference::Event> event);

  SatTracedInterference (const SatTracedInterference &o);
  SatTracedInterference &operator = (const SatTracedInterference &o);

  bool m_rxing;

  double m_power;

  Ptr<SatInputFileStreamDoubleContainer> m_tracedInterference;
};

} // namespace ns3

#endif /* SATELLITE_TRACED_INTERFERENCE_H */
