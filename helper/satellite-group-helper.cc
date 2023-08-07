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

#include <ns3/log.h>
#include <ns3/singleton.h>
#include <ns3/satellite-id-mapper.h>

#include "satellite-group-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGroupHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGroupHelper);

TypeId
SatGroupHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGroupHelper")
    .SetParent<Object> ()
    .AddConstructor<SatGroupHelper> ()
  ;
  return tid;
}

TypeId
SatGroupHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGroupHelper::SatGroupHelper ()
  : m_scenarioCreated (false),
  m_satConstellationEnabled (false)
{
  NS_LOG_FUNCTION (this);
}

void
SatGroupHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_groupsMap.clear ();
}

void
SatGroupHelper::Init (NodeContainer uts)
{
  NS_LOG_FUNCTION (this);
  m_uts = uts;

  m_scenarioCreated = true;

  for (std::map<Ptr<Node>, uint32_t>::iterator it = m_nodesToAdd.begin(); it != m_nodesToAdd.end(); it++)
    {
      AddUtNodeToGroup (it->second, it->first);
    }
}

void
SatGroupHelper::AddUtNodeToGroup (uint32_t groupId, Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << groupId << node);

  if (m_scenarioCreated == false)
  {
    NS_FATAL_ERROR ("Method SatGroupHelper::AddUtNodeToGroup has to be called after SimulationHelper::CreateSatScenario");
  }

  if (groupId == 0 && !m_satConstellationEnabled)
    {
      NS_FATAL_ERROR ("Group ID 0 is reserved for UTs not manually assigned to a group");
    }

  if (GetGroupId (node) != 0)
    {
      NS_FATAL_ERROR ("Node " << node << " is already in group " << GetGroupId (node) << ". It cannot be added to group " << groupId);
    }

  if (IsGroupExisting (groupId) == false)
  {
    m_groupsList.push_back (groupId);
    m_groupsMap[groupId] = std::set<Ptr<Node> > ();
  }
  m_groupsMap[groupId].insert (node);
  Singleton<SatIdMapper>::Get ()->AttachMacToGroupId (Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (node), groupId);
}

void
SatGroupHelper::AddUtNodesToGroup (uint32_t groupId, NodeContainer nodes)
{
  NS_LOG_FUNCTION (this << groupId);

  if (m_scenarioCreated == false)
  {
    NS_FATAL_ERROR ("Method SatGroupHelper::AddUtNodesToGroup has to be called after SimulationHelper::CreateSatScenario");
  }

  for (NodeContainer::Iterator it = nodes.Begin (); it != nodes.End (); it++)
    {
      AddUtNodeToGroup (groupId, *it);
    }
}

void
SatGroupHelper::CreateGroupFromPosition (uint32_t groupId, NodeContainer nodes, GeoCoordinate center, uint32_t radius)
{
  NS_LOG_FUNCTION (this << groupId << center << radius);

  if (m_scenarioCreated == false)
  {
    NS_FATAL_ERROR ("Method SatGroupHelper::CreateGroupFromPosition has to be called after SimulationHelper::CreateSatScenario");
  }

  if (groupId == 0)
    {
      NS_FATAL_ERROR ("Cannot create new geographical group with a group ID of zero.");
    }
  if (GetUtNodes (groupId).GetN () != 0)
    {
      NS_FATAL_ERROR ("Cannot create new geographical group with a group ID already used.");
    }

  Vector centerPosition = center.ToVector ();
  GeoCoordinate nodePosition;
  double distance;
  NodeContainer nodesNotAlreadyAdded = GetNodesNotAddedFromPosition (nodes);
  for (NodeContainer::Iterator it = nodesNotAlreadyAdded.Begin (); it != nodesNotAlreadyAdded.End (); it++)
    {
      nodePosition = (*it)->GetObject<SatMobilityModel> ()->GetGeoPosition ();
      distance = CalculateDistance (centerPosition, nodePosition.ToVector ());
      if (distance <= radius)
        {
          AddUtNodeToGroup (groupId, *it);
        }
    }
}

void
SatGroupHelper::CreateGroupsUniformly (std::vector<uint32_t> groupIds, NodeContainer nodes)
{
  NS_LOG_FUNCTION (this << groupIds);

  if (m_scenarioCreated == false)
  {
    NS_FATAL_ERROR ("Method SatGroupHelper::CreateGroupsUniformly has to be called after SimulationHelper::CreateSatScenario");
  }

  for (uint32_t groupId : groupIds)
    {
      if (GetUtNodes (groupId).GetN () != 0)
        {
          NS_FATAL_ERROR ("Cannot create new group with a group ID already used: " << groupId);
        }
    }

  NodeContainer nodesNotAlreadyAdded = GetNodesNotAddedFromPosition (nodes);

  uint32_t nbNodes = nodesNotAlreadyAdded.GetN ();
  uint32_t counter = 0;

  for (uint32_t i = 0; i < nbNodes; i++)
    {
      AddUtNodeToGroup (groupIds[counter], nodesNotAlreadyAdded.Get (i));
      counter++;
      counter %= groupIds.size();
    }
}

