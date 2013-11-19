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

#ifndef SAT_OUTPUT_FSTREAM_DOUBLE_CONTAINER_H
#define SAT_OUTPUT_FSTREAM_DOUBLE_CONTAINER_H

#include <fstream>
#include "ns3/object.h"
#include "satellite-output-fstream-wrapper.h"

namespace ns3 {

class SatOutputFileStreamDoubleContainer : public Object
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
  SatOutputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  /**
   * \brief
   */
  SatOutputFileStreamDoubleContainer ();

  /**
   * \brief
   */
  ~SatOutputFileStreamDoubleContainer ();

  /**
   * \brief
   */
  void WriteContainerToFile ();

  /**
   * \brief
   */
  void AddToContainer (std::vector<double> newItem);

  /**
   * \brief Do needed dispose actions
   */
  void DoDispose ();

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
   * \brief
   */
  void OpenStream ();

  /**
   * \brief
   */
  SatOutputFileStreamWrapper* m_outputFileStreamWrapper;

  /**
   * \brief
   */
  std::ofstream* m_outputFileStream;

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
};

} // namespace ns3

#endif /* SAT_OUTPUT_FSTREAM_DOUBLE_CONTAINER_H */
