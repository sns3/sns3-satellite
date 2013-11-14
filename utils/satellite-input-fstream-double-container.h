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

  static TypeId GetTypeId (void);

  /**
   *
   * @param filename
   * @param filemode
   */
  SatInputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  SatInputFileStreamDoubleContainer ();

  /**
   *
   */
  ~SatInputFileStreamDoubleContainer ();

  void UpdateContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  std::vector<double> ProceedToLastSmallerThanAndReturnIt (double comparisonValue, uint32_t column);

private:

  void Reset ();

  void ResetStream ();

  void ClearContainer ();

  SatInputFileStreamWrapper* m_inputFileStreamWrapper;

  std::ifstream* m_inputFileStream;

  std::vector<std::vector<double> > m_container;

  std::string m_fileName;

  std::ios::openmode m_fileMode;

  uint32_t m_valuesInRow;
};

} // namespace ns3

#endif /* SAT_INPUT_FSTREAM_DOUBLE_CONTAINER_H */
