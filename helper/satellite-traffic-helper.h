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
#include <ns3/cbr-application.h>
#include <ns3/application-container.h>

namespace ns3 {

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
    CBR, //TODO -> implemented
    CUSTOM, //TODO -> implemented
    POISSON, //TODO -> implemented, but verify formula and repartition of offTime
    HTTP, //TODO
    NRTV, //TODO
    VISIO, //TODO
    VOIP //TODO -> set codecs
  } TrafficType_t;

  typedef enum
  {
    G_711_1,
    G_711_2,
    G_723_1,
    G_729_2,
    G_729_3
  } VoipCodec_t;

  typedef enum
  {
    RTN_LINK,
    FWD_LINK
  } TrafficDirection_t;

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
   * Add a new CBR traffic between chosen GWs and UTs
   * \param direction Direction of traffic
   * \param interval Wait time between transmission of two packets
   * \param packetSize Packet size in bytes
   * \param gws The Gateways
   * \param uts The User Terminals
   * \param startTime Application Start time
   * \param stopTime Application stop time
   * \param startDelay application start delay between each user
   */
  void AddCbrTraffic (TrafficDirection_t direction,
                      std::string interval,
                      uint32_t packetSize,
                      NodeContainer gws,
                      NodeContainer uts,
                      Time startTime,
                      Time stopTime,
                      Time startDelay);

  /**
   * Add a new Poisson traffic between chosen GWs and UTs
   * \param direction Direction of traffic
   * \param onTime On time duration in seconds
   * \param offTimeExpMean Off time mean in seconds. The off time follows an exponential law of mean offTimeExpMean
   * \param rate The rate with the unit
   * \param packetSize Packet size in bytes
   * \param gws The Gateways
   * \param uts The User Terminals
   * \param startTime Application Start time
   * \param stopTime Application stop time
   * \param startDelay application start delay between each user
   */
  void AddPoissonTraffic (TrafficDirection_t direction,
                          Time onTime,
                          Time offTimeExpMean,
                          std::string rate,
                          uint32_t packetSize,
                          NodeContainer gws,
                          NodeContainer uts,
                          Time startTime,
                          Time stopTime,
                          Time startDelay);

// VoIP (cf Fractal Analysis and Modeling of VoIP Traffic)
// Pkt size = 210B TODO with header ?
// Rate = 64kb/s
// Distribution pareto (ATM it is constant -> TODO change)
// Burst time = 500ms (G.711.1)
// Idle time = 50ms (G.711.1)
  /**
   * Add a new Poisson traffic between chosen GWs and UTs
   * \param direction Direction of traffic
   * \param codec the Codec used
   * \param gws The Gateways
   * \param uts The User Terminals
   * \param startTime Application Start time
   * \param stopTime Application stop time
   * \param startDelay application start delay between each user
   */
  void AddVoipTraffic (TrafficDirection_t direction,
                       VoipCodec_t codec,
                       NodeContainer gws,
                       NodeContainer uts,
                       Time startTime,
                       Time stopTime,
                       Time startDelay);


  /**
   * Add a new CBR traffic between chosen GWs and UTs that can be customized
   * \param direction Direction of traffic
   * \param interval Initial wait time between transmission of two packets
   * \param packetSize Packet size in bytes
   * \param gws The Gateways
   * \param uts The User Terminals
   * \param startTime Application Start time
   * \param stopTime Application stop time
   * \param startDelay application start delay between each user
   */
  void AddCustomTraffic (TrafficDirection_t direction,
                         std::string interval,
                         uint32_t packetSize,
                         NodeContainer gws,
                         NodeContainer uts,
                         Time startTime,
                         Time stopTime,
                         Time startDelay);

  /**
   * Change the parameters of the last traffic created
   * \param time Delay after traffic launch to apply the changes
   * \param interval New wait time between transmission of two packets
   * \param packetSize New packet size in bytes
   */
  void ChangeCustomTraffic (Time delay,
                            std::string interval,
                            uint32_t packetSize);

//TODO same with add/remove nodes ?

//TODO check if correct if several GWs -> stats per GW

private:

  /**
   * \brief Struct for info on last custom trafic created
   */
  typedef struct
    {
      ApplicationContainer application;
      Time start;
      Time stop;
      bool created;
    } CustomTrafficInfo_s;

  Ptr<SatHelper> m_satHelper;	// Pointer to the SatHelper objet

  CustomTrafficInfo_s m_last_custom_application; // Last application container of custom traffic

  /**
   * TODO where I put this ?
   * Update the chosen attribute of a custom traffic
   * \param application The CBR application to update
   * \param interval The new interval
   * \param packetSize the new packet size
   */
  void UpdateAttribute (Ptr<CbrApplication> application, std::string interval, uint32_t packetSize);

  /**
   * \brief Check if node has a PacketSink installed at certain port.
   */
  bool HasSinkInstalled (Ptr<Node> node, uint16_t port);

};

} // namespace ns3

#endif /* __SATELLITE_TRAFFIC_HELPER_H__ */



// More generic
// call functons AddVoipTraffic, AddPoissonTraffic, etc.
// each one call a generic function (with for loops). Has a additionnal function -> subfunction, for traffic with parameters
// Subfunction (private) call GW*UT times