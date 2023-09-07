/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "satellite-link-results.h"

#include "../utils/satellite-env-variables.h"
#include "satellite-enums.h"

#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/singleton.h>
#include <ns3/string.h>

#include <sstream>

NS_LOG_COMPONENT_DEFINE("SatLinkResults");

namespace ns3
{

/*
 * SATLINKRESULTS PARENT ABSTRACT CLASS
 */

NS_OBJECT_ENSURE_REGISTERED(SatLinkResults);

SatLinkResults::SatLinkResults()
    : m_isInitialized(false)
{
    std::string dataPath = Singleton<SatEnvVariables>::Get()->GetDataPath();
    m_inputPath = Singleton<SatEnvVariables>::Get()->LocateDirectory(dataPath + "/linkresults/");
}

TypeId
SatLinkResults::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResults").SetParent<Object>();
    return tid;
}

void
SatLinkResults::Initialize()
{
    NS_LOG_FUNCTION(this);
    DoInitialize();
    m_isInitialized = true;
}

/*
 * SATLINKRESULTSRTN CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsRtn);

SatLinkResultsRtn::SatLinkResultsRtn()
    : SatLinkResults(),
      m_table()
{
}

TypeId
SatLinkResultsRtn::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResultsRtn").SetParent<SatLinkResults>();
    return tid;
}

double
SatLinkResultsRtn::GetBler(uint32_t waveformId, double ebNoDb) const
{
    NS_LOG_FUNCTION(this << waveformId << ebNoDb);

    if (!m_isInitialized)
    {
        NS_FATAL_ERROR("Error retrieving link results, call Initialize first");
    }

    return m_table.at(waveformId)->GetBler(ebNoDb);
}

double
SatLinkResultsRtn::GetEbNoDb(uint32_t waveformId, double blerTarget) const
{
    NS_LOG_FUNCTION(this << waveformId << blerTarget);

    if (!m_isInitialized)
    {
        NS_FATAL_ERROR("Error retrieving link results, call Initialize first");
    }

    return m_table.at(waveformId)->GetEsNoDb(blerTarget);
}

/*
 * SATLINKRESULTSDVBRCS2 CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsDvbRcs2);

SatLinkResultsDvbRcs2::SatLinkResultsDvbRcs2()
    : SatLinkResultsRtn()
{
}

TypeId
SatLinkResultsDvbRcs2::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResultsDvbRcs2").SetParent<SatLinkResultsRtn>();
    return tid;
}

void
SatLinkResultsDvbRcs2::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    // Waveform ids 2-22 currently supported
    for (uint32_t i = 2; i <= 22; ++i)
    {
        std::ostringstream ss;
        ss << i;
        std::string filePathName = m_inputPath + "rcs2_waveformat" + ss.str() + ".txt";
        m_table.insert(std::make_pair(i, CreateObject<SatLookUpTable>(filePathName)));
    }
} // end of void SatLinkResultsDvbRcs2::DoInitialize

/*
 * SATLINKRESULTSFSIM CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsFSim);

SatLinkResultsFSim::SatLinkResultsFSim()
    : SatLinkResultsRtn()
{
}

TypeId
SatLinkResultsFSim::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResultsFSim").SetParent<SatLinkResultsRtn>();
    return tid;
}

void
SatLinkResultsFSim::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    // Waveform ids 1-2 currently supported
    for (uint32_t i = 1; i <= 2; ++i)
    {
        std::ostringstream ss;
        ss << i;
        std::string filePathName = m_inputPath + "fsim_waveformat" + ss.str() + ".txt";
        m_table.insert(std::make_pair(i, CreateObject<SatLookUpTable>(filePathName)));
    }

    // Initialize Mutual Information table
    m_mutualInformationTable =
        CreateObject<SatMutualInformationTable>(m_inputPath + "mutualInformationTable.txt");
} // end of void SatLinkResultsFSim::DoInitialize

/*
 * SATLINKRESULTSLORA CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsLora);

SatLinkResultsLora::SatLinkResultsLora()
    : SatLinkResultsRtn()
{
}

TypeId
SatLinkResultsLora::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResultsLora").SetParent<SatLinkResultsRtn>();
    return tid;
}

void
SatLinkResultsLora::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    // Waveform ids 1-2 currently supported
    for (uint32_t i = 1; i <= 2; ++i)
    {
        std::ostringstream ss;
        ss << i;
        std::string filePathName = m_inputPath + "lora_waveformat" + ss.str() + ".txt";
        m_table.insert(std::make_pair(i, CreateObject<SatLookUpTable>(filePathName)));
    }

    // Initialize Mutual Information table
    m_mutualInformationTable =
        CreateObject<SatMutualInformationTable>(m_inputPath + "mutualInformationTable.txt");
} // end of void SatLinkResultsLora::DoInitialize

/*
 * SATLINKRESULTSFWD ABSTRACT CLASS
 */

NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsFwd);

