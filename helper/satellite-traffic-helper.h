/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2020 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef __SATELLITE_TRAFFIC_HELPER_H__
#define __SATELLITE_TRAFFIC_HELPER_H__

#include <ns3/object.h>
#include <ns3/config.h>
#include <ns3/string.h>

#include <ns3/satellite-helper.h>

namespace ns3 {

class SimulationHelper;

/**
 * \brief Creates pre-defined trafics.
 *        Utilizes SatUserHelper and SatBeamHelper helper objects. TODO update
 */
class SatTrafficHelper : public Object
{
public:
  /**
   * \brief List of available traffics
   */
  typedef enum
  {
    NONE, //TODO
    CBR, //TODO
    CUSTOM, //TODO
    POISSON, //TODO
    HTTP, //TODO
    NRTV, //TODO
    VISIO, //
    VOIP //TODO
  } TrafficType_t;

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of object instance
   * \return the TypeId of object instance
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Default constructor. Not used.
   */
  SatTrafficHelper ();

  /**
   * \brief Create a base SatTrafficHelper for creating customized traffics.
   */
  SatTrafficHelper (Ptr<SatHelper> satHelper);

  /**
   * Destructor for SatTrafficHelper
   */
  virtual ~SatTrafficHelper ()
  {
  }

  /**
   * Add a new traffic between GWs and UTs
   * \param interval Wait time between transmission of two packets
   * \param packetSize Packet size in bytes
   * \param gws The Gateways
   * \param uts The User Terminals
   * \param startTime Application Start time
   * \param stopTime Application stop time
   * \param startDelay application start delay between each user
   */
  void AddCbrTraffic (std::string interval, uint32_t packetSize, NodeContainer gws, NodeContainer uts, Time startTime, Time stopTime, Time startDelay);

private:

  Ptr<SatHelper> m_satHelper;			// Pointer to the SatHelper objet

  /**
   * \brief Check if node has a PacketSink installed at certain port.
   */
  bool HasSinkInstalled (Ptr<Node> node, uint16_t port);

};

} // namespace ns3

#endif /* __SATELLITE_TRAFFIC_HELPER_H__ */