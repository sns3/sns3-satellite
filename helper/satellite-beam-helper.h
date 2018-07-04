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
#include "ns3/satellite-bstp-controller.h"
#include "ns3/satellite-phy-rx-carrier-conf.h"
#include "ns3/satellite-mobility-observer.h"
#include "ns3/satellite-markov-container.h"
#include "ns3/satellite-packet-trace.h"
#include "ns3/satellite-superframe-sequence.h"
#include "ns3/satellite-typedefs.h"
#include "satellite-geo-helper.h"
#include "satellite-gw-helper.h"
#include "satellite-ut-helper.h"


namespace ns3 {

/**
 * \brief SatBeamHelper builds a set Satellite beams with needed objects and configuration.
 *        It utilizes SatUtHelper, SatGwHelper and SatGeoHelper to create needed objects.
 *
 *        SatBeamHelper creates needed routes between nodes inside satellite network.
 *
 *
 */
class SatBeamHelper : public Object
{
public:

  /**
   * Define type CarrierFreqConverter
   */
  typedef SatChannel::CarrierFreqConverter CarrierFreqConverter;

  /**
   * Define type CarrierBandwidthConverter
   */
  typedef SatTypedefs::CarrierBandwidthConverter_t      CarrierBandwidthConverter;

  typedef std::map<std::pair<SatEnums::ChannelType_t,
                   uint32_t>, Ptr<SatChannel> >         ChannelContainer_t;
  typedef std::pair<uint32_t, uint32_t >                FrequencyPair_t;  // user = first, feeder = second
  typedef std::pair<uint32_t, uint32_t>                 GwLink_t;         // first GW ID, second feeder link frequency id

  typedef std::set<Ptr<Node> >                          MulticastBeamInfoItem_t;  // set container having receiving UT nodes in beam
  typedef std::map<uint32_t, std::set<Ptr<Node> > >     MulticastBeamInfo_t;      // key = beam ID, value = receiving UT nodes in beam

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

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
  SatBeamHelper (Ptr<Node> geoNode,
                 SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                 uint32_t fwdLinkCarrierCount,
                 uint32_t rtnLinkCarrierCount,
                 Ptr<SatSuperframeSeq> seq);

  /**
   * Destructor for SatBeamHelper.
   */
  virtual ~SatBeamHelper ()
  {
  }

  /**
   * \brief Init method is called after all the initial configurations
   * have been done by the SatHelper and SatBeamHelper.
   */
  void Init ();

  /**
   * Set the antenna gain patterns to be used when configuring the beams
   * to the GEO satellite. Note, that currently we have only one set of
   * antenna patterns, which are utilized in both user return (Rx gain)
   * and user forward (Tx gain) links. Antenna gain patterns are not utilized
   * in feeder link at all.
   *
   * \param antennaPatterns to antenna gain pattern container
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
  void SetBaseAddress (const Ipv4Address& network, const Ipv4Mask& mask, Ipv4Address base = "0.0.0.1");

  /**
   * \param ut a set of UT nodes
   * \param gwNode pointer of GW node
   * \param gwId id of the GW
   * \param beamId  id of the beam
   * \param rtnUlFreqId id of the return user link frequency
   * \param rtnFlFreqId id of the return feeder link frequency
   * \param fwdUlFreqId id of the forward user link frequency
   * \param fwdFlFreqId id of the forward feeder link frequency
   *
   * This method creates a beam  with the requested attributes
   * and associate the resulting ns3::NetDevices with the ns3::Nodes.
   * \return node GW node of the beam.
   */
  Ptr<Node> Install (NodeContainer ut,
                     Ptr<Node> gwNode,
                     uint32_t gwId,
                     uint32_t beamId,
                     uint32_t rtnUlFreqId,
                     uint32_t rtnFlFreqId,
                     uint32_t fwdUlFreqId,
                     uint32_t fwdFlFreqId);

  /**
   * \param beamId beam ID
   * \return the ID of the GW serving the specified beam, or zero if the ID is
   *         invalid
   */
  uint32_t GetGwId (uint32_t beamId) const;

  /**
   * \return container having all GW nodes in satellite network.
   */
  NodeContainer GetGwNodes () const;

  /**
   * \return container having all UT nodes in satellite network.
   */
  NodeContainer GetUtNodes () const;

  /**
   * \param beamId beam ID
   * \return container having all UT nodes of a specific beam.
   */
  NodeContainer GetUtNodes (uint32_t beamId) const;

  /**
   * \return a list of beam IDs which are currently activated.
   */
  std::list<uint32_t> GetBeams () const;

  /**
   * Enables creation traces to be written in given file
   * \param stream  stream for creation trace outputs
   * \param cb  callback to connect traces
   */
  void EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

  /**
   * \return info of created beams as std::string with GW info..
   */
  std::string GetBeamInfo () const;

  /**
   * \return Information of UTs.
   */
  std::string GetUtInfo () const;

  /**
   * Gets GW node according to given id.
   * In case that GW is not found NULL is returned.
   *
   * \param gwId ID of the GW
   * \return pointer to found GW node or NULL.
   */
  Ptr<Node> GetGwNode (uint32_t gwId) const;

  /**
   * Gets Geo Satellite node.
   *
   * \return pointer to Geo Satellite node.
   */
  Ptr<Node> GetGeoSatNode () const;

  /**
   * \return pointer to UT helper.
   */
  Ptr<SatUtHelper> GetUtHelper () const;

  /**
   * \return pointer to GW helper.
   */
  Ptr<SatGwHelper> GetGwHelper () const;

  /**
   * \return pointer to Geo helper.
   */
  Ptr<SatGeoHelper> GetGeoHelper () const;

