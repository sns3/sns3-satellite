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

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <limits>
#include <utility>
#include <map>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-utils.h>
#include <ns3/satellite-link-results.h>
#include "satellite-wave-form-conf.h"


NS_LOG_COMPONENT_DEFINE ("SatWaveformConf");

namespace ns3 {


SatWaveform::SatWaveform ()
  : m_waveformId (0),
    m_modulatedBits (0),
    m_codingRate (0.0),
    m_modCod (SatEnums::SAT_NONVALID_MODCOD),
    m_payloadBytes (0),
    m_lengthInSymbols (0),
    m_ebnoRequirement (0.0)
{
  NS_ASSERT (false);
}


SatWaveform::SatWaveform (uint32_t wfId, uint32_t modulatedBits, double codingRate, SatEnums::SatModcod_t modcod, uint32_t payloadBytes, uint32_t lengthInSymbols)
  : m_waveformId (wfId),
    m_modulatedBits (modulatedBits),
    m_codingRate (codingRate),
    m_modCod (modcod),
    m_payloadBytes (payloadBytes),
    m_lengthInSymbols (lengthInSymbols),
    m_ebnoRequirement (0.0)
{

}

uint32_t
SatWaveform::GetWaveformId () const
{
  NS_LOG_FUNCTION (this);
  return m_waveformId;
}

SatEnums::SatModcod_t
SatWaveform::GetModCod () const
{
  NS_LOG_FUNCTION (this);
  return m_modCod;
}

uint32_t
SatWaveform::GetPayloadInBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_payloadBytes;
}

uint32_t
SatWaveform::GetBurstLengthInSymbols () const
{
  NS_LOG_FUNCTION (this);
  return m_lengthInSymbols;
}

Time
SatWaveform::GetBurstDuration (double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << symbolRateInBaud);
  return Seconds (m_lengthInSymbols / symbolRateInBaud);
}

double
SatWaveform::GetSpectralEfficiency (double carrierBandwidthInHz, double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << carrierBandwidthInHz << symbolRateInBaud);
  return ( SatConstVariables::BITS_PER_BYTE * m_payloadBytes ) / (m_lengthInSymbols / symbolRateInBaud) / carrierBandwidthInHz;
}

double
SatWaveform::GetThroughputInBitsPerSecond (double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << symbolRateInBaud);
  return SatConstVariables::BITS_PER_BYTE * m_payloadBytes / ( m_lengthInSymbols / symbolRateInBaud );
}

double
SatWaveform::GetCNoThreshold (double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << symbolRateInBaud);

  /**
   * Convert the Eb/No requirement into C/No requirement by using the carrier
   * symbol rate and log2(modulatedBits).
   * Eb/No = (Es/log2M)/No = (Es/No)*(1/log2M)  = C/N * (1/log2M) = C/No * (1/fs) * (1/log2M)
   */
  double cnoRequirement = m_ebnoRequirement *
                           symbolRateInBaud *
                           m_codingRate *
                           m_modulatedBits;

  return cnoRequirement;
}

void
SatWaveform::SetEbNoRequirement (double ebnoRequirement)
{
  NS_LOG_FUNCTION (this << ebnoRequirement);
  m_ebnoRequirement = ebnoRequirement;
}

void
SatWaveform::Dump (double carrierBandwidthInHz, double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << carrierBandwidthInHz << symbolRateInBaud);

  std::cout << "ModulatedBits: " << m_modulatedBits << ", CodingRate: " << m_codingRate <<
  ", Payload: " << m_payloadBytes << ", BurstLength: " << m_lengthInSymbols <<
  ", EbNoRequirement: " << SatUtils::LinearToDb (m_ebnoRequirement) <<
  ", BurstDuration: " << GetBurstDuration (symbolRateInBaud) <<
  ", Throughput: " << GetThroughputInBitsPerSecond (symbolRateInBaud) <<
  ", SpectralEfficiency: " << GetSpectralEfficiency (carrierBandwidthInHz, symbolRateInBaud) <<
  ", C/No threshold: " << SatUtils::LinearToDb (GetCNoThreshold (symbolRateInBaud)) << std::endl;
}

