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
#include "satellite-env-variables.h"
#include "ns3/string.h"
#include <stdio.h>
#include <unistd.h>
#include "ns3/fatal-error.h"

NS_LOG_COMPONENT_DEFINE ("SatEnvVariables");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatEnvVariables);

TypeId 
SatEnvVariables::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatEnvVariables")
    .SetParent<Object> ()
    .AddConstructor<SatEnvVariables>();
  return tid;
}

SatEnvVariables::SatEnvVariables () :
  m_currentWorkingDirectory (""),
  m_pathToExecutable ("")
{
  NS_LOG_FUNCTION (this);

  /// TODO: load attributes here
  /// m_currentWorkingDirectory = ...
  /// m_pathToExecutable = ...

  if (!m_currentWorkingDirectory.length() > 0)
    {
      char currentPath[FILENAME_MAX];
      std::stringstream path;

       if (!getcwd (currentPath, sizeof (currentPath)))
         {
           NS_FATAL_ERROR ("SatEnvVariables - Could not determine current working directory.");
         }

       currentPath[sizeof (currentPath) - 1] = '\0';
       path << currentPath;
       m_currentWorkingDirectory = path.str ();
    }

  if (!m_pathToExecutable.length() > 0)
    {
      char currentPath[FILENAME_MAX];
      std::stringstream path;

      if (readlink("/proc/self/exe", currentPath, sizeof (currentPath)) < 0)
        {
          NS_FATAL_ERROR ("SatEnvVariables - Could not determine the path to executable.");
        }

      currentPath[sizeof (currentPath) - 1] = '\0';
      path << currentPath;
      m_pathToExecutable = path.str ();
    }
}

SatEnvVariables::~SatEnvVariables ()
{
  NS_LOG_FUNCTION (this);

  m_currentWorkingDirectory = "";
  m_pathToExecutable = "";
}

std::string
SatEnvVariables::GetCurrentWorkingDirectory ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatEnvVariables - Current working directory: " << m_currentWorkingDirectory);

  return m_currentWorkingDirectory;
}

std::string
SatEnvVariables::GetPathToExecutable ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatEnvVariables - Path to executable: " << m_pathToExecutable);

  return m_pathToExecutable;
}

} // namespace ns3
