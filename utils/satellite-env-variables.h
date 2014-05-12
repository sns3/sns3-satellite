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
#ifndef SATELLITE_ENV_VARIABLES_H
#define SATELLITE_ENV_VARIABLES_H

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for environmental variables
 */
class SatEnvVariables : public Object
{
public:

  /**
   * \brief NS-3 function for type id
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  SatEnvVariables ();

  /**
   * \brief Destructor
   */
  ~SatEnvVariables ();

  /**
   * \brief Function for getting the path to current working directory
   * \return path to current working directory
   */
  std::string GetCurrentWorkingDirectory ();

  /**
   * \brief Function for getting the path to executable
   * \return path to executable
   */
  std::string GetPathToExecutable ();

  /**
   * \brief Function for setting the path to current working directory
   * \param currentWorkingDirectory path to current working directory
   */
  void SetCurrentWorkingDirectory (std::string currentWorkingDirectory);

  /**
   * \brief Function for setting the path to executable
   * \param pathToExecutable path to executable
   */
  void SetPathToExecutable (std::string pathToExecutable);

  /**
   * \brief Function for locating the data folder
   * \return path to data folder
   */
  std::string GetDataPath () { return m_dataPath; }

  /**
   * \brief Function for locating the data directory within the NS-3 simulator folder
   * \return path to the data directory
   */
  std::string LocateDataDirectory ();

  /**
   * \brief Function for locating a specific directory within the NS-3 simulator folder
   * \param initialPath initial search path
   * \return path to the located directory
   */
  std::string LocateDirectory (std::string initialPath);

  /**
   * \brief Function for checking if the directory exists
   * \return does the directory exist
   */
  bool IsValidDirectory (std::string path);


private:

  /**
   * \brief Path to current working directory
   */
  std::string m_currentWorkingDirectory;

  /**
   * \brief Path to executable
   */
  std::string m_pathToExecutable;

  /**
   * \brief Path to current working directory (attribute value)
   */
  std::string m_currentWorkingDirectoryFromAttribute;

  /**
   * \brief Path to executable (attribute value)
   */
  std::string m_pathToExecutableFromAttribute;

  /**
   * \brief How many directory levels to check for data path
   */
  uint32_t m_levelsToCheck;

  /**
   * \brief Default data path
   */
  std::string m_dataPath;
};

} // namespace ns3

#endif /* SATELLITE_ENV_VARIABLES_H */
