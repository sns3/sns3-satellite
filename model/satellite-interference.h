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

#ifndef SATELLITE_INTERFERENCE_H
#define SATELLITE_INTERFERENCE_H

#include "ns3/object.h"
#include "ns3/simple-ref-count.h"
#include "ns3/nstime.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief abstract class defining interface for interference calculations objects
 */
class SatInterference : public Object
{
public:
  /**
   * Event for identifying interferences (receiving)
   */
  class Event : public SimpleRefCount<SatInterference::Event>
  {
  public:
    /**
      * Constructor of Event for satellite interference
      * \param id indentifier of the event
      * \param duration duration of the interference event
      * \param rxPower  RX power of interference
      */
    Event (uint32_t id, Time duration, double rxPower);

    /**
     * Destructor of Event for satellite interference
     */
    ~Event ();

    /**
      * \return id identifier of the event
      */
    uint32_t GetId (void) const;

    /**
      * \return duration of the interference event
      */
    Time GetDuration (void) const;

    /**
      * \return start time of the interference event
      */
    Time GetStartTime (void) const;

    /**
      * \return end time of the interference event
      */
    Time GetEndTime (void) const;

    /**
      * \return RX power of the interference event
      */
    double GetRxPower (void) const;

  private:
    Time m_startTime;
    Time m_endTime;
    double m_rxPower;
    uint32_t m_id;
  };

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Constructor for Satellite interference base class
   */
  SatInterference ();

  /**
   * Destructor for Satellite interference base class
   */
  ~SatInterference ();

  /**
   * Adds interference power to interference object.
   * Behavior depends on class actually implementing interference.
   *
   * \param rxDuration Duration of the receiving.
   * \param rxPower Receiving power.
   *
   * \return the pointer to interference event as a reference of the addition
   */
  Ptr<SatInterference::Event> Add (Time rxDuration, double rxPower);

  /**
   * Calculates interference power for the given reference
   * Sets final power at end time to given variable pointed by finalPower.
   *
   * \param event Reference event which for interference is calculated.
   * \param finalPower Pointer to variable to store calculated power value at end of receiving
   *
   * \return Calculated power value at end of receiving
   */
  double Calculate (Ptr<SatInterference::Event> event );

  /**
   * Resets current interference.
   */
  void Reset (void);

  /**
   * Notifies that RX is started by a receiver.
   * \param event Interference reference event of receiver
   */
  virtual void NotifyRxStart (Ptr<SatInterference::Event> event);

  /**
   * Notifies that RX is ended by a receiver.
   * \param event Interference reference event of receiver
   */
  virtual void NotifyRxEnd (Ptr<SatInterference::Event> event);

private:
  /**
   * Adds interference power to interference object.
   *
   * \param rxDuration Duration of the receiving.
   * \param rxPower Receiving power.
   *
   * \return the pointer to interference event as a reference of the addition
   *
   * Concrete subclasses of this base class must implement this method.
   */
  virtual Ptr<SatInterference::Event> DoAdd (Time rxDuration, double rxPower) = 0;

  /**
   * Calculates interference power for the given reference
   * Sets final power at end time to finalPower.
   *
   * \param event Reference event which for interference is calculated.
   *
   * \return Final power value at end of receiving
   *
   * Concrete subclasses of this base class must implement this method.
   */
  virtual double DoCalculate (Ptr<SatInterference::Event> event ) = 0;

  /**
   * Resets current interference.
   *
   * Concrete subclasses of this base class must implement this method.
   */
  virtual void DoReset (void) = 0;

  /**
   * Notifies that RX is started by a receiver.
   *
   * Concrete subclasses of this base class must implement this method.
   *
   * \param event Interference reference event of receiver
   */
  virtual void DoNotifyRxStart (Ptr<SatInterference::Event> event) = 0;

  /**
   * Notifies that RX is ended by a receiver.
   *
   * Concrete subclasses of this base class must implement this method.
   *
   * \param event Interference reference event of receiver
   */
  virtual void DoNotifyRxEnd (Ptr<SatInterference::Event> event) = 0;

  SatInterference (const SatInterference &o);
  SatInterference &operator = (const SatInterference &o);
};

} // namespace ns3

#endif /* SATELLITE_INTERFERENCE_H */
