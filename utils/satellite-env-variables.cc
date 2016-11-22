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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ns3/fatal-error.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "satellite-env-variables.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

NS_LOG_COMPONENT_DEFINE ("SatEnvVariables");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatEnvVariables);

TypeId
SatEnvVariables::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatEnvVariables")
    .SetParent<Object> ()
    .AddConstructor<SatEnvVariables> ()
    .AddAttribute ("CurrentWorkingDirectory",
                   "Current working directory for the simulator.",
                   StringValue (""),
                   MakeStringAccessor (&SatEnvVariables::m_currentWorkingDirectoryFromAttribute),
                   MakeStringChecker ())
    .AddAttribute ("PathToExecutable",
                   "Path to the simulator executable.",
                   StringValue (""),
                   MakeStringAccessor (&SatEnvVariables::m_pathToExecutableFromAttribute),
                   MakeStringChecker ())
    .AddAttribute ("DataPath",
                   "Path to the data folder.",
                   StringValue ("contrib/satellite/data"),
                   MakeStringAccessor (&SatEnvVariables::m_dataPath),
                   MakeStringChecker ())
    .AddAttribute ("SimulationCampaignName",
                   "Simulation campaign name. Affects the simulation folder.",
                   StringValue (""),
                   MakeStringAccessor (&SatEnvVariables::m_campaignName),
                   MakeStringChecker ())
    .AddAttribute ("SimulationTag",
                   "Tag related to the current simulation.",
                   StringValue ("default"),
                   MakeStringAccessor (&SatEnvVariables::m_simTag),
                   MakeStringChecker ())
    .AddAttribute ("EnableSimulationOutputOverwrite",
                   "Enable simulation output overwrite.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatEnvVariables::m_enableOutputOverwrite),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableSimInfoOutput",
                   "Enable simulation information output.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatEnvVariables::m_enableSimInfoOutput),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableSimInfoDiffOutput",
                   "Enable simulation information diff output.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatEnvVariables::m_enableSimInfoDiffOutput),
                   MakeBooleanChecker ())
    .AddAttribute ("ExcludeSatelliteDataFolderFromSimInfoDiff",
                   "Exclude satellite data folder from the revision diff.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatEnvVariables::m_excludeDataFolderFromDiff),
                   MakeBooleanChecker ());
  return tid;
}

TypeId
SatEnvVariables::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatEnvVariables::SatEnvVariables ()
  : m_currentWorkingDirectory (""),
    m_pathToExecutable (""),
    m_currentWorkingDirectoryFromAttribute (""),
    m_pathToExecutableFromAttribute (""),
    m_levelsToCheck (10),
    m_dataPath ("contrib/satellite/data"),
    m_outputPath (""),
    m_campaignName (""),
    m_simRootPath ("contrib/satellite/data/sims"),
    m_simTag ("default"),
    m_enableOutputOverwrite (true),
    m_isOutputPathInitialized (false),
    m_enableSimInfoOutput (true),
    m_enableSimInfoDiffOutput (true),
    m_excludeDataFolderFromDiff (true),
    m_isInitialized (false)
{
  NS_LOG_FUNCTION (this);

  // Attributes are needed already in construction phase:
  // - ConstructSelf call in constructor
  // - GetInstanceTypeId needs to be implemented
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  Initialize ();
}

void
SatEnvVariables::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  if (!m_isInitialized)
    {
      char currentWorkingDirectory[FILENAME_MAX] = "";

      if (!getcwd (currentWorkingDirectory, sizeof (currentWorkingDirectory)))
        {
          NS_FATAL_ERROR ("SatEnvVariables::SatEnvVariables - Could not determine current working directory.");
        }
      currentWorkingDirectory[sizeof (currentWorkingDirectory) - 1] = '\0';
      m_currentWorkingDirectory = std::string (currentWorkingDirectory);

      char pathToExecutable[FILENAME_MAX] = "";

      int res;
#ifdef __linux__
      res = readlink ("/proc/self/exe",
              pathToExecutable,
              sizeof (pathToExecutable));
#elif __APPLE__
      uint32_t size = sizeof (pathToExecutable);
      res = _NSGetExecutablePath (pathToExecutable, &size);
#else
      NS_FATAL_ERROR ("SatEnvVariables::SatEnvVariables - Unknown compiler.");
#endif
      if (res < 0)
        {
          NS_FATAL_ERROR ("SatEnvVariables::SatEnvVariables - Could not determine the path to executable.");
        }
      pathToExecutable[sizeof (pathToExecutable) - 1] = '\0';
      m_pathToExecutable = std::string (pathToExecutable);
      m_isInitialized = true;
    }
}

