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
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "satellite-wave-form-conf.h"
#include "../model/satellite-link-results.h"
#include "../model/satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatWaveformConf");

namespace ns3 {


SatWaveform::SatWaveform ()
{
  NS_ASSERT (true);
}

SatWaveform::SatWaveform (uint32_t modulatedBits, double codingRate, uint32_t payloadBytes, uint32_t lengthInSymbols)
:m_modulatedBits (modulatedBits),
 m_codingRate (codingRate),
 m_payloadBytes (payloadBytes),
 m_lengthInSymbols (lengthInSymbols)
{

}

uint32_t
SatWaveform::GetBurstLengthInSymbols () const
{
  return m_lengthInSymbols;
}

double
SatWaveform::GetBurstDurationInSeconds (double symbolRateInBaud) const
{
  return m_lengthInSymbols / symbolRateInBaud;
}

double
SatWaveform::GetSpectralEfficiency (double carrierBandwidthInHz, double symbolRateInBaud) const
{
  return ( 8.0 * m_payloadBytes ) / (m_lengthInSymbols / symbolRateInBaud) / carrierBandwidthInHz;
}

double
SatWaveform::GetThroughputInBitsPerSecond (double symbolRateInBaud) const
{
  return 8.0 * m_payloadBytes / ( m_lengthInSymbols / symbolRateInBaud );
}

double
SatWaveform::GetCNoThreshold (double symbolRateInBaud) const
{
  return m_esnoThreshold * symbolRateInBaud * m_modulatedBits;
}

void
SatWaveform::SetEsNoThreshold (double esnoThreshold)
{
  m_esnoThreshold = esnoThreshold;
}

void
SatWaveform::Dump (double carrierBandwidthInHz, double symbolRateInBaud) const
{
  std::cout << "ModulatedBits: " << m_modulatedBits << ", CodingRate: " << m_codingRate <<
      ", Payload: " << m_payloadBytes << ", BurstLength: " << m_lengthInSymbols <<
      ", EsNoThreshold: " << SatUtils::LinearToDb (m_esnoThreshold) <<
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
    .AddAttribute( "PerTarget",
                   "Packet error rate target",
                    DoubleValue (0.00001),
                    MakeDoubleAccessor(&SatWaveformConf::m_perTarget),
                    MakeDoubleChecker<double>())
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
      Ptr<SatWaveform> wf = Create<SatWaveform> (modulatedBits, dCodingRate, payloadBytes, durationInSymbols);
      m_waveforms.insert (std::make_pair (wfIndex, wf));

      // get next row
      *ifs >> wfIndex >> modulatedBits >> sCodingRate >> payloadBytes >> durationInSymbols;
    }

  ifs->close ();
  delete ifs;
}


void SatWaveformConf::InitializeEsNoRequirements( Ptr<SatLinkResultsDvbRcs2> linkResults )
{
  for ( std::map< uint32_t, Ptr<SatWaveform> >::iterator it = m_waveforms.begin ();
      it != m_waveforms.end ();
      ++it )
    {
      double esnoTresholdDb = linkResults->GetEsNoDb (it->first, m_perTarget);
      it->second->SetEsNoThreshold (SatUtils::DbToLinear (esnoTresholdDb));
    }
}


bool
SatWaveformConf::GetBestWaveformId (double cno, double symbolRateInBaud, uint32_t& wfId, uint32_t burstLength) const
{
  bool success (false);

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
  for ( std::map< uint32_t, Ptr<SatWaveform> >::const_iterator it = m_waveforms.begin ();
      it != m_waveforms.end ();
      ++it )
    {
      std::cout << "WaveformId: " << it->first << " ";
      it->second->Dump (carrierBandwidthInHz, symbolRateInBaud);
    }
}


}; // namespace ns3
