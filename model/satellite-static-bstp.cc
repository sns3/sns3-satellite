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

#include <istream>
#include <sstream>

#include "satellite-static-bstp.h"

#include "ns3/log.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"

NS_LOG_COMPONENT_DEFINE ("SatStaticBstp");

namespace ns3 {

SatStaticBstp::SatStaticBstp ()
:m_bstp (),
 m_currentIterator (0)
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
  if (m_currentIterator > m_bstp.size ())
    {
      m_currentIterator = 0;
    }

  return m_bstp.at (iter);
}


} // namespace ns3