void
SatEnvVariables::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  if (m_isInitialized)
    {
      if (m_enableSimInfoOutput)
        {
          if (!m_isOutputPathInitialized)
            {
              InitializeOutputFolders (m_campaignName, m_simTag, m_enableOutputOverwrite);
            }

          DumpSimulationInformation ();
        }

      m_currentWorkingDirectory = "";
      m_pathToExecutable = "";
      m_isOutputPathInitialized = false;
      m_isInitialized = false;
    }
}

void
SatEnvVariables::SetCurrentWorkingDirectory (std::string currentWorkingDirectory)
{
  NS_LOG_FUNCTION (this);

  m_currentWorkingDirectory = currentWorkingDirectory;
}

void
SatEnvVariables::SetPathToExecutable (std::string pathToExecutable)
{
  NS_LOG_FUNCTION (this);

  m_pathToExecutable = pathToExecutable;
}

std::string
SatEnvVariables::GetOutputPath ()
{
  NS_LOG_FUNCTION (this);

  if (!m_isOutputPathInitialized)
    {
      InitializeOutputFolders (m_campaignName, m_simTag, m_enableOutputOverwrite);
    }

  return m_outputPath;
}

void
SatEnvVariables::SetOutputPath (std::string outputPath)
{
	NS_ASSERT_MSG (IsValidDirectory (outputPath), outputPath << " is not a valid directory");
	m_outputPath = outputPath;
	m_isOutputPathInitialized = true;
}

void
SatEnvVariables::SetOutputVariables (std::string campaignName, std::string simTag, bool enableOutputOverwrite)
{
  NS_LOG_FUNCTION (this);

  m_campaignName = campaignName;
  m_simTag = simTag;
  m_enableOutputOverwrite = enableOutputOverwrite;

  InitializeOutputFolders (m_campaignName, m_simTag, m_enableOutputOverwrite);
}

SatEnvVariables::~SatEnvVariables ()
{
  NS_LOG_FUNCTION (this);

  Dispose ();
}

std::string
SatEnvVariables::GetCurrentWorkingDirectory ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatEnvVariables::GetCurrentWorkingDirectory - Current working directory: " << m_currentWorkingDirectory);
  NS_LOG_INFO ("SatEnvVariables::GetCurrentWorkingDirectory - Current working directory (attribute): " << m_currentWorkingDirectoryFromAttribute);

  if (m_currentWorkingDirectoryFromAttribute.empty ())
    {
      NS_LOG_INFO ("SatEnvVariables::GetCurrentWorkingDirectory - Attribute string is empty, using detected working directory");
      return m_currentWorkingDirectory;
    }
  else
    {
      NS_LOG_INFO ("SatEnvVariables::GetCurrentWorkingDirectory - Using attributed working directory");
      return m_currentWorkingDirectoryFromAttribute;
    }
}

std::string
SatEnvVariables::GetPathToExecutable ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatEnvVariables::GetPathToExecutable - Path to executable: " << m_pathToExecutable);
  NS_LOG_INFO ("SatEnvVariables::GetPathToExecutable - Path to executable (attribute): " << m_pathToExecutableFromAttribute);

  if (m_pathToExecutableFromAttribute.empty ())
    {
      NS_LOG_INFO ("SatEnvVariables::GetPathToExecutable - Attribute string is empty, using detected path to executable");
      return m_pathToExecutable;
    }
  else
    {
      NS_LOG_INFO ("SatEnvVariables::GetPathToExecutable - Using attributed path to executable");
      return m_pathToExecutableFromAttribute;
    }
}

bool
SatEnvVariables::IsValidDirectory (std::string path)
{
  NS_LOG_FUNCTION (this);

  struct stat st;
  bool validDirectory = false;

  if (stat (path.c_str (),&st) == 0)
    {
      if (st.st_mode && S_IFDIR != 0)
        {
          validDirectory = true;
        }
    }

  NS_LOG_INFO ("SatEnvVariables::IsValidDirectory - " << path << " validity: " << validDirectory);

  return validDirectory;
}

