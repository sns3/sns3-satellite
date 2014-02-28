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
#include "ns3/random-variable-stream.h"
#include "ns3/satellite-enums.h"
#include "satellite-wave-form-conf.h"

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
   * \param bandwidthHz      Allocated bandwidth of BTU in Hertz
   * \param rollOff           Roll-off factor
   * \param spacing           Spacing factor
   */
  SatBtuConf (double bandwidthHz, double rollOff, double spacing);

  /**
   * Destructor for SatBtuConf
   */
  ~SatBtuConf ();

  /**
   * Get bandwidth of BTU.
   *
   * \return The bandwidth of BTU in hertz.
   */
  inline double GetAllocatedBandwidthHz () const { return m_allocatedBandwidthHz; }

  /**
   * Get occupied bandwidth of BTU.
   *
   * \return The occupied bandwidth of BTU in hertz.
   */
  inline double GetOccupiedBandwidthHz () const { return m_occupiedBandwidthHz; }

  /**
   * Get occupied bandwidth of BTU.
   *
   * \return The occupied bandwidth of BTU in hertz.
   */
  inline double GetEffectiveBandwidthHz () const { return m_effectiveBandwidthHz; }

  /**
   * Get symbol rate of BTU.
   *
   * \return The symbol rate of BTU in bauds.
   */
  inline double GetSymbolRateInBauds () const { return GetEffectiveBandwidthHz ();}

private:
  double m_allocatedBandwidthHz;
  double m_occupiedBandwidthHz;
  double m_effectiveBandwidthHz;  // i.e. symbol rate
  double m_lengthInSeconds;       // length field reserved, but not used currently
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

  /**FRAME_ATTRIBUTE_ACCESSOR_DEFINE
   * Constructor for SatTimeSlotConf
   *
   * \param startTimeInSeconds  Start time of time slot in side frame.
   * \param waveFormId          Wave form id of time slot
   * \param carrierId           Carrier id of time slot
   */
  SatTimeSlotConf (double startTimeInSeconds, uint32_t waveFormId, uint32_t carrierId);

  /**
   * Destructor for SatTimeSlotConf
   */
  ~SatTimeSlotConf ();

  /**
   * Get start time of time slot. (inside frame)
   *
   * \return The start time of time slot in seconds.
   */
  inline double GetStartTimeInSeconds () const { return m_startTimeInSeconds; }

  /**
   * Get wave form id of time slot.
   *
   * \return The wave form id of time slot.
   */
  inline uint32_t GetWaveFormId () const { return m_waveFormId; }

  /**
   * Get carrier id of time slot (inside frame).
   *
   * \return The carrier id of time slot.
   */
  inline uint32_t GetCarrierId () const { return m_frameCarrierId; }

private:
  double  m_startTimeInSeconds;
  uint32_t m_waveFormId;
  uint32_t m_frameCarrierId;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for frames (for super frames)
 */

class SatFrameConf : public SimpleRefCount<SatFrameConf>
{
public:
  typedef std::map<uint16_t, Ptr<SatTimeSlotConf> > SatTimeSlotConfContainer_t;
  typedef std::vector<uint16_t>                     SatTimeSlotIdContainer_t;

  static const uint16_t maxTimeSlotCount = 2048;
  static const uint16_t maxTimeSlotIndex = maxTimeSlotCount - 1;

  /**
   * Default constructor for SatFrameConf
   */
  SatFrameConf ();

  /**
   * Constructor for SatFrameConf.
   *
   * \param bandwidthHz       Bandwidth of the frame in Hertz
   * \param durationInSeconds Duration of the frame in seconds
   * \param btu               BTU conf of the frame
   * \param timeSlots         Time slot of the frame.
   */
  SatFrameConf ( double bandwidthHz, double durationInSeconds, Ptr<SatBtuConf> btu,
                 SatTimeSlotConfContainer_t * timeSlots, bool isRandomAccess );

  /**
   * Destructor for SatFrameConf
   */
  ~SatFrameConf ();

