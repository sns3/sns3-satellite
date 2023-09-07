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
 * Inspired and adapted from Hypatia: https://github.com/snkas/hypatia
 *
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-isl-arbiter-unicast-helper.h"

#include <ns3/node-container.h>
#include <ns3/node.h>

NS_LOG_COMPONENT_DEFINE("SatIslArbiterUnicastHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatIslArbiterUnicastHelper);

TypeId
SatIslArbiterUnicastHelper::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatIslArbiterUnicastHelper")
                            .SetParent<Object>()
                            .AddConstructor<SatIslArbiterUnicastHelper>();
    return tid;
}

SatIslArbiterUnicastHelper::SatIslArbiterUnicastHelper()
{
    NS_FATAL_ERROR("Default constructor not in use");
}

SatIslArbiterUnicastHelper::SatIslArbiterUnicastHelper(
    NodeContainer geoNodes,
    std::vector<std::pair<uint32_t, uint32_t>> isls)
    : m_geoNodes(geoNodes),
      m_isls(isls)
{
    NS_LOG_FUNCTION(this);
}

void
SatIslArbiterUnicastHelper::InstallArbiters()
{
    NS_LOG_FUNCTION(this);

    std::vector<std::map<uint32_t, uint32_t>> globalState = CalculateGlobalState();

    for (uint32_t satIndex = 0; satIndex < globalState.size(); satIndex++)
    {
        Ptr<Node> satelliteNode = m_geoNodes.Get(satIndex);
        Ptr<SatGeoNetDevice> satelliteGeoNetDevice;
        for (uint32_t ndIndex = 0; ndIndex < satelliteNode->GetNDevices(); ndIndex++)
        {
            Ptr<SatGeoNetDevice> nd =
                DynamicCast<SatGeoNetDevice>(satelliteNode->GetDevice(ndIndex));
            if (nd != nullptr)
            {
                satelliteGeoNetDevice = nd;
            }
        }

        NS_ASSERT_MSG(satelliteGeoNetDevice != nullptr, "SatGeoNetDevice not found on satellite");

        std::vector<Ptr<PointToPointIslNetDevice>> islNetDevices =
            satelliteGeoNetDevice->GetIslsNetDevices();
        Ptr<SatIslArbiterUnicast> arbiter = CreateObject<SatIslArbiterUnicast>(satelliteNode);

        for (uint32_t islInterfaceIndex = 0; islInterfaceIndex < islNetDevices.size();
             islInterfaceIndex++)
        {
            uint32_t interfaceNextHopNodeId =
                islNetDevices[islInterfaceIndex]->GetDestinationNode()->GetId();
            for (std::map<uint32_t, uint32_t>::iterator it = globalState[satIndex].begin();
                 it != globalState[satIndex].end();
                 it++)
            {
                uint32_t destinationNodeId = it->first;
                uint32_t nextHopNodeId = it->second;

                if (interfaceNextHopNodeId == nextHopNodeId)
                {
                    arbiter->AddNextHopEntry(destinationNodeId, islInterfaceIndex);
                }
            }
        }
        satelliteGeoNetDevice->SetArbiter(arbiter);
    }
}

void
SatIslArbiterUnicastHelper::UpdateArbiters()
{
    NS_LOG_FUNCTION(this);

    this->InstallArbiters();
}

std::vector<std::map<uint32_t, uint32_t>>
SatIslArbiterUnicastHelper::CalculateGlobalState()
{
    NS_LOG_FUNCTION(this);

    // Final result
    std::vector<std::vector<std::vector<uint32_t>>> globalCandidateList;

    ///////////////////////////
    // Floyd-Warshall

    int64_t n = m_geoNodes.GetN();

    // Enforce that more than 40000 nodes is not permitted (sqrt(2^31) ~= 46340, so let's call it an
    // even 40000)
    if (n > 40000)
    {
        NS_FATAL_ERROR("Cannot handle more than 40000 nodes");
    }

    // Initialize with 0 distance to itself, and infinite distance to all others
    int32_t n2 = n * n;
    int32_t* dist = new int32_t[n2];
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                dist[n * i + j] = 0;
            }
            else
            {
                dist[n * i + j] = 100000000;
            }
        }
    }

    // If there is an edge, the distance is 1
    for (std::pair<uint64_t, uint64_t> edge : m_isls)
    {
        dist[n * edge.first + edge.second] = 1;
        dist[n * edge.second + edge.first] = 1;
    }

    // Floyd-Warshall core
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (dist[n * i + j] > dist[n * i + k] + dist[n * k + j])
                {
                    dist[n * i + j] = dist[n * i + k] + dist[n * k + j];
                }
            }
        }
    }

    ///////////////////////////
    // Determine from the shortest path distances
    // the possible next hops

    // ECMP candidate list: candidate_list[current][destination] = [ list of next hops ]
    globalCandidateList.reserve(n);
    for (int i = 0; i < n; i++)
    {
        std::vector<std::vector<uint32_t>> v;
        v.reserve(n);
        for (int j = 0; j < n; j++)
        {
            v.push_back(std::vector<uint32_t>());
        }
        globalCandidateList.push_back(v);
    }

    // Candidate next hops are determined in the following way:
    // For each edge a -> b, for a destination t:
    // If the shortest_path_distance(b, t) == shortest_path_distance(a, t) - 1
    // then a -> b must be part of a shortest path from a towards t.
    for (std::pair<uint64_t, uint64_t> edge : m_isls)
    {
        for (int j = 0; j < n; j++)
        {
            if (dist[edge.first * n + j] - 1 == dist[edge.second * n + j])
            {
                globalCandidateList[edge.first][j].push_back(edge.second);
            }
            if (dist[edge.second * n + j] - 1 == dist[edge.first * n + j])
            {
                globalCandidateList[edge.second][j].push_back(edge.first);
            }
        }
    }

    // Free up the distance matrix
    delete[] dist;

    std::vector<std::map<uint32_t, uint32_t>> returnList;
    for (uint32_t i = 0; i < globalCandidateList.size(); i++)
    {
        returnList.push_back(std::map<uint32_t, uint32_t>());
        std::vector<std::vector<uint32_t>> v = globalCandidateList[i];
        for (uint32_t j = 0; j < v.size(); j++)
        {
            if (v[j].size() > 0)
            {
                returnList[i].insert(
                    std::make_pair(j, v[j][0])); // TODO for ECMP, keep all values in v[j]
            }
        }
    }

    // Return the final global candidate list
    return returnList;
}

} // namespace ns3
