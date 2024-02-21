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

#include "satellite-enums.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <map>
#include <vector>

namespace ns3
{

class SatLinkResultsRtn;

/**
 * \ingroup satellite
 * \brief This class implements the content of one individual DVB-RCS2 waveform.
 * Waveform is defined by modulation scheme, coding rate, payload size in bytes and
 * burst length in symbols.
 * \see ETSI EN 301 545-2 Digital Video Broadcasting (DVB); Second Generation
 * Interactive Satellite System (DVB-RCS2); Part 2: Lower Layers for Satellite Standard
 * - Annex A Reference waveforms
 */
class SatWaveform : public SimpleRefCount<SatWaveform>
{
  public:
    /**
     * Default constructor for SatWaveform
     */
    SatWaveform();

    /**
     * Constructor for SatWaveform
     * \param wfId Waveform id
     * \param modulatedBits Modulated bits
     * \param codingRate Coding rate
     * \param modcod ModCod
     * \param payloadBytes Payload in bytes
     * \param lengthInSymbols Duration in symbols
     * \param preambleLengthInSymbols Preamble duration in symbols
     */
    SatWaveform(uint32_t wfId,
                uint32_t modulatedBits,
                double codingRate,
                SatEnums::SatModcod_t modcod,
                uint32_t payloadBytes,
                uint32_t lengthInSymbols,
                uint32_t preambleLengthInSymbols);

    /**
     * \brief Get waveform id
     * \return Waveform id
     */
    uint32_t GetWaveformId() const;

    /**
     * \brief Get MODCOD enum
     * \return MODCOD enum
     */
    SatEnums::SatModcod_t GetModCod() const;

    /**
     * \brief Get payload of a waveform in bytes
     * \return Payload in bytes
     */
    uint32_t GetPayloadInBytes() const;

    /**
     * \brief Get preamble length of the waveform in symbols
     * \return Burst length
     */
    uint32_t GetPreambleLengthInSymbols() const;

    /**
     * \brief Get burst length of the waveform in symbols
     * \return Burst length
     */
    uint32_t GetBurstLengthInSymbols() const;

    /**
     * \brief Get/calculate the preamble duration of a waveform based on symbol rate
     * \param symbolRateInBaud Symbol rate
     * \return Preamble duration
     */
    Time GetPreambleDuration(double symbolRateInBaud) const;

    /**
     * \brief Get/calculate the burst duration of a waveform based on symbol rate
     * \param symbolRateInBaud Symbol rate
     * \return Burst duration
     */
    Time GetBurstDuration(double symbolRateInBaud) const;

    /**
     * \brief Get/calculate the spectral efficiency of a waveform
     * \param carrierBandwidthInHz Carrier bandwidth in Hz
     * \param symbolRateInBaud Symbol rate in baud
     * \return spectral efficiency in bits/s/Hz
     */
    double GetSpectralEfficiency(double carrierBandwidthInHz, double symbolRateInBaud) const;

    /**
     * \brief Get/calculate the throughput of a waveform based on symbol rate
     * \param symbolRateInBaud Symbol rate in baud
     * \return Throughput in bits per second
     */
    double GetThroughputInBitsPerSecond(double symbolRateInBaud) const;

    /**
     * \brief Get the C/No threshold of the waveform in linear domain
     * \param symbolRateInBaud Symbol rate in baud
     * \return C/No threshold
     */
    double GetCNoThreshold(double symbolRateInBaud) const;

    /**
     * \brief Set the Eb/No requirement of the waveform  in linear domain
     * based on the used link results
     * \param ebnoRequirement EbNo requirement in linear domain
     */
    void SetEbNoRequirement(double ebnoRequirement);

    /**
     * \brief Dump the contents of the waveform. The total carrier bandwidth and
     * symbol rate are needed for spectral efficiency calculation.
     * \param carrierBandwidthInHz Total carrier bandwidth including e.g. guard band.
     * \param symbolRateInBaud Effective symbol rate where guard band and roll-off has been deduced.
     */
    void Dump(double carrierBandwidthInHz, double symbolRateInBaud) const;

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
     * Length of the preamble in symbols
     */
    uint32_t m_preambleLengthInSymbols;

    /**
     * Eb/No threshold calculated with a certain BLER target
     * from the link results
     */
    double m_ebnoRequirement;
};

/**
 * \ingroup satellite
 * \brief This class implements the available waveform configurations of DVB-RCS2
 * return link. Currently the waveforms 3-22 are in use: 3-12 with shorter burst
 * duration and 13-22 with longer burst duration.
 * \see ETSI EN 301 545-2 Digital Video Broadcasting (DVB); Second Generation
 * Interactive Satellite System (DVB-RCS2); Part 2: Lower Layers for Satellite Standard
 * - Annex A Reference waveforms
 */
class SatWaveformConf : public Object
{
  public:
    /**
     * Define BurstLengthContainer
     */
    typedef std::vector<uint32_t> BurstLengthContainer_t;

    /**
     * Default constructor, which is not to be used
     */
    SatWaveformConf();

    /**
     * Constructor
     * \param directoryPathName Path and to the directory of the waveform configurations
     */
    SatWaveformConf(std::string directoryPathName);

