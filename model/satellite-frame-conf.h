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

#include "satellite-enums.h"
#include "satellite-const-variables.h"
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
   * \param bandwidthInHz     Allocated bandwidth of BTU in Hertz
   * \param rollOff           Roll-off factor
   * \param spacing           Spacing factor
   */
  SatBtuConf (double bandwidthInHz, double rollOff, double spacing);

  /**
   * Destructor for SatBtuConf
   */
  ~SatBtuConf ();

  /**
   * Get bandwidth of BTU.
   *
   * \return The bandwidth of BTU in hertz.
   */
  inline double GetAllocatedBandwidthInHz () const
  {
    return m_allocatedBandwidthInHz;
  }

  /**
   * Get occupied bandwidth of BTU.
   *
   * \return The occupied bandwidth of BTU in hertz.
   */
  inline double GetOccupiedBandwidthInHz () const
  {
    return m_occupiedBandwidthInHz;
  }

  /**
   * Get occupied bandwidth of BTU.
   *
   * \return The occupied bandwidth of BTU in hertz.
   */
  inline double GetEffectiveBandwidthInHz () const
  {
    return m_effectiveBandwidthInHz;
  }

  /**
   * Get symbol rate of BTU.
   *
   * \return The symbol rate of BTU in bauds.
   */
  inline double GetSymbolRateInBauds () const
  {
    return GetEffectiveBandwidthInHz ();
  }

private:
  double m_allocatedBandwidthInHz;
  double m_occupiedBandwidthInHz;
  double m_effectiveBandwidthInHz;  // i.e. symbol rate
  Time m_duration;                  // duration field reserved, but not used currently
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for time slots (for super frames / frames)
 */

class SatTimeSlotConf : public SimpleRefCount<SatTimeSlotConf>
{
public:
  /**
   * Types for time slot.
   */
  typedef enum
  {
    SLOT_TYPE_C, //!< Control slot
    SLOT_TYPE_TR, //!< Traffic slot
    SLOT_TYPE_TRC //!< Control or traffic slot

  } SatTimeSlotType_t;

  /**
   * Default constructor for SatTimeSlotConf
   */
  SatTimeSlotConf ();

  /**
   * Constructor for SatTimeSlotConf
   *
   * \param startTime       Start time of time slot in side frame.
   * \param waveFormId      Wave form id of time slot
   * \param carrierId       Carrier id of time slot
   * \param slotType        Time slot type
   */
  SatTimeSlotConf (Time startTime, uint32_t waveFormId, uint16_t carrierId, SatTimeSlotConf::SatTimeSlotType_t slotType);

  /**
   * Destructor for SatTimeSlotConf
   */
  ~SatTimeSlotConf ();

  /**
   * Get start time of time slot. (inside frame)
   *
   * \return The start time of time slot.
   */
  inline Time GetStartTime () const
  {
    return m_startTime;
  }

  /**
   * Get wave form id of time slot.
   *
   * \return The wave form id of time slot.
   */
  inline uint32_t GetWaveFormId () const
  {
    return m_waveFormId;
  }

  /**
   * Get carrier id of time slot (inside frame).
   *
   * \return The carrier id of time slot.
   */
  inline uint16_t GetCarrierId () const
  {
    return m_frameCarrierId;
  }

  /**
   * Set RC index of the time slot.
   *
   * \param rcIndex RC index of the time slot.
   */
  inline void SetRcIndex (uint8_t rcIndex)
  {
    m_rcIndex = rcIndex;
  }

  /**
   * Get RC index of the time slot.
   *
   * \return RC index of the time slot.
   */
  inline uint8_t GetRcIndex ()
  {
    return m_rcIndex;
  }

  /**
   * Get time slot type.
   *
   * \return Type of the time slot.
   */
  inline SatTimeSlotConf::SatTimeSlotType_t GetSlotType () const
  {
    return m_slotType;
  }

  /**
   * Set time slot type.
   *
   * \param slotType Type of the time slot to set.
   */
  inline void SetSlotType (SatTimeSlotConf::SatTimeSlotType_t slotType)
  {
    m_slotType = slotType;
  }

private:
  Time     m_startTime;
  uint32_t m_waveFormId;
  uint16_t m_frameCarrierId;
  uint8_t  m_rcIndex;

  SatTimeSlotType_t m_slotType;
};

