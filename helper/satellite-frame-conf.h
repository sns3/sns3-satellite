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
 * \brief This class implements configuration for Bandwidth Time Unit (BTU).
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
  inline double GetBandwidth_hz () const { return m_bandwidth_hz; }

  /**
   * Get length of BTU.
   *
   * \return The length of BTU in seconds.
   */
  inline double GetLength_s () const { return m_length_s; }

  /**
   * Get symbol rate of BTU.
   *
   * \return The symbol rate of BTU in bauds.
   */
  inline double GetSymbolRate_baud () const { return m_symbolRate_baud; }

private:
  double m_bandwidth_hz;
  double m_length_s;
  double m_symbolRate_baud;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for time slots (for super frames / frames)
 */

class SatTimeSlotConf : public SimpleRefCount<SatTimeSlotConf>
{
public:

  /**
   * Default constructor for SatTimeSlotConf
   */
  SatTimeSlotConf ();

  /**
   * Constructor for SatTimeSlotConf
   *
   * \param startTime_s Start time of time slot in side frame.
   * \param duration_s  Duration of time slot in seconds
   * \param waveFormId  Wave form id of time slot
   * \param carrierId   Carrier id of time slot
   */
  SatTimeSlotConf (double startTime_s, double duration_s, uint8_t waveFormId, uint32_t carrierId);

  /**
   * Destructor for SatTimeSlotConf
   */
  ~SatTimeSlotConf ();

  /**
   * Get start time of time slot. (inside frame)
   *
   * \return The start time of time slot in seconds.
   */
  inline double GetStartTime_s () const { return m_startTime_s; }

  /**
   * Get duration of time slot.
   *
   * \return The duration of time slot in seconds.
   */
  inline double GetDuration_s () const { return m_duration_s; }

  /**
   * Get wave form id of time slot.
   *
   * \return The wave form id of time slot.
   */
  inline double GetWaveFormId () const { return m_waveFormId; }

  /**
   * Get carrier id of time slot (inside frame).
   *
   * \return The carrier id of time slot.
   */
  inline uint32_t GetCarrierId () const { return m_frameCarrierId; }

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
  typedef std::vector<Ptr<SatTimeSlotConf> > SatTimeSlotConfList;

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
   * \param timeSlots         Time slot of the frame. (Timeslots must be in acsending order according to start time).
   */
  SatFrameConf ( double bandwidth_hz, double duration_s, Ptr<SatBtuConf> btu,
                 SatTimeSlotConfList * timeSlots );

  void AddTimeSlotConf ( Ptr<SatTimeSlotConf> conf);

  /**
   * Get bandwidth of the frame.
   *
   * \return The bandwidth of frame in Hertz.
   */
  inline double GetBandwidth_hz () const { return m_bandwidth_hz; }

  /**
   * Get duration of frame.
   *
   * \return The duration of frame in seconds.
   */
  inline double GetDuration_s () const { return m_duration_s; }

  /**
   * Get carrier center frequency in frame.
   *
   * \return The carrier bandwidth in frame in Hertz.
   */
  double GetCarrierFrequency_hz ( uint32_t carrierId ) const;

  /**
   * Get carrier bandwidth in frame.
   *
   * \return The carrier bandwidth in frame in Hertz.
   */
  inline double GetCarrierBandwidth_hz() const { return m_btu->GetBandwidth_hz(); }

  /**
   * Get BTU conf of the frame.
   *
   * \return The BTU conf of frame.
   */
  inline Ptr<SatBtuConf> GetBtuConf () const { return m_btu; }

  /**
   * Get carrier count of the frame.
   *
   * \return The carrier count of the frame.
   */
  inline uint32_t GetCarrierCount () { return m_carrierCount; }

  /**
   * Get time slot count of the frame.
   *
   * \param index Id of the time slot requested.
   * \return      The requested time slot conf of frame.
   */
  inline uint32_t GetTimeSlotrCount () const { return m_timeSlots.size(); }

  /**
   * Get time slot conf of the frame. Possible values for id are from 0 to 2047.
   *
   * \param index Id of the time slot requested.
   * \return      The requested time slot conf of frame.
   */
  Ptr<SatTimeSlotConf> GetTimeSlotConf (uint16_t id) const;

  /**
   * Destructor for SatFrameConf
   */
  ~SatFrameConf ();

private:
  double    m_bandwidth_hz;
  double    m_duration_s;
  uint32_t  m_carrierCount;

  Ptr<SatBtuConf> m_btu;
  SatTimeSlotConfList m_timeSlots;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for super frames
 */
class SatSuperframeConf : public SimpleRefCount<SatSuperframeConf>
{
public:
  typedef std::vector<Ptr<SatFrameConf> > SatFrameConfList;

  static const uint16_t maxFrameCount = 256;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf ();

  /**
   * Constructor for SatSuperframeConf.
   *
   * \param bandwidth_hz      Bandwidth of the frame in Hertz
   * \param duration_s        Duration of the frame in seconds
   * \param frames            Frames of the super frame. (In acsending order according to frequency inside superframe).
   */
  SatSuperframeConf ( double bandwidth_hz, double duration_s,
                      SatFrameConfList * frames );

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf ();

  void AddFrameConf (Ptr<SatFrameConf> conf);

  /**
   * Get bandwidth of the superframe.
   *
   * \return The bandwidth of superframe in Hertz.
   */
  inline double GetBandwidth_hz () const { return m_bandwidth_hz; }

  /**
   * Get duration of super frame.
   *
   * \return The duration of superframe in seconds.
   */
  inline double GetDuration_s () const { return m_duration_s; }

  /**
   * Get frame conf of the superframe.
   *
   * \param id    Id of the frame requested.
   * \return      The requested frame conf of the super frame.
   */
  Ptr<SatFrameConf> GetFrameConf (uint8_t id) const;

  /**
   * Get carrier id of the superframe. Converts frame specific id to superframe specific id.
   *
   * \param frameId         Id of the frame requested.
   * \param frameCarrierId  Id of the carrier inside frame requested.
   *
   * \return The requested carrier id of the superframe.
   */
  uint32_t GetCarrierId( uint8_t frameId, uint16_t frameCarrierId ) const;

  /**
   * Get carrier count in the superframe.
   *
   * \return The superframe carrier count.
   */
  uint32_t GetCarrierCount () const;

  /**
   * Get the center frequency of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside superframe which center frequency is requested.
   *
   * \return The center frequency of the requested carrier.
   */
  double GetCarrierFrequency_hz (uint32_t carrierId) const;

  /**
   * Get the bandwidth of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside superframe which bandwidth is requested.
   *
   * \return The bandwidth of the requested carrier.
   */
  double GetCarrierBandwidth_hz (uint32_t carrierId) const;

private:
  double m_bandwidth_hz;
  double m_duration_s;
  SatFrameConfList m_frames;
};

} // namespace ns3

#endif // SATELLITE_FRAME_CONF_H
