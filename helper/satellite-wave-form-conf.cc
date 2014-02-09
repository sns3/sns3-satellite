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
#include <string>
#include <algorithm>
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "satellite-wave-form-conf.h"
#include "../model/satellite-link-results.h"
#include "../model/satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatWaveformConf");

namespace ns3 {


SatWaveform::SatWaveform ()
{
  NS_ASSERT (true);
}

SatWaveform::SatWaveform (uint32_t wfId, uint32_t modulatedBits, double codingRate, uint32_t payloadBytes, uint32_t lengthInSymbols)
:m_waveformId (wfId),
 m_modulatedBits (modulatedBits),
 m_codingRate (codingRate),
 m_payloadBytes (payloadBytes),
 m_lengthInSymbols (lengthInSymbols)
{

}

uint32_t
SatWaveform::GetWaveformId () const
{
  return m_waveformId;
}


uint32_t
SatWaveform::GetPayloadInBytes () const
{
  return m_payloadBytes;
}

uint32_t
SatWaveform::GetBurstLengthInSymbols () const
{
  NS_LOG_FUNCTION (this);

  return m_lengthInSymbols;
}

double
SatWaveform::GetBurstDurationInSeconds (double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << symbolRateInBaud);

  return m_lengthInSymbols / symbolRateInBaud;
}

double
SatWaveform::GetSpectralEfficiency (double carrierBandwidthInHz, double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << carrierBandwidthInHz << symbolRateInBaud);

  return ( 8.0 * m_payloadBytes ) / (m_lengthInSymbols / symbolRateInBaud) / carrierBandwidthInHz;
}

double
SatWaveform::GetThroughputInBitsPerSecond (double symbolRateInBaud) const
{
  NS_LOG_FUNCTION (this << symbolRateInBaud);

  return 8.0 * m_payloadBytes / ( m_lengthInSymbols / symbolRateInBaud );
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
  return m_ebnoRequirement * symbolRateInBaud * log2 (m_modulatedBits);
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
      ", BurstDuration: " << GetBurstDurationInSeconds (symbolRateInBaud) <<
      ", Throughput: " << GetThroughputInBitsPerSecond (symbolRateInBaud) <<
      ", SpectralEfficiency: " << GetSpectralEfficiency (carrierBandwidthInHz, symbolRateInBaud) <<
      ", C/No threshold: " << SatUtils::LinearToDb (GetCNoThreshold (symbolRateInBaud)) << std::endl;
}

NS_OBJECT_ENSURE_REGISTERED (SatWaveformConf);


SatWaveformConf::SatWaveformConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatWaveformConf::SatWaveformConf (std::string filePathName)
{
  NS_LOG_FUNCTION (this);
  ReadFromFile (filePathName);
}

TypeId
SatWaveformConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatWaveformConf")
    .SetParent<Object> ()
    .AddAttribute( "TargetBLER",
                   "Block error rate target",
                    DoubleValue (0.00001),
                    MakeDoubleAccessor(&SatWaveformConf::m_targetBLER),
                    MakeDoubleChecker<double>())
    .AddAttribute( "AcmEnabled",
                   "Enable ACM",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatWaveformConf::m_acmEnabled),
                   MakeBooleanChecker ())
    .AddAttribute( "DefaultWfId",
                   "Default waveform id",
                   UintegerValue (3),
                   MakeUintegerAccessor (&SatWaveformConf::m_defaultWfId),
                   MakeUintegerChecker<uint32_t> ())
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

  while (ifs->good())
    {
      // Store temporarily all wfIds
      wfIds.push_back(wfIndex);

      // Convert the coding rate fraction into double
      std::istringstream ss(sCodingRate);
      std::string token;
      std::vector<uint32_t> output;

      while(std::getline(ss, token, '/'))
        {
          uint32_t i;
          std::stringstream s;
          s.str(token);
          s >> i;
          output.push_back (i);
        }

      NS_ASSERT (output.size () == 2);

      double dCodingRate = double(output[0]) / output[1];

      // Create new waveform and insert it to the waveform map
      Ptr<SatWaveform> wf = Create<SatWaveform> (wfIndex, modulatedBits, dCodingRate, payloadBytes, durationInSymbols);
      m_waveforms.insert (std::make_pair (wfIndex, wf));

      // get next row
      *ifs >> wfIndex >> modulatedBits >> sCodingRate >> payloadBytes >> durationInSymbols;
    }

  ifs->close ();
  delete ifs;

  // Note, currently we assume that the waveform ids are consecutive!
  m_minWfId = *std::min_element(wfIds.begin (), wfIds.end ());
  m_maxWfId = *std::max_element(wfIds.begin (), wfIds.end ());
}


void SatWaveformConf::InitializeEbNoRequirements( Ptr<SatLinkResultsDvbRcs2> linkResults )
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
  NS_ASSERT(m_minWfId <= wfId && wfId <= m_maxWfId);

  return m_waveforms.at(wfId);
}

uint32_t
SatWaveformConf::GetDefaultWaveformId () const
{
  NS_LOG_FUNCTION (this << m_defaultWfId);
  NS_ASSERT(m_minWfId <= m_defaultWfId && m_defaultWfId <= m_maxWfId);

  return m_defaultWfId;
}

bool
SatWaveformConf::GetBestWaveformId (double cno, double symbolRateInBaud, uint32_t& wfId, uint32_t burstLength) const
{
  NS_LOG_FUNCTION (this << cno << symbolRateInBaud << wfId << burstLength);

  bool success (false);

  // If ACM is disabled, return the default waveform
  if (!m_acmEnabled)
    {
      wfId = m_defaultWfId;
      success = true;
      return success;
    }

  // Return the waveform with best spectral efficiency

  // JPU: Note, that this algorithm is not final, but just a skeleton which shall be enhanced
  // when implementing the actual NCC RTN link burst scheduler algorithm!

  for ( std::map< uint32_t, Ptr<SatWaveform> >::const_reverse_iterator rit = m_waveforms.rbegin ();
      rit != m_waveforms.rend ();
      ++rit )
    {
      if (rit->second->GetBurstLengthInSymbols() == burstLength)
        {
          double cnoThr = rit->second->GetCNoThreshold(symbolRateInBaud);
          // The first waveform over the threshold
          if (cnoThr <= cno)
            {
              wfId = rit->first;
              success = true;
              break;
            }
        }
    }

  return success;
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


}; // namespace ns3
