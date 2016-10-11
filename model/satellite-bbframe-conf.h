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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_BBFRAME_CONF_H
#define SATELLITE_BBFRAME_CONF_H

#include <map>
#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/simple-ref-count.h>
#include <ns3/nstime.h>
#include <ns3/satellite-enums.h>

namespace ns3 {

class SatLinkResultsDvbS2;

/**
 * \ingroup satellite
 *
 * \brief This class implements the content of one individual DVB-S2 waveform.
 * Waveform is defined by modulation scheme, coding rate, payload size in bytes and
 * frame duration in Time.
*/

class SatDvbS2Waveform : public SimpleRefCount<SatDvbS2Waveform>
{
public:
  /**
   * Default constructor for SatDvbS2Waveform
   */
  SatDvbS2Waveform ();

  /**
   * \brief Constructor for SatWaveform
   * \param modcod MODCOD
   * \param fType BB frame type (short, normal)
   * \param frameLen BB frame length in Time
   * \param payloadBits Payload in bits
   */
  SatDvbS2Waveform (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t fType, Time frameLen, uint32_t payloadBits);

  /**
   * \brief Get MODCOD of this waveform
   * \return MODCOD enum
   */
  SatEnums::SatModcod_t GetModcod () const;

  /**
   * \brief Get BB frame type
   * \return BB frame type enum
   */
  SatEnums::SatBbFrameType_t GetBbFrameType () const;

  /**
   * \brief Get payload of this waveform in bits
   * \return Payload in bits
   */
  uint32_t GetPayloadInBits () const;

  /**
   * \brief Get the frame duration in Time
   * \return Frame duration in Time
   */
  Time GetFrameDuration () const;

  /**
   * \brief Set the C/No requirement of the waveform  in linear domain.
   * based on the used link results.
   * \param cnoRequirement C/No Requirement
   */
  void SetCNoRequirement (double cnoRequirement);

  /**
   * \brief Get C/No requirement corresponding a given BLER target.
   * \return C/No threshold in linear format
   */
  double GetCNoRequirement () const;

  /**
   * \brief Dump the contents of the waveform.
   */
  void Dump () const;

private:
  /**
   * MODCOD
   */
  SatEnums::SatModcod_t m_modcod;

  /**
   * Frame type: short, normal
   */
  SatEnums::SatBbFrameType_t m_frameType;

  /**
   * Frame duration in Time
   */
  Time m_frameDuration;

  /**
   * Payload in bits
   */
  uint32_t m_payloadBits;

  /**
   * C/No requirement calculated with a certain BLER target
   * from the link results
   */
  double m_cnoRequirement;
};

/**
 * \ingroup satellite
 *
 * \brief This class implements the BB Frame configurations for DVB-S2. Available
 * waveforms are for two BB frame types (short and normal) and for all supported
 * DVB-S2 MODCODs defined in SatEnums.
 */
class SatBbFrameConf : public Object
{
public:
  /**
   * BBFrame usage modes.
   */
  typedef enum
  {
    SHORT_FRAMES,          //!< SHORT_FRAMES
    NORMAL_FRAMES,         //!< NORMAL_FRAMES
    SHORT_AND_NORMAL_FRAMES //!< SHORT_AND_NORMAL_FRAMES
  } BbFrameUsageMode_t;

  /**
   * Default constructor
   */
  SatBbFrameConf ();

  /**
   * SatBbFrameConf constructor
   * \param symbolRate Symbol rate in baud
   */
  SatBbFrameConf (double symbolRate);

  /**
   * Destructor for SatBbFrameConf
   */
  virtual ~SatBbFrameConf ();

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
   * Define type waveformMap_t
   */
  typedef std::map<std::pair<SatEnums::SatModcod_t, SatEnums::SatBbFrameType_t>, Ptr<SatDvbS2Waveform> > waveformMap_t;

  /**
   * \brief Get BB frame header size in bytes.
   *
   * \return BB frame header size in bytes
   */
  inline uint32_t GetBbFrameHeaderSizeInBytes () const
  {
    return m_bbFrameHeaderSizeInBytes;
  }

  /**
   * \brief Get configured BB frame high occupancy threshold.
   *
   * \return BB frame high occupancy threshold
   */
  inline double GetBbFrameHighOccupancyThreshold () const
  {
    return m_bbFrameHighOccupancyThreshold;
  }

  /**
   * \brief Get configured BB frame low occupancy threshold.
   *
   * \return BB frame low occupancy threshold
   */
  inline double GetBbFrameLowOccupancyThreshold () const
  {
    return m_bbFrameLowOccupancyThreshold;
  }

  /**
   * Get configured BB frame usage mode.
   *
   * \return BB frame usage mode
   */
  inline SatBbFrameConf::BbFrameUsageMode_t GetBbFrameUsageMode () const
  {
    return m_bbFrameUsageMode;
  }

