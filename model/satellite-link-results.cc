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
#include <ns3/log.h>
#include <ns3/string.h>

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


double
SatLinkResults::GetBler (SatLinkResults::SatModcod_e modcod,
                         uint16_t burstLength, double sinrDb) const
{
  NS_LOG_FUNCTION (this << modcod << burstLength << sinrDb);

  if (!m_isInitialized)
    {
      NS_FATAL_ERROR ("Error retrieving link results, call Initialize first");
    }

  uint8_t i = DoGetTableIndex (modcod, burstLength);
  return m_table[i]->GetBler (sinrDb);
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
  m_table.resize (21);

  m_table[0] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat2.txt");
  m_table[1] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat3.txt");
  m_table[2] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat4.txt");
  m_table[3] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat5.txt");
  m_table[4] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat6.txt");
  m_table[5] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat7.txt");
  m_table[6] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat8.txt");
  m_table[7] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat9.txt");
  m_table[8] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat10.txt");
  m_table[9] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat11.txt");
  m_table[10] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat12.txt");
  m_table[11] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat13.txt");
  m_table[12] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat14.txt");
  m_table[13] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat15.txt");
  m_table[14] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat16.txt");
  m_table[15] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat17.txt");
  m_table[16] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat18.txt");
  m_table[17] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat19.txt");
  m_table[18] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat20.txt");
  m_table[19] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat21.txt");
  m_table[20] = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_wafeformat22.txt");

} // end of void SatLinkResultsDvbRcs2::DoInitialize


uint8_t
SatLinkResultsDvbRcs2::DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                                        int16_t burstLength) const
{
  uint8_t waveformId = 0;

  switch (burstLength)
  {
    case 262:
      if (modcod == SAT_MODCOD_QPSK_1_TO_3)
        {
          waveformId = 2;
        }
      break;

    case 536:
      switch (modcod)
      {
        case SAT_MODCOD_QPSK_1_TO_3:
          waveformId = 3;
          break;
        case SAT_MODCOD_QPSK_1_TO_2:
          waveformId = 4;
          break;
        case SAT_MODCOD_QPSK_2_TO_3:
          waveformId = 5;
          break;
        case SAT_MODCOD_QPSK_3_TO_4:
          waveformId = 6;
          break;
        case SAT_MODCOD_QPSK_5_TO_6:
          waveformId = 7;
          break;
        case SAT_MODCOD_8PSK_2_TO_3:
          waveformId = 8;
          break;
        case SAT_MODCOD_8PSK_3_TO_4:
          waveformId = 9;
          break;
        case SAT_MODCOD_8PSK_5_TO_6:
          waveformId = 10;
          break;
        case SAT_MODCOD_16QAM_3_TO_4:
          waveformId = 11;
          break;
        case SAT_MODCOD_16QAM_5_TO_6:
          waveformId = 12;
          break;
        default:
          NS_FATAL_ERROR ("Invalid Modcod for DVB-RCS2: " << modcod);
          break;
      } // end of switch (modcod)

      break; // end of case 536

    case 1616:
      switch (modcod)
      {
        case SAT_MODCOD_QPSK_1_TO_3:
          waveformId = 13;
          break;
        case SAT_MODCOD_QPSK_1_TO_2:
          waveformId = 14;
          break;
        case SAT_MODCOD_QPSK_2_TO_3:
          waveformId = 15;
          break;
        case SAT_MODCOD_QPSK_3_TO_4:
          waveformId = 16;
          break;
        case SAT_MODCOD_QPSK_5_TO_6:
          waveformId = 17;
          break;
        case SAT_MODCOD_8PSK_2_TO_3:
          waveformId = 18;
          break;
        case SAT_MODCOD_8PSK_3_TO_4:
          waveformId = 19;
          break;
        case SAT_MODCOD_8PSK_5_TO_6:
          waveformId = 20;
          break;
        case SAT_MODCOD_16QAM_3_TO_4:
          waveformId = 21;
          break;
        case SAT_MODCOD_16QAM_5_TO_6:
          waveformId = 22;
          break;
        default:
          NS_FATAL_ERROR ("Invalid Modcod for DVB-RCS2: " << modcod);
          break;
      } // end of switch (modcod)

      break; // end of case 1616

    default:
      NS_FATAL_ERROR ("Invalid burst length for DVB-RCS2: " << burstLength);
      break;

  } // end of switch (burstLength)

  if (waveformId == 0)
    {
      NS_FATAL_ERROR ("Invalid Modcod/burst length for DVB-RCS2: "
                      << modcod << "/" << burstLength);
    }

  return (waveformId - 2);

} // end of uint8_t SatLinkResultsDvbRcs2::DoGetTableIndex



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
  m_table.resize (24);

  // QPSK

  m_table[0] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_1_to_2.txt");
  m_table[1] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_2_to_3.txt");
  m_table[2] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_3_to_4.txt");
  m_table[3] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_3_to_5.txt");
  m_table[4] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_4_to_5.txt");
  m_table[5] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_5_to_6.txt");
  m_table[6] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_8_to_9.txt");
  m_table[7] = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_9_to_10.txt");

  // 8PSK

  m_table[8] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_2_to_3.txt");
  m_table[9] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_3_to_4.txt");
  m_table[10] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_3_to_5.txt");
  m_table[11] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_5_to_6.txt");
  m_table[12] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_8_to_9.txt");
  m_table[13] = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_9_to_10.txt");

  // 16APSK

  m_table[14] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_2_to_3.txt");
  m_table[15] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_3_to_4.txt");
  m_table[16] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_4_to_5.txt");
  m_table[17] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_5_to_6.txt");
  m_table[18] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_8_to_9.txt");
  m_table[19] = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_9_to_10.txt");

  // 32APSK

  m_table[20] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_3_to_4.txt");
  m_table[21] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_4_to_5.txt");
  m_table[22] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_5_to_6.txt");
  m_table[23] = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_8_to_9.txt");

} // end of void SatLinkResultsDvbS2::DoInitialize


