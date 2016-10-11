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

#ifndef SATELLITE_FADING_EXTERNAL_INPUT_TRACE_H
#define SATELLITE_FADING_EXTERNAL_INPUT_TRACE_H

#include <vector>
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief The class for satellite fading external input trace. The class reads
 * fading trace input samples from a file and provides the current fading value
 * for this specific fading file.
 */
class SatFadingExternalInputTrace : public SimpleRefCount <SatFadingExternalInputTrace>
{
public:
  enum TraceFileType_e
  {
    FT_TWO_COLUMN,
    FT_THREE_COLUMN
  };

  /**
   * Default constructor.
   */
  SatFadingExternalInputTrace ();

  /**
   * Constructor with initialization parameters.
   * \param type 
   * \param filePathName 
   */
  SatFadingExternalInputTrace (TraceFileType_e type, std::string filePathName);

  /**
   * Destructor for SatFadingExternalInputTrace
   */
  ~SatFadingExternalInputTrace ();

  /**
   * Get the current fading value for this specific fading file
   * \return fading value in linear format
   */
  double GetFading () const;

  /**
   * A method to test that the fading trace is according to
   * assumptions.
   * \return boolean value indicating success or failure
   */
  bool TestFadingTrace () const;

private:
  /**
   * Read the fading trace from a binary file
   * \param filePathName Path and file name of the fading file
   */
  void ReadTrace (std::string filePathName);

  /**
   * There may be different fading file types.
   * - FT_TWO_COLUMN
   *    - First = time in seconds
   *    - Second = fading in dB
   * - FT_THREE_COLUMN
   *    - First = time in seconds
   *    - Second = fading in dB
   *    - Third = scintillation in dB
   */
  TraceFileType_e m_traceFileType;

  /**
   * Constant indices used in the fading container
   */
  static const uint32_t TIME_INDEX = 0;
  static const uint32_t FADING_INDEX = 1;
  static const uint32_t SCINTILLATION_INDEX = 2;

  /**
   * Fading start time and interval calculated from the actual trace file.
   * Note, that the current implementation assumes that we have constant
   * time interval between fading samples. This assumption has been made
   * for speed-up issues.
   */
  float m_startTime;
  float m_timeInterval;

  /**
   * Container for the fading trace.
   */
  std::vector<std::vector<float> > m_traceVector;
};

} // namespace ns3

#endif /* SATELLITE_FADING_EXTERNAL_INPUT_TRACE_H */