bool
SatEnvVariables::IsValidFile (std::string pathToFile)
{
  NS_LOG_FUNCTION (this);

  struct stat st;
  bool validFile = (stat (pathToFile.c_str (), &st) == 0);

  NS_LOG_INFO ("SatEnvVariables::IsValidFile - " << pathToFile << " validity: " << validFile);

  return validFile;
}

std::string
SatEnvVariables::LocateDataDirectory ()
{
  NS_LOG_FUNCTION (this);

  return LocateDirectory (GetDataPath ());
}

std::string
SatEnvVariables::LocateDirectory (std::string initialPath)
{
  NS_LOG_FUNCTION (this);

  std::string path;
  bool directoryFound = false;

  NS_LOG_INFO ("SatEnvVariables::LocateDirectory - Initial path " << initialPath);

  for (uint32_t i = 0; i < m_levelsToCheck; i++)
    {
      std::stringstream dataPath;

      for (uint32_t j = 0; j < i; j++)
        {
          dataPath << "../";
        }

      dataPath << initialPath;

      NS_LOG_INFO ("SatEnvVariables::LocateDirectory - Checking " << dataPath.str ());

      if (IsValidDirectory (dataPath.str ()))
        {
          NS_LOG_INFO ("SatEnvVariables::LocateDirectory - Data directory located in " << dataPath.str ());
          path = dataPath.str ();
          directoryFound = true;
          break;
        }
    }

  if (!directoryFound)
    {
      NS_FATAL_ERROR ("SatEnvVariables::LocateDirectory - Directory not found within " << m_levelsToCheck << " levels: " << initialPath);
    }

  return path;
}

std::string
SatEnvVariables::LocateFile (std::string initialPath)
{
  NS_LOG_FUNCTION (this);

  std::string path;
  bool fileFound = false;

  NS_LOG_INFO ("SatEnvVariables::LocateDirectory - Initial path " << initialPath);

  for (uint32_t i = 0; i < m_levelsToCheck; i++)
    {
      std::stringstream dataPath;

      for (uint32_t j = 0; j < i; j++)
        {
          dataPath << "../";
        }

      dataPath << initialPath;

      NS_LOG_INFO ("SatEnvVariables::LocateFile - Checking " << dataPath.str ());

      if (IsValidFile (dataPath.str ()))
        {
          NS_LOG_INFO ("SatEnvVariables::LocateDirectory - Data directory located in " << dataPath.str ());
          path = dataPath.str ();
          fileFound = true;
          break;
        }
    }

  if (!fileFound)
    {
      NS_FATAL_ERROR ("SatEnvVariables::LocateFile - File not found within " << m_levelsToCheck << " levels: " << initialPath);
    }

  return path;
}

std::string
SatEnvVariables::InitializeOutputFolders (std::string campaignName, std::string simTag, bool enableOutputOverwrite)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatEnvVariables::CreateOutputDirectory - Creating output directory");

  uint32_t safety = 0;
  std::string safetyTag = "";
  std::string outputPath = "";
  bool directoryExists = false;
  std::string simRootPath = LocateDirectory (m_simRootPath);

  // If we have set a campaign name
  if (!campaignName.empty ())
    {
      std::string tempString = AddToPath (simRootPath, campaignName);

      // If the campaign name directory does not exist, create it
      if (!IsValidDirectory (tempString))
        {
          CreateDirectory (tempString);
        }
    }

  // As long as the full output folder is created.
  // Start with false.
  while (!directoryExists)
    {
      outputPath = FormOutputPath (simRootPath, campaignName, simTag, safetyTag);

      // Create new simulation folder or overwrite
      if ( (!IsValidDirectory (outputPath) && !enableOutputOverwrite) || enableOutputOverwrite)
        {
          CreateDirectory (outputPath);
          directoryExists = true;
        }
      // Do not overwrite, but add a new safety string tag to simulation folder
      else
        {
          safety++;

          NS_LOG_INFO ("SatEnvVariables::CreateOutputDirectory - Directory " << outputPath << " exists, increasing safety number to " << safety);

          std::stringstream ss;
          ss << safety;
          safetyTag = ss.str ();

          // Continue loop and try to create a new directory
          directoryExists = false;
        }
    }

  m_isOutputPathInitialized = true;
  m_outputPath = outputPath;
  return outputPath;
}

