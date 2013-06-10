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
#include "ns3/deprecated.h"
#include "ns3/trace-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/satellite-arp-cache.h"

#include "satellite-geo-helper.h"
#include "satellite-gw-helper.h"
#include "satellite-ut-helper.h"


namespace ns3 {

class NetDevice;
class Node;

/**
 * \brief Build a set Satellite beams with needed objects and configuration
 *
 */
class SatBeamHelper : public Object
{
public:
  typedef std::pair<Ptr<SatChannel>, Ptr<SatChannel> > SatLink;

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
  Ptr<Node> Install (NodeContainer ut, uint16_t gwId, uint16_t beamId, uint16_t ulFreqId, uint16_t flFreqId );

  /**
   * returns A container having all GW nodes in satellite network.
   */
  NodeContainer GetGwNodes();

  void SetArpCacheForGws();

  /**
   * Enables creation traces to be written in given file
   * /param stream  stream for creation trace outputs
   * /param cb  callback to connect traces
   */
  void EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

private:

    ObjectFactory     m_channelFactory;
    SatGeoHelper      m_geoHelper;
    SatGwHelper       m_gwHelper;
    SatUtHelper       m_utHelper;
    Ipv4AddressHelper m_ipv4Helper;
    Ptr<SatArpCache>  m_gwArpCache;
    NodeContainer     m_gwNodeList;
    Ptr<Node> m_geoNode;
    std::set<uint16_t> m_beam;
    std::set<std::pair<uint16_t, uint16_t> > m_gwLink;
    std::map<uint16_t, Ptr<Node> > m_gwNode;
    std::map<uint16_t, SatLink > m_ulChannels;  //first forward and second return
    std::map<uint16_t, SatLink > m_flChannels;  //first forward and second return

    /**
     * Trace callback for creation traces
     */
    TracedCallback<std::string> m_creation;
};

} // namespace ns3

#endif /* SAT_BEAM_HELPER_H */
