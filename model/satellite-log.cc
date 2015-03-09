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

SatLog::SatLog ()
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());
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
SatLog::CreateLog (LogType_t logType, std::string fileTag)
{
  NS_LOG_FUNCTION (this);

  std::stringstream filename;
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->GetOutputPath ();

  filename << dataPath << "/log" << fileTag;

  key_t key = std::make_pair (logType, fileTag);

  std::pair <container_t::iterator, bool> result = m_container.insert (std::make_pair (key, CreateObject<SatOutputFileStreamStringContainer> (filename.str ().c_str (), std::ios::out)));

  if (result.second == false)
    {
      NS_FATAL_ERROR ("SatLog::CreateLog failed");
    }

  NS_LOG_INFO ("SatLog::CreateLog - Created type " << logType << " log with file tag " << fileTag);

  return result.first->second;
}

Ptr<SatOutputFileStreamStringContainer>
SatLog::FindLog (LogType_t logType, std::string fileTag)
{
  NS_LOG_FUNCTION (this);

  key_t key = std::make_pair (logType, fileTag);

  container_t::iterator iter = m_container.find (key);

  if (iter == m_container.end ())
    {
      return CreateLog (logType, fileTag);
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
SatLog::AddToLog (LogType_t logType, std::string fileTag, std::string message)
{
  NS_LOG_FUNCTION (this);

  if (logType != LOG_CUSTOM)
    {
      fileTag = GetFileTag (logType);
    }

  Ptr<SatOutputFileStreamStringContainer> log = FindLog (logType, fileTag);

  NS_LOG_INFO ("SatLog::AddToLog - Type: " << logType << ", file tag: " << fileTag << ", message: " << message);

  if (log != NULL)
    {
      log->AddToContainer (message);
    }
}

std::string
SatLog::GetFileTag (LogType_t logType)
{
  std::string fileTag = "";

  switch (logType)
    {
    case LOG_GENERIC:
      {
        fileTag = "";
        break;
      }
    case LOG_INFO:
      {
        fileTag = "_info";
        break;
      }
    case LOG_WARNING:
      {
        fileTag = "_warning";
        break;
      }
    case LOG_ERROR:
      {
        fileTag = "_error";
        break;
      }
    case LOG_CUSTOM:
      {
        NS_FATAL_ERROR ("SatLog::GetFileTag - This function should not be called for custom logs");
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatLog::GetFileTag - Invalid log type");
        break;
      }
    }
  return fileTag;
}

} // namespace ns3
