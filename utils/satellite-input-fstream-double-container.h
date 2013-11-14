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

#ifndef SAT_INPUT_FSTREAM_DOUBLE_CONTAINER_H
#define SAT_INPUT_FSTREAM_DOUBLE_CONTAINER_H

#include <fstream>
#include "ns3/object.h"
#include "satellite-input-fstream-wrapper.h"

namespace ns3 {

class SatInputFileStreamDoubleContainer : public Object
{
public:

  /**
   * \brief
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   * \brief
   * \param filename
   * \param filemode
   * \param valuesInRow
   */
  SatInputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  /**
   * \brief
   */
  SatInputFileStreamDoubleContainer ();

  /**
   * \brief
   */
  ~SatInputFileStreamDoubleContainer ();

  /**
   * \brief
   * \param filename
   * \param filemode
   * \param valuesInRow
   */
  void UpdateContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  /**
   * \brief
   * \param comparisonValue
   * \param column
   * \return
   */
  std::vector<double> ProceedToNextClosestTimeSample ();

private:

  /**
   * \brief
   */
  void Reset ();

  /**
   * \brief
   */
  void ResetStream ();

  /**
   * \brief
   */
  void ClearContainer ();

  /**
   *
   * \param lastValidPosition
   * \param column
   * \param shiftValue
   * \param comparisonValue
   * \return
   */
  bool FindNextClosest (uint32_t lastValidPosition, uint32_t column, double shiftValue, double comparisonValue);

  /**
   * \brief
   */
  SatInputFileStreamWrapper* m_inputFileStreamWrapper;

  /**
   * \brief
   */
  std::ifstream* m_inputFileStream;

  /**
   * \brief
   */
  std::vector<std::vector<double> > m_container;

  /**
   * \brief
   */
  std::string m_fileName;

  /**
   * \brief
   */
  std::ios::openmode m_fileMode;

  /**
   * \brief
   */
  uint32_t m_valuesInRow;

  /**
   * \brief
   */
  uint32_t m_currentPosition;

  /**
   * \brief
   */
  uint32_t m_numOfPasses;

  /**
   * \brief
   */
  double m_shiftValue;

  /**
   * \brief
   */
  uint32_t m_timeColumn;
};

} // namespace ns3

#endif /* SAT_INPUT_FSTREAM_DOUBLE_CONTAINER_H */
