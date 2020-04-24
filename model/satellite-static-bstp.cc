/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions Ltd
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

#include <algorithm>
#include <istream>
#include <sstream>

#include "ns3/log.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"

#include "satellite-static-bstp.h"

NS_LOG_COMPONENT_DEFINE ("SatStaticBstp");

namespace ns3 {

SatStaticBstp::SatStaticBstp ()
:m_bstp (),
 m_currentIterator (0),
 m_beamGwMap (),
 m_beamFeederFreqIdMap (),
 m_enabledBeams ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);
}

SatStaticBstp::SatStaticBstp (std::string fileName)
:m_bstp (),
 m_currentIterator (0)
{
  NS_LOG_FUNCTION (this);

  std::string dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory () + "/";

  // Load satellite configuration file
  LoadBstp (dataPath + fileName);
}

void
SatStaticBstp::LoadBstp (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

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

  std::string line;

  // Read the lines in the file one by one
  while (std::getline (*ifs, line, '\n'))
    {
      // Convert the line to stringstream
      std::stringstream ssLine;
      ssLine << line;
      std::string col;
      std::vector<uint32_t> tempVector;

      // Go through the line with a comma as a delimiter, convert
      // the read strings to double and add to temp container.
      while (std::getline (ssLine, col, ','))
        {
          std::stringstream ssCol;
          ssCol << col;
          uint32_t uCol;
          ssCol >> uCol;
          tempVector.push_back (uCol);
        }

      NS_ASSERT (tempVector.size () >= 2);

      m_bstp.push_back (tempVector);
    }

  NS_ASSERT (!m_bstp.empty ());

  ifs->close ();
  delete ifs;
}

std::vector<uint32_t>
SatStaticBstp::GetNextConf () const
{
  NS_LOG_FUNCTION (this);

  uint32_t iter = m_currentIterator;

  NS_ASSERT (iter < m_bstp.size ());

  /**
   * Increase iterator and start from the beginning if
   * we run out of samples in the BSTP.
   */
  m_currentIterator++;
  if (m_currentIterator >= m_bstp.size ())
    {
      m_currentIterator = 0;
    }

  return m_bstp.at (iter);
}

void
SatStaticBstp::AddEnabledBeamInfo (uint32_t beamId,
                                   uint32_t userFreqId,
                                   uint32_t feederFreqId,
                                   uint32_t gwId)
{
  NS_LOG_FUNCTION (this << beamId << userFreqId << feederFreqId << gwId);

  NS_ASSERT (userFreqId == 1);

  m_beamGwMap.insert (std::make_pair (beamId, gwId));
  m_beamFeederFreqIdMap.insert (std::make_pair (beamId, feederFreqId));

  m_enabledBeams.push_back (beamId);
}

void
SatStaticBstp::CheckValidity ()
{
  NS_LOG_FUNCTION (this);

  if (m_bstp.empty ())
    {
      NS_FATAL_ERROR ("BSTP container is empty!");
    }

  std::vector<uint32_t> enabledBeams = m_enabledBeams;
  std::map<uint32_t, uint32_t> beamIds;
  std::map<uint32_t, std::vector<uint32_t> > gwFeederFreqs;

  // All lines
  for (uint32_t i = 0; i < m_bstp.size (); i++)
    {
      beamIds.clear ();
      gwFeederFreqs.clear ();

      // A single BSTP configuration
      std::vector<uint32_t> confEntry = m_bstp.at (i);
      for (uint32_t j = 1; j < confEntry.size (); j++)
        {
          uint32_t beamId = confEntry.at (j);

          std::vector<uint32_t>::iterator eIt =
              std::find (enabledBeams.begin (), enabledBeams.end (), beamId);
          if (eIt != enabledBeams.end ())
            {
              enabledBeams.erase (eIt);
            }

          // One beam id can only exist once at each line
          if ((beamIds.insert (std::make_pair (beamId, 0))).second == false)
            {
              NS_FATAL_ERROR ("Beam id: " << confEntry.at (j) <<
                              " is located twice in the BSTP line: " << i);
            }

          // Find the GW and feeder freq for this beamId
          std::map<uint32_t, uint32_t>::iterator ffIt =
              m_beamFeederFreqIdMap.find (beamId);
          std::map<uint32_t, uint32_t>::iterator gwIt =
              m_beamGwMap.find (beamId);

          // Check that the beam is enabled!
          if (ffIt != m_beamFeederFreqIdMap.end () && gwIt != m_beamGwMap.end ())
            {
              uint32_t feederFreqId = ffIt->second;
              uint32_t gwId = gwIt->second;

              // Find GW and add this feeder freq for it.
              std::map<uint32_t, std::vector<uint32_t> >::iterator findGw =
                  gwFeederFreqs.find (gwId);

              // If it is not found, add a proper vector for it.
              if (findGw == gwFeederFreqs.end ())
                {
                  std::vector <uint32_t> ffIds;
                  ffIds.push_back (feederFreqId);
                  gwFeederFreqs.insert (std::make_pair (gwId, ffIds));
                }
              else
                {
                  // Find whether we have already stored this feeder freq for
                  // this GW!
                  std::vector<uint32_t>::iterator findFfIt =
                      std::find(findGw->second.begin (), findGw->second.end (), feederFreqId);

                  // If we have, we should trigger a fatal error since GW cannot serve
                  // two beams with the same feeder freq at the same time.
                  if (findFfIt != findGw->second.end ())
                    {
                      NS_FATAL_ERROR ("Feeder link freq id: " << feederFreqId <<
                                      " already found for GW: " << findGw->first);
                    }

                  // Otherwise just store the feeder freq
                  findGw->second.push_back (feederFreqId);
                }
            }
          else
            {
              NS_LOG_WARN ("Beam id: " << beamId << " is not enabled, but it located at BSTP!");
            }
        }
    }

  // All enabled spot-beams need to have at least one
  // scheduling entry
  if (enabledBeams.size () > 0)
    {
      NS_FATAL_ERROR ("All enabled beams are not in the BSTP configuration!");
    }
}

} // namespace ns3



