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
   * \param modulated bits
   * \param coding rate
   * \param payload in bytes
   * \param duration in symbols
   */
  SatWaveform (uint32_t modulatedBits, double codingRate, uint32_t payloadBytes, uint32_t lengthInSymbols);

  /**
   * Get payload of a waveform in bits
   * \return payload in bits
   */
  uint32_t GetPayloadInBits () const;

  /**
   * Get burst lenght of the waveform in symbols
   * \return burst length
   */
  uint32_t GetBurstLengthInSymbols () const;

  /**
   * Get/calculate the burst duration of a waveform based on symbol rate
   * \param symbol rate
   * \return burst duration in seconds
   */
  double GetBurstDurationInSeconds (double symbolRateInBaud) const;

  /**
   * Get/calculate the spectral efficiency of a waveform
   * \return spectral efficiency in bits/s/Hz
   */
  double GetSpectralEfficiency (double carrierBandwidthInHz, double symbolRateInBaud) const;

  /**
   * Get/calculate the throughput of a waveform based on symbol rate
   * \return throughput in bits per second
   */
  double GetThroughputInBitsPerSecond (double symbolRateBaud) const;

  /**
   * Get the C/No threshold of the waveform in linear domain
   * \return C/No threshold
   */
  double GetCNoThreshold (double symbolRateInBaud) const;

  /**
   * Set the EsNo threshold of the waveform  in linear domain
   * based on the used link results
   * \param EsNo threshold
   */
  void SetEsNoThreshold (double esnoThreshold);

  /**
   * Dump the contents of the waveform. For spectral efficiency calculation,
   * the total carrier bandwidth and symbol rate are needed.
   * \param carrierBandwidthInHz Total carrier bandwidth including e.g. guard band.
   * \param symbolRateInBaud Effective symbol rate where guard band and roll-off has been deduced.
   */
  void Dump (double carrierBandwidthInHz, double symbolRateInBaud) const;

private:

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
   * Payload in bytes
   */
  uint32_t m_payloadBytes;

  /**
   * Length of the burst in symbols
   */
  uint32_t m_lengthInSymbols;

  /**
   * Es/No threshold calculated with a certain BLER target
   * from the link results
   */
  double m_esnoThreshold;
};


class SatWaveformConf : public Object
{
public:
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
   * Initialize the Es/No requirements of the waveforms based on
   * the used DVB-RCS2 link results.
   * \param Pointer to DVB-RCS2 link results
   */
  void InitializeEsNoRequirements( Ptr<SatLinkResultsDvbRcs2> linkResults );

  /**
   * Get the details of a certain waveform
   * \param wfId Waveform id
   * \return SatWaveform holding all the details of a certain wf
   */
  Ptr<SatWaveform> GetWaveform (uint32_t wfId) const;

  /**
   * Get the details of default waveform
   * \return SatWaveform holding all the details of the default wf
   */
  Ptr<SatWaveform> GetDefaultWaveform () const;

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

  /**
   * Static variables defining the available burst lengths
   */
  static const uint32_t SHORT_BURST_LENGTH = 536;
  static const uint32_t LONG_BURST_LENGTH = 1616;

private:

  /**
   * Read the waveform table from a file
   * \param path and file name
   */
  void ReadFromFile (std::string filePathName);

  /**
   * Container of the waveforms
   */
  std::map< uint32_t, Ptr<SatWaveform> > m_waveforms;

  /**
   * Default waveform id
   */
  uint32_t m_defaultWfId;

  /**
   * Packet error rate target for the waveforms. Default value
   * set as an attribute to 10^(-5).
   */
  double m_perTarget;

  /**
   * Minimum and maximum waveform ids. Note, that currently it is
   * assumed that all the wfs between min and max are valid!
   */
  uint32_t m_minWfId;
  uint32_t m_maxWfId;
};

} // namespace ns3

#endif // SATELLITE_WAVE_FORM_CONF_H