NS_OBJECT_ENSURE_REGISTERED (SatWaveformConf);


SatWaveformConf::SatWaveformConf ()
  : m_waveforms (),
    m_targetBLER (0.00001),
    m_acmEnabled (false),
    m_defaultWfId (3),
    m_minWfId (0),
    m_maxWfId (23)
{
  // default constructor should not be used
  NS_ASSERT (false);
}


SatWaveformConf::SatWaveformConf (std::string filePathName)
  : m_waveforms (),
    m_targetBLER (0.00001),
    m_acmEnabled (false),
    m_defaultWfId (3),
    m_minWfId (0),
    m_maxWfId (23)
{
  NS_LOG_FUNCTION (this);
  ReadFromFile (filePathName);

  m_supportedBurstLengthsInSymbols.push_back ((uint32_t) SHORT_BURST_LENGTH);
  m_supportedBurstLengthsInSymbols.push_back ((uint32_t) LONG_BURST_LENGTH);
}

TypeId
SatWaveformConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatWaveformConf")
    .SetParent<Object> ()
    .AddAttribute ( "TargetBLER",
                    "Block error rate target",
                    DoubleValue (0.00001),
                    MakeDoubleAccessor (&SatWaveformConf::m_targetBLER),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "AcmEnabled",
                    "Enable ACM",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatWaveformConf::m_acmEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ( "DefaultWfId",
                    "Default waveform id",
                    UintegerValue (3),
                    MakeUintegerAccessor (&SatWaveformConf::m_defaultWfId),
                    MakeUintegerChecker<uint32_t> (3, 22))
    .AddConstructor<SatWaveformConf> ()
  ;
  return tid;
}

SatWaveformConf::~SatWaveformConf ()
{
  NS_LOG_FUNCTION (this);
}

void
SatWaveformConf::ReadFromFile (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  std::vector<uint32_t> wfIds;

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

  if (!ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete ifs;
      filePathName = "../../" + filePathName;
      ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

      if (!ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << filePathName << " is not found.");
        }
    }

  // Row vector containing the waveform information for a certain waveform index
  std::vector<double> rowVector;

  // Start conditions
  int32_t wfIndex, modulatedBits, payloadBytes, durationInSymbols;
  std::string sCodingRate;

  // Read a row
  *ifs >> wfIndex >> modulatedBits >> sCodingRate >> payloadBytes >> durationInSymbols;

  while (ifs->good ())
    {
      // Store temporarily all wfIds
      wfIds.push_back (wfIndex);

      // Convert the coding rate fraction into double
      std::istringstream ss (sCodingRate);
      std::string token;
      std::vector<uint32_t> output;

      while (std::getline (ss, token, '/'))
        {
          uint32_t i;
          std::stringstream s;
          s.str (token);
          s >> i;
          output.push_back (i);
        }

      if (output.size () != 2)
        {
          NS_FATAL_ERROR ("SatWaveformConf::ReadFromFile - Temp fraction vector has unexpected amount of elements!");
        }

      double dCodingRate = double(output[0]) / output[1];

      // Convert modulated bits and coding rate to MODCOD enum
      SatEnums::SatModcod_t modcod = ConvertToModCod (modulatedBits, output[0], output[1]);

      // Create new waveform and insert it to the waveform map
      Ptr<SatWaveform> wf = Create<SatWaveform> (wfIndex, modulatedBits, dCodingRate, modcod, payloadBytes, durationInSymbols);
      m_waveforms.insert (std::make_pair (wfIndex, wf));

      // get next row
      *ifs >> wfIndex >> modulatedBits >> sCodingRate >> payloadBytes >> durationInSymbols;
    }

  ifs->close ();
  delete ifs;

  // Note, currently we assume that the waveform ids are consecutive!
  m_minWfId = *std::min_element (wfIds.begin (), wfIds.end ());
  m_maxWfId = *std::max_element (wfIds.begin (), wfIds.end ());
}


