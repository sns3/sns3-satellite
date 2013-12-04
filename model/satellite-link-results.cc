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

#include <ns3/log.h>
#include <ns3/string.h>

#include "satellite-link-results.h"

NS_LOG_COMPONENT_DEFINE ("SatLinkResults");

namespace ns3 {


/*
 * SATLINKRESULTS PARENT ABSTRACT CLASS
 */

NS_OBJECT_ENSURE_REGISTERED (SatLinkResults);


SatLinkResults::SatLinkResults ()
  : m_isInitialized (false)
{
}


TypeId
SatLinkResults::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLinkResults")
    .SetParent<Object> ()
    .AddAttribute ("InputPath",
                   "The path relative to ns-3 project directory "
                   "where the link results file can be found",
                   StringValue ("src/satellite/data/linkresults/"),
                   MakeStringAccessor (&SatLinkResults::m_inputPath),
                   MakeStringChecker ())
  ;
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


TypeId
SatLinkResultsDvbRcs2::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLinkResultsDvbRcs2")
    .SetParent<SatLinkResults> ()
  ;
  return tid;
}

void
SatLinkResultsDvbRcs2::DoInitialize ()
{
  // Waveform ids 2-22 currently supported
  for (uint32_t i = 2; i <= 22; ++i)
    {
      std::ostringstream ss;
      ss << i;
      std::string filePathName = m_inputPath + "rcs2_waveformat" + ss.str() + ".txt";
      m_table.insert (std::make_pair(i, CreateObject<SatLookUpTable> (filePathName)));
    }

} // end of void SatLinkResultsDvbRcs2::DoInitialize


double
SatLinkResultsDvbRcs2::GetBler (uint32_t waveformId, double esNoDb) const
{
  NS_LOG_FUNCTION (this << waveformId << esNoDb);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  return m_table.at(waveformId)->GetBler (esNoDb);
}


double
SatLinkResultsDvbRcs2::GetEsNoDb (uint32_t waveformId, double blerTarget) const
{
  NS_LOG_FUNCTION (this << waveformId << blerTarget);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  return m_table.at(waveformId)->GetEsNoDb (blerTarget);
}

/*
 * SATLINKRESULTSDVBS2 CHILD CLASS
 */

NS_OBJECT_ENSURE_REGISTERED (SatLinkResultsDvbS2);


TypeId
SatLinkResultsDvbS2::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLinkResultsDvbS2")
    .SetParent<SatLinkResults> ()
  ;
  return tid;
}


void
SatLinkResultsDvbS2::DoInitialize ()
{
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
SatLinkResultsDvbS2::GetBler (SatEnums::SatModcod_t modcod, double esNoDb) const
{
  NS_LOG_FUNCTION (this << modcod << esNoDb);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  return m_table.at(modcod)->GetBler (esNoDb);
}


double
SatLinkResultsDvbS2::GetEsNoDb (SatEnums::SatModcod_t modcod, double blerTarget) const
{
  NS_LOG_FUNCTION (this << modcod << blerTarget);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  return m_table.at(modcod)->GetEsNoDb (blerTarget);
}


} // end of namespace ns3