  /**
   * Add time slot.
   *
   * \param conf  Time slot conf added.
   * \return ID of the added timeslot.
   */
  uint16_t AddTimeSlotConf ( Ptr<SatTimeSlotConf> conf);

  /**
   * Get bandwidth of the frame.
   *
   * \return The bandwidth of frame in Hertz.
   */
  inline double GetBandwidthHz () const { return m_bandwidthHz; }

  /**
   * Get duration of frame.
   *
   * \return The duration of frame in seconds.
   */
  inline double GetDurationInSeconds () const { return m_durationInSeconds; }

  /**
   * Get carrier center frequency in frame.
   *
   * \return The carrier bandwidth in frame in hertz.
   */
  double GetCarrierFrequencyHz ( uint32_t carrierId ) const;

  /**
   * Get carrier bandwidth in frame.
   *
   * \param bandwidthType Type of bandwidth requested.
   *
   * \return The carrier bandwidth in frame in hertz.
   */
  inline double GetCarrierBandwidthHz (SatEnums::CarrierBandwidthType_t bandwidthType) const
  {
    double bandwidth = 0.0;

    switch (bandwidthType)
    {
      case SatEnums::ALLOCATED_BANDWIDTH:
        bandwidth = m_btu->GetAllocatedBandwidthHz();
        break;

      case SatEnums::OCCUPIED_BANDWIDTH:
        bandwidth = m_btu->GetOccupiedBandwidthHz();
        break;

      case SatEnums::EFFECTIVE_BANDWIDTH:
        bandwidth = m_btu->GetEffectiveBandwidthHz();
        break;

      default:
        NS_FATAL_ERROR ("Invalid bandwidth type!!!");
        break;
    }

    return bandwidth;
  }

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
   * \return      The requested time slot count of frame.
   */
  inline uint32_t GetTimeSlotCount () const { return m_timeSlotConfs.size(); }

  /**
   * Get time slot conf of the frame. Possible values for id are from 0 to 2047.
   *
   * \param index Id of the time slot requested.
   * \return      The requested time slot conf of frame.
   */
  Ptr<SatTimeSlotConf> GetTimeSlotConf (uint16_t index) const;

  /**
   * Get time slot ids of the specific carrier.
   *
   * \param carrierId Id of the carrier which time slots are requested.
   * \return  List (map) containing timeslots.
   */
  SatTimeSlotIdContainer_t GetTimeSlotIds (uint32_t carrierId) const;

  /**
   * Get state if frame is random access frame.
   *
   * \return Is frame random access frame [true or false]
   */
  inline bool IsRandomAccess () const { return m_isRandomAccess;}

private:
  //
  typedef std::multimap<uint32_t, uint16_t> SatCarrierTimeSlotMap_t;

  double    m_bandwidthHz;
  double    m_durationInSeconds;
  uint16_t  m_nextTimeSlotId;
  bool      m_isRandomAccess;

  Ptr<SatBtuConf>         m_btu;
  uint32_t                m_carrierCount;
  SatTimeSlotConfContainer_t   m_timeSlotConfs;
  SatCarrierTimeSlotMap_t  m_carrierTimeSlotIds;
};


/**
 * \ingroup satellite
 * \brief This abstract class defines and implements interface of configuration for super frames
 */
class SatSuperframeConf : public Object
{
public:
  typedef std::vector<Ptr<SatFrameConf> > SatFrameConfList_t;

  static const uint8_t m_maxFrameCount = 10;
  static const uint32_t m_maxFrameConfigTypeIndex = 3;

  /**
   * Template method to convert number to string
   * \param number number to convert as string
   * \return number as string
   */
  template <class T>
  static std::string GetNumberAsString (T number)
  {
    std::stringstream ss;   //create a string stream
    ss << number;           //add number to the stream

    return ss.str();
  }

  /**
   * Method to convert frame index to frame name.
   *
   * \param index index to convert as frame name
   * \return frame name
   */
  static std::string GetIndexAsFrameName(uint32_t index);

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf ();

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf ();

