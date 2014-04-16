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

#ifndef SATELLITE_PACKET_TRACE_H_
#define SATELLITE_PACKET_TRACE_H_

#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "satellite-enums.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief The SatPacketTrace implements a packet trace functionality.
 * The movement of packet through the satellite stack can be traced
 * in different protocol layers and direction.
 */

class SatPacketTrace : public Object
{
public:

  /**
   * \brief Constructor
   */
  SatPacketTrace ();

  /**
   * \brief Destructor
   */
  virtual ~SatPacketTrace ();


  TypeId GetInstanceTypeId () const;
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * \brief Add a packet trace entry to the log
   * \param Time time of a trace event
   * \param Packet event(SND, RCV, DRP, ENQ)
   * \param Node type (UT, SAT, GW, NCC, TER)
   * \param Node id
   * \param MAC address
   * \param Log level (ND, LLC, MAC, PHY, CH)
   * \param Link direction (FWD, RTN)
   * \param Packet info (List of: Packet id, source MAC address, destination MAC address)
   */
  void AddTraceEntry (Time now,
                      SatEnums::SatPacketEvent_t packetEvent,
                      SatEnums::SatNodeType_t nodeType,
                      uint32_t nodeId,
                      Mac48Address macAddress,
                      SatEnums::SatLogLevel_t logLevel,
                      SatEnums::SatLinkDir_t linkDir,
                      std::string packetInfo);

private:

  /**
   * \brief Print header to the packet trace log
   */
  void PrintHeader ();

  /**
   * File name of the packet trace log
   */
  std::string m_fileName;

  /**
   * Stream wrapper used for packet traces
   */
  Ptr<OutputStreamWrapper> m_packetTraceStream;

};

}


#endif /* SATELLITE_PACKET_TRACE_H_ */
