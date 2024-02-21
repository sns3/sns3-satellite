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

#include "satellite-conf.h"

#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/satellite-const-variables.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-wave-form-conf.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/string.h"

NS_LOG_COMPONENT_DEFINE("SatConf");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatConf);

TypeId
SatConf::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatConf")
            .SetParent<Object>()
            .AddConstructor<SatConf>()
            .AddAttribute("FwdFeederLinkBandwidth",
                          "Bandwidth of the forward link.",
                          DoubleValue(2.0e9),
                          MakeDoubleAccessor(&SatConf::m_fwdFeederLinkBandwidthHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("FwdFeederLinkBaseFrequency",
                          "Base frequency of the forward feeder link band.",
                          DoubleValue(27.5e9),
                          MakeDoubleAccessor(&SatConf::m_fwdFeederLinkFreqHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("RtnFeederLinkBandwidth",
                          "Bandwidth of the return feeder link band.",
                          DoubleValue(2.0e9),
                          MakeDoubleAccessor(&SatConf::m_rtnFeederLinkBandwidthHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("RtnFeederLinkBaseFrequency",
                          "Base frequency of the return feeder link band.",
                          DoubleValue(17.7e9),
                          MakeDoubleAccessor(&SatConf::m_rtnFeederLinkFreqHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("FwdUserLinkBandwidth",
                          "Bandwidth of the forward user link band.",
                          DoubleValue(0.5e9),
                          MakeDoubleAccessor(&SatConf::m_fwdUserLinkBandwidthHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("FwdUserLinkBaseFrequency",
                          "Base frequency of the forward user link band.",
                          DoubleValue(19.7e9),
                          MakeDoubleAccessor(&SatConf::m_fwdUserLinkFreqHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("RtnUserLinkBandwidth",
                          "Bandwidth of the return user link band.",
                          DoubleValue(0.5e9),
                          MakeDoubleAccessor(&SatConf::m_rtnUserLinkBandwidthHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("RtnUserLinkBaseFrequency",
                          "Base frequency of the return user link band.",
                          DoubleValue(29.5e9),
                          MakeDoubleAccessor(&SatConf::m_rtnUserLinkFreqHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("FwdUserLinkChannels",
                          "The number of channels in forward user link",
                          UintegerValue(4),
                          MakeUintegerAccessor(&SatConf::m_fwdUserLinkChannelCount),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("RtnUserLinkChannels",
                          "The number of channels in return user link",
                          UintegerValue(4),
                          MakeUintegerAccessor(&SatConf::m_rtnUserLinkChannelCount),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("FwdFeederLinkChannels",
                          "The number of channels in forward feeder link",
                          UintegerValue(16),
                          MakeUintegerAccessor(&SatConf::m_fwdFeederLinkChannelCount),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("RtnFeederLinkChannels",
                          "The number of channels in return feeder link",
                          UintegerValue(16),
                          MakeUintegerAccessor(&SatConf::m_rtnFeederLinkChannelCount),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("SuperFrameConfForSeq0",
                          "Super frame configuration used for super frame sequence 0.",
                          EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_0),
                          MakeEnumAccessor<SatSuperframeConf::SuperFrameConfiguration_t>(&SatConf::m_SuperFrameConfForSeq0),
                          MakeEnumChecker(SatSuperframeConf::SUPER_FRAME_CONFIG_0,
                                          "Configuration_0",
                                          SatSuperframeConf::SUPER_FRAME_CONFIG_1,
                                          "Configuration_1",
                                          SatSuperframeConf::SUPER_FRAME_CONFIG_2,
                                          "Configuration_2",
                                          SatSuperframeConf::SUPER_FRAME_CONFIG_3,
                                          "Configuration_3",
                                          SatSuperframeConf::SUPER_FRAME_CONFIG_4,
                                          "Configuration_4"))
            .AddAttribute("FwdCarrierAllocatedBandwidth",
                          "The allocated carrier bandwidth for forward link carriers [Hz].",
                          DoubleValue(0.125e9),
                          MakeDoubleAccessor(&SatConf::m_fwdCarrierAllocatedBandwidthHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("FwdCarrierRollOff",
                          "The roll-off factor for forward link carriers.",
                          DoubleValue(0.20),
                          MakeDoubleAccessor(&SatConf::m_fwdCarrierRollOffFactor),
                          MakeDoubleChecker<double>(0.00, 1.00))
            .AddAttribute("FwdCarrierSpacing",
                          "The carrier spacing factor for forward link carriers.",
                          DoubleValue(0.00),
                          MakeDoubleAccessor(&SatConf::m_fwdCarrierSpacingFactor),
                          MakeDoubleChecker<double>(0.00, 1.00))
            .AddAttribute("RtnScpcCarrierAllocatedBandwidth",
                          "The allocated carrier bandwidth for SCPC link carriers [Hz].",
                          DoubleValue(0.125e9),
                          MakeDoubleAccessor(&SatConf::m_rtnCarrierAllocatedBandwidthHz),
                          MakeDoubleChecker<double>())
            .AddAttribute("RtnScpcCarrierRollOff",
                          "The roll-off factor for SCPC link carriers.",
                          DoubleValue(0.20),
                          MakeDoubleAccessor(&SatConf::m_rtnCarrierRollOffFactor),
                          MakeDoubleChecker<double>(0.00, 1.00))
            .AddAttribute("RtnScpcCarrierSpacing",
                          "The carrier spacing factor for SCPC link carriers.",
                          DoubleValue(0.00),
                          MakeDoubleAccessor(&SatConf::m_rtnCarrierSpacingFactor),
                          MakeDoubleChecker<double>(0.00, 1.00))
            .AddAttribute("ForwardLinkRegenerationMode",
                          "The regeneration mode used in satellites for forward link.",
                          EnumValue(SatEnums::TRANSPARENT),
                          MakeEnumAccessor<SatEnums::RegenerationMode_t>(&SatConf::m_forwardLinkRegenerationMode),
                          MakeEnumChecker(SatEnums::TRANSPARENT,
                                          "TRANSPARENT",
                                          SatEnums::REGENERATION_PHY,
                                          "REGENERATION_PHY",
                                          SatEnums::REGENERATION_NETWORK,
                                          "REGENERATION_NETWORK"))
            .AddAttribute("ReturnLinkRegenerationMode",
                          "The regeneration mode used in satellites for return link.",
                          EnumValue(SatEnums::TRANSPARENT),
                          MakeEnumAccessor<SatEnums::RegenerationMode_t>(&SatConf::m_returnLinkRegenerationMode),
                          MakeEnumChecker(SatEnums::TRANSPARENT,
                                          "TRANSPARENT",
                                          SatEnums::REGENERATION_PHY,
                                          "REGENERATION_PHY",
                                          SatEnums::REGENERATION_LINK,
                                          "REGENERATION_LINK",
                                          SatEnums::REGENERATION_NETWORK,
                                          "REGENERATION_NETWORK"))

        ;
    return tid;
}

TypeId
SatConf::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatConf::SatConf()
    : m_beamCount(0),
      m_fwdFeederLinkFreqHz(0.0),
      m_fwdFeederLinkBandwidthHz(0.0),
      m_fwdUserLinkFreqHz(0.0),
      m_fwdUserLinkBandwidthHz(0.0),
      m_rtnFeederLinkFreqHz(0.0),
      m_rtnFeederLinkBandwidthHz(0.0),
      m_rtnUserLinkFreqHz(0.0),
      m_rtnUserLinkBandwidthHz(0.0),
      m_fwdUserLinkChannelCount(0),
      m_rtnUserLinkChannelCount(0),
      m_fwdFeederLinkChannelCount(0),
      m_rtnFeederLinkChannelCount(0),
      m_SuperFrameConfForSeq0(SatSuperframeConf::SUPER_FRAME_CONFIG_0),
      m_fwdCarrierAllocatedBandwidthHz(0.0),
      m_fwdCarrierRollOffFactor(0.0),
      m_fwdCarrierSpacingFactor(0.0),
      m_rtnCarrierAllocatedBandwidthHz(0.0),
      m_rtnCarrierRollOffFactor(0.0),
      m_rtnCarrierSpacingFactor(0.0),
      m_forwardLinkRegenerationMode(SatEnums::TRANSPARENT),
      m_returnLinkRegenerationMode(SatEnums::TRANSPARENT)
{
    NS_LOG_FUNCTION(this);

    // Nothing done here
}

void
SatConf::Initialize(std::string rtnConf,
                    std::string fwdConf,
                    std::string gwPos,
                    std::string satPos,
                    std::string utPos,
                    std::string wfConf,
                    bool isConstellation)
{
    NS_LOG_FUNCTION(this << rtnConf << fwdConf << gwPos << satPos << utPos << wfConf
                         << isConstellation);

    m_isConstellation = isConstellation;

    // Load satellite configuration file
    m_rtnConf = LoadSatConf(rtnConf);
    m_fwdConf = LoadSatConf(fwdConf);

    NS_ASSERT(m_rtnConf.size() == m_fwdConf.size());
    m_beamCount = m_rtnConf.size();

    NS_ASSERT(m_beamCount < SatConstVariables::MAX_BEAMS_PER_SATELLITE);

    // Load GW positions
    LoadPositions(gwPos, m_gwPositions);

    // Load satellite position
    if (!isConstellation)
    {
        LoadPositions(satPos, m_geoSatPosition);
    }

    // Load UT positions
    LoadPositions(utPos, m_utPositions);

    // Update fwdConf & rtnConf with correct nb of GWs
    if (m_isConstellation)
    {
        uint32_t nbGws = m_gwPositions.size();
        uint32_t gwId;
        for (uint32_t i = 0; i < m_fwdConf.size(); i++)
        {
            gwId = i % nbGws;
            m_fwdConf[i][GW_ID_INDEX] = gwId + 1;
            m_rtnConf[i][GW_ID_INDEX] = gwId + 1;
        }
    }

    Configure(wfConf);
}

void
SatConf::Configure(std::string wfConf)
{
    NS_LOG_FUNCTION(this);

    // *** configure forward link ***

    // currently only one carrier in forward link is used.
    double fwdFeederLinkChannelBandwidthHz =
        m_fwdFeederLinkBandwidthHz / m_fwdFeederLinkChannelCount;
    double fwdUserLinkChannelBandwidthHz = m_fwdUserLinkBandwidthHz / m_fwdUserLinkChannelCount;

    // channel bandwidths for the forward feeder and user links is expected to be equal if forward
    // generation is physical or transparent
    if ((fwdFeederLinkChannelBandwidthHz != fwdUserLinkChannelBandwidthHz) &&
        (m_forwardLinkRegenerationMode == SatEnums::TRANSPARENT ||
         m_forwardLinkRegenerationMode == SatEnums::REGENERATION_PHY))
    {
        NS_FATAL_ERROR("Channel bandwidths for forward feeder and user links are not equal!!!");
    }

    if (m_fwdCarrierAllocatedBandwidthHz > fwdFeederLinkChannelBandwidthHz)
    {
        NS_FATAL_ERROR("Fwd Link carrier bandwidth exceeds channel bandwidth!!!");
    }

    // create forward link carrier configuration and one carrier pushing just one carrier to
    // container only one carrier supported in forward link currently
    Ptr<SatFwdCarrierConf> fwdCarrierConf =
        Create<SatFwdCarrierConf>(m_fwdCarrierAllocatedBandwidthHz,
                                  m_fwdCarrierRollOffFactor,
                                  m_fwdCarrierSpacingFactor);
    m_forwardLinkCarrierConf.push_back(fwdCarrierConf);

    // create return link carrier configuration and one carrier pushing just one carrier to
    // container only one carrier supported in return link currently only used for SCPC
    Ptr<SatFwdCarrierConf> rtnCarrierConf =
        Create<SatFwdCarrierConf>(m_rtnCarrierAllocatedBandwidthHz,
                                  m_rtnCarrierRollOffFactor,
                                  m_rtnCarrierSpacingFactor);
    m_returnLinkCarrierConf.push_back(rtnCarrierConf);

    // *** configure return link ***

    double rtnFeederLinkBandwidthHz = m_rtnFeederLinkBandwidthHz / m_rtnFeederLinkChannelCount;
    double rtnUserLinkBandwidthHz = m_rtnUserLinkBandwidthHz / m_rtnUserLinkChannelCount;

    // bandwidths of the return feeder and user links is expected to be equal if return generation
    // is physical or transparent
    if ((rtnFeederLinkBandwidthHz != rtnUserLinkBandwidthHz) &&
        (m_returnLinkRegenerationMode == SatEnums::TRANSPARENT ||
         m_returnLinkRegenerationMode == SatEnums::REGENERATION_PHY))
    {
        NS_FATAL_ERROR("Bandwidths of return feeder and user links are not equal!!!");
    }

    // Create super frame sequence
    m_superframeSeq = CreateObject<SatSuperframeSeq>();

    // Create a waveform configuration and add it to super frame sequence.
    Ptr<SatWaveformConf> waveFormConf = CreateObject<SatWaveformConf>(wfConf);
    m_superframeSeq->AddWaveformConf(waveFormConf);

    Ptr<SatSuperframeConf> superFrameConf =
        SatSuperframeConf::CreateSuperframeConf(m_SuperFrameConfForSeq0);

    superFrameConf->Configure(rtnUserLinkBandwidthHz,
                              m_superframeSeq->GetTargetDuration(),
                              waveFormConf);
    m_superframeSeq->AddSuperframe(superFrameConf);
}

double
SatConf::GetCarrierFrequencyHz(SatEnums::ChannelType_t chType, uint32_t freqId, uint32_t carrierId)
{
    NS_LOG_FUNCTION(this << chType << freqId << carrierId);

    double centerFrequencyHz = 0.0;
    double baseFreqHz = 0.0;
    double channelBandwidthHz = 0.0;
    double carrierBandwidthHz = 0.0;

    switch (chType)
    {
    case SatEnums::FORWARD_FEEDER_CH:
        if (carrierId >= m_forwardLinkCarrierConf.size())
        {
            NS_FATAL_ERROR("Fwd Carrier id out of the range!!");
        }

        channelBandwidthHz = m_fwdFeederLinkBandwidthHz / m_fwdFeederLinkChannelCount;
        carrierBandwidthHz = m_forwardLinkCarrierConf[carrierId]->GetAllocatedBandwidthInHz();
        baseFreqHz = m_fwdFeederLinkFreqHz + (channelBandwidthHz * (freqId - 1));
        centerFrequencyHz =
            baseFreqHz + (carrierBandwidthHz * carrierId) + (carrierBandwidthHz / 2);
        break;

    case SatEnums::FORWARD_USER_CH:
        if (carrierId >= m_forwardLinkCarrierConf.size())
        {
            NS_FATAL_ERROR("Fwd Carrier id out of the range!!");
        }

        channelBandwidthHz = m_fwdUserLinkBandwidthHz / m_fwdUserLinkChannelCount;
        carrierBandwidthHz = m_forwardLinkCarrierConf[carrierId]->GetAllocatedBandwidthInHz();
        baseFreqHz = m_fwdUserLinkFreqHz + (channelBandwidthHz * (freqId - 1));
        centerFrequencyHz =
            baseFreqHz + (carrierBandwidthHz * carrierId) + (carrierBandwidthHz / 2);
        break;

    case SatEnums::RETURN_FEEDER_CH:
        channelBandwidthHz = m_rtnFeederLinkBandwidthHz / m_rtnFeederLinkChannelCount;
        baseFreqHz = m_rtnFeederLinkFreqHz + (channelBandwidthHz * (freqId - 1));
        centerFrequencyHz = baseFreqHz + m_superframeSeq->GetCarrierFrequencyHz(carrierId);
        break;

    case SatEnums::RETURN_USER_CH:
        channelBandwidthHz = m_rtnUserLinkBandwidthHz / m_rtnUserLinkChannelCount;
        baseFreqHz = m_rtnUserLinkFreqHz + (channelBandwidthHz * (freqId - 1));
        centerFrequencyHz = baseFreqHz + m_superframeSeq->GetCarrierFrequencyHz(carrierId);
        break;

    default:
        NS_ASSERT(false);
        break;
    }

    return centerFrequencyHz;
}

double
SatConf::GetCarrierBandwidthHz(SatEnums::ChannelType_t chType,
                               uint32_t carrierId,
                               SatEnums::CarrierBandwidthType_t bandwidthType)
{
    NS_LOG_FUNCTION(this << chType << carrierId << bandwidthType);

    double carrierBandwidthHz = 0.0;

    switch (chType)
    {
    case SatEnums::FORWARD_FEEDER_CH:
        carrierBandwidthHz = GetFwdLinkCarrierBandwidthHz(carrierId, bandwidthType);
        break;

    case SatEnums::FORWARD_USER_CH:
        carrierBandwidthHz = GetFwdLinkCarrierBandwidthHz(carrierId, bandwidthType);
        break;

    case SatEnums::RETURN_FEEDER_CH:
        carrierBandwidthHz = m_superframeSeq->GetCarrierBandwidthHz(carrierId, bandwidthType);
        if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
            m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
        {
            carrierBandwidthHz = GetRtnLinkCarrierBandwidthHz(0, bandwidthType);
        }
        break;

    case SatEnums::RETURN_USER_CH:
        carrierBandwidthHz = m_superframeSeq->GetCarrierBandwidthHz(carrierId, bandwidthType);
        break;

    default:
        NS_ASSERT(false);
        break;
    }

    return carrierBandwidthHz;
}

std::ifstream*
SatConf::OpenFile(std::string filePathName) const
{
    NS_LOG_FUNCTION(this << filePathName);

    // READ FROM THE SPECIFIED INPUT FILE
    std::ifstream* ifs = new std::ifstream(filePathName.c_str(), std::ifstream::in);

    if (!ifs->is_open())
    {
        // script might be launched by test.py, try a different base path
        delete ifs;
        filePathName = "../../" + filePathName;
        ifs = new std::ifstream(filePathName.c_str(), std::ifstream::in);

        if (!ifs->is_open())
        {
            NS_FATAL_ERROR("The file " << filePathName << " is not found.");
        }
    }
    return ifs;
}

std::vector<std::vector<uint32_t>>
SatConf::LoadSatConf(std::string filePathName) const
{
    NS_LOG_FUNCTION(this << filePathName);

    std::vector<std::vector<uint32_t>> conf;

    // READ FROM THE SPECIFIED INPUT FILE
    std::ifstream* ifs = OpenFile(filePathName);

    uint32_t beamId, userChannelId, gwId, feederChannelId;
    *ifs >> beamId >> userChannelId >> gwId >> feederChannelId;

    while (ifs->good())
    {
        NS_LOG_DEBUG(this << " beamId = " << beamId << ", userChannelId = " << userChannelId
                          << ", gwId = " << gwId << ", feederChannelId = " << feederChannelId);

        // Store the values
        std::vector<uint32_t> beamConf;

        beamConf.push_back(beamId);
        beamConf.push_back(userChannelId);
        beamConf.push_back(gwId);
        beamConf.push_back(feederChannelId);

        conf.push_back(beamConf);

        // get next row
        *ifs >> beamId >> userChannelId >> gwId >> feederChannelId;
    }

    ifs->close();
    delete ifs;

    return conf;
}

void
SatConf::SetUtPositionsPath(std::string inputFileUtListPositions)
{
    NS_LOG_FUNCTION(this << inputFileUtListPositions);

    // Load UT positions
    LoadPositions(inputFileUtListPositions, m_utPositions);
}

void
SatConf::LoadPositions(std::string filePathName, PositionContainer_t& container)
{
    NS_LOG_FUNCTION(this << filePathName);

    // READ FROM THE SPECIFIED INPUT FILE
    std::ifstream* ifs = OpenFile(filePathName);

    double lat, lon, alt;
    *ifs >> lat >> lon >> alt;

    while (ifs->good())
    {
        NS_LOG_DEBUG(this << " latitude [deg] = " << lat << ", longitude [deg] = " << lon
                          << ", altitude [m] = ");

        // Store the values
        GeoCoordinate coord(lat, lon, alt);
        container.push_back(coord);

        // get next row
        *ifs >> lat >> lon >> alt;
    }

    ifs->close();
    delete ifs;
}

std::vector<std::string>
SatConf::LoadTles(std::string filePathName, std::string startDatePathName)
{
    NS_LOG_FUNCTION(this << filePathName);

    // READ START TIME
    std::ifstream* ifs = OpenFile(startDatePathName);

    std::getline(*ifs, m_startTimeStr);

    ifs->close();

    // READ TLE
    std::vector<std::string> tles;

    ifs = OpenFile(filePathName);

    double size;
    uint32_t i = 0;
    std::string firstLine;
    std::getline(*ifs, firstLine);
    std::istringstream iss(firstLine);
    iss >> size;

    tles.reserve(size);

    while (ifs->good() && i < size)
    {
        std::string tle;
        std::string name;
        std::string line1;
        std::string line2;

        std::getline(*ifs, name);
        std::getline(*ifs, line1);
        std::getline(*ifs, line2);

        tle = line1 + '\n' + line2;
        tles.push_back(tle);

        i += 1;
    }

    NS_ASSERT(tles.size() < SatConstVariables::MAX_SATELLITES);

    ifs->close();
    delete ifs;

    m_tles = tles;

    return tles;
}

std::vector<std::pair<uint32_t, uint32_t>>
SatConf::LoadIsls(std::string filePathName)
{
    NS_LOG_FUNCTION(this << filePathName);

    std::vector<std::pair<uint32_t, uint32_t>> isls;

    // READ FROM THE SPECIFIED INPUT FILE
    std::ifstream* ifs = OpenFile(filePathName);

    double size;
    uint32_t i = 0;
    std::string firstLine;
    std::getline(*ifs, firstLine);
    std::istringstream iss(firstLine);
    iss >> size;

    isls.reserve(size);

    while (ifs->good() && i < size)
    {
        std::string line;
        std::string sat1;
        std::string sat2;

        std::getline(*ifs, line);

        std::stringstream ss(line);
        ss >> sat1;
        ss >> sat2;

        isls.push_back(std::make_pair(std::stoi(sat1), std::stoi(sat2)));

        i += 1;
    }

    ifs->close();
    delete ifs;

    return isls;
}

uint32_t
SatConf::GetBeamCount() const
{
    NS_LOG_FUNCTION(this);

    return m_beamCount;
}

uint32_t
SatConf::GetGwCount() const
{
    NS_LOG_FUNCTION(this);

    return m_gwPositions.size();
}

uint32_t
SatConf::GetUtCount() const
{
    NS_LOG_FUNCTION(this);

    return m_utPositions.size();
}

uint32_t
SatConf::GetSatCount() const
{
    NS_LOG_FUNCTION(this);

    if (m_isConstellation)
    {
        return m_tles.size();
    }
    else
    {
        return m_geoSatPosition.size();
    }
}

std::vector<uint32_t>
SatConf::GetBeamConfiguration(uint32_t beamId, SatEnums::SatLinkDir_t dir) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT((beamId > 0) && (beamId <= m_beamCount));

    return (dir == SatEnums::LD_RETURN) ? m_rtnConf[beamId - 1] : m_fwdConf[beamId - 1];
}

uint32_t
SatConf::GetRtnLinkCarrierCount() const
{
    NS_LOG_FUNCTION(this);

    return m_superframeSeq->GetCarrierCount();
}

uint32_t
SatConf::GetFwdLinkCarrierCount() const
{
    NS_LOG_FUNCTION(this);

    return m_forwardLinkCarrierConf.size();
}

SatEnums::RegenerationMode_t
SatConf::GetForwardLinkRegenerationMode() const
{
    NS_LOG_FUNCTION(this);

    return m_forwardLinkRegenerationMode;
}

SatEnums::RegenerationMode_t
SatConf::GetReturnLinkRegenerationMode() const
{
    NS_LOG_FUNCTION(this);

    return m_returnLinkRegenerationMode;
}

double
SatConf::GetFwdLinkCarrierBandwidthHz(uint32_t carrierId,
                                      SatEnums::CarrierBandwidthType_t bandwidthType) const
{
    NS_LOG_FUNCTION(this);

    double bandwidtHz = 0.0;

    if (carrierId >= m_forwardLinkCarrierConf.size())
    {
        NS_FATAL_ERROR("Fwd Carrier id out of the range!!");
    }

    switch (bandwidthType)
    {
    case SatEnums::ALLOCATED_BANDWIDTH:
        bandwidtHz = m_forwardLinkCarrierConf[carrierId]->GetAllocatedBandwidthInHz();
        break;

    case SatEnums::OCCUPIED_BANDWIDTH:
        bandwidtHz = m_forwardLinkCarrierConf[carrierId]->GetOccupiedBandwidthInHz();
        break;

    case SatEnums::EFFECTIVE_BANDWIDTH:
        bandwidtHz = m_forwardLinkCarrierConf[carrierId]->GetEffectiveBandwidthInHz();
        break;

    default:
        NS_FATAL_ERROR("Invalid bandwidth type");
        break;
    }

    return bandwidtHz;
}

double
SatConf::GetRtnLinkCarrierBandwidthHz(uint32_t carrierId,
                                      SatEnums::CarrierBandwidthType_t bandwidthType) const
{
    NS_LOG_FUNCTION(this);

    double bandwidtHz = 0.0;

    if (carrierId >= m_returnLinkCarrierConf.size())
    {
        NS_FATAL_ERROR("Rtn Carrier id out of the range!!");
    }

    switch (bandwidthType)
    {
    case SatEnums::ALLOCATED_BANDWIDTH:
        bandwidtHz = m_returnLinkCarrierConf[carrierId]->GetAllocatedBandwidthInHz();
        break;

    case SatEnums::OCCUPIED_BANDWIDTH:
        bandwidtHz = m_returnLinkCarrierConf[carrierId]->GetOccupiedBandwidthInHz();
        break;

    case SatEnums::EFFECTIVE_BANDWIDTH:
        bandwidtHz = m_returnLinkCarrierConf[carrierId]->GetEffectiveBandwidthInHz();
        break;

    default:
        NS_FATAL_ERROR("Invalid bandwidth type");
        break;
    }

    return bandwidtHz;
}

GeoCoordinate
SatConf::GetGwPosition(uint32_t gwId) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT((gwId > 0) && (gwId <= m_gwPositions.size()));

    return m_gwPositions[gwId - 1];
}

GeoCoordinate
SatConf::GetUtPosition(uint32_t utId) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT((utId > 0) && (utId <= m_utPositions.size()));

    return m_utPositions[utId - 1];
}

GeoCoordinate
SatConf::GetGeoSatPosition() const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_geoSatPosition.size() == 1);

    return m_geoSatPosition[0];
}

std::string
SatConf::GetStartTimeStr() const
{
    NS_LOG_FUNCTION(this);

    return m_startTimeStr;
}

} // namespace ns3
