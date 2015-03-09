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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/trace-helper.h"
#include "ns3/string.h"
#include "ns3/mac48-address.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"
#include "satellite-packet-trace.h"

NS_LOG_COMPONENT_DEFINE ("SatPacketTrace");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPacketTrace);

SatPacketTrace::SatPacketTrace ()
{
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  AsciiTraceHelper asciiTraceHelper;

  std::stringstream outputPath;
  outputPath << Singleton<SatEnvVariables>::Get ()->GetOutputPath () << "/" << m_fileName << ".log";

  m_packetTraceStream = asciiTraceHelper.CreateFileStream (outputPath.str ());

  PrintHeader ();
}

SatPacketTrace::~SatPacketTrace ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatPacketTrace::GetInstanceTypeId () const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

TypeId
SatPacketTrace::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatPacketTrace")
    .SetParent<Object> ()
    .AddAttribute ("FileName",
                   "File name for the packet trace output",
                   StringValue ("PacketTrace"),
                   MakeStringAccessor (&SatPacketTrace::m_fileName),
                   MakeStringChecker ())
  ;
  return tid;
}

void
SatPacketTrace::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatPacketTrace::PrintHeader ()
{
  NS_LOG_FUNCTION (this);

  *m_packetTraceStream->GetStream () << "COLUMN DESCRIPTIONS" << std::endl;
  *m_packetTraceStream->GetStream () << "-------------------" << std::endl;
  *m_packetTraceStream->GetStream () << "Time" << std::endl;
  *m_packetTraceStream->GetStream () << "Packet event (SND, RCV, DRP, ENQ)" << std::endl;
  *m_packetTraceStream->GetStream () << "Node type (UT, SAT, GW, NCC, TER)" << std::endl;
  *m_packetTraceStream->GetStream () << "Node id" << std::endl;
  *m_packetTraceStream->GetStream () << "MAC address" << std::endl;
  *m_packetTraceStream->GetStream () << "Log level (ND, LLC, MAC, PHY, CH)" << std::endl;
  *m_packetTraceStream->GetStream () << "Link direction (FWD, RTN)" << std::endl;
  *m_packetTraceStream->GetStream () << "Packet info (List of: Packet id, source MAC address, destination MAC address)" << std::endl;
  *m_packetTraceStream->GetStream () << "-------------------" << std::endl << std::endl;
}

void
SatPacketTrace::AddTraceEntry (Time now,
                               SatEnums::SatPacketEvent_t packetEvent,
                               SatEnums::SatNodeType_t nodeType,
                               uint32_t nodeId,
                               Mac48Address macAddress,
                               SatEnums::SatLogLevel_t logLevel,
                               SatEnums::SatLinkDir_t linkDir,
                               std::string packetInfo)
{
  NS_LOG_FUNCTION (this << now.GetSeconds ());

  /**
   * TODO: Currently the packet trace logs all entries updated by the protocol layers.
   * A filtering functionality may be implemented later, to restrict the amount of output
   * data. The user may configure the filtering rules by attributes, e.g.
   * - Entries from only certain type packet events
   * - Entries from only certain node types
   * - Entries from only certain node ids
   * - Entries with certain log level (i.e. protocol layer)
   * - Entries from one simulation direction
   */

  std::ostringstream oss;
  oss << now.GetSeconds () << " "
      << SatEnums::GetPacketEventName (packetEvent) << " "
      << SatEnums::GetNodeTypeName (nodeType) << " "
      << nodeId << " "
      << macAddress << " "
      << SatEnums::GetLogLevelName (logLevel) << " "
      << SatEnums::GetLinkDirName (linkDir) << " "
      << packetInfo;

  *m_packetTraceStream->GetStream () << oss.str () << std::endl;
}

}
