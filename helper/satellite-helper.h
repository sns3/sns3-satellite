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
#include "satellite-user-helper.h"
#include "satellite-beam-helper.h"
#include "satellite-beam-user-info.h"
#include "satellite-conf.h"

namespace ns3 {

/**
 * \brief Build a satellite network set with needed objects and configuration
 *
 */
class SatHelper : public Object
{
public:
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
    }PreDefinedScenario;

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Create a base SatHelper for creating customized Satellite topologies.
   */
  SatHelper ();

  /**
   * \brief Create a pre-defined SatHelper to make life easier when creating Satellite topologies.
   */
  void CreateScenario (PreDefinedScenario scenario);
  virtual ~SatHelper () {}

  /**
   * Sets beam info for sceanrio creation.
   * Only in user defined scenario beam Ids have effect to number of beams to be creates.
   * In other scenarios only UT count and their user count have meaning. So for this default
   * count can be overidden.
   *
   * \param info information of the users in beams (for user defined scenario defines beams to create)
   */
  void SetBeamUserInfo(std::map<uint32_t, SatBeamUserInfo> info);

  /**
   * Sets beam info for sceanrio creation.
   * Only in user defined scenario beamId  have effect to number of beams to be creates.
   * In other scenarios only UT count and their user count have meaning. So for this default
   * count can be overidden.
   *
   * \param beamId id of the beam which info is set (for user defined scenario defines beam to create)
   * \param info information of the user in beam
   */
  void SetBeamUserInfo(uint32_t beamId, SatBeamUserInfo info);

  /**
   * \param  node pointer to user node.
   *
   * \return address of the user.
   */
  Ipv4Address GetUserAddress(Ptr<Node> node);

  /**
   * \return container having UT users
   */
  NodeContainer  GetUtUsers();

  /**
   * \return container having GW users.
   */
  NodeContainer  GetGwUsers();

  /**
   * Enables creation traces to be written in given file
   * /param filename  name to the file for trace writing
   * /param details true means that lower layer details are printed also,
   *                false means that only creation summary is printed
   */
  void EnableCreationTraces(std::string filename, bool details);

  NodeContainer GwNodes() {return m_beamHelper->GetGwNodes();}
  NodeContainer UtNodes() {return m_beamHelper->GetUtNodes();}
  Ptr<Node> GeoSatNode() {return m_beamHelper->GetGeoSatNode();}

private:
  /**
     * definition for beam map key and value.
     */
  typedef std::map<uint32_t, SatBeamUserInfo > BeamMap;

  /**
   * Enables creation traces in sub-helpers.
   */
  void EnableDetailedCreationTraces();
  /**
   * Sink for creation details traces
   * /param stream stream for traces
   * /param context context for traces
   * /param info creation info
   */
  static void CreationDetailsSink (Ptr<OutputStreamWrapper> stream, std::string context, std::string info);
  /**
   * Sink for creation summary traces
   * /param title creation summary title
   */
  void CreationSummarySink (std::string title);
  /**
   * Creates satellite objects according to simple scenario.
   */
  void CreateSimpleScenario();
  /**
   * Creates satellite objects according to larger scenario.
   */
  void CreateLargerScenario();
  /**
   * Creates satellite objects according to full scenario.
   */
  void CreateFullScenario();
  /**
   * Creates satellite objects according to user defined scenario.
   * Beams to create with number of the UTs are set by method SetBeamUserInfo.
   */
  void CreateUserDefinedScenario();

  /**
   * Creates satellite objects according to given beam info.
   * /param beamInfo information of the beam to create (and beams which are given in map)
   * /param gwUsers number of the users in GW(s) side
   */
  void CreateScenario(BeamMap beamInfo, uint32_t gwUsers);

  /**
   * Creates trace summary starting with give title.
   * /param title title for summary
   * /returns std::string as summary
   */
  std::string CreateCreationSummary(std::string title);

  /**
   * Sets mobilities to created GW nodes.
   *
   * /param gws node container of UTs to set mobility
   */
  void SetGwMobility(NodeContainer gws);

  /**
   * Sets mobility to created Sat Geo node.
   *
   * /param geoSat node pointer of Geo Satellite to set mobility
   */
  void SetGeoSatMobility(Ptr<Node> geoSat);

  /**
   * Sets mobility to created UT nodes.
   *
   * /param uts node container of UTs to set mobility
   *
   */
  void SetUtMobility(NodeContainer uts);

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
  SatConf m_satConf;

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
  BeamMap m_beamInfo;

  /**
   * Default value for variable m_utsInBeam
   */
  static const uint32_t DEFAULT_UTS_IN_BEAM = 3;

  /**
   * Minimum value for variable m_utsInBeam
   */
  static const uint32_t MIN_UTS_IN_BEAM = 1;

  /**
   * Default value for variable m_gwUsers
   */
  static const uint32_t DEFAULT_GW_USERS = 5;

  /**
   * Minimum value for variable m_gwUsers
   */
  static const uint32_t MIN_GW_USERS = 1;
  /**
   * Default value for variable m_utUsers
   */
  static const uint32_t DEFAULT_UT_USERS = 3;

  /**
   * Minimum value for variable m_utUsers
   */
  static const uint32_t MIN_UT_USERS = 1;

};

} // namespace ns3


#endif /* __SATELLITE_HELPER_H__ */