SatLinkResultsFwd::SatLinkResultsFwd()
    : SatLinkResults(),
      m_table(),
      m_shortFrameOffsetInDb(0.0)
{
}

TypeId
SatLinkResultsFwd::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatLinkResultsFwd")
            .SetParent<SatLinkResults>()
            .AddAttribute("EsNoOffsetForShortFrame",
                          "EsNo increase offset for short BB frame with a given BLER",
                          DoubleValue(0.4),
                          MakeDoubleAccessor(&SatLinkResultsFwd::m_shortFrameOffsetInDb),
                          MakeDoubleChecker<double_t>());
    return tid;
}

double
SatLinkResultsFwd::GetBler(SatEnums::SatModcod_t modcod,
                           SatEnums::SatBbFrameType_t frameType,
                           double esNoDb) const
{
    NS_LOG_FUNCTION(this << modcod << esNoDb);

    if (!m_isInitialized)
    {
        NS_FATAL_ERROR("Error retrieving link results, call Initialize first");
    }

    /**
     * Short BB frame is assumed to be requiring m_shortFrameOffsetInDb dB
     * higher Es/No if compared to normal BB frame.
     * TODO: Proper link results need to be added for short BB frame in FWD link.
     */
    if (frameType == SatEnums::SHORT_FRAME)
    {
        esNoDb -= m_shortFrameOffsetInDb;
    }

    return m_table.at(modcod)->GetBler(esNoDb);
}

double
SatLinkResultsFwd::GetEsNoDb(SatEnums::SatModcod_t modcod,
                             SatEnums::SatBbFrameType_t frameType,
                             double blerTarget) const
{
    NS_LOG_FUNCTION(this << modcod << blerTarget);

    if (!m_isInitialized)
    {
        NS_FATAL_ERROR("Error retrieving link results, call Initialize first");
    }

    // Get Es/No requirement for normal BB frame
    double esno = m_table.at(modcod)->GetEsNoDb(blerTarget);

    /**
     * Short BB frame is assumed to be requiring "m_shortFrameOffsetInDb" dB
     * higher Es/No if compared to normal BB frame.
     * TODO: Proper link results need to be added for short BB frame in FWD link.
     */
    if (frameType == SatEnums::SHORT_FRAME)
    {
        esno += m_shortFrameOffsetInDb;
    }

    return esno;
}