uint8_t
SatLinkResultsDvbS2::DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                                      uint16_t burstLength) const
{
  uint8_t ret = 255;
  switch (modcod)
  {
    // QPSK

    case SAT_MODCOD_QPSK_1_TO_2:
      ret = 0;
      break;
    case SAT_MODCOD_QPSK_2_TO_3:
      ret = 1;
      break;
    case SAT_MODCOD_QPSK_3_TO_4:
      ret = 2;
      break;
    case SAT_MODCOD_QPSK_3_TO_5:
      ret = 3;
      break;
    case SAT_MODCOD_QPSK_4_TO_5:
      ret = 4;
      break;
    case SAT_MODCOD_QPSK_5_TO_6:
      ret = 5;
      break;
    case SAT_MODCOD_QPSK_8_TO_9:
      ret = 6;
      break;
    case SAT_MODCOD_QPSK_9_TO_10:
      ret = 7;
      break;

    // 8PSK

    case SAT_MODCOD_8PSK_2_TO_3:
      ret = 8;
      break;
    case SAT_MODCOD_8PSK_3_TO_4:
      ret = 9;
      break;
    case SAT_MODCOD_8PSK_3_TO_5:
      ret = 10;
      break;
    case SAT_MODCOD_8PSK_5_TO_6:
      ret = 11;
      break;
    case SAT_MODCOD_8PSK_8_TO_9:
      ret = 12;
      break;
    case SAT_MODCOD_8PSK_9_TO_10:
      ret = 13;
      break;

    // 16APSK

    case SAT_MODCOD_16APSK_2_TO_3:
      ret = 14;
      break;
    case SAT_MODCOD_16APSK_3_TO_4:
      ret = 15;
      break;
    case SAT_MODCOD_16APSK_4_TO_5:
      ret = 16;
      break;
    case SAT_MODCOD_16APSK_5_TO_6:
      ret = 17;
      break;
    case SAT_MODCOD_16APSK_8_TO_9:
      ret = 18;
      break;
    case SAT_MODCOD_16APSK_9_TO_10:
      ret = 19;
      break;

    // 32APSK

    case SAT_MODCOD_32APSK_3_TO_4:
      ret = 19;
      break;
    case SAT_MODCOD_32APSK_4_TO_5:
      ret = 19;
      break;
    case SAT_MODCOD_32APSK_5_TO_6:
      ret = 19;
      break;
    case SAT_MODCOD_32APSK_8_TO_9:
      ret = 19;
      break;

    default:
      NS_FATAL_ERROR ("Invalid Modcod for DVB-S2: " << modcod);
      break;

  } // end of switch (modcod)

  return ret;

} // end of uint8_t SatLinkResultsDvbS2::DoGetTableIndex


} // end of namespace ns3
