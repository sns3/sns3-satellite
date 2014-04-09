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

#ifndef SATELLITE_WAVE_FORM_CONF_H
#define SATELLITE_WAVE_FORM_CONF_H

#include <vector>
#include "ns3/simple-ref-count.h"
#include "ns3/object.h"
#include "ns3/satellite-link-results.h"

namespace ns3 {


/**
 * \ingroup satellite
 * \brief This class implements the usable waveform configurations of DVB-RCS2
 */


class SatWaveform : public SimpleRefCount<SatWaveform>
{
public:
  /**
   * Default constructor for SatWaveform
   */
  SatWaveform ();

  /**
   * Constructor for SatWaveform
   * \param wfId waveform id
   * \param modulatedBits modulated bits
   * \param codingRate coding rate
   * \param payloadBytes payload in bytes
   * \param lengthInSymbols duration in symbols
   */
  SatWaveform (uint32_t wfId, uint32_t modulatedBits, double codingRate, SatEnums::SatModcod_t modcod, uint32_t payloadBytes, uint32_t lengthInSymbols);

  /**
   * Get waveform id
   * \return payload in bits
   */
  uint32_t GetWaveformId () const;

  /**
   * Get MODCOD enum
   * \return SatModCod_t MODCOD enum
   */
  SatEnums::SatModcod_t GetModCod () const;

  /**
   * Get payload of a waveform in bytes
   * \return payload in bytes
   */
  uint32_t GetPayloadInBytes () const;

  /**
   * Get burst length of the waveform in symbols
   * \return burst length
   */
  uint32_t GetBurstLengthInSymbols () const;

  /**
   * Get/calculate the burst duration of a waveform based on symbol rate
   * \param symbolRateInBaud symbol rate
   * \return burst duration
   */
  Time GetBurstDuration (double symbolRateInBaud) const;

  /**
   * Get/calculate the spectral efficiency of a waveform
   * \param carrierBandwidthInHz
   * \param symbolRateInBaud symbol rate
   * \return spectral efficiency in bits/s/Hz
   */
  double GetSpectralEfficiency (double carrierBandwidthInHz, double symbolRateInBaud) const;

  /**
   * Get/calculate the throughput of a waveform based on symbol rate
   * \param symbolRateInBaud symbol rate
   * \return throughput in bits per second
   */
  double GetThroughputInBitsPerSecond (double symbolRateInBaud) const;

  /**
   * Get the C/No threshold of the waveform in linear domain
   * \param symbolRateInBaud symbol rate
   * \return C/No threshold
   */
  double GetCNoThreshold (double symbolRateInBaud) const;

  /**
   * Set the Eb/No requirement of the waveform  in linear domain
   * based on the used link results
   * \param ebnoRequirement EbNo requirement
   */
  void SetEbNoRequirement (double ebnoRequirement);

  /**
   * Dump the contents of the waveform. For spectral efficiency calculation,
   * the total carrier bandwidth and symbol rate are needed.
   * \param carrierBandwidthInHz Total carrier bandwidth including e.g. guard band.
   * \param symbolRateInBaud Effective symbol rate where guard band and roll-off has been deduced.
   */
  void Dump (double carrierBandwidthInHz, double symbolRateInBaud) const;

private:

  /**
   * Id of this waveform
   */
  uint32_t m_waveformId;

  /**
   * Modulated bits
   * QPSK = 2
   * 8PSK = 3
   * 16QAM = 4
   */
  uint32_t m_modulatedBits;

  /**
   * Coding rate
   */
  double m_codingRate;

  /**
   * MODCOD enum
   */
  SatEnums::SatModcod_t m_modCod;

  /**
   * Payload in bytes
   */
  uint32_t m_payloadBytes;

  /**
   * Length of the burst in symbols
   */
  uint32_t m_lengthInSymbols;

  /**
   * Eb/No threshold calculated with a certain BLER target
   * from the link results
   */
  double m_ebnoRequirement;
};


class SatWaveformConf : public Object
{
public:
  typedef std::vector<uint32_t> BurstLengthContainer_t;
  /**
   * Default constructor, which is not to be used
   */
  SatWaveformConf ();

  /**
   * Constructor
   */
  SatWaveformConf (std::string filePathName);

  /**
   * Destructor for SatWaveformConf
   */
  virtual ~SatWaveformConf ();

  static TypeId GetTypeId (void);

