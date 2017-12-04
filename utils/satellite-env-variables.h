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
#include "satellite-output-fstream-string-container.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for environmental variables. The class is used for
 * storing information on environmental variables, e.g., the current
 * working directory and path to data folder. The class implements
 * functionality for locating directories within the NS-3 simulator
 * folder. Certain variables can be overwritten with attributes.
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
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

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
   * Function for creating a directory
   * \param path path to the directory to be created
   */
  void CreateDirectory (std::string path);

  /**
   * \brief Function for locating the data folder
   * \return path to data folder
   */
  std::string GetDataPath ()
  {
    return m_dataPath;
  }

  /**
   * \brief Function for getting the simulation folder
   * \return path to the simulation folder
   */
  std::string GetOutputPath ();

  /**
   * \brief Method for setting the simulation output path. The path must exist.
   * \param outputPath The new output path.
   */
  void SetOutputPath (std::string outputPath);

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
   * \brief Function for locating a specific file within the NS-3 simulator folder
   * \param initialPath initial search path for the file
   * \return path to the located file
   */
  std::string LocateFile (std::string initialPath);

  /**
   * \brief Function for checking if the directory exists
   * \return does the directory exist
   */
  bool IsValidDirectory (std::string path);

  /**
   * \brief Function for checking if the file exists
   * \return does the file exist
   */
  bool IsValidFile (std::string pathToFile);

  /**
   * \brief Returns current real world date and time
   * \return date and time as a string
   */
  std::string GetCurrentDateAndTime ();

  /**
   * \brief Function for setting the output variables. The function also creates the output folder based on the new variables.
   * NOTICE: this function is meant to me used only in test cases, where issues with singletons might arise. In any other case
   * the class attributes should be used instead.
   * \param campaignName simulation campaign name
   * \param simTag simulation tag
   * \param enableOutputOverwrite is simulation output overwrite enabled
   */
  void SetOutputVariables (std::string campaignName, std::string simTag, bool enableOutputOverwrite);

  /**
   * \brief Initialize class
   * NOTICE: this function is meant to me used only in test cases, where issues with singletons might arise.
   */
  void DoInitialize ();

  /**
   * \brief Reset class
   * NOTICE: this function is meant to me used only in test cases, where issues with singletons might arise.
   */
  void DoDispose ();

private:
  /**
   * \brief Function for forming the output path and directory structure
   * \param campaignName simulation campaign name
   * \param simTag simulation tag
   * \param enableOutputOverwrite is simulation output overwrite enabled
   * \return path to the created folder
   */
  std::string InitializeOutputFolders (std::string campaignName, std::string simTag, bool enableOutputOverwrite);

  /**
   * \brief Function for outputting the most essential simulation specific environmental information
   */
  void DumpSimulationInformation ();

  /**
   * \brief
   * \param dataPath
   */
  void DumpRevisionDiff (std::string dataPath);

  /**
   * \brief Function for executing the command and inserting the output into a string container
   * \param command command to be executed
   * \param outputContainer pointer to output container
   */
  void ExecuteCommandAndReadOutput (std::string command, Ptr<SatOutputFileStreamStringContainer> outputContainer);

  /**
   * \brief Function for forming the next level of a path
   * \param path path
   * \param stringToAdd string to be added to the path
   * \return combined path
   */
  std::string AddToPath (std::string path, std::string stringToAdd);

  /**
   * \brief Function for forming the output path
   * \param simRootPath simulation output root path
   * \param campaignName simulation campaign name
   * \param simTag simulation tag
   * \param safetyTag safety tag for avoiding overwrites
   * \return path to the created folder
   */
  std::string FormOutputPath (std::string simRootPath, std::string campaignName, std::string simTag, std::string safetyTag);

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

  /**
   * \brief Result output path
   */
  std::string m_outputPath;

  /**
   * \brief Simulation campaign name. Affects the simulation output folder.
   */
  std::string m_campaignName;

  /**
   * \brief Path to the simulation output root folder
   */
  std::string m_simRootPath;

  /**
   * \brief Tag related to the current simulation
   */
  std::string m_simTag;

  /**
   * \brief Enable simulation output overwrite
   */
  bool m_enableOutputOverwrite;

  /**
   * \brief Is output path initialized
   */
  bool m_isOutputPathInitialized;

  /**
   * \brief Is simulation information output enabled
   */
  bool m_enableSimInfoOutput;

  /**
   * \brief Is simulation information diff output enabled
   */
  bool m_enableSimInfoDiffOutput;

  /**
   * \brief Is data folder excluded from the simulation information diff
   */
  bool m_excludeDataFolderFromDiff;

  /**
   * \brief Flag for disposing and initializing
   */
  bool m_isInitialized;
};

} // namespace ns3

#endif /* SATELLITE_ENV_VARIABLES_H */
