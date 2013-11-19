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

#ifndef SATELLITE_BEAM_HELPER_H
#define SATELLITE_BEAM_HELPER_H

#include <string>
#include <set>
#include <map>
#include <stdint.h>

#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"

#include "ns3/satellite-ncc.h"
#include "ns3/satellite-antenna-gain-pattern-container.h"
#include "ns3/satellite-phy-rx-carrier-conf.h"
#include "ns3/satellite-mobility-observer.h"
#include "ns3/satellite-markov-container.h"
#include "satellite-geo-helper.h"
#include "satellite-gw-helper.h"
#include "satellite-ut-helper.h"
#include "satellite-superframe-sequence.h"


namespace ns3 {

/**
 * \brief Build a set Satellite beams with needed objects and configuration
 *
 */
class SatBeamHelper : public Object
{
public:
  typedef SatChannel::CarrierFreqConverter CarrierFreqConverter;
  typedef SatPhyRxCarrierConf::CarrierBandwidthConverter CarrierBandwidthConverter;

  typedef std::pair<Ptr<SatChannel>, Ptr<SatChannel> >  ChannelPair_t;    //forward = first, return  = second
  typedef std::pair<uint32_t, uint32_t >                FrequencyPair_t;  // user = first, feeder = second
  typedef std::pair<uint32_t, uint32_t>                 GwLink_t;         // first GW ID, second feeder link frequency id

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Used fading model
   */
  enum FadingModel
  {
    FADING_OFF, FADING_MARKOV
  };

  /**
   * Default constructor for SatBeamHelper (should not be used).
   */
  SatBeamHelper ();


  /**
   * Constructor for SatBeamHelper.
   *
   * \param geoNode               Pointer to Geo Satellite node
   * \param bandwidthConverterCb  Callback to convert bandwidth
   * \param fwdLinkCarrierCount   Number of carriers used in forward link
   * \param rtnLinkCarrierCount   Number of carriers used in return link
   * \param seq                   Pointer to used superframe sequence configuration (containing superframe configurations).
   */
  SatBeamHelper (Ptr<Node> geoNode, CarrierBandwidthConverter bandwidthConverterCb,
                 uint32_t fwdLinkCarrierCount, uint32_t rtnLinkCarrierCount, Ptr<SatSuperframeSeq> seq);

  /**
   * Destructor for SatBeamHelper.
   */
  virtual ~SatBeamHelper () {}

  /**
   * Set the antenna gain patterns to be used when configuring the beams
   * to the GEO satellite. Note, that currently we have only one set of
   * antenna patterns, which are utilized in both user return (Rx gain)
   * and user forward (Tx gain) links. Antenna gain patterns are not utilized
   * in feeder link at all.
   *
   * \param pointer to antenna gain pattern container
   */
  void SetAntennaGainPatterns (Ptr<SatAntennaGainPatternContainer> antennaPatterns);

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
   * by SatBeamHelper::Install
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
  */
  void SetBaseAddress (const Ipv4Address network, const Ipv4Mask mask, Ipv4Address base = "0.0.0.1");

  /**
   * \param ut a set of UT nodes
   * \param node pointer of GW node
   * \param gwId id of the GW
   * \param beamId  id of the beam
   * \param ulFreqId id of the user link frequency
   * \param flFreqId id of the feeder link frequency
   *
   * This method creates a beam  with the requested attributes
   * and associate the resulting ns3::NetDevices with the ns3::Nodes.
   * \return node GW node of the beam.
   */
  Ptr<Node> Install (NodeContainer ut, Ptr<Node> gwNode, uint32_t gwId, uint32_t beamId, uint32_t ulFreqId, uint32_t flFreqId );

  /**
   * /return A container having all GW nodes in satellite network.
   */
  NodeContainer GetGwNodes ();

  /**
   * /return A container having all UT nodes in satellite network.
   */
  NodeContainer GetUtNodes ();

  /**
   * Enables creation traces to be written in given file
   * \param stream  stream for creation trace outputs
   * \param cb  callback to connect traces
   */
  void EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

  /**
   * /return info of created beams as std::string.
   */
  std::string GetBeamInfo ();

  /**
   * \param printMacAddress flag to indicated, if mac addresses of the UTs is wanted to print.
   *
   * /return info of UT positions
   */
  std::string GetUtPositionInfo (bool printMacAddress);