  void AddFrameConf (Ptr<SatFrameConf> conf);

  /**
   * Get bandwidth of the super frame.
   *
   * \return The bandwidth of super frame in hertz.
   */
  inline double GetBandwidthHz () const { return m_usedBandwidthHz; }

  /**
   * Get duration of super frame.
   *
   * \return The duration of super frame in seconds.
   */
  inline double GetDurationInSeconds () const { return m_durationInSeconds; }

  /**
   * Get frame conf of the super frame.
   *
   * \param id    Id of the frame requested.
   * \return      The requested frame conf of the super frame.
   */
  Ptr<SatFrameConf> GetFrameConf (uint8_t id) const;

  /**
   * Get carrier id of the super frame. Converts frame specific id to super frame specific id.
   *
   * \param frameId         Id of the frame requested.
   * \param frameCarrierId  Id of the carrier inside frame requested.
   *
   * \return The requested carrier id of the super frame.
   */
  uint32_t GetCarrierId (uint8_t frameId, uint16_t frameCarrierId) const;

  /**
   * Get carrier count in the super frame.
   *
   * \return The super frame carrier count.
   */
  uint32_t GetCarrierCount () const;

  /**
   * Get the center frequency of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside super frame which center frequency is requested.
   *
   * \return The center frequency of the requested carrier.
   */
  double GetCarrierFrequencyHz (uint32_t carrierId) const;

  /**
   * Get the bandwidth of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside super frame which bandwidth is requested.
   * \param bandwidthType Type of bandwidth requested.
   *
   * \return The bandwidth of the requested carrier.
   */
  double GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const;

  /**
   * Configures super frame configuration according to set attributes.
   *
   * \param bandwidthHz Allocated bandwidth for super frame.
   * \param targetDuration Target duration for super frame sequence
   * \param waveFormConf Wave Form Configuration
   */
  void Configure (double allocatedBandwidthHz, Time targetDuration, Ptr<SatWaveformConf> waveFormConf);

  /**
   * Do frame specific configuration as needed
   *
   */
  virtual void DoConfigure () = 0;

  /**
   * Get RA channel time slots
   *
   * \param raChannel RA channel, which slot are requested
   * \return RA channel time slots
   */
  SatFrameConf::SatTimeSlotIdContainer_t GetRaSlots (uint32_t raChannel);

  /**
   * Get the number of the RA channels in super frame configuration.
   *
   * \return Number of the RA channels
   */
  uint32_t GetRaChannelCount () const;

  /**
   * Get RA channel frame ID.
   *
   * \param raChannel RA channel, which frame ID is requested
   * \return RA channel frame ID
   */
  uint8_t GetRaChannelFrameId (uint32_t raChannel) const;

  /**
   * Set number of frames to be used in super frame.
   *
   * \param frameCount Number of the frames in use
   */
  inline void SetFrameCount (uint8_t frameCount) { m_frameCount = frameCount; }

  /**
   * Get number of frames to be used in super frame.
   *
   * \return Number of the frames in use
   */
  inline uint8_t GetFrameCount () const { return m_frameCount; }

  /**
   * Set frame configuration type to be used in super frame.
   *
   * \param configType index of the frame configuration type
   */
  inline void SetConfigType (uint32_t index) { m_configTypeIndex = index; }

  /**
   * Get frame configuration type to be used in super frame.
   * \return index of the frame configuration type
   */
  inline uint32_t GetConfigType () const { return m_configTypeIndex; }

  // Frame specific getter and setter method for attributes (called by methods of objects derived from this object)
  void SetFrameAllocatedBandwidthHz (uint8_t frameIndex, double bandwidhtHz);
  void SetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex, double bandwidhtHz);
  void SetFrameCarrierSpacing (uint8_t frameIndex, double spacing);
  void SetFrameCarrierRollOff (uint8_t frameIndex, double rollOff);
  void SetFrameRandomAccess (uint8_t frameIndex, bool randomAccess);

  double GetFrameAllocatedBandwidthHz (uint8_t frameIndex) const;
  double GetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex) const;
  double GetFrameCarrierSpacing (uint8_t frameIndex) const;
  double GetFrameCarrierRollOff (uint8_t frameIndex) const;
  bool GetFrameIsRandomAccess (uint8_t frameIndex) const;