/**
 * \ingroup satellite
 * \brief This class implements configuration for frames (for super frames)
 */

class SatFrameConf : public SimpleRefCount<SatFrameConf>
{
public:

  /**
   * Define type SatTimeSlotConfContainer_t
   */
  typedef std::vector<Ptr<SatTimeSlotConf> > SatTimeSlotConfContainer_t;

  static const uint16_t m_maxTimeSlotCount = SatConstVariables::MAXIMUM_TIME_SLOT_ID + 1;

  /**
   * Default constructor for SatFrameConf
   */
  SatFrameConf ();

  /**
   * Constructor for SatFrameConf.
   *
   * \param bandwidthHz           Bandwidth of the frame in hertz
   * \param targetDuration        Target duration of the frame
   * \param btuConf               BTU configuration of the frame
   * \param waveformConf          Waveform configuration
   * \param isRandomAccess        Flag telling if random access frame
   * \param defaultWaveformInUse  Flag telling if default waveform should be used with frame
   * \param checkSlotLimit        Flag telling if slot limit should be checked already in frame creation phase
   */
  SatFrameConf ( double bandwidthHz, Time targetDuration, Ptr<SatBtuConf> btuConf, Ptr<SatWaveformConf> waveformConf,
                 bool isRandomAccess, bool defaultWaveformInUse, bool checkSlotLimit );

  /**
   * Destructor for SatFrameConf
   */
  ~SatFrameConf ();

  /**
   * Get time slot configuration of the frame. Possible values for id are from 0 to 2047.
   *
   * \param index Id of the time slot requested in frame.
   * \return      The requested time slot configuration of frame.
   */
  Ptr<SatTimeSlotConf> GetTimeSlotConf (uint16_t index) const;

  /**
   * Get time slot conf of the frame. Possible values for id are from 0 to Carrier count - 1.
   *
   * \param carrierId Id of carrier which time slot is requested.
   * \param index Id of the time slot requested in the carrier of the frame.
   * \return The requested time slot configuration of frame.
  */
  Ptr<SatTimeSlotConf> GetTimeSlotConf (uint16_t carrierId, uint16_t index) const;

  /**
   * Get bandwidth of the frame.
   *
   * \return The bandwidth of frame in Hertz.
   */
  inline double GetBandwidthHz () const
  {
    return m_bandwidthHz;
  }

  /**
   * Get duration of frame.
   *
   * \return The duration of frame.
   */
  inline Time GetDuration () const
  {
    return m_duration;
  }

  /**
   * Get maximum symbols in carrier
   *
   * \return maximum symbols in carrier
   */
  inline uint32_t GetCarrierMaxSymbols () const
  {
    return m_maxSymbolsPerCarrier;
  }

  /**
   * Get minimum payload of a carrier in bytes
   *
   * \return minimum payload of a carrier in bytes
   */
  inline uint32_t GetCarrierMinPayloadInBytes () const
  {
    return m_minPayloadPerCarrierInBytes;
  }

  /**
   * Get carrier center frequency in frame.
   *
   * \return The carrier bandwidth in frame in hertz.
   */
  double GetCarrierFrequencyHz ( uint16_t carrierId ) const;

  /**
   * Get carrier bandwidth in frame.
   *
   * \param bandwidthType Type of bandwidth requested.
   * \return The carrier bandwidth in frame in hertz.
   */
  double GetCarrierBandwidthHz (SatEnums::CarrierBandwidthType_t bandwidthType) const;

  /**
   * Get BTU conf of the frame.
   *
   * \return The BTU conf of frame.
   */
  inline Ptr<SatBtuConf> GetBtuConf () const
  {
    return m_btuConf;
  }

  /**
   * Get carrier count of the frame.
   *
   * \return The carrier count of the frame.
   */
  inline uint16_t GetCarrierCount ()
  {
    return m_carrierCount;
  }

  /**
   * Get time slot count of the frame.
   *
   * \return      The requested time slot count of frame.
   */
  uint16_t GetTimeSlotCount () const;

  /**
   * Get time slot of the specific carrier.
   *
   * \param carrierId Id of the carrier which time slots are requested.
   * \return  Container containing time slots.
   */
  SatTimeSlotConfContainer_t GetTimeSlotConfs (uint16_t carrierId) const;

  /**
   * Get state if frame is random access frame.
   *
   * \return Is frame random access frame [true or false]
   */
  inline bool IsRandomAccess () const
  {
    return m_isRandomAccess;
  }