  /**
   * Initialize the Eb/No requirements of the waveforms based on
   * the used DVB-RCS2 link results.
   * \param linkResults Pointer to DVB-RCS2 link results
   */
  void InitializeEbNoRequirements( Ptr<SatLinkResultsDvbRcs2> linkResults );

  /**
   * Get the details of a certain waveform
   * \param wfId Waveform id
   * \return SatWaveform holding all the details of a certain wf
   */
  Ptr<SatWaveform> GetWaveform (uint32_t wfId) const;

  /**
   * Get MODCOD enum corresponding to a waveform id
   * \param wfId Waveform id
   * \return SatModcod_t MODCOD enum
   */
  SatEnums::SatModcod_t GetModCod (uint32_t wfId) const;

  /**
   * Get default waveform id
   * \return SatWaveform id of the configuration holding all the details of the default wf
   */
  uint32_t GetDefaultWaveformId () const;

  /**
   * Get default burst length
   * \return Configured default burst length.
   */
  inline uint32_t GetDefaultBurstLength () const { return GetWaveform (m_defaultWfId)->GetBurstLengthInSymbols (); }

  /**
   * Get supported burst lengths.
   * \return Supported burst lengths.
   */
  inline const BurstLengthContainer_t& GetSupportedBurstLengths () const { return m_supportedBurstLengthsInSymbols;}

  /**
   * Get the best waveform id based on UT's C/No and C/No thresholds
   * JPU: Note, that this algorithm is not final, but just a skeleton which shall be enhanced
   * when implementing the actual NCC RTN link burst scheduler algorithm!
   * \param cno UTs estimated C/No
   * \param symbolRateInBaud Frame's symbol rate used for waveform C/No requirement calculation
   * \param wfId Waveform id variable used for passing the best waveform id to the client
   * \param burstLength Requested burst length in symbols
   * \return boolean value presenting whether or not a suitable waveform was found.
   */
  bool GetBestWaveformId (double cno, double symbolRateInBaud, uint32_t& wfId, uint32_t burstLength = SHORT_BURST_LENGTH) const;

  /**
   * Dump the contents of the waveform. For spectral efficiency calculation,
   * the total carrier bandwidth and symbol rate are needed.
   * \param carrierBandwidthInHz Total carrier bandwidth including e.g. guard band.
   * \param symbolRateInBaud Effective symbol rate where guard band and roll-off has been deduced.
   */
  void Dump (double carrierBandwidthInHz, double symbolRateInBaud) const;

  inline uint32_t GetMinWfId () {return m_minWfId;};
  inline uint32_t GetMaxWfId () {return m_maxWfId;};

  /**
   * Static variables defining the available burst lengths
   */
  static const uint32_t SHORT_BURST_LENGTH = 536;
  static const uint32_t LONG_BURST_LENGTH = 1616;

private:

  /**
   * Read the waveform table from a file
   * \param filePathName path and file name
   */
  void ReadFromFile (std::string filePathName);

  /**
   * Convert modulated bits and coding rate to a MODCOD enum
   * \param modulatedBits Modulated bits of the MODCOD
   * \param codingRateNumerator Coding rate numerator of the MODCOD
   * \param codingRateDenominator Coding rate denominator of the MODCOD
   * \return SatEnums::SatModcod_t MODCOD enum
   */
  SatEnums::SatModcod_t ConvertToModCod (uint32_t modulatedBits, uint32_t codingRateNumerator, uint32_t codingRateDenominator) const;

  /**
   * Container of the waveforms
   */
  std::map< uint32_t, Ptr<SatWaveform> > m_waveforms;

  /**
   * Block error rate target for the waveforms. Default value
   * set as an attribute to 10^(-5).
   */
  double m_targetBLER;

  /**
   * Flag to indicate whether ACM is enabled or disabled. If ACM is
   * disabled, the m_defaultWfId is used.
   */
  bool m_acmEnabled;

  /**
   * Default waveform id
   */
  uint32_t m_defaultWfId;

  /**
   * Minimum and maximum waveform ids. Note, that currently it is
   * assumed that all the wfs between min and max are valid!
   */
  uint32_t m_minWfId;
  uint32_t m_maxWfId;

  /**
   * Container to store supported burst lengths.
   */
  BurstLengthContainer_t  m_supportedBurstLengthsInSymbols;
};

} // namespace ns3

#endif // SATELLITE_WAVE_FORM_CONF_H
