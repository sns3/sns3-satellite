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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

#ifndef SAT_INPUT_FSTREAM_TIME_DOUBLE_CONTAINER_H
#define SAT_INPUT_FSTREAM_TIME_DOUBLE_CONTAINER_H

#include <fstream>
#include "ns3/object.h"
#include "satellite-input-fstream-wrapper.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for input file stream container for storing double values.
 * The class implements reading the values from a file, storing the values
 * and iterating the stored values.
 *
 * Row format is [time, value1, ..., value n].
 */
class SatInputFileStreamTimeDoubleContainer : public Object
{
public:
  /**
   * \brief NS-3 function for type id
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   * \param filename file name
   * \param filemode file mode
   * \param valuesInRow number of values in a row
   */
  SatInputFileStreamTimeDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  /**
   * \brief Constructor
   */
  SatInputFileStreamTimeDoubleContainer ();

  /**
   * \brief Destructor
   */
  ~SatInputFileStreamTimeDoubleContainer ();

  /**
   * \brief Function for updating the container
   * \param filename file name
   * \param filemode file mode
   * \param valuesInRow number of values in a row
   */
  void UpdateContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  /**
   * \brief Function for locating the next closest time sample and returning the values related to it
   * \return matching values
   */
  std::vector<double> ProceedToNextClosestTimeSample ();

  /**
   * \brief Do needed dispose actions
   */
  void DoDispose ();

private:
  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

  /**
   * \brief Function for resetting the stream
   */
  void ResetStream ();

  /**
   * \brief Function for clearing the container
   */
  void ClearContainer ();

  /**
   * \brief Function for reading a row from file
   * \return the row
   */
  std::vector<double> ReadRow ();

  /**
   * \brief Function for locating the next closest value index. This locator loops the samples if the container does not have enough samples. Next closest index value is saved to a separate member variable.
   * \param lastValidPosition position of last matching value
   * \param shiftValue value to shift the time if needed
   * \param comparisonValue value which next closest match to find
   * \return was next time sample found
   */
  bool FindNextClosest (uint32_t lastValidPosition, double timeShiftValue, double comparisonTimeValue);

  /**
   * \brief Check container time sample sanity
   */
  void CheckContainerSanity ();

  /**
   * \brief Pointer to input file stream wrapper
   */
  SatInputFileStreamWrapper* m_inputFileStreamWrapper;

  /**
   * \brief Pointer to input file stream
   */
  std::ifstream* m_inputFileStream;

  /**
   * \brief Container for value rows
   */
  std::vector<std::vector<double> > m_container;

  /**
   * \brief File name
   */
  std::string m_fileName;

  /**
   * \brief File mode
   */
  std::ios::openmode m_fileMode;

  /**
   * \brief Number of values in a row
   */
  uint32_t m_valuesInRow;

  /**
   * \brief Last valid position
   */
  uint32_t m_lastValidPosition;

  /**
   * \brief Number for how many times the available samples have been looped over
   */
  uint32_t m_numOfPasses;

  /**
   * \brief Shift value for sample time
   */
  double m_timeShiftValue;

  /**
   * \brief Index for column which contains time information
   */
  uint32_t m_timeColumn;
};

} // namespace ns3

#endif /* SAT_INPUT_FSTREAM_TIME_DOUBLE_CONTAINER_H */
