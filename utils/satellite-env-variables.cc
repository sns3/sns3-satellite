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
#include "satellite-env-variables.h"


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
                   StringValue ("src/satellite/data"),
                   MakeStringAccessor (&SatEnvVariables::m_dataPath),
                   MakeStringChecker ());
  return tid;
}

TypeId
SatEnvVariables::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatEnvVariables::SatEnvVariables () :
  m_currentWorkingDirectory (""),
  m_pathToExecutable (""),
  m_currentWorkingDirectoryFromAttribute (""),
  m_pathToExecutableFromAttribute (""),
  m_levelsToCheck (10),
  m_dataPath ("src/satellite/data")
{
  NS_LOG_FUNCTION (this);

  // Attributes are needed already in construction phase:
  // - ConstructSelf call in constructor
  // - GetInstanceTypeId is needed to be implemented
  ObjectBase::ConstructSelf(AttributeConstructionList ());

  char currentWorkingDirectory[FILENAME_MAX] = "";

  if (!getcwd (currentWorkingDirectory, sizeof (currentWorkingDirectory)))
    {
      NS_FATAL_ERROR("SatEnvVariables::SatEnvVariables - Could not determine current working directory.");
    }
  currentWorkingDirectory[sizeof (currentWorkingDirectory) - 1] = '\0';
  m_currentWorkingDirectory = std::string (currentWorkingDirectory);

  char pathToExecutable[FILENAME_MAX] = "";

  if (readlink ("/proc/self/exe",
                pathToExecutable,
                sizeof (pathToExecutable)) < 0)
    {
      NS_FATAL_ERROR("SatEnvVariables::SatEnvVariables - Could not determine the path to executable.");
    }
  pathToExecutable[sizeof (pathToExecutable) - 1] = '\0';
  m_pathToExecutable = std::string (pathToExecutable);
}

void
SatEnvVariables::SetCurrentWorkingDirectory (std::string currentWorkingDirectory)
{
  m_currentWorkingDirectory = currentWorkingDirectory;
}

void
SatEnvVariables::SetPathToExecutable (std::string pathToExecutable)
{
  m_pathToExecutable = pathToExecutable;
}

SatEnvVariables::~SatEnvVariables ()
{
  NS_LOG_FUNCTION (this);

  m_currentWorkingDirectory = "";
  m_pathToExecutable = "";
  m_currentWorkingDirectoryFromAttribute = "";
  m_pathToExecutableFromAttribute = "";
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

  if (stat(path.c_str (),&st) == 0)
    {
      if (st.st_mode && S_IFDIR != 0)
        {
          validDirectory = true;
        }
    }

  NS_LOG_INFO ("SatEnvVariables::IsValidDirectory - " << path << " validity: " << validDirectory);

  return validDirectory;
}

std::string
SatEnvVariables::LocateDataDirectory ()
{
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

} // namespace ns3