  /**
   * \brief Initialize the C/No requirements for a given BLER target.
   *
   * \param linkResults DVB-S2 link results
   */
  void InitializeCNoRequirements ( Ptr<SatLinkResultsDvbS2> linkResults );

  /**
   * \brief Get the dummy frame duration in Time.
   *
   * \return The dummy BBFrame length in Time
   */
  Time GetDummyBbFrameDuration () const;

  /**
   * \brief Get the BB frame frame duration.
   *
   * \param modcod MODCOD
   * \param frameType BB frame type: short, normal
   */
  Time GetBbFrameDuration (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * \brief Get the BB frame payload in bits.
   * \param modcod MODCOD
   * \param frameType BB frame type: short, normal
   */
  uint32_t GetBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * \brief Get the best MODCOD with a given BB frame type.
   * \param cNo C/No of the UT to be scheduled
   * \param frameType Used BBFrame type (short OR normal)
   * \return SatModcod_t The best MODCOD
   */
  SatEnums::SatModcod_t GetBestModcod (double cNo, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * Get the default MODCOD
   * \return SatModcod_t The default MODCOD
   */
  SatEnums::SatModcod_t GetDefaultModCod () const;

  /**
   * \brief Get the most robust MODCOD with a given BB frame type.
   * \param frameType Used BBFrame type (short OR normal)
   * \return SatModcod_t The most robust MODCOD
   */
  SatEnums::SatModcod_t GetMostRobustModcod (SatEnums::SatBbFrameType_t frameType) const;

  /**
   * \brief Dump waveform details for debugging purposes
   */
  void DumpWaveforms () const;

private:
  /**
   * \brief Calculate the BBrame higher layer payload in bits.
   * \param modcod Used MODCOD in the BBFrame
   * \param frameType Used BBFrame type (short OR normal)
   * \return The maximum payload in bits
   */
  uint32_t CalculateBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * \brief Calculate the BBFrame duration in Time
   * \param modcod Used MODCOD in the BBFrame
   * \param frameType Used BBFrame type (short OR normal)
   * \param symbolRate The symbol rate of the scheduled carrier
   * \return The BBFrame duration in Time
   */
  Time CalculateBbFrameDuration (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * Symbol rate in baud
   */
  double m_symbolRate;

  /**
   * The size of the (data) slot in symbols
   */
  uint32_t m_symbolsPerSlot;

  /**
   * The size of the pilot block in symbols
   */
  uint32_t m_pilotBlockInSymbols;

  /**
   * Interval of pilot block in slots
   */
  uint32_t m_pilotBlockIntervalInSlots;

  /**
   * The PL header size in slots
   */
  uint32_t m_plHeaderInSlots;

  /**
   * Dummy BBFrame length in slots
   */
  uint32_t m_dummyFrameInSlots;

  /**
   * The BB frame header size in bytes
   */
  uint32_t m_bbFrameHeaderSizeInBytes;

  /**
   * The BB frame high occupancy threshold in range 0 - 1
   */
  double m_bbFrameHighOccupancyThreshold;

  /**
   * The BB frame low occupancy threshold in range 0 - 1
   */
  double m_bbFrameLowOccupancyThreshold;

  /**
   * Block error rate target for the waveforms. Default value
   * set as an attribute to 10^(-5).
   */
  double m_targetBler;

  /**
   * Flag to indicate whether ACM is enabled or disabled. If ACM is
   * disabled, the m_defaultModCod is used.
   */
  bool m_acmEnabled;

  /**
   * Default MODCOD is used
   * - For broadcast control messages
   * - When ACM is disabled
   * - When there is not valid C/No information
   *
   * TODO: The attribute for m_defaultModCod does not currently accept
   * all MODCODs due to maximum arguments limitation (<=22) of MakeEnumChecker (...)
   */
  SatEnums::SatModcod_t m_defaultModCod;

  std::map<uint32_t, uint32_t> m_shortFramePayloadInSlots;
  std::map<uint32_t, uint32_t> m_normalFramePayloadInSlots;

  /**
   * Available "waveforms", i.e. MODCOD + BBFrame combinations
   */
  waveformMap_t m_waveforms;

  /**
   * BBFrame usage mode.
   */
  BbFrameUsageMode_t m_bbFrameUsageMode;

  /**
   * The most robust MODCOD for short frame.
   */
  SatEnums::SatModcod_t m_mostRobustShortFrameModcod;

  /**
   * The most robust MODCOD for long frame.
   */
  SatEnums::SatModcod_t m_mostRobustNormalFrameModcod;
};

} // namespace ns3

#endif // SATELLITE_BBFRAME_CONF_H
