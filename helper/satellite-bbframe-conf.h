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
#include "ns3/object.h"
#include "ns3/satellite-enums.h"
#include "ns3/satellite-link-results.h"

namespace ns3 {


/**
 * \ingroup satellite
 * \brief This class implements the BBFrame configuration for DVB-S2.
 */

class SatDvbS2Waveform : public SimpleRefCount<SatDvbS2Waveform>
{
public:
  /**
   * Default constructor for SatDvbS2Waveform
   */
  SatDvbS2Waveform ();

  /**
   * Constructor for SatWaveform
   * \param modcod MODCOD
   * \param fType BB frame type (short, normal)
   * \param frameLen BB frame length in Time
   * \param payloadBits payload in bits
   */
  SatDvbS2Waveform (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t fType, Time frameLen, uint32_t payloadBits);

  /**
   * Get MODCOD of this waveform
   */
  SatEnums::SatModcod_t GetModcod () const;

  /**
   * Get BB frame type
   */
  SatEnums::SatBbFrameType_t GetBbFrameType () const;

  /**
   * Get payload of this waveform in bits
   */
  uint32_t GetPayloadInBits () const;

  /**
   * Get the frame length in Time
   */
  Time GetFrameLength () const;

  /**
   * Set the C/No requirement of the waveform  in linear domain
   * based on the used link results
   * \param esnoThreshold EsNo threshold
   */
  void SetCNoRequirement (double cnoRequirement);

  /**
   * Get C/No requirement corresponding a given PER target
   * \return C/No threshold in linear format
   */
  double GetCNoRequirement () const;

  /**
   * Dump the contents of the waveform. For spectral efficiency calculation,
   * the total carrier bandwidth and symbol rate are needed.
   * \param carrierBandwidthInHz Total carrier bandwidth including e.g. guard band.
   * \param symbolRateInBaud Effective symbol rate where guard band and roll-off has been deduced.
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
   * Frame length in Time
   */
  Time m_frameLength;

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


class SatBbFrameConf : public Object
{
public:
  /**
   * Default constructor
   */
  SatBbFrameConf ();

  /**
   * SatBbFrameConf constructor
   */
  SatBbFrameConf (double symbolRate);

  typedef std::map<std::pair<SatEnums::SatModcod_t, SatEnums::SatBbFrameType_t>, Ptr<SatDvbS2Waveform> > waveformMap_t;

  /**
   * Destructor for SatBbFrameConf
   */
  virtual ~SatBbFrameConf ();

  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Initialize the C/No requirements for a given PER target
   * \param linkResults DVB-S2 link results
   */
  void InitializeCNoRequirements( Ptr<SatLinkResultsDvbS2> linkResults );

  /**
   * Get the dummy frame length in Time
   * \return The dummy BBFrame length in Time
   */
  Time GetDummyBbFrameLength () const;

  /**
   * Get the BB frame frame length
   * \param modcod MODCOD
   * \param frameType BB frame type: short, normal
   */
  Time GetBbFrameLength (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * Get the BB frame payload in bits
   * \param modcod MODCOD
   * \param frameType BB frame type: short, normal
   */
  uint32_t GetBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * Get the best MODCOD with a given BB frame type
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


private:

  /**
   * Calculate the BBrame higher layer payload in bytes
   * \param modcod Used MODCOD in the BBFrame
   * \param frameType Used BBFrame type (short OR normal)
   * \return The maximum payload in bytes
   */
  uint32_t CalculateBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * Calculate the BBFrame length in Time
   * \param modcod Used MODCOD in the BBFrame
   * \param frameType Used BBFrame type (short OR normal)
   * \param symbolRate The symbol rate of the scheduled carrier
   * \return The BBFrame length in Time
   */
  Time CalculateBbFrameLength (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;

  /**
   * Symbol rate
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
   * Packet error rate target for the waveforms. Default value
   * set as an attribute to 10^(-5).
   */
  double m_perTarget;

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

  std::map<uint32_t, uint32_t> m_payloadsShortFrame;
  std::map<uint32_t, uint32_t> m_payloadsNormalFrame;

  /**
   * Available "waveforms", i.e. MODCOD + BBFrame combinations
   */
  waveformMap_t m_waveforms;
};

} // namespace ns3

#endif // SATELLITE_BBFRAME_CONF_H
