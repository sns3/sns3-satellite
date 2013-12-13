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
#ifndef __SATELLITE_HELPER_H__
#define __SATELLITE_HELPER_H__

#include <string>

#include "ns3/object.h"
#include "ns3/trace-helper.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/csma-helper.h"
#include "ns3/satellite-antenna-gain-pattern-container.h"
#include "satellite-user-helper.h"
#include "satellite-beam-helper.h"
#include "satellite-beam-user-info.h"
#include "satellite-conf.h"
#include "ns3/satellite-rx-power-input-trace-container.h"
#include "ns3/satellite-rx-power-output-trace-container.h"
#include "ns3/satellite-interference-input-trace-container.h"
#include "ns3/satellite-interference-output-trace-container.h"
#include "ns3/satellite-fading-output-trace-container.h"
#include "ns3/satellite-fading-input-trace-container.h"

namespace ns3 {

/**
 * \brief Build a satellite network set with needed objects and configuration
 *
 */
class SatHelper : public Object
{
public:

  /**
   * definition for beam map key and value.
   */
  typedef std::map<uint32_t, SatBeamUserInfo > BeamUserInfoMap_t;

  /**
   * \enum values for pre-defined scenarios to be used by helper when building
             satellite network topology base.
   *
   *  - NONE:         Not used.
   *  - SIMPLE:       Simple scenario used as base.
   *  - LARGER:       Larger scenario used as base.
   *  - FULL:         Full scenario used as base.
   *  - USER_DEFINED: User defined scenario as base.
   */
  typedef enum
  {
    NONE,
    SIMPLE,
    LARGER,
    FULL,
    USER_DEFINED
  } PreDefinedScenario_t;

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Default constructor
   */
  SatHelper ();

  /**
   * \brief Create a base SatHelper for creating customized Satellite topologies.
   */
  SatHelper (std::string scenarioName);

  /**
   * \brief Create a pre-defined SatHelper to make life easier when creating Satellite topologies.
   */
  void CreateScenario (PreDefinedScenario_t scenario);
  virtual ~SatHelper () {}

  /**
   * Sets beam info for sceanrio creation.
   * Only in user defined scenario beam Ids have effect to number of beams to be creates.
   * In other scenarios only UT count and their user count have meaning. So for this default
   * count can be overidden.
   *
   * \param info information of the users in beams (for user defined scenario defines beams to create)
   */
  void SetBeamUserInfo (BeamUserInfoMap_t info);

  /**
   * Sets beam info for sceanrio creation.
   * Only in user defined scenario beamId  have effect to number of beams to be creates.
   * In other scenarios only UT count and their user count have meaning. So for this default
   * count can be overidden.
   *
   * \param beamId id of the beam which info is set (for user defined scenario defines beam to create)
   * \param info information of the user in beam
   */
  void SetBeamUserInfo (uint32_t beamId, SatBeamUserInfo info);

  /**
   * \param  node pointer to user node.
   *
   * \return address of the user.
   */
  Ipv4Address GetUserAddress (Ptr<Node> node);

  /**
   * \return container having UT users
   */
  NodeContainer  GetUtUsers ();

  /**
   * \return container having GW users.
   */
  NodeContainer  GetGwUsers ();

  /**
   * \return pointer to beam helper.
   */
  Ptr<SatBeamHelper>  GetBeamHelper () { return m_beamHelper;}

  /**
   * Enables creation traces to be written in given file
   * \param filename  name to the file for trace writing
   * \param details true means that lower layer details are printed also,
   *                false means that only creation summary is printed
   */
  void EnableCreationTraces(std::string filename, bool details);

  inline NodeContainer GwNodes () { return m_beamHelper->GetGwNodes(); }
  inline NodeContainer UtNodes () { return m_beamHelper->GetUtNodes(); }
  inline Ptr<Node> GeoSatNode () { return m_beamHelper->GetGeoSatNode(); }