void SatWaveformConf::InitializeEbNoRequirements ( Ptr<SatLinkResultsDvbRcs2> linkResults )
{
  NS_LOG_FUNCTION (this);

  for ( std::map< uint32_t, Ptr<SatWaveform> >::iterator it = m_waveforms.begin ();
        it != m_waveforms.end ();
        ++it )
    {
      /**
       * In return link the link results are in Eb/No format. Since, the C/No is dependent
       * on the symbol rate, we cannot store the requirement in C/No format, but in Eb/No.
       * Eb/No = (Es/log2M)/No = (Es/No)*(1/log2M)  = C/N * (1/log2M) = C/No * (1/fs) * (1/log2M)
      */
      double ebnoRequirementDb = linkResults->GetEbNoDb (it->first, m_targetBLER);
      it->second->SetEbNoRequirement (SatUtils::DbToLinear (ebnoRequirementDb));
    }
}

Ptr<SatWaveform>
SatWaveformConf::GetWaveform (uint32_t wfId) const
{
  NS_LOG_FUNCTION (this << wfId);

  if (m_minWfId > wfId || wfId > m_maxWfId )
    {
      NS_FATAL_ERROR ("SatWaveformConf::GetWaveform - unsupported waveform id: " << wfId);
    }

  return m_waveforms.at (wfId);
}

uint32_t
SatWaveformConf::GetDefaultWaveformId () const
{
  NS_LOG_FUNCTION (this << m_defaultWfId);

  if (m_minWfId > m_defaultWfId || m_defaultWfId > m_maxWfId )
    {
      NS_FATAL_ERROR ("SatWaveformConf::GetDefaultWaveformId - unsupported waveform id: " << m_defaultWfId);
    }

  return m_defaultWfId;
}

bool
SatWaveformConf::GetBestWaveformId (double cno, double symbolRateInBaud, uint32_t& wfId, uint32_t burstLength) const
{
  NS_LOG_FUNCTION (this << cno << symbolRateInBaud << wfId << burstLength);

  bool success (false);

  // If ACM is disabled, return the default waveform
  if (!m_acmEnabled || std::isnan (cno))
    {
      wfId = m_defaultWfId;
      success = true;
      return success;
    }

  // Return the waveform with best spectral efficiency
  for ( std::map< uint32_t, Ptr<SatWaveform> >::const_reverse_iterator rit = m_waveforms.rbegin ();
        rit != m_waveforms.rend ();
        ++rit )
    {
      if (rit->second->GetBurstLengthInSymbols () == burstLength)
        {
          double cnoThr = rit->second->GetCNoThreshold (symbolRateInBaud);
          // The first waveform over the threshold
          if (cnoThr <= cno)
            {
              wfId = rit->first;
              success = true;
              break;
            }
        }
    }

  NS_LOG_INFO ("Get best waveform in RTN link (ACM)! CNo: " << SatUtils::LinearToDb(cno) << ", Symbol rate: " << symbolRateInBaud << ", burst length: " << burstLength << ", WF: " << wfId);

  return success;
}

bool
SatWaveformConf::GetMostRobustWaveformId (uint32_t& wfId, uint32_t burstLength) const
{
  NS_LOG_FUNCTION (this << burstLength);

  bool found = false;

  uint32_t payloadInBytes = std::numeric_limits<uint32_t>::max ();

  // find the waveform with the more robust waveform than previous one
  for ( std::map< uint32_t, Ptr<SatWaveform> >::const_reverse_iterator rit = m_waveforms.rbegin ();
        rit != m_waveforms.rend ();
        ++rit )
    {
      if (rit->second->GetBurstLengthInSymbols () == burstLength)
        {
          // The waveform more robust than previous one
          if ( rit->second->GetPayloadInBytes () < payloadInBytes)
            {
              payloadInBytes = rit->second->GetPayloadInBytes ();
              wfId = rit->first;
              found = true;
            }
        }
    }

  return found;
}