/*
 * SATLINKRESULTSDVBS2 CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsDvbS2);

SatLinkResultsDvbS2::SatLinkResultsDvbS2()
    : SatLinkResultsFwd()
{
}

TypeId
SatLinkResultsDvbS2::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResultsDvbS2").SetParent<SatLinkResultsFwd>();
    return tid;
}

void
SatLinkResultsDvbS2::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    // QPSK
    m_table[SatEnums::SAT_MODCOD_QPSK_1_TO_2] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_1_to_2.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_2_TO_3] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_2_to_3.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_3_TO_4] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_3_to_4.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_3_TO_5] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_3_to_5.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_4_TO_5] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_4_to_5.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_5_TO_6] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_5_to_6.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_8_TO_9] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_8_to_9.txt");
    m_table[SatEnums::SAT_MODCOD_QPSK_9_TO_10] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_qpsk_9_to_10.txt");

    // 8PSK
    m_table[SatEnums::SAT_MODCOD_8PSK_2_TO_3] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_8psk_2_to_3.txt");
    m_table[SatEnums::SAT_MODCOD_8PSK_3_TO_4] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_8psk_3_to_4.txt");
    m_table[SatEnums::SAT_MODCOD_8PSK_3_TO_5] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_8psk_3_to_5.txt");
    m_table[SatEnums::SAT_MODCOD_8PSK_5_TO_6] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_8psk_5_to_6.txt");
    m_table[SatEnums::SAT_MODCOD_8PSK_8_TO_9] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_8psk_8_to_9.txt");
    m_table[SatEnums::SAT_MODCOD_8PSK_9_TO_10] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_8psk_9_to_10.txt");

    // 16APSK
    m_table[SatEnums::SAT_MODCOD_16APSK_2_TO_3] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_16apsk_2_to_3.txt");
    m_table[SatEnums::SAT_MODCOD_16APSK_3_TO_4] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_16apsk_3_to_4.txt");
    m_table[SatEnums::SAT_MODCOD_16APSK_4_TO_5] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_16apsk_4_to_5.txt");
    m_table[SatEnums::SAT_MODCOD_16APSK_5_TO_6] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_16apsk_5_to_6.txt");
    m_table[SatEnums::SAT_MODCOD_16APSK_8_TO_9] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_16apsk_8_to_9.txt");
    m_table[SatEnums::SAT_MODCOD_16APSK_9_TO_10] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_16apsk_9_to_10.txt");

    // 32APSK
    m_table[SatEnums::SAT_MODCOD_32APSK_3_TO_4] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_32apsk_3_to_4.txt");
    m_table[SatEnums::SAT_MODCOD_32APSK_4_TO_5] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_32apsk_4_to_5.txt");
    m_table[SatEnums::SAT_MODCOD_32APSK_5_TO_6] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_32apsk_5_to_6.txt");
    m_table[SatEnums::SAT_MODCOD_32APSK_8_TO_9] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2_32apsk_8_to_9.txt");

    m_table[SatEnums::SAT_MODCOD_BPSK_1_TO_3] =
        CreateObject<SatLookUpTable>(m_inputPath + "lora_waveformat2.txt");

} // end of void SatLinkResultsDvbS2::DoInitialize

/*
 * SATLINKRESULTSDVBS2X CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED(SatLinkResultsDvbS2X);

SatLinkResultsDvbS2X::SatLinkResultsDvbS2X()
    : SatLinkResultsFwd()
{
}

TypeId
SatLinkResultsDvbS2X::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatLinkResultsDvbS2X").SetParent<SatLinkResultsFwd>();
    return tid;
}

void
SatLinkResultsDvbS2X::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    // QPSK
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_5_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_2_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_14_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_3_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_4_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_9_to_20_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_4_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_4_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_4_to_5_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_5_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_2_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_3_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_11_to_20_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_3_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_5_to_6_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_32_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_9_to_10_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_8_to_9_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_3_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_5_to_6_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_8_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_13_to_45_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_4_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_4_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_3_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_4_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_14_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_4_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_2_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_4_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_7_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_3_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_11_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_2_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_8_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_13_to_45_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_5_to_6_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_4_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_11_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_4_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_4_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_1_to_3_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_8_to_9_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_7_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_9_to_10_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_11_to_20_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_4_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_32_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_8_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_9_to_20_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_2_to_3_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_8_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_5_to_6_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_qpsk_3_to_4_normal_pilots.txt");

    // 8PSK
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_8_to_9_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_2_to_3_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_4_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_4_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_13_to_18_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_13_to_18_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_7_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_2_to_3_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_8_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_8_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_2_to_3_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_32_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_5_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_9_to_10_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_5_to_6_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_25_to_36_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_4_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_26_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_23_to_36_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_23_to_36_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_25_to_36_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_2_to_3_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_5_to_6_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_7_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_26_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_8_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_8_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_5_to_6_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_5_to_6_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_32_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_9_to_10_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_8_to_9_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_4_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8psk_3_to_5_normal_pilots.txt");

    // 8APSK
    m_table[SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8apsk_26_to_45_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8apsk_26_to_45_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8apsk_5_to_9_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_8apsk_5_to_9_l_normal_nopilots.txt");

    // 16APSK
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_7_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_5_to_6_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_5_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_9_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_25_to_36_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_1_to_2_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_2_to_3_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_13_to_18_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_4_to_5_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_9_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_28_to_45_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_5_to_6_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_2_to_3_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_4_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_23_to_36_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_5_to_9_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_4_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_26_to_45_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_15_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_5_to_6_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_5_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_23_to_36_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_5_to_6_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_28_to_45_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_7_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_7_to_15_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_25_to_36_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_5_to_9_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_32_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_26_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_1_to_2_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_13_to_18_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_2_to_3_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_9_to_10_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_32_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_2_to_3_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_26_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_4_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_4_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_2_to_3_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_15_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_4_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_77_to_90_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_26_to_45_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_77_to_90_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_7_to_15_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_8_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_2_to_3_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_4_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_5_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_3_to_4_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_16apsk_9_to_10_normal_pilots.txt");

    // 32APSK
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_7_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_5_to_6_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_8_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_11_to_15_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_32_to_45_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_2_to_3_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_2_to_3_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_8_to_9_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_2_to_3_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_5_to_6_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_32_to_45_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_3_to_4_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_32_to_45_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_4_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_5_to_6_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_8_to_9_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_11_to_15_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_4_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_3_to_4_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_7_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_5_to_6_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_2_to_3_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_3_to_4_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_9_to_10_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_3_to_4_short_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_32_to_45_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_9_to_10_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_8_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_4_to_5_short_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_32apsk_4_to_5_short_pilots.txt");

    // 64APSK
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_4_to_5_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_32_to_45_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_5_to_6_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_4_to_5_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_7_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_5_to_6_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_11_to_15_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_7_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_11_to_15_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_64apsk_32_to_45_l_normal_nopilots.txt");

    // 128APSK
    m_table[SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_128apsk_7_to_9_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_128apsk_7_to_9_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_128apsk_3_to_4_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_128apsk_3_to_4_normal_nopilots.txt");

    // 256APSK
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_29_to_45_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_31_to_45_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_3_to_4_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_11_to_15_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_29_to_45_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_31_to_45_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_2_to_3_l_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_11_to_15_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_32_to_45_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_32_to_45_normal_pilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_2_to_3_l_normal_nopilots.txt");
    m_table[SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS] =
        CreateObject<SatLookUpTable>(m_inputPath + "s2x_256apsk_3_to_4_normal_nopilots.txt");

} // end of void SatLinkResultsDvbS2X::DoInitialize

} // end of namespace ns3
