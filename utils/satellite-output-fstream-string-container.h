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

#ifndef SAT_OUTPUT_FSTREAM_STRING_CONTAINER_H
#define SAT_OUTPUT_FSTREAM_STRING_CONTAINER_H

#include <fstream>
#include "ns3/object.h"
#include "satellite-output-fstream-wrapper.h"
#include <ns3/gnuplot.h>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for output file stream container for strings.
 * The class implements storing the values and writing the stored
 * values into a file.
 */
class SatOutputFileStreamStringContainer : public Object
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
   */
  SatOutputFileStreamStringContainer (std::string filename, std::ios::openmode filemode);

  /**
   * \brief Constructor
   */
  SatOutputFileStreamStringContainer ();

  /**
   * \brief Destructor
   */
  ~SatOutputFileStreamStringContainer ();

  /**
   * \brief Function for writing the container contents to file
   */
  void WriteContainerToFile ();

  /**
   * \brief Function for adding the line to container
   */
  void AddToContainer (std::string newLine);

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
   * \brief Function for opening the output file stream
   */
  void OpenStream ();

  /**
   * \brief Pointer to output file stream wrapper
   */
  SatOutputFileStreamWrapper* m_outputFileStreamWrapper;

  /**
   * \brief Pointer to output file stream
   */
  std::ofstream* m_outputFileStream;

  /**
   * \brief Container for lines
   */
  std::vector<std::string > m_container;

  /**
   * \brief File name
   */
  std::string m_fileName;

  /**
   * \brief File mode
   */
  std::ios::openmode m_fileMode;
};

} // namespace ns3

#endif /* SAT_OUTPUT_FSTREAM_STRING_CONTAINER_H */