void
SatWaveformConf::Dump (double carrierBandwidthInHz, double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << carrierBandwidthInHz << symbolRateInBaud);

  for ( std::map< uint32_t, Ptr<SatWaveform> >::const_iterator it = m_waveforms.begin ();
        it != m_waveforms.end ();
        ++it )
    {
      std::cout << "WaveformId: " << it->first << " ";
      it->second->Dump (carrierBandwidthInHz, symbolRateInBaud);
    }
}

SatEnums::SatModcod_t
SatWaveformConf::GetModCod (uint32_t wfId) const
{
  NS_LOG_FUNCTION (this << wfId);

  if (m_minWfId > wfId || wfId > m_maxWfId )
    {
      NS_FATAL_ERROR ("SatWaveformConf::GetModCod - unsupported waveform id: " << wfId);
    }

  std::map< uint32_t, Ptr<SatWaveform> >::const_iterator it = m_waveforms.find (wfId);

  if (it != m_waveforms.end ())
    {
      return m_waveforms.at (wfId)->GetModCod ();
    }
  else
    {
      NS_FATAL_ERROR ("Waveform id: " << wfId << " not found in the waveform container!");
    }

  return SatEnums::SAT_NONVALID_MODCOD;
}


SatEnums::SatModcod_t
SatWaveformConf::ConvertToModCod (uint32_t modulatedBits, uint32_t codingRateNumerator, uint32_t codingRateDenominator) const
{
  NS_LOG_FUNCTION (this << modulatedBits << codingRateNumerator << codingRateDenominator);

  switch (modulatedBits)
    {
    // QPSK
    case 2:
      {
        if (codingRateNumerator == 1 && codingRateDenominator == 3)
          {
            return SatEnums::SAT_MODCOD_QPSK_1_TO_3;
          }
        else if (codingRateNumerator == 1 && codingRateDenominator == 2)
          {
            return SatEnums::SAT_MODCOD_QPSK_1_TO_2;
          }
        else if (codingRateNumerator == 2 && codingRateDenominator == 3)
          {
            return SatEnums::SAT_MODCOD_QPSK_2_TO_3;
          }
        else if (codingRateNumerator == 3 && codingRateDenominator == 4)
          {
            return SatEnums::SAT_MODCOD_QPSK_3_TO_4;
          }
        else if (codingRateNumerator == 5 && codingRateDenominator == 6)
          {
            return SatEnums::SAT_MODCOD_QPSK_5_TO_6;
          }
        else
          {
            NS_FATAL_ERROR ("Unsupported coding rate numerator: " << codingRateNumerator << ", denominator: " << codingRateDenominator);
          }
        break;
      }
    // 8PSK
    case 3:
      {
        if (codingRateNumerator == 2 && codingRateDenominator == 3)
          {
            return SatEnums::SAT_MODCOD_8PSK_2_TO_3;
          }
        else if (codingRateNumerator == 3 && codingRateDenominator == 4)
          {
            return SatEnums::SAT_MODCOD_8PSK_3_TO_4;
          }
        else if (codingRateNumerator == 5 && codingRateDenominator == 6)
          {
            return SatEnums::SAT_MODCOD_8PSK_5_TO_6;
          }
        else
          {
            NS_FATAL_ERROR ("Unsupported coding rate numerator: " << codingRateNumerator << ", denominator: " << codingRateDenominator);
          }
        break;

      }
    // 16 QAM
    case 4:
      {
        if (codingRateNumerator == 3 && codingRateDenominator == 4)
          {
            return SatEnums::SAT_MODCOD_16QAM_3_TO_4;
          }
        else if (codingRateNumerator == 5 && codingRateDenominator == 6)
          {
            return SatEnums::SAT_MODCOD_16QAM_5_TO_6;
          }
        else
          {
            NS_FATAL_ERROR ("Unsupported coding rate numerator: " << codingRateNumerator << ", denominator: " << codingRateDenominator);
          }

        break;

      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported modulated bits:" << modulatedBits);
        break;
      }
    }
  return SatEnums::SAT_NONVALID_MODCOD;
}



}; // namespace ns3
