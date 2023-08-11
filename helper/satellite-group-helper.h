/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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

#ifndef SATELLITE_GROUP_HELPER_H
#define SATELLITE_GROUP_HELPER_H

#include <ns3/geo-coordinate.h>
#include <ns3/mobility-helper.h>
#include <ns3/node-container.h>
#include <ns3/satellite-mobility-model.h>
#include <ns3/satellite-position-allocator.h>
#include <ns3/satellite-ut-handover-module.h>
#include <ns3/vector.h>

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <vector>

namespace ns3
{

class SatHelper;

/**
 * \brief This helper is used to create groups of UTs.
 */
class SatGroupHelper : public Object
{
  public:
    /**
     * \brief Get the type ID
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Get the type ID of instance
     * \return the object TypeId
     */
    virtual TypeId GetInstanceTypeId(void) const;

    /**
     * Default constructor for SatGroupHelper
     */
    SatGroupHelper();

    /**
     * Destructor for SatGroupHelper.
     */
    virtual ~SatGroupHelper()
    {
    }

    /**
     * \brief Initialize the helper. Should be call only by SatHelper internally
     * \param uts The list of all UTs in the simulation
     */
    void Init(NodeContainer uts);

    /**
     * \brief Add a node to a group
     * \param groupId The group ID where the node is added
     * \param node The node to add
     */
    void AddUtNodeToGroup(uint32_t groupId, Ptr<Node> node);

    /**
     * \brief Add several nodes to a group
     * \param groupId The group ID where the nodes are added
     * \param nodes The nodes to add
     */
    void AddUtNodesToGroup(uint32_t groupId, NodeContainer nodes);

    /**
     * \brief Create a new group using a central position and a radius
     * \param groupId The ID of created group. Cannot be an already existing group
     * \param nodes The input nodes, used to determine if each one belong to the group or not
     * \param center The center of the circle
     * \param radius The radius of the circle in meters
     */
    void CreateGroupFromPosition(uint32_t groupId,
                                 NodeContainer nodes,
                                 GeoCoordinate center,
                                 uint32_t radius);

    /**
     * \brief Create several groups and distribute nodes among them
     * \param groupIds The list of group IDs to create. They all must be empty
     * \param nodes The nodes to distribute in the groups
     */
    void CreateGroupsUniformly(std::vector<uint32_t> groupIds, NodeContainer nodes);

    /**
     * \brief Create a new group using a central position and a radius, and create UT nodes inside
     * this area \param groupId The ID of created group. Cannot be an already existing group \param
     * nb The number of nodes to create in this circle \param center The center of the circle \param
     * radius The radius of the circle in meters
     */
    void CreateUtNodesFromPosition(uint32_t groupId,
                                   uint32_t nb,
                                   GeoCoordinate center,
                                   uint32_t radius);

    /**
     * \brief Schedule a node to be added to a group when scenario creation is finished.
     * This should not be used in a user scenario, only in intern calls.
     * \param groupId The ID of created group. Cannot be an already existing group
     * \param node The associated node
     */
    void AddNodeToGroupAfterScenarioCreation(uint32_t groupId, Ptr<Node> node);

    /**
     * \brief Get the position of nodes to add to the scenario
     * \return The vector of points and group IDs
     */
    std::vector<std::pair<GeoCoordinate, uint32_t>> GetAdditionalNodesPerBeam();

    /**
     * \param groupId The group ID
     * \return container having all UT nodes associated to a group
     */
    NodeContainer GetUtNodes(uint32_t groupId) const;

    /**
     * \brief Count the number of groups created
     * \return The number of groups
     */
    uint32_t GetN();

    /**
     * \brief Get the list of groups created
     * \return The list of groups created
     */
    std::list<uint32_t> GetGroups();

    /**
     * Dispose of this class instance
     */
    virtual void DoDispose();

    void SetSatConstellationEnabled();

  private:
    /**
     * Tells if the groupId is already existing in the database
     * \param groupId The group to test
     * \return true if the group already exists
     */
    bool IsGroupExisting(uint32_t groupId) const;

    /**
     * Get the group to which a node belongs
     * \param node The node to analyse
     * \return The groupID where the node is registered. Return 0 if no group is found
     */
    uint32_t GetGroupId(Ptr<Node> node) const;

    /**
     * \brief Get list of nodes not created from position by group helper
     * \param nodes The list of nodes to filter
     * \return The list of nodes not created from position by group helper
     */
    NodeContainer GetNodesNotAddedFromPosition(NodeContainer nodes);

    /**
     * The list of all the UTs in the simulation
     */
    NodeContainer m_uts;

    /**
     * Container to associate nodes to the groups
     */
    std::map<uint32_t, std::set<Ptr<Node>>> m_groupsMap;

    /**
     * List of group ID created
     */
    std::list<uint32_t> m_groupsList;

    /**
     * Nodes created by position to add to scenario. vector: <position, group ID>
     */
    std::vector<std::pair<GeoCoordinate, uint32_t>> m_additionalNodesPerBeam;

    /*
     * Map to store node to add to groups after scenario is created
     */
    std::map<Ptr<Node>, uint32_t> m_nodesToAdd;

    /*
     * Tells if the scenario has been created by simulation helper.
     * Some methods have to be called before, some after.
     */
    bool m_scenarioCreated;

    /**
     * Use a constellation of satellites
     */
    bool m_satConstellationEnabled;
};

} // namespace ns3

#endif /* SATELLITE_GROUP_HELPER_H */