  void DoDispose();

private:
  /**
   * Enables creation traces in sub-helpers.
   */
  void EnableDetailedCreationTraces ();
  /**
   * Sink for creation details traces
   * \param stream stream for traces
   * \param context context for traces
   * \param info creation info
   */
  static void CreationDetailsSink (Ptr<OutputStreamWrapper> stream, std::string context, std::string info);
  /**
   * Sink for creation summary traces
   * \param title creation summary title
   */
  void CreationSummarySink (std::string title);
  /**
   * Creates satellite objects according to simple scenario.
   */
  void CreateSimpleScenario ();
  /**
   * Creates satellite objects according to larger scenario.
   */
  void CreateLargerScenario ();
  /**
   * Creates satellite objects according to full scenario.
   */
  void CreateFullScenario ();
  /**
   * Creates satellite objects according to user defined scenario.
   * Beams to create with number of the UTs are set by method SetBeamUserInfo.
   */
  void CreateUserDefinedScenario ();

  /**
   * Creates satellite objects according to given beam info.
   * \param beamInfos information of the beam to create (and beams which are given in map)
   * \param gwUsers number of the users in GW(s) side
   */
  void DoCreateScenario (BeamUserInfoMap_t beamInfos, uint32_t gwUsers);

  /**
   * Creates trace summary starting with give title.
   * \param title title for summary
   * \returns std::string as summary
   */
  std::string CreateCreationSummary(std::string title);

  /**
   * Sets mobilities to created GW nodes.
   *
   * \param gws node container of UTs to set mobility
   */
  void SetGwMobility (NodeContainer gws);

  /**
   * Sets mobility to created Sat Geo node.
   *
   * \param node node pointer of Geo Satellite to set mobility
   */
  void SetGeoSatMobility(Ptr<Node> node);

  /**
   * Sets mobility to created UT nodes.
   *
   * \param uts node container of UTs to set mobility
   * \param beamId the spot-beam id, where the UTs should be placed
   *
   */
  void SetUtMobility (NodeContainer uts, uint32_t beamId);

  /**
   * Install Satellite Mobility Observer to nodes, if observer doesn't exist already in a node
   *
   * \param nodes Nodecontainer of nodes to install mobility observer.
   */
  void  InstallMobilityObserver (NodeContainer nodes) const;

  /**
   * User helper
   */
  Ptr<SatUserHelper> m_userHelper;

  /**
   * Beam helper
   */
  Ptr<SatBeamHelper> m_beamHelper;

  /**
   * Gateway container
   */
  NodeContainer m_gwUser;

  /**
   * Configuration for satellite network.
   */
  Ptr<SatConf> m_satConf;

  /**
   * Trace callback for creation traces (details)
   */
  TracedCallback<std::string> m_creation;

  /**
   * Trace callback for creation traces (summary)
   */

  TracedCallback<std::string> m_creationSummary;

  /**
   * Stream wrapper used for creation traces
   */
  Ptr<OutputStreamWrapper> m_creationTraceStream;

  /**
   * Stream wrapper used for UT position traces
   */
  Ptr<OutputStreamWrapper>m_utPosTraceStream;

  /**
   * flag to check if scenario is alreay created.
   */
  bool m_scenarioCreated;

  /**
   * flag to indicate if detailed trace should be enables before scenario creation.
   */
  bool m_detailedCreationTraces;

  /**
   * Number of UTs created per Beam in full or user-defined scenario
   */
  uint32_t m_utsInBeam;

  /**
   * Number of users created in public network (behind GWs) in full or user-defined scenario
   */
  uint32_t m_gwUsers;

  /**
   * Number of users created in end user network (behind every UT) in full or user-defined scenario
   */
  uint32_t m_utUsers;

  /**
   * Info for beam creation in user defined scenario.
   * first is ID of the beam and second is number of beam created in beam.
   * If second is zero then default number of UTs is created (number set by attribute UtCount)
   *
   * Info is set by attribute BeamInfo
   */
  BeamUserInfoMap_t m_beamUserInfos;

  /**
   * Antenna gain patterns for all spot-beams. Used for beam selection.
   */
  Ptr<SatAntennaGainPatternContainer> m_antennaGainPatterns;

};

} // namespace ns3


#endif /* __SATELLITE_HELPER_H__ */