    /**
     * Destructor for SatWaveformConf
     */
    virtual ~SatWaveformConf();

    /**
     * Derived from Object
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Get the type ID of instance
     * \return the object TypeId
     */
    virtual TypeId GetInstanceTypeId(void) const;

    /**
     * Check if ACM is enabled.
     */
    inline bool IsAcmEnabled() const
    {
        return m_acmEnabled;
    }

    /**
     * \brief Initialize the Eb/No requirements of the waveforms based on
     * the used return link results.
     * \param linkResults Pointer to return link results
     */
    void InitializeEbNoRequirements(Ptr<SatLinkResultsRtn> linkResults);

    /**
     * \brief Get the details of a certain waveform
     * \param wfId Waveform id
     * \return SatWaveform holding all the details of a certain waveform
     */
    Ptr<SatWaveform> GetWaveform(uint32_t wfId) const;

    /**
     * \brief Get MODCOD enum corresponding to a waveform id
     * \param wfId Waveform id
     * \return MODCOD enum
     */
    SatEnums::SatModcod_t GetModCod(uint32_t wfId) const;

    /**
     * \brief Get default waveform id
     * \return SatWaveform id of the configuration holding all the details of the default waveform
     */
    uint32_t GetDefaultWaveformId() const;

    /**
     * \brief Get default burst length
     * \return Configured default burst length.
     */
    inline uint32_t GetDefaultBurstLength() const
    {
        return GetWaveform(m_defaultWfId)->GetBurstLengthInSymbols();
    }

    /**
     * \brief Get supported burst lengths.
     * \return Supported burst lengths.
     */
    inline const BurstLengthContainer_t& GetSupportedBurstLengths() const
    {
        return m_supportedBurstLengthsInSymbols;
    }

    /**
     * \brief Get the best waveform id based on UT's C/No and C/No thresholds
     * \param cno UTs estimated C/No
     * \param symbolRateInBaud Frame's symbol rate used for waveform C/No requirement calculation
     * \param wfId Waveform id variable used for passing the best waveform id to the client
     * \param cnoThreshold variable used for passing the C/No threshold of the selected waveform to
     * the client \param burstLength Requested burst length in symbols \return boolean value
     * presenting whether or not a suitable waveform was found.
     */
    bool GetBestWaveformId(double cno,
                           double symbolRateInBaud,
                           uint32_t& wfId,
                           double& cnoThreshold,
                           uint32_t burstLength = SHORT_BURST_LENGTH) const;

    /**
     * \brief Get the most robust waveform id based payload of the waveform in bytes
     * \param wfId Waveform id variable used for passing the best waveform id to the client
     * \param burstLength Requested burst length in symbols
     * \return boolean value presenting whether or not a suitable waveform was found.
     */
    bool GetMostRobustWaveformId(uint32_t& wfId, uint32_t burstLength = SHORT_BURST_LENGTH) const;

    /**
     * \brief Dump the contents of the waveform. The total carrier bandwidth and symbol rate
     * are needed for spectral efficiency calculation.
     * \param carrierBandwidthInHz Total carrier bandwidth including e.g. guard band.
     * \param symbolRateInBaud Effective symbol rate where guard band and roll-off has been deduced.
     */
    void Dump(double carrierBandwidthInHz, double symbolRateInBaud) const;

    /**
     * \brief Get minimum supported waveform id
     * return Minimum waveform id
     */
    inline uint32_t GetMinWfId()
    {
        return m_minWfId;
    }

    /**
     * \brief Get maximum supported waveform id
     * return Maximum waveform id
     */
    inline uint32_t GetMaxWfId()
    {
        return m_maxWfId;
    }

    /**
     * Static variable defining short burst length
     */
    static const uint32_t SHORT_BURST_LENGTH = 536;

    /**
     * Static variable defining long burst length
     */
    static const uint32_t LONG_BURST_LENGTH = 1616;

  private:
    /**
     * \brief Read the default waveform ID from a file
     * \param filePathName path and file name
     */
    void ReadFromFileDefaultWaveform(std::string filePathName);

    /**
     * \brief Read the waveform table from a file
     * \param filePathName path and file name
     */
    void ReadFromFile(std::string filePathName);

    /**
     * \brief Convert modulated bits and coding rate to a MODCOD enum
     * \param modulatedBits Modulated bits of the MODCOD
     * \param codingRateNumerator Coding rate numerator of the MODCOD
     * \param codingRateDenominator Coding rate denominator of the MODCOD
     * \return MODCOD enum
     */
    SatEnums::SatModcod_t ConvertToModCod(uint32_t modulatedBits,
                                          uint32_t codingRateNumerator,
                                          uint32_t codingRateDenominator) const;

    /**
     * Container of the waveforms
     */
    std::map<uint32_t, Ptr<SatWaveform>> m_waveforms;

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
     * Burst length used.
     */
    SatEnums::SatWaveFormBurstLength_t m_burstLength;

    /**
     * Container to store supported burst lengths.
     */
    BurstLengthContainer_t m_supportedBurstLengthsInSymbols;
};

} // namespace ns3

#endif // SATELLITE_WAVE_FORM_CONF_H
