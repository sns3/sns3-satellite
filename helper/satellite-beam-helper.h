/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SAT_BEAM_HELPER_H
#define SAT_BEAM_HELPER_H

#include <string>
#include <set>
#include <map>
#include <stdint.h>

#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"

#include "satellite-geo-helper.h"
#include "satellite-gw-helper.h"
#include "satellite-ut-helper.h"


namespace ns3 {

/**
 * \brief Build a set Satellite beams with needed objects and configuration
 *
 */
class SatBeamHelper : public Object
{
public:
  typedef std::pair<Ptr<SatChannel>, Ptr<SatChannel> >  ChannelPair;    //forward = first, return  = second
  typedef std::pair<uint32_t, uint32_t >                FrequencyPair;  // user = first, feeder = second
  typedef std::pair<uint32_t, uint32_t>                 GwLink;         // first GW ID, second feeder link frequency id

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  /**
   * Create a SatBeamHelper to make life easier when creating Satellite beams.
   */
  SatBeamHelper ();
  virtual ~SatBeamHelper () {}

  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatNetDevice created
   * by SatBeamHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Channel created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::SatChannel created
   * by SatNetDevHelper::Install
   */
  void SetChannelAttribute (std::string name, const AttributeValue &value);

  /**
  * \param network The Ipv4Address containing the initial network number to
  * use for satellite network allocation. The bits outside the network mask are not used.
  * \param mask The Ipv4Mask containing one bits in each bit position of the
  * network number.
  * \param base An optional Ipv4Address containing the initial address used for
  * IP address allocation.  Will be combined (ORed) with the network number to
  * generate the first IP address.  Defaults to 0.0.0.1.
  * \returns Nothing.
  */
  void SetBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

  /**
   * \param ut a set of UT nodes
   * \param gwId id of the GW
   * \param beamId  id of the beam
   * \param ulFreqId id of the user link frequency
   * \param flFreqId id of the feeder link frequency
   *
   * This method creates a beam  with the requested attributes
   * and associate the resulting ns3::NetDevices with the ns3::Nodes.
   * \returns node GW node of the beam.
   */
  Ptr<Node> Install (NodeContainer ut, uint32_t gwId, uint32_t beamId, uint32_t ulFreqId, uint32_t flFreqId );

  /**
   * returns A container having all GW nodes in satellite network.
   */
  NodeContainer GetGwNodes();

  /**
   * Enables creation traces to be written in given file
   * /param stream  stream for creation trace outputs
   * /param cb  callback to connect traces
   */
  void EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

  /**
   * /returns info of created beams as std::string.
   */
  std::string GetBeamInfo();

private:

    ObjectFactory         m_channelFactory;
    Ptr<SatGeoHelper>     m_geoHelper;
    Ptr<SatGwHelper>      m_gwHelper;
    Ptr<SatUtHelper>      m_utHelper;
    Ipv4AddressHelper     m_ipv4Helper;
    NodeContainer         m_gwNodeList;
    Ptr<Node>             m_geoNode;

    std::map<uint32_t, uint32_t >             m_beam;        // first beam ID, second GW ID
    std::set<GwLink >                         m_gwLinks;     // gateway links (GW id and feeder frequency id pairs).
    std::map<uint32_t, Ptr<Node> >            m_gwNode;      // first GW ID, second node pointer
    std::map<uint32_t, ChannelPair >          m_ulChannels;  // user link ID, channel pointers pair
    std::map<uint32_t, ChannelPair >          m_flChannels;  // feeder link ID, channel pointers pair
    std::map<uint32_t, FrequencyPair >        m_beamFreqs;   // first beam ID, channel frequency IDs pair

    /**
     * Trace callback for creation traces
     */
    TracedCallback<std::string> m_creation;

    /**
     * Creates info of the beam.
     * /returns info for beams as std::string.
     */
    std::string CreateBeamInfo();

    /**
     * Gets satellite channel pair from requested map.
     * In case that channel pair is not found, new is created and returned.
     * /param chPairMap map where channel pair is get
     * /param frequencyId ID of the frequency
     */
    ChannelPair GetChannelPair(std::map<uint32_t, ChannelPair > chPairMap, uint32_t frequencyId);

    /**
     * Gets GW node according to given id from map where GWs are stored.
     * In case that GW is not found, new node is created and returned.
     * /param id ID of the GW
     */
    Ptr<Node> GetGw(uint32_t id);

    /**
     * Set needed routings of satellite network and fill ARC cache for the network.
     * /param ut    container having UTs of the beam
     * /param utNd  container having UT netdevices of the beam
     * /param gw    pointer to gateway node
     * /param gw    pointer to gateway netdevice
     * /param gwAddr address of the gateway
     * /param utIfs container having UT ipv2 interfaces (for addresses)
     */
    void PopulateRoutings(NodeContainer ut, NetDeviceContainer utNd,
                           Ptr<Node> gw, Ptr<NetDevice> gwNd, Ipv4Address gwAddr, Ipv4InterfaceContainer utIfs);

};

} // namespace ns3

#endif /* SAT_BEAM_HELPER_H */