  /**
   * Gets GW node according to given id.
   * In case that GW is not found NULL is returned.
   *
   * \param id ID of the GW
   * /return pointer to found GW node or NULL.
   */
  Ptr<Node> GetGwNode (uint32_t id);

  /**
   * Gets Geo Satellite node.
   *
   * /return pointer to Geo Satellite node.
   */
  Ptr<Node> GetGeoSatNode ();

  /**
   * \return pointer to UT helper.
   */
  Ptr<SatUtHelper>  GetUtHelper () { return m_utHelper;}

  /**
   * \return pointer to GW helper.
   */
  Ptr<SatGwHelper>  GetGwHelper () { return m_gwHelper;}

  /**
   * \return pointer to UT helper.
   */
  Ptr<SatGeoHelper>  GetGeoHelper () { return m_geoHelper;}

  virtual void DoDispose ();

private:
  CarrierFreqConverter m_carrierFreqConverter;
  Ptr<SatSuperframeSeq> m_superframeSeq;

  ObjectFactory         m_channelFactory;
  Ptr<SatGeoHelper>     m_geoHelper;
  Ptr<SatGwHelper>      m_gwHelper;
  Ptr<SatUtHelper>      m_utHelper;
  Ipv4AddressHelper     m_ipv4Helper;
  Ptr<Node>             m_geoNode;
  Ptr<SatNcc>           m_ncc;

  Ptr<SatAntennaGainPatternContainer>   m_antennaGainPatterns;

  std::map<uint32_t, uint32_t >             m_beam;        // first beam ID, second GW ID
  std::set<GwLink_t >                       m_gwLinks;     // gateway links (GW id and feeder frequency id pairs).
  std::map<uint32_t, Ptr<Node> >            m_gwNode;      // first GW ID, second node pointer
  std::multimap<uint32_t, Ptr<Node> >       m_utNode;      // first Beam ID, second node pointer of the UT
  std::map<uint32_t, ChannelPair_t >        m_ulChannels;  // user link ID, channel pointers pair
  std::map<uint32_t, ChannelPair_t >        m_flChannels;  // feeder link ID, channel pointers pair
  std::map<uint32_t, FrequencyPair_t >      m_beamFreqs;   // first beam ID, channel frequency IDs pair

  /**
   * Trace callback for creation traces
   */
  TracedCallback<std::string> m_creation;

  /**
   * Configured fading model. Set as an attribute.
   */
  SatBeamHelper::FadingModel m_fadingModel;

  /**
   * Common configuration for Markov model
   */
  Ptr<SatMarkovConf> m_markovConf;

  /**
   * Creates info of the beam.
   * /returns info for beams as std::string.
   */
  std::string CreateBeamInfo ();

  /**
   * Gets satellite channel pair from requested map.
   * In case that channel pair is not found, new is created and returned.
   * \param chPairMap map where channel pair is get
   * \param frequencyId ID of the frequency
   * \param isUserLink flag indicating if link user link is requested (otherwise feeder link).
   */
  ChannelPair_t GetChannelPair (std::map<uint32_t, ChannelPair_t >& chPairMap, uint32_t frequencyId, bool isUserLink);

  /**
   * Creates GW node according to given id and stores GW to map.
   *
   * \param id ID of the GW
   * \param node pointer to the GW
   *
   * /return result of storing
   */
  bool StoreGwNode (uint32_t id, Ptr<Node> node);

  /**
   * Set needed routings of satellite network and fill ARC cache for the network.
   * \param ut    container having UTs of the beam
   * \param utNd  container having UT netdevices of the beam
   * \param gw    pointer to gateway node
   * \param gw    pointer to gateway netdevice
   * \param gwAddr address of the gateway
   * \param utIfs container having UT ipv2 interfaces (for addresses)
   */
  void PopulateRoutings (NodeContainer ut, NetDeviceContainer utNd, Ptr<Node> gw,
                         Ptr<NetDevice> gwNd, Ipv4Address gwAddr, Ipv4InterfaceContainer utIfs);

  /**
   * Install fading model to node, if fading model doesn't exist already in node
   *
   * \param node Pointer to node
   * \return Pointer to fading container
   */
  Ptr<SatFading>  InstallFadingContainer (Ptr<Node> node) const;

};

} // namespace ns3

#endif /* SATELLITE_BEAM_HELPER_H */
