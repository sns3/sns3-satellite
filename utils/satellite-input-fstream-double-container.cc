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
#include "ns3/simulator.h"

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
    m_valuesInRow (valuesInRow),
    m_currentPosition (0),
    m_numOfPasses (0),
    m_shiftValue (0),
    m_timeColumn (0)
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
    m_valuesInRow (),
    m_currentPosition (),
    m_numOfPasses (),
    m_shiftValue (),
    m_timeColumn ()
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
SatInputFileStreamDoubleContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  Object::DoDispose();
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
            tempValue = atof (tempString.c_str ());
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
SatInputFileStreamDoubleContainer::ProceedToNextClosestTimeSample ()
{
  NS_LOG_FUNCTION (this);

  while (!FindNextClosest(m_currentPosition,m_timeColumn,m_shiftValue, Now ().GetDouble()))
    {
      m_numOfPasses++;
      m_shiftValue = m_numOfPasses * m_container[m_container.size () - 1].at (m_timeColumn);
    }

  if (m_numOfPasses > 0)
    {
      std::cout << "WARNING WARNING WARNING! - SatInputFileStreamDoubleContainer OUT OF SAMPLES! Looping old samples!" << std::endl;
    }

  return m_container[m_currentPosition];
}

bool
SatInputFileStreamDoubleContainer::FindNextClosest (uint32_t lastValidPosition, uint32_t column, double shiftValue, double comparisonValue)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (column < m_valuesInRow);
  NS_ASSERT (m_container.size () > 0);
  NS_ASSERT (lastValidPosition >= 0 && lastValidPosition < m_container.size ());

  bool valueFound = false;

  for (uint32_t i = lastValidPosition; i < m_container.size (); i++)
    {
      if (m_container[i].at (column) + shiftValue > comparisonValue)
        {
          double difference1 = (m_container[lastValidPosition].at (column) + shiftValue - comparisonValue);
          double difference2 = (m_container[i].at (column) + shiftValue - comparisonValue);

          if (difference1 < difference2)
            {
              m_currentPosition = lastValidPosition;
            }
          else
            {
              m_currentPosition = i;
            }
          valueFound = true;
          break;
        }
      lastValidPosition = i;
    }

  if (valueFound && m_numOfPasses > 0 && m_currentPosition == 0)
    {
      double difference1 = (m_container[m_currentPosition].at (column) + shiftValue - comparisonValue);
      double difference2 = (m_container[m_container.size() - 1].at (column) + ((m_numOfPasses - 1) * m_container[m_container.size () - 1].at (column)) - comparisonValue);

      if (difference1 > difference2)
        {
          m_currentPosition = m_container.size() - 1;
        }
    }

  return valueFound;
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

  if (m_inputFileStreamWrapper != NULL)
    {
      delete m_inputFileStreamWrapper;
      m_inputFileStreamWrapper = 0;
    }
  m_inputFileStream = 0;

  m_fileName = "";
  m_fileMode = std::ifstream::in;
}

void
SatInputFileStreamDoubleContainer::ClearContainer ()
{
  NS_LOG_FUNCTION (this);

  if (!m_container.empty())
    {
      for (uint32_t i = 0; i < m_container.size (); i++)
        {
          if (!m_container[i].empty ())
            {
              m_container[i].clear ();
            }
        }
      m_container.clear ();
    }

  m_valuesInRow = 0;
  m_currentPosition = 0;
  m_numOfPasses = 0;
  m_shiftValue = 0;
}

} // namespace ns3