void
SatGroupHelper::CreateUtNodesFromPosition (uint32_t groupId, uint32_t nb, GeoCoordinate center, uint32_t radius)
{
  NS_LOG_FUNCTION (this << groupId << nb << center << radius);

  if (m_scenarioCreated == true)
  {
    NS_FATAL_ERROR ("Method SatGroupHelper::CreateUtNodesFromPosition has to be called before SimulationHelper::CreateSatScenario");
  }

  if (groupId == 0)
    {
      NS_FATAL_ERROR ("Cannot call CreateUtNodesFromPosition with a group ID of zero.");
    }
  if (std::find(m_groupsList.begin(), m_groupsList.end(), groupId) != m_groupsList.end())
    {
      NS_FATAL_ERROR ("Cannot call CreateUtNodesFromPosition with a group ID already used: " << groupId);
    }

  Ptr<SatRandomCirclePositionAllocator> circleAllocator = CreateObject<SatRandomCirclePositionAllocator> (center, radius);

  for (uint32_t i = 0; i < nb; i++)
    {
      GeoCoordinate position = circleAllocator->GetNextGeoPosition ();
      m_additionalNodesPerBeam.push_back (std::make_pair(position, groupId));
    }

  m_groupsList.push_back(groupId);
}

void
SatGroupHelper::AddNodeToGroupAfterScenarioCreation (uint32_t groupId, Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << groupId << node);

  m_nodesToAdd[node] = groupId;
}

std::vector<std::pair<GeoCoordinate, uint32_t>>
SatGroupHelper::GetAdditionalNodesPerBeam ()
{
  NS_LOG_FUNCTION (this);

  return m_additionalNodesPerBeam;
}

NodeContainer
SatGroupHelper::GetUtNodes (uint32_t groupId) const
{
  NS_LOG_FUNCTION (this << groupId);

  if (groupId == 0)
    {
      bool found;
      NodeContainer groupIdZeroUts;
      for (NodeContainer::Iterator it = m_uts.Begin (); it != m_uts.End (); it++)
        {
          found = false;
          for (std::map<uint32_t, std::set<Ptr<Node> > >::const_iterator it2 = m_groupsMap.begin(); it2 != m_groupsMap.end(); it2++)
            {
              if (it2->second.find(*it) != it2->second.end())
                {
                  found = true;
                  break;
                }
            }
          if (found == false)
            {
              groupIdZeroUts.Add (*it);
            }
        }
      return groupIdZeroUts;
    }

  if (IsGroupExisting (groupId) == false)
  {
    return NodeContainer ();
  }

  NodeContainer utNodes;
  std::set<Ptr<Node> > nodes = m_groupsMap.at (groupId);

  for (std::set<Ptr<Node> >::const_iterator i = nodes.begin (); i != nodes.end (); i++)
    {
      utNodes.Add (*i);
    }

  return utNodes;
}

uint32_t
SatGroupHelper::GetN ()
{
  NS_LOG_FUNCTION (this);

  return m_groupsMap.size ();
}

std::list<uint32_t>
SatGroupHelper::GetGroups ()
{
  return m_groupsList;
}

void
SatGroupHelper::SetSatConstellationEnabled ()
{
  m_satConstellationEnabled = true;
}

bool
SatGroupHelper::IsGroupExisting (uint32_t groupId) const
{
  return m_groupsMap.find(groupId) != m_groupsMap.end();
}

uint32_t
SatGroupHelper::GetGroupId (Ptr<Node> node) const
{
  for (std::map<uint32_t, std::set<Ptr<Node> > >::const_iterator it = m_groupsMap.begin(); it != m_groupsMap.end(); it++)
    {
      if (it->second.find(node) != it->second.end())
        {
          return it->first;
        }
    }
  return 0;
}

NodeContainer
SatGroupHelper::GetNodesNotAddedFromPosition (NodeContainer nodes)
{
  NS_LOG_FUNCTION (this);

  NodeContainer nodesFiltered;
  Ptr<Node> node;
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      node = nodes.Get (i);
      if (m_nodesToAdd.count(node) == 0)
        {
          nodesFiltered.Add (node);
        }
    }
  return nodesFiltered;
}

} // namespace ns3
