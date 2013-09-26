/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#ifndef SATELLITE_FRAME_CONF_H
#define SATELLITE_FRAME_CONF_H

#include <vector>
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements configuration for Wave forms.
 */

class SatBtuConf : public SimpleRefCount<SatBtuConf>
{
public:

  /**
   * Default constructor for SatBtuConf
   */
  SatBtuConf ();

  /**
   * Constructor for SatBtuConf
   *
   * \param bandwidth_hz      Bandwidth of BTU in Hertz
   * \param length_s          Length of BTU in seconds
   * \param symbolRate_baud   Symbol rate in bauds
   */
  SatBtuConf (double bandwidth_hz, double length_s, double symbolRate_baud);

  /**
   * Destructor for SatBtuConf
   */
  ~SatBtuConf ();

  /**
   * Get bandwidth of BTU.
   *
   * \return The bandwidth of BTU in Hertz.
   */
  inline double GetBandwidth_hz () { return m_bandwidth_hz; }

  /**
   * Get length of BTU.
   *
   * \return The length of BTU in seconds.
   */
  inline double GetLength_s () { return m_length_s; }

  /**
   * Get symbol rate of BTU.
   *
   * \return The symbol rate of BTU in bauds.
   */
  inline double GetSymbolRate_baud () { return m_symbolRate_baud; }

private:
  double m_bandwidth_hz;
  double m_length_s;
  double m_symbolRate_baud;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for time slots (for super frames / frames)
 */

class SatTimelSlotConf : public SimpleRefCount<SatTimelSlotConf>
{
public:

  /**
   * Default constructor for SatTimelSlotConf
   */
  SatTimelSlotConf ();

  /**
   * Constructor for SatTimelSlotConf
   *
   * \param startTime_s Start time of time slot in side frame.
   * \param duration_s  Duration of time slot in seconds
   * \param waveFormId  Wave form id of time slot
   * \param carrierId   Carrier id of time slot
   */
  SatTimelSlotConf (double startTime_s, double duration_s, uint8_t waveFormId, uint32_t carrierId);

  /**
   * Destructor for SatTimelSlotConf
   */
  ~SatTimelSlotConf ();

  /**
   * Get start time of time slot. (inside frame)
   *
   * \return The start time of time slot in seconds.
   */
  inline double GetStartTime_s () { return m_startTime_s; }

  /**
   * Get duration of time slot.
   *
   * \return The duration of time slot in seconds.
   */
  inline double GetDuration_s () { return m_duration_s; }

  /**
   * Get wave form id of time slot.
   *
   * \return The wave form id of time slot.
   */
  inline double GetWaveFormId () { return m_waveFormId; }

  /**
   * Get carrier id of time slot (inside frame).
   *
   * \return The carrier id of time slot.
   */
  inline double GetCarrierId () { return m_frameCarrierId; }

private:
  double  m_startTime_s;
  double  m_duration_s;
  uint8_t m_waveFormId;
  uint32_t m_frameCarrierId;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for frames (for super frames)
 */

class SatFrameConf : public SimpleRefCount<SatFrameConf>
{
public:

  static const uint16_t maxTimeSlotCount = 2048;
  static const uint16_t maxTimeSlotIndex = maxTimeSlotCount - 1;

  /**
   * Default constructor for SatFrameConf
   */
  SatFrameConf ();

  /**
   * Constructor for SatFrameConf.
   *
   * \param bandwidth_hz      Bandwidth of the frame in Hertz
   * \param duration_s        Duration of the frame in seconds
   * \param btu               BTU conf of the frame
   * \param timeSlots         Time slot of the frame. (Timeslot must be in acsending order by start time).
   */
  SatFrameConf ( double bandwidth_hz, double duration_s, Ptr<SatBtuConf> btu,
                 std::vector<Ptr<SatTimelSlotConf> > * timeSlots );

  /**
   * Get bandwidth of the frame.
   *
   * \return The bandwidth of frame in Hertz.
   */
  inline double GetBandwidth_hz () { return m_bandwidth_hz; }

  /**
   * Get BTU conf of the frame.
   *
   * \return The BTU conf of frame.
   */
  inline Ptr<SatBtuConf> GetBtuConf () { return m_btu; }

  /**
   * Get carrier count of the frame.
   *
   * \return The carrier count of the frame.
   */
  inline uint32_t GetCarrierCount () { return m_carrierCount; }

  /**
   * Get time slot conf of the frame. Possible values for id are from 0 to 2047.
   *
   * \param index Id of the time slot requested.
   * \return      The requested time slot conf of frame.
   */
  Ptr<SatTimelSlotConf> GetTimeSlotConf (uint16_t id);

  /**
   * Destructor for SatFrameConf
   */
  ~SatFrameConf ();

private:
  double    m_bandwidth_hz;
  double    m_duration_s;
  uint32_t  m_carrierCount;

  Ptr<SatBtuConf> m_btu;
  std::vector<Ptr<SatTimelSlotConf> > m_timeSlots;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for super frames
 */
class SatSuperFrameConf : public SimpleRefCount<SatSuperFrameConf>
{
public:

  static const uint16_t maxFrameCount = 256;

  /**
   * Default constructor for SatSuperFrameConf
   */
  SatSuperFrameConf ();

  /**
   * Constructor for SatSuperFrameConf. Timeslot must be in acsending order by time
   *
   * \param frequency_hz      Center frequency of the frame in Hertz
   * \param bandwidth_hz      Bandwidth of the frame in Hertz
   * \param duration_s        Duration of the frame in seconds
   * \param m_frames          Frames of the super frame
   */
  SatSuperFrameConf ( double frequency_hz, double bandwidth_hz, double duration_s,
                      std::vector<Ptr<SatFrameConf> > * m_frames );

  /**
   * Destructor for SatSuperFrameConf
   */
  ~SatSuperFrameConf ();

  /**
   * Get center frequency of the frame.
   *
   * \return The center frequency of frame in Hertz.
   */
  inline double GetFrequency_hz() { return m_frequency_hz; }

  /**
   * Get bandwidth of the frame.
   *
   * \return The bandwidth of frame in Hertz.
   */
  inline double GetBandwidth_hz() { return m_bandwidth_hz; }

  /**
   * Get duration of super frame.
   *
   * \return The duration of super frame in seconds.
   */
  inline double GetDuration_s() { return m_duration_s; }

  /**
   * Get frame conf of the super frame.
   *
   * \param id    Id of the frame requested.
   * \return      The requested frame conf of the super frame.
   */
  Ptr<SatFrameConf> GetFrameConf(uint8_t id);

  /**
   * Get carrier id of the super frame. Converts frame specific id to super frame specific id.
   *
   * \param frameId         Id of the frame requested.
   * \param frameCarrierId  Id of the carrier ID inside frame requested.
   *
   * \return The requested carrier id of the super frame.
   */
  uint32_t GetCarriedId( uint8_t frameId, uint16_t frameCarrierId );

private:
  double m_frequency_hz;   // center frequency
  double m_bandwidth_hz;
  double m_duration_s;
  std::vector<Ptr<SatFrameConf> > m_frames;
};

} // namespace ns3

#endif // SATELLITE_FRAME_CONF_H