private:
  typedef std::pair<uint8_t, uint32_t> RaChannelInfo_t;

  double    m_usedBandwidthHz;
  double    m_durationInSeconds;

  uint8_t   m_frameCount;
  uint32_t  m_configTypeIndex;

  double    m_frameAllocatedBandwidth[m_maxFrameCount];
  double    m_frameCarrierAllocatedBandwidth[m_maxFrameCount];
  double    m_frameCarrierSpacing[m_maxFrameCount];
  double    m_frameCarrierRollOff[m_maxFrameCount];
  bool      m_frameIsRandomAccess[m_maxFrameCount];

  SatFrameConfList_t            m_frames;
  std::vector<RaChannelInfo_t>  m_raChannels;
  uint32_t                      m_carrierCount;

public:
  // macro to ease definition of access methods for frame specific attributes
  #define FRAME_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
    inline void SetFrame ## index ## AllocatedBandwidthHz (double value)  \
      { return SetFrameAllocatedBandwidthHz (index, value); } \
    inline double GetFrame ## index ## AllocatedBandwidthHz () const  \
      { return GetFrameAllocatedBandwidthHz (index); }  \
    inline void SetFrame ## index ## CarrierAllocatedBandwidthHz (double value)  \
      { return SetFrameCarrierAllocatedBandwidthHz (index, value); } \
    inline double GetFrame ## index ## CarrierAllocatedBandwidthHz () const      \
      { return GetFrameCarrierAllocatedBandwidthHz (index); } \
    inline void SetFrame ## index ## CarrierSpacing (double value)  \
      { return SetFrameCarrierSpacing (index, value); } \
    inline double GetFrame ## index ## CarrierSpacing () const      \
      { return GetFrameCarrierSpacing (index); } \
    inline void SetFrame ## index ## CarrierRollOff (double value)  \
      { return SetFrameCarrierRollOff (index, value); } \
    inline double GetFrame ## index ## CarrierRollOff () const      \
      { return GetFrameCarrierRollOff (index); } \
    inline void SetFrame ## index ## RandomAccess (bool value)  \
      { return SetFrameRandomAccess (index, value); } \
    inline double GetFrame ## index ## RandomAccess () const      \
      { return GetFrameIsRandomAccess (index); }

  // Access method definition for frame specific attributes
  // there should be as many macro calls as m_maxFrameCount defines
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (0);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (1);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (2);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (3);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (4);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (5);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (6);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (7);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (8);
  FRAME_ATTRIBUTE_ACCESSOR_DEFINE (9);

};

/**
 * \ingroup satellite
 * \brief This class implements super frame configuration 0
 */
class SatSuperframeConf0 : public SatSuperframeConf
{
public:

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf0 ();

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf0 ();

  virtual void DoConfigure ();

private:

};

/**
 * \ingroup satellite
 * \brief This class implements super frame configuration 1
 */
class SatSuperframeConf1 : public SatSuperframeConf
{
public:

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf1 ();

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf1 ();

  virtual void DoConfigure ();

private:

};

/**
 * \ingroup satellite
 * \brief This class implements super frame configuration 2
 */
class SatSuperframeConf2 : public SatSuperframeConf
{
public:

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf2 ();

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf2 ();

  virtual void DoConfigure ();

private:

};

/**
 * \ingroup satellite
 * \brief This class implements super frame configuration 3
 */
class SatSuperframeConf3 : public SatSuperframeConf
{
public:

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf3 ();

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf3 ();

  virtual void DoConfigure ();

private:

};


} // namespace ns3

#endif // SATELLITE_FRAME_CONF_H
