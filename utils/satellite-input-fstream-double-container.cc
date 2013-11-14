/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Washington
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

#include "satellite-input-fstream-double-container.h"
#include "ns3/log.h"
#include "ns3/abort.h"

NS_LOG_COMPONENT_DEFINE ("SatInputFileStreamDoubleContainer");

namespace ns3 {

TypeId
SatInputFileStreamDoubleContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatInputFileStreamDoubleContainer")
    .SetParent<Object> ()
    .AddConstructor<SatInputFileStreamDoubleContainer> ();
  return tid;
}

SatInputFileStreamDoubleContainer::SatInputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow) :
    m_inputFileStreamWrapper (),
    m_inputFileStream (),
    m_container (),
    m_fileName (filename),
    m_fileMode (filemode),
    m_valuesInRow (valuesInRow)
{
  NS_LOG_FUNCTION (this << m_fileName << m_fileMode);

  UpdateContainer (m_fileName, m_fileMode, m_valuesInRow);
}

SatInputFileStreamDoubleContainer::SatInputFileStreamDoubleContainer () :
    m_inputFileStreamWrapper (),
    m_inputFileStream (),
    m_container (),
    m_fileName (),
    m_fileMode (),
    m_valuesInRow ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (0);
}

SatInputFileStreamDoubleContainer::~SatInputFileStreamDoubleContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatInputFileStreamDoubleContainer::UpdateContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow)
{
  NS_LOG_FUNCTION (this);

  ClearContainer ();

  m_fileName = filename;
  m_fileMode = filemode;
  m_valuesInRow = valuesInRow;

  m_inputFileStreamWrapper = new SatInputFileStreamWrapper (filename,filemode);
  m_inputFileStream = m_inputFileStreamWrapper->GetStream ();

  std::string tempString;
  double tempValue;

  if (m_inputFileStream->is_open ())
  {
    while (m_inputFileStream->good ())
      {
        std::vector<double> tempVector;

        for( uint32_t i = 0; i < m_valuesInRow; i++ )
          {
            *m_inputFileStream >> tempString;
            tempValue = atof (tempString.c_str());
            tempVector.push_back (tempValue);
          }
        m_container.push_back (tempVector);
      }
    m_inputFileStream->close ();
  }
  else
    {
      NS_ABORT_MSG ("Input stream is not valid for reading.");
    }

  ResetStream ();
}

std::vector<double>
SatInputFileStreamDoubleContainer::ProceedToLastSmallerThanAndReturnIt (double comparisonValue, uint32_t column)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (column < m_valuesInRow);
  NS_ASSERT (m_container.size () > 0);

  uint32_t lastValidPosition = 0;

  for (uint32_t i = 0; i < m_container.size(); i++)
    {
      if (m_container[i].at(column) > comparisonValue)
        {
          lastValidPosition = i - 1;
        }
    }

  if (lastValidPosition < 0)
    {
      lastValidPosition = 0;
    }

  return m_container[lastValidPosition];
}

void
SatInputFileStreamDoubleContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  ResetStream ();
  ClearContainer ();
}

void
SatInputFileStreamDoubleContainer::ResetStream ()
{
  NS_LOG_FUNCTION (this);

  delete m_inputFileStreamWrapper;
  m_inputFileStream = 0;

  m_fileName = "";
  m_fileMode = std::ifstream::in;
}

void
SatInputFileStreamDoubleContainer::ClearContainer ()
{
  NS_LOG_FUNCTION (this);

  for( uint32_t i = 0; i < m_container.size (); i++ )
    {
      m_container[i].clear ();
    }
  m_container.clear();

  m_valuesInRow = 0;
}

} // namespace ns3