  /**
   * Get waveform configuration of this frame
   */
  inline Ptr<SatWaveformConf> GetWaveformConf () const
  {
    return m_waveformConf;
  }

private:
  typedef std::map<uint16_t, SatTimeSlotConfContainer_t > SatTimeSlotConfMap_t; // key = carrier ID

  double    m_bandwidthHz;
  Time      m_duration;
  bool      m_isRandomAccess;

  Ptr<SatBtuConf>       m_btuConf;
  Ptr<SatWaveformConf>  m_waveformConf;
  uint16_t              m_carrierCount;
  uint32_t              m_maxSymbolsPerCarrier;
  uint32_t              m_minPayloadPerCarrierInBytes;
  SatTimeSlotConfMap_t  m_timeSlotConfMap;

  /**
   * Add time slot.
   *
   * \param conf  Time slot configuration added.
   * \return ID of the added time slot.
   */
  uint16_t AddTimeSlotConf ( Ptr<SatTimeSlotConf> conf);
};


/**
 * \ingroup satellite
 * \brief This abstract class defines and implements interface of configuration for super frames
 */
class SatSuperframeConf : public Object
{
public:
  /**
   * Configurable Super Frames
   */
  typedef enum
  {
    SUPER_FRAME_CONFIG_0,  //!< SUPER_FRAME_CONFIG_0
    SUPER_FRAME_CONFIG_1,  //!< SUPER_FRAME_CONFIG_1
    SUPER_FRAME_CONFIG_2,  //!< SUPER_FRAME_CONFIG_2
    SUPER_FRAME_CONFIG_3,  //!< SUPER_FRAME_CONFIG_3
  } SuperFrameConfiguration_t;

  /**
   * Define type SatFrameConfList_t
   */
  typedef std::vector<Ptr<SatFrameConf> > SatFrameConfList_t;

  /**
   * Enum for configuration types
   */
  typedef enum
  {
    CONFIG_TYPE_0, //!< Configuration type 0
    CONFIG_TYPE_1, //!< Configuration type 1
    CONFIG_TYPE_2, //!< Configuration type 2
    CONFIG_TYPE_3 //!< Configuration type 3
  } ConfigType_t;

  static const uint8_t m_maxFrameCount = 10;

  /**
   * Create pre-configured super frame configuration-
   *
   * \param conf Pre-configured super frame configuration to create
   * \return Pointer to created super frame configuration
   */
  static Ptr<SatSuperframeConf> CreateSuperframeConf (SuperFrameConfiguration_t conf);

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

