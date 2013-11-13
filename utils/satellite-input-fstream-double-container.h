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
  SatInputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode);

  SatInputFileStreamDoubleContainer ();

  /**
   *
   */
  ~SatInputFileStreamDoubleContainer ();

  void UpdateContainer (std::string filename, std::ios::openmode filemode);

private:

  void Reset ();

  SatInputFileStreamWrapper* m_inputFileStreamWrapper;

  std::ifstream* m_inputFileStream;

  double** m_container;
};

} // namespace ns3

#endif /* SAT_INPUT_FSTREAM_DOUBLE_CONTAINER_H */
