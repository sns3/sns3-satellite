/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#include <fstream>
#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "satellite-fading-external-input-trace.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatFadingExternalInputTrace");

namespace ns3 {


SatFadingExternalInputTrace::SatFadingExternalInputTrace ()
  : m_traceFileType (),
    m_startTime (),
    m_timeInterval ()
{
  NS_FATAL_ERROR ("SatFadingExternalInputTrace::SatFadingExternalInputTrace - Constructor not in use");
}

SatFadingExternalInputTrace::SatFadingExternalInputTrace (TraceFileType_e type, std::string fileName)
  : m_startTime (-1.0),
    m_timeInterval (-1.0)
{
  NS_LOG_FUNCTION (this);

  m_traceFileType = type;
  ReadTrace (fileName);
}


SatFadingExternalInputTrace::~SatFadingExternalInputTrace ()
{
  NS_LOG_FUNCTION (this);
}


void SatFadingExternalInputTrace::ReadTrace (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = new std::ifstream (filePathName.c_str (), std::ios::in | std::ios::binary);

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

  // Currently supports two or three column formats
  uint32_t columns = (m_traceFileType == FT_TWO_COLUMN) ? 2 : 3;

  float temp;
  int32_t count (0);
  std::vector<float> values;
  ifs->read ((char*)&temp, sizeof(float));
  m_startTime = temp;

  // While state is good
  while (ifs->good ())
    {
      // Store previous value
      values.push_back (temp);

      // Read the new value
      ++count;
      ifs->read ((char*)&temp, sizeof(float));

      // Handle previous value
      if (count % columns == 0)
        {
          NS_ASSERT (values.size () == columns);

          m_traceVector.push_back (values);
          values.clear ();

          // Calculate the sampling interval
          if (m_timeInterval < 0.0)
            {
              m_timeInterval = temp - m_startTime;
            }
        }
    }
  ifs->close ();
  delete ifs;
}

double
SatFadingExternalInputTrace::GetFading () const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_traceVector.empty ());

  float simTime = Simulator::Now ().GetSeconds ();

  if (simTime < m_startTime)
    {
      NS_LOG_ERROR (this << " requested time is smaller than the minimum time value!");
    }

  // Calculate the index to the time sample just before current time
  uint32_t lowerIndex = (uint32_t)(std::floor (std::abs (simTime - m_startTime) / m_timeInterval));

  if (lowerIndex >= m_traceVector.size ())
    {
      NS_LOG_ERROR (this << " calculated index exceeds trace file size!");
    }

  float lowerKey = m_traceVector.at (lowerIndex).at (TIME_INDEX);
  float upperKey = m_traceVector.at (lowerIndex + 1).at (TIME_INDEX);

  // Interpolation in linear domain
  float lowerVal = SatUtils::DbToLinear (m_traceVector.at (lowerIndex).at (FADING_INDEX));
  float upperVal = SatUtils::DbToLinear (m_traceVector.at (lowerIndex + 1).at (FADING_INDEX));

  // y = y0 + (y1 - y0) * (x - x0) / (x1 - x0)
  double fading = lowerVal + (upperVal - lowerVal)
    * (simTime - lowerKey) / (upperKey - lowerKey);

  /*
  std::cout << "Now: " << simTime <<
      ", interpolated fading: " << fading_dB <<
      ", interpolated fading: " << SatUtils::DbToLinear (fading_dB) <<
      ", Lower time: " << lowerKey <<
      ", Upper time: " << upperKey <<
      ", Lower fading: " << lowerVal <<
      ", Upper fading: " << upperVal << std::endl;
  */
  return fading;
}

bool
SatFadingExternalInputTrace::TestFadingTrace () const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_traceVector.empty ());

  std::vector<std::vector<float> >::const_iterator cit;
  float prevTime (-1.0);
  float currTime (-1.0);

  for (cit = m_traceVector.begin (); cit != m_traceVector.end (); ++cit)
    {
      if (prevTime > 0)
        {
          currTime = cit->at (TIME_INDEX);
          double diff = std::abs ( std::abs (currTime - prevTime) - m_timeInterval);

          // Test that the the time samples are from constant interval and
          // the time samples are always increasing.
          if ( diff > 0.0003 || currTime < prevTime)
            {
              return false;
            }
        }
      prevTime = cit->at (TIME_INDEX);
    }

  // Succeeded
  return true;
}

} // namespace ns3
