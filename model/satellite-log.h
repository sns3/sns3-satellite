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
 * The format for messages is (type, custom file tag, message). Custom file tag
 * is in effect only with LOG_CUSTOM. With other types it does not matter and can
 * be left empty. It is also possible to define a simulation specific tag, which
 * is useful with simulation campaigns for avoiding log file overwrite.
 *
 * The class specifies the following log types:
 * LOG_GENERIC - for all messages
 * LOG_INFO - for info messages
 * LOG_WARNING - for warning messages
 * LOG_ERROR - for error messages
 * LOG_CUSTOM - for custom messages specified by the second parameter
 *
 * The output files are located in contrib/satellite/data/logs folder. The output file
 * format is the following: log<type><custom file tag><simulation tag>
 *
 * With (LOG_CUSTOM, "_exampleTag", "Example message for custom log") and simulation tag
 * "_ut30_beam1" the file log_exampleTag_ut30_beam1 would contain the message
 * "Example message for custom log".
 */
class SatLog : public Object
{
public:
  /**
   * \brief Enum for log types
   */
  typedef enum
  {
    LOG_GENERIC = 0, //!< LOG_GENERIC
    LOG_INFO = 1,   //!< LOG_INFO
    LOG_WARNING = 2, //!< LOG_WARNING
    LOG_ERROR = 3,  //!< LOG_ERROR
    LOG_CUSTOM = 4  //!< LOG_CUSTOM
  } LogType_t;

  /**
   * \brief typedef for container key
   */
  typedef std::pair <LogType_t, std::string> key_t;

  /**
   * \brief typedef for map of containers
   */
  typedef std::map <key_t, Ptr<SatOutputFileStreamStringContainer> > container_t;

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
   * \param logType log type
   * \param fileTag file tag for the filename
   * \param message line to be added
   */
  void AddToLog (LogType_t logType, std::string fileTag, std::string message);

  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

private:
  /**
   * \brief Function for getting the file tag for predefined log types
   * \param logType log type
   * \return file tag
   */
  std::string GetFileTag (LogType_t logType);

  /**
   * \brief Function for creating a log
   * \param logType log type
   * \param fileTag file tag for the filename
   * \return the created log
   */
  Ptr<SatOutputFileStreamStringContainer> CreateLog (LogType_t logType, std::string fileTag);

  /**
   * \brief Function for finding a log based on the key
   * \param logType log type
   * \param fileTag file tag for the filename
   * \return the log
   */
  Ptr<SatOutputFileStreamStringContainer> FindLog (LogType_t logType, std::string fileTag);

  /**
   * \brief Write the contents of a container matching to the key into a file
   */
  void WriteToFile ();

  /**
   * \brief Map for containers
   */
  container_t m_container;
};

} // namespace ns3

#endif /* SATELLITE_LOG_H */
