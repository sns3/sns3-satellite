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

#include <sstream>
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/object.h"
#include "satellite-enums.h"
#include "satellite-link-results.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"

NS_LOG_COMPONENT_DEFINE ("SatLinkResults");

namespace ns3 {


/*
 * SATLINKRESULTS PARENT ABSTRACT CLASS
 */

NS_OBJECT_ENSURE_REGISTERED (SatLinkResults);


SatLinkResults::SatLinkResults ()
  : m_isInitialized (false)
{
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->GetDataPath ();
  m_inputPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory (dataPath + "/linkresults/");
}


TypeId
SatLinkResults::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLinkResults")
    .SetParent<Object> ();
  return tid;
}

void
SatLinkResults::Initialize ()
{
  NS_LOG_FUNCTION (this);
  DoInitialize ();
  m_isInitialized = true;
}

/*
 * SATLINKRESULTSDVBRCS2 CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED (SatLinkResultsDvbRcs2);

SatLinkResultsDvbRcs2::SatLinkResultsDvbRcs2 ()
  : SatLinkResults (),
    m_table ()
{
}

TypeId
SatLinkResultsDvbRcs2::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLinkResultsDvbRcs2")
    .SetParent<SatLinkResults> ();
  return tid;
}

void
SatLinkResultsDvbRcs2::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  // Waveform ids 2-22 currently supported
  for (uint32_t i = 2; i <= 22; ++i)
    {
      std::ostringstream ss;
      ss << i;
      std::string filePathName = m_inputPath + "rcs2_waveformat" + ss.str () + ".txt";
      m_table.insert (std::make_pair (i, CreateObject<SatLookUpTable> (filePathName)));
    }
} // end of void SatLinkResultsDvbRcs2::DoInitialize

double
SatLinkResultsDvbRcs2::GetBler (uint32_t waveformId, double ebNoDb) const
{
  NS_LOG_FUNCTION (this << waveformId << ebNoDb);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  return m_table.at (waveformId)->GetBler (ebNoDb);
}

double
SatLinkResultsDvbRcs2::GetEbNoDb (uint32_t waveformId, double blerTarget) const
{
  NS_LOG_FUNCTION (this << waveformId << blerTarget);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  return m_table.at (waveformId)->GetEsNoDb (blerTarget);
}

/*
 * SATLINKRESULTSDVBS2 CHILD CLASS
 */
NS_OBJECT_ENSURE_REGISTERED (SatLinkResultsDvbS2);

SatLinkResultsDvbS2::SatLinkResultsDvbS2 ()
  : SatLinkResults (),
    m_table (),
    m_shortFrameOffsetInDb (0.0)
{

}

TypeId
SatLinkResultsDvbS2::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLinkResultsDvbS2")
    .SetParent<SatLinkResults> ()
    .AddAttribute ( "EsNoOffsetForShortFrame",
                    "EsNo increase offset for short BB frame with a given BLER",
                    DoubleValue (0.4),
                    MakeDoubleAccessor (&SatLinkResultsDvbS2::m_shortFrameOffsetInDb),
                    MakeDoubleChecker <double_t> ())
  ;
  return tid;
}

void
SatLinkResultsDvbS2::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  // QPSK
  m_table[SatEnums::SAT_MODCOD_QPSK_1_TO_2] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_1_to_2.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_2_TO_3] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_2_to_3.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_3_TO_4] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_3_to_4.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_3_TO_5] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_3_to_5.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_4_TO_5] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_4_to_5.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_5_TO_6] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_5_to_6.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_8_TO_9] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_8_to_9.txt");
  m_table[SatEnums::SAT_MODCOD_QPSK_9_TO_10] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_9_to_10.txt");

  // 8PSK
  m_table[SatEnums::SAT_MODCOD_8PSK_2_TO_3] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_2_to_3.txt");
  m_table[SatEnums::SAT_MODCOD_8PSK_3_TO_4] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_3_to_4.txt");
  m_table[SatEnums::SAT_MODCOD_8PSK_3_TO_5] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_3_to_5.txt");
  m_table[SatEnums::SAT_MODCOD_8PSK_5_TO_6] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_5_to_6.txt");
  m_table[SatEnums::SAT_MODCOD_8PSK_8_TO_9] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_8_to_9.txt");
  m_table[SatEnums::SAT_MODCOD_8PSK_9_TO_10] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_9_to_10.txt");

  // 16APSK
  m_table[SatEnums::SAT_MODCOD_16APSK_2_TO_3] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_2_to_3.txt");
  m_table[SatEnums::SAT_MODCOD_16APSK_3_TO_4] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_3_to_4.txt");
  m_table[SatEnums::SAT_MODCOD_16APSK_4_TO_5] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_4_to_5.txt");
  m_table[SatEnums::SAT_MODCOD_16APSK_5_TO_6] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_5_to_6.txt");
  m_table[SatEnums::SAT_MODCOD_16APSK_8_TO_9] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_8_to_9.txt");
  m_table[SatEnums::SAT_MODCOD_16APSK_9_TO_10] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_9_to_10.txt");

  // 32APSK
  m_table[SatEnums::SAT_MODCOD_32APSK_3_TO_4] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_3_to_4.txt");
  m_table[SatEnums::SAT_MODCOD_32APSK_4_TO_5] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_4_to_5.txt");
  m_table[SatEnums::SAT_MODCOD_32APSK_5_TO_6] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_5_to_6.txt");
  m_table[SatEnums::SAT_MODCOD_32APSK_8_TO_9] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_8_to_9.txt");

} // end of void SatLinkResultsDvbS2::DoInitialize

double
SatLinkResultsDvbS2::GetBler (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType, double esNoDb) const
{
  NS_LOG_FUNCTION (this << modcod << esNoDb);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
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

  return m_table.at (modcod)->GetBler (esNoDb);
}

double
SatLinkResultsDvbS2::GetEsNoDb (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType, double blerTarget) const
{
  NS_LOG_FUNCTION (this << modcod << blerTarget);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  // Get Es/No requirement for normal BB frame
  double esno = m_table.at (modcod)->GetEsNoDb (blerTarget);

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

} // end of namespace ns3