  /**
   * \return pointer to the NCC.
   */
  Ptr<SatNcc> GetNcc () const;

  /**
   * Get beam Id of the given UT.
   *
   * \param utNode Pointer to UT node
   * return Id of the beam of the requested UT. O in case that given node is not UT node.
   */
  uint32_t GetUtBeamId (Ptr<Node> utNode) const;

  /**
   *
   * \param beamInfo Multicast info for the beams. Receiver UTs in a  beam for the multicast group.
   * \param sourceUtNode Source UT node. (NULL in case that source is behind gateway/backbone network)
   * \param sourceAddress Source address of the group.
   * \param groupAddress Address of the multicast group.
   * \param routeToGwUsers Flag indicating if there are GW receivers in public network behind IP router.
   * \param gwOutputDev Pointer to device which delivers multicast traffic to public/backbone network from satellite network.
   *                    In case that traffic is not needed to deliver to backbone network gwOutputDev is set to NULL.
   *                    (source is not UT or no receivers behind routing GW.)
   * \return Net device container including devices where multicast traffic shall be forwarded (GWs and users in public network) by IP router.
   */
  NetDeviceContainer AddMulticastGroupRoutes (MulticastBeamInfo_t beamInfo, Ptr<Node> sourceUtNode, Ipv4Address sourceAddress,
                                              Ipv4Address groupAddress, bool routeToGwUsers, Ptr<NetDevice>& gwOutputDev );

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * Enable packet traces.
   * (Called by SatHelper after scenario creation, if packet traces are enabled.)
   *
   */
  void EnablePacketTrace ();

private:
  CarrierFreqConverter m_carrierFreqConverter;
  SatTypedefs::CarrierBandwidthConverter_t m_carrierBandwidthConverter;

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
  std::map<uint32_t, FrequencyPair_t >      m_beamFreqs;   // first beam ID, channel frequency IDs pair

  ChannelContainer_t m_channels;

  /**
   * Trace callback for creation traces
   */
  TracedCallback<std::string> m_creationTrace;

  /**
   * Flag indicating whether to print detailed information to
   * the creation traces
   */
  bool m_printDetailedInformationToCreationTraces;

  /**
   * Control message store time in container for forward link.
   */
  Time m_ctrlMsgStoreTimeFwdLink;

  /**
   * Control message store in container for return link.
   */
  Time m_ctrlMsgStoreTimeRtnLink;

  /**
   * Configured fading model. Set as an attribute.
   */
  SatEnums::FadingModel_t m_fadingModel;

  /**
   * Common configuration for Markov model
   */
  Ptr<SatMarkovConf> m_markovConf;

  /**
   * Propagation delay model
   * - Constant
   * - Constant speed (speed of light)
   */
  SatEnums::PropagationDelayModel_t m_propagationDelayModel;

  /**
   * Constant propagation delay. Note, that this is valid
   * only if SatConstantPropagationDelay is used.
   */
  Time m_constantPropagationDelay;

  /**
   *  The used random access model
   */
  SatEnums::RandomAccessModel_t m_randomAccessModel;

  /**
   * The used interference model for random access
   */
  SatPhyRxCarrierConf::InterferenceModel m_raInterferenceModel;

  /**
   * The used collision model for random access
   */
  SatPhyRxCarrierConf::RandomAccessCollisionModel m_raCollisionModel;

  /**
   * Constant error rate for random access. Used if RA collision
   * model is RA_CONSTANT_COLLISION_PROBABILITY.
   */
  double m_raConstantErrorRate;

  /**
   * Flag indicating whether beam hopping is enabled in
   * FWD link. If enabled, SatBstpController is created with
   * proper callbacks and reuse 1 is configured for FWD link.
   */
  bool m_enableFwdLinkBeamHopping;

  /**
   * Beam Switching Time Plan controller, which is created
   * if FWD link beam hopping is enabled (m_enableFwdLinkBeamHopping).
   */
  Ptr<SatBstpController> m_bstpController;

  /**
   * Packet trace
   */
  Ptr<SatPacketTrace> m_packetTrace;

  /**
   * Creates info of the beam.
   * \return info for beams as std::string.
   */
  std::string CreateBeamInfo () const;

  /**
   * \brief Get channel of a certain channel type and frequency id. If it
   * is not found, the method creates and stores such for future purposes.
   * \param channelType Channel type, i.e. RTN user, RTN feeder, FWD user, FWD feeder
   * \param freqId Frequency id
   * \return Channel pointer
   */
  Ptr<SatChannel> GetChannel (SatEnums::ChannelType_t channelType, uint32_t freqId);

  /**
   * Creates GW node according to given id and stores GW to map.
   *
   * \param id ID of the GW
   * \param node pointer to the GW
   *
   * \return result of storing
   */
  bool StoreGwNode (uint32_t id, Ptr<Node> node);

  /**
   * Set needed routings of satellite network and fill ARC cache for the network.
   * \param ut    container having UTs of the beam
   * \param utNd  container having UT netdevices of the beam
   * \param gw    pointer to gateway node
   * \param gwNd  pointer to gateway netdevice
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
  Ptr<SatBaseFading>  InstallFadingContainer (Ptr<Node> node) const;

  /**
   * Add multicast route to UT node.
   *
   * \param utNode Pointer to UT node where to add route.
   * \param sourceAddress Source address of the multicast group.
   * \param groupAddress Address of the multicast group.
   * \param routeToSatellite Flag indicating if traffic is going toward satellite network.
   */
  void AddMulticastRouteToUt (Ptr<Node> utNode, Ipv4Address sourceAddress, Ipv4Address groupAddress, bool routeToSatellite);

};

} // namespace ns3

#endif /* SATELLITE_BEAM_HELPER_H */
