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
#include "satellite-interference-input-trace-container.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Satellite traced interference. Interference values is based on trace file.
 */
class SatTracedInterference : public SatInterference
{
public:
  /**
   * Derived from Object
   * \return TypeId of class
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   * \return TypeId of instance
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * Constructor
   * \param channeltype
   * \param rxBandwidth
   */
  SatTracedInterference (SatEnums::ChannelType_t channeltype, double rxBandwidth);

  /**
   * Default constructor, not used
   */
  SatTracedInterference ();

  /**
   * Destructor
   */
  ~SatTracedInterference ();

  /**
   * Dispose of SatTracedInterference
   */
  void DoDispose ();

  /**
   * Set Rx bandwidth
   * \param rxBandwidth
   */
  void SetRxBandwidth (double rxBandwidth);

private:
  /**
   * Adds interference power to interference object.
   * No effect in this implementation.
   *
   * \param rxDuration Duration of the receiving.
   * \param rxPower Receiving power.
   * \param rxAddress
   *
   * \return the pointer to interference event as a reference of the addition
   */
  virtual Ptr<SatInterference::InterferenceChangeEvent> DoAdd (Time rxDuration, double rxPower, Address rxAddress);
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
  virtual double DoCalculate (Ptr<SatInterference::InterferenceChangeEvent> event);

  /**
   * Resets current interference.
   */
  virtual void DoReset (void);

  /**
   * Notifies that RX is started by a receiver.
   *
   * \param event Interference reference event of receiver (ignored in this implementation)
   */
  virtual void DoNotifyRxStart (Ptr<SatInterference::InterferenceChangeEvent> event);

  /**
   * Notifies that RX is ended by a receiver.
   *
   * \param event Interference reference event of receiver (ignored in this implementation)
   */
  virtual void DoNotifyRxEnd (Ptr<SatInterference::InterferenceChangeEvent> event);

  /**
   *
   * \param o
   */
  SatTracedInterference (const SatTracedInterference &o);

  /**
   *
   * \param o
   * \return
   */
  SatTracedInterference &operator = (const SatTracedInterference &o);

  /**
   *
   */
  bool m_rxing;

  /**
   *
   */
  double m_power;

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

#endif /* SATELLITE_TRACED_INTERFERENCE_H */
