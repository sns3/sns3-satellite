/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
#ifndef SATELLITE_LOG_H
#define SATELLITE_LOG_H

#include "ns3/satellite-output-fstream-string-container.h"
#include <map>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for simulator output logging such as warnings and error messages.
 *
 * Contains 4 different log types:
 * 0 - for all messages
 * 1 - for info messages
 * 2 - for warning messages
 * 3 - for error messages
 *
 * Messages for types 1 to 3 will have their own files in addition to the log for all messages.
 * This is to enable quick checking for specific message types.
 */
class SatLog : public Object
{
public:

  /**
   * \brief typedef for map of containers
   */
  typedef std::map <uint32_t, Ptr<SatOutputFileStreamStringContainer> > container_t;

  /**
   * \brief Constructor
   */
  SatLog ();

  /**
   * \brief Destructor
   */
  ~SatLog ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NS-3 instance type id function
   * \return Instance type is
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Do needed dispose actions.
   */
  void DoDispose ();

  /**
   * \brief Function for adding a line to a specific log
   * \param key key to log
   * \param newLine line to be added
   */
  void AddToLog (key_t key, std::string newLine);

  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

private:

  /**
   * \brief Function for creating a log
   * \param key key to the log
   * \param tag tag for the filename
   * \return the created log
   */
  Ptr<SatOutputFileStreamStringContainer> CreateLog (uint32_t key, std::string tag);

  /**
   * \brief Function for finding a log based on the key
   * \param key key for the log
   * \return the log
   */
  Ptr<SatOutputFileStreamStringContainer> FindLog (uint32_t key);

  /**
   * \brief Write the contents of a container matching to the key into a file
   */
  void WriteToFile ();

  /**
   * \brief Map for containers
   */
  container_t m_container;

  /**
   * \brief Path to current working directory
   */
  std::string m_currentWorkingDirectory;
};

} // namespace ns3

#endif /* SATELLITE_LOG_H */
