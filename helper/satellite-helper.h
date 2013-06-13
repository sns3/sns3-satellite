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

       *  - Simple: Simple scenario used as base.
       *  - Larger: Larger scenario used as base.
       *  - Full:   Full scenario used as base.
       */
    enum PREDEFINED_SCENARIO
    {
      None,
      Simple,
      Larger,
      Full
    };

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Create a base SatHelper for creating customized Satellite topologies.
   */
  SatHelper ();

  /**
   * \brief Create a pre-defined SatHelper to make life easier when creating Satellite topologies.
   */
  void CreateScenario (PREDEFINED_SCENARIO scenario);
  virtual ~SatHelper () {}

  /**
   * \param  node pointer to user node.
   *
   * Returns address of the user.
   */
  Ipv4Address GetUserAddress(Ptr<Node> node);

  /**
   * Returns container having UT users
   */
  NodeContainer  GetUtUsers();

  /**
   * Returns container having GW users.
   */
  NodeContainer  GetGwUsers();

  /**
   * Enables creation traces to be written in given file
   * /param filename  name to the file for trace writing
   * /param details true means that lower layer details are printed also,
   *                false means that only creation summary is printed
   */
  void EnableCreationTraces(std::string filename, bool details);

private:

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
   * Creates trace summary starting with give title.
   * /param title title for summary
   * /returns std::string as summary
   */
  std::string CreateCreationSummary(std::string title);

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
   * flag to check if sceanrio is alreay created.
   */
  bool m_scenarioCreated;

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