std::string
SatEnvVariables::FormOutputPath (std::string simRootPath, std::string campaignName, std::string simTag, std::string safetyTag)
{
  NS_LOG_FUNCTION (this);

  std::string outputPath = "";
  std::stringstream tempTag;

  tempTag << simTag;

  if (!safetyTag.empty ())
    {
      tempTag << safetyTag;
    }

  outputPath = AddToPath (outputPath, simRootPath);
  outputPath = AddToPath (outputPath, campaignName);
  outputPath = AddToPath (outputPath, tempTag.str ());

  NS_LOG_INFO ("SatEnvVariables::FormOutputPath - Formed path " + outputPath);

  return outputPath;
}

std::string
SatEnvVariables::AddToPath (std::string path, std::string stringToAdd)
{
  NS_LOG_FUNCTION (this);

  std::stringstream tempPath;
  tempPath << path;

  if (!stringToAdd.empty ())
    {
      if (tempPath.str ().empty ())
        {
          tempPath << stringToAdd;
        }
      else
        {
          tempPath << "/" << stringToAdd;
        }
    }
  return tempPath.str ();
}

void
SatEnvVariables::CreateDirectory (std::string path)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatEnvVariables::CreateDirectory - Creating directory " + path);

  mkdir (path.c_str (), 0777);
}

std::string
SatEnvVariables::GetCurrentDateAndTime ()
{
  NS_LOG_FUNCTION (this);

  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
  std::string str (buffer);

  NS_LOG_INFO ("SatEnvVariables::GetCurrentRealDateAndTime - " + str);

  return str;
}

void
SatEnvVariables::ExecuteCommandAndReadOutput (std::string command, Ptr<SatOutputFileStreamStringContainer> outputContainer)
{
  NS_LOG_FUNCTION (this);

  FILE* pipe = popen (command.c_str (), "r");
  if (pipe)
    {
      std::string data = "";
      char buffer[1024];
      while (!feof (pipe))
        {
          if (fgets (buffer, 1024, pipe) != NULL)
            {
              buffer[strlen (buffer) - 1] = '\0';
              data.append (buffer);
              outputContainer->AddToContainer (data);
              data = "";
            }
        }
      pclose (pipe);
    }
}

void
SatEnvVariables::DumpSimulationInformation ()
{
  NS_LOG_FUNCTION (this);

  std::string dataPath = LocateDirectory (m_outputPath);

  std::ostringstream fileName;
  fileName << dataPath << "/SimInfo.log";
  Ptr<SatOutputFileStreamStringContainer> outputContainer = CreateObject<SatOutputFileStreamStringContainer> (fileName.str ().c_str (), std::ios::out);

  std::ostringstream revisionCommand;
  revisionCommand << "cd contrib/satellite"
                  << " && hg log | head --lines=5 2>&1";
  ExecuteCommandAndReadOutput (revisionCommand.str (), outputContainer);

  std::stringstream line1;
  line1 << "Simulation finished at " << GetCurrentDateAndTime ();

  outputContainer->AddToContainer (line1.str ());

  outputContainer->WriteContainerToFile ();

  if (m_enableSimInfoDiffOutput)
    {
      DumpRevisionDiff (dataPath);
    }
}

void
SatEnvVariables::DumpRevisionDiff (std::string dataPath)
{
  NS_LOG_FUNCTION (this);

  std::ostringstream fileName;
  fileName << dataPath << "/SimDiff.log";
  Ptr<SatOutputFileStreamStringContainer> outputContainer = CreateObject<SatOutputFileStreamStringContainer> (fileName.str ().c_str (), std::ios::out);

  std::ostringstream diffCommand;
  diffCommand << "cd contrib/satellite"
              << " && hg diff"
              << " --show-function"
              << " --ignore-all-space"
              << " --ignore-space-change"
              << " --ignore-blank-lines";

  if (m_excludeDataFolderFromDiff)
    {
      diffCommand << " --exclude " << m_dataPath;
    }

  diffCommand << " 2>&1";

  ExecuteCommandAndReadOutput (diffCommand.str (), outputContainer);

  outputContainer->WriteContainerToFile ();
}

} // namespace ns3