    return ss.str ();
  }

  /**
   * Method to convert frame index to frame name.
   *
   * \param index index to convert as frame name
   * \return frame name
   */
  static std::string GetIndexAsFrameName (uint32_t index);

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeConf ();

  /**
   * Destructor for SatSuperframeConf
   */
  ~SatSuperframeConf ();

  /**
   * Get bandwidth of the super frame.
   *
   * \return The bandwidth of super frame in hertz.
   */
  inline double GetBandwidthHz () const
  {
    return m_usedBandwidthHz;
  }

  /**
   * Get duration of super frame.
   *
   * \return The duration of super frame in seconds.
   */
  inline Time GetDuration () const
  {
    return m_duration;
  }

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
   * Check if given carrier is random access carrier.
   *
   * \param carrierId Id of the carrier inside super frame which is requested.
   * @return true if carrier is random access, false otherwise
   */
  bool IsRandomAccessCarrier (uint32_t carrierId) const;

  /**
   * Configures super frame configuration according to set attributes.
   *
   * \param allocatedBandwidthHz Allocated bandwidth for super frame.
   * \param targetDuration Target duration for super frame sequence
   * \param waveformConf Wave Form Configuration
   */
  void Configure (double allocatedBandwidthHz, Time targetDuration, Ptr<SatWaveformConf> waveformConf);

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
  SatFrameConf::SatTimeSlotConfContainer_t GetRaSlots (uint8_t raChannel);

  /**
   * Get RA channel time slot count
   *
   * \param raChannel RA channel, which slot count is requested
   * \return RA channel time slot count
   */
  uint16_t GetRaSlotCount (uint8_t raChannel);

  /**
   * Get the number of the RA channels in super frame configuration.
   *
   * \return Number of the RA channels
   */
  uint8_t GetRaChannelCount () const;

  /**
   * Get the RA channel id (index) corresponding to given (global) carrier id.
   * Note! In case that carrier id requested does not belong to RA frame, is O eturned.
   *
   * \param carrierId
   * \return RA channel id
   */
  uint8_t GetRaChannel (uint32_t carrierId) const;

  /**
   * Get RA channel frame ID.
   *
   * \param raChannel RA channel, which frame ID is requested
   * \return RA channel frame ID
   */
  uint8_t GetRaChannelFrameId (uint8_t raChannel) const;

  /**
   * Get the payload of the RA channel time slot in bytes.
   *
   * \return Payload of the RA channel time slot [bytes]
   */
  uint32_t GetRaChannelTimeSlotPayloadInBytes (uint8_t raChannel) const;

  /**
   * Set number of frames to be used in super frame.
   *
   * \param frameCount Number of the frames in use
   */
  inline void SetFrameCount (uint8_t frameCount)
  {
    m_frameCount = frameCount;
  }

  /**
   * Get number of frames to be used in super frame.
   *
   * \return Number of the frames in use
   */
  inline uint8_t GetFrameCount () const
  {
    return m_frameCount;
  }

  /**
   * Set frame configuration type to be used in super frame.
   *
   * \param type Index of the frame configuration type
   */
  inline void SetConfigType (SatSuperframeConf::ConfigType_t type)
  {
    m_configType = type;
  }

  /**
   * Get frame configuration type to be used in super frame.
   * \return the frame configuration type
   */
  inline SatSuperframeConf::ConfigType_t GetConfigType () const
  {
    return m_configType;
  }

  // Frame specific getter and setter method for attributes (called by methods of objects derived from this object)
  void SetFrameAllocatedBandwidthHz (uint8_t frameIndex, double bandwidthHz);
  void SetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex, double bandwidthHz);
  void SetFrameCarrierSpacing (uint8_t frameIndex, double spacing);
  void SetFrameCarrierRollOff (uint8_t frameIndex, double rollOff);
  void SetFrameRandomAccess (uint8_t frameIndex, bool randomAccess);

  double GetFrameAllocatedBandwidthHz (uint8_t frameIndex) const;
  double GetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex) const;
  double GetFrameCarrierSpacing (uint8_t frameIndex) const;
  double GetFrameCarrierRollOff (uint8_t frameIndex) const;
  bool IsFrameRandomAccess (uint8_t frameIndex) const;

private:
  // first = frame ID, second = RA channel id (index)
  typedef std::pair<uint8_t, uint8_t> RaChannelInfo_t;

  double    m_usedBandwidthHz;
  Time      m_duration;

  uint8_t       m_frameCount;
  ConfigType_t  m_configType;

  double    m_frameAllocatedBandwidth[m_maxFrameCount];
  double    m_frameCarrierAllocatedBandwidth[m_maxFrameCount];
  double    m_frameCarrierSpacing[m_maxFrameCount];
  double    m_frameCarrierRollOff[m_maxFrameCount];
  bool      m_frameIsRandomAccess[m_maxFrameCount];

  SatFrameConfList_t            m_frames;
  std::vector<RaChannelInfo_t>  m_raChannels;
  uint32_t                      m_carrierCount;

  /**
   * Get frame id where given global carrier ID belongs to.
   *
   * \param carrierId Carried ID which frame ID is requested.
   * \return frame id where given global carrier ID belongs to.
   */
  uint8_t GetCarrierFrame (uint32_t carrierId) const;

  /**
   * Add frame configuration to super frame configuration.
   *
   * \param conf  Frame configuration to add super frame configuration
   */
  void AddFrameConf (Ptr<SatFrameConf> conf);

public:
  // macro to ease definition of access methods for frame specific attributes
  #define FRAME_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetFrame ## index ## AllocatedBandwidthHz (double value)  \
  { return SetFrameAllocatedBandwidthHz (index, value); \
  } \
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
  inline double IsFrame ## index ## RandomAccess () const      \
  { return IsFrameRandomAccess (index); }

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

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
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

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
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
 * \brief This class implements sup./ns3/satellite-frame-conf.h: In member function ‘double ns3::SatSuperframeConf::GetFrame0RandomAccess() const’:
 * er frame configuration 2
 */
class SatSuperframeConf2 : public SatSuperframeConf
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
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

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
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
