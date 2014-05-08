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
#include "satellite-log.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/singleton.h"
#include "ns3/string.h"

NS_LOG_COMPONENT_DEFINE ("SatLog");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLog);

TypeId 
SatLog::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLog")
    .SetParent<Object> ()
    .AddConstructor<SatLog> ();
  return tid;
}

TypeId
SatLog::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatLog::SatLog () :
  m_currentWorkingDirectory (Singleton<SatEnvVariables>::Get ()->GetCurrentWorkingDirectory ())
{
  NS_LOG_FUNCTION (this);

  CreateLog (LOG_GENERIC,"");
  CreateLog (LOG_INFO,"_info");
  CreateLog (LOG_WARNING,"_warning");
  CreateLog (LOG_ERROR,"_error");
}

SatLog::~SatLog ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatLog::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatLog::Reset ()
{
  NS_LOG_FUNCTION (this);

  if (!m_container.empty ())
    {
      WriteToFile ();

      m_container.clear ();
    }
}

Ptr<SatOutputFileStreamStringContainer>
SatLog::CreateLog (LogType_t logType, std::string tag)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;

  filename << m_currentWorkingDirectory << "/src/satellite/data/logs/log" << tag;

  std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair (logType, CreateObject<SatOutputFileStreamStringContainer> (filename.str ().c_str (), std::ios::out)));

  if (result.second == false)
    {
      NS_FATAL_ERROR ("SatLog::AddLog failed");
    }

  NS_LOG_INFO ("SatLog::AddLog: Added log with tag " << tag);

  return result.first->second;
}

Ptr<SatOutputFileStreamStringContainer>
SatLog::FindLog (LogType_t logType)
{
  NS_LOG_FUNCTION (this);

  container_t::iterator iter = m_container.find (logType);

  if (iter == m_container.end ())
    {
      NS_FATAL_ERROR ("SatLog::FindLog - No log for key " << logType);
    }

  return iter->second;
}

void
SatLog::WriteToFile ()
{
  NS_LOG_FUNCTION (this);

  container_t::iterator iter;

  for (iter = m_container.begin (); iter != m_container.end (); iter++)
    {
      iter->second->WriteContainerToFile ();
    }
}

void
SatLog::AddToLog (LogType_t logType, std::string newLine)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatOutputFileStreamStringContainer> log = FindLog (logType);

  if (log != NULL)
    {
      log->AddToContainer (newLine);
    }

  if (logType != LOG_GENERIC)
    {
      Ptr<SatOutputFileStreamStringContainer> log_all = FindLog (LOG_GENERIC);

      if (log_all != NULL)
        {
          log_all->AddToContainer (newLine);
        }
    }
}

} // namespace ns3
