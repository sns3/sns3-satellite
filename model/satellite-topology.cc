/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#include "satellite-topology.h"

#include "ns3/log.h"

#include <iostream>
#include <limits>
#include <map>
#include <ostream>
#include <set>
#include <utility>

NS_LOG_COMPONENT_DEFINE("SatTopology");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatTopology);

TypeId
SatTopology::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatTopology").SetParent<Object>().AddConstructor<SatTopology>();
    return tid;
}

SatTopology::SatTopology()
    : m_enableMapPrint(false)
{
    NS_LOG_FUNCTION(this);
}

SatTopology::~SatTopology()
{
    NS_LOG_FUNCTION(this);

    Reset();
}

void
SatTopology::DoDispose()
{
    NS_LOG_FUNCTION(this);

    Reset();

    Object::DoDispose();
}

void
SatTopology::Reset()
{
    NS_LOG_FUNCTION(this);

    m_gws = NodeContainer();
    m_gwIds.clear();
    m_uts = NodeContainer();
    m_orbiters = NodeContainer();
    m_gwUsers = NodeContainer();
    m_utUsers = NodeContainer();
    m_utToGwMap.clear();
    m_beamToGwMap.clear();
    m_orbiterFeederMacMap.clear();

    m_enableMapPrint = false;
}

void
SatTopology::SetStandard(SatEnums::Standard_t standard)
{
    NS_LOG_FUNCTION(this << standard);

    m_standard = standard;
}

SatEnums::Standard_t
SatTopology::GetStandard()
{
    NS_LOG_FUNCTION(this);

    return m_standard;
}

void
SatTopology::SetForwardLinkRegenerationMode(
    SatEnums::RegenerationMode_t forwardLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << forwardLinkRegenerationMode);

    m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
}

SatEnums::RegenerationMode_t
SatTopology::GetForwardLinkRegenerationMode()
{
    NS_LOG_FUNCTION(this);

    return m_forwardLinkRegenerationMode;
}

void
SatTopology::SetReturnLinkRegenerationMode(SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << returnLinkRegenerationMode);

    m_returnLinkRegenerationMode = returnLinkRegenerationMode;
}

SatEnums::RegenerationMode_t
SatTopology::GetReturnLinkRegenerationMode()
{
    NS_LOG_FUNCTION(this);

    return m_returnLinkRegenerationMode;
}

void
SatTopology::AddGwNode(uint32_t gwId, Ptr<Node> gw)
{
    NS_LOG_FUNCTION(this << gw);

    m_gwIds.insert(std::make_pair(gwId, gw));

    m_gws = NodeContainer();
    for (std::map<uint32_t, Ptr<Node>>::const_iterator i = m_gwIds.begin(); i != m_gwIds.end(); i++)
    {
        m_gws.Add(i->second);
    }
}

void
SatTopology::AddUtNode(Ptr<Node> ut)
{
    NS_LOG_FUNCTION(this << ut);

    m_uts.Add(ut);
}

void
SatTopology::AddOrbiterNode(Ptr<Node> orbiter)
{
    NS_LOG_FUNCTION(this << orbiter);

    m_orbiters.Add(orbiter);
}

void
SatTopology::AddGwUserNode(Ptr<Node> gwUser)
{
    NS_LOG_FUNCTION(this << gwUser);

    m_gwUsers.Add(gwUser);
}

void
SatTopology::AddUtUserNode(Ptr<Node> utUser, Ptr<Node> ut)
{
    NS_LOG_FUNCTION(this << utUser);

    if (m_detailledUtUsers.find(ut) == m_detailledUtUsers.end())
    {
        m_detailledUtUsers[ut] = NodeContainer();
    }
    m_detailledUtUsers[ut].Add(utUser);
    m_utUsers.Add(utUser);
}

void
SatTopology::ConnectGwToUt(Ptr<Node> ut, Ptr<Node> gw)
{
    NS_LOG_FUNCTION(this << ut << gw);

    if (m_utToGwMap.count(ut) > 0)
    {
        NS_FATAL_ERROR("UT " << ut << " already in GW to UT map. Connected to GW "
                             << m_utToGwMap[ut]);
    }

    m_utToGwMap[ut] = gw;
}

void
SatTopology::UpdateGwConnectedToUt(Ptr<Node> ut, Ptr<Node> gw)
{
    NS_LOG_FUNCTION(this << ut << gw);

    if (m_utToGwMap.count(ut) == 0)
    {
        NS_FATAL_ERROR("UT " << ut << " not in GW to UT map.");
    }

    m_utToGwMap[ut] = gw;
}

void
SatTopology::DisconnectGwFromUt(Ptr<Node> ut)
{
    NS_LOG_FUNCTION(this << ut);

    const std::map<Ptr<Node>, Ptr<Node>>::iterator it = m_utToGwMap.find(ut);
    if (it == m_utToGwMap.end())
    {
        NS_FATAL_ERROR("UT " << ut << " not in GW to UT map.");
    }
    else
    {
        m_utToGwMap.erase(it);
    }
}

Ptr<Node>
SatTopology::GetGwFromUt(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    const std::map<Ptr<Node>, Ptr<Node>>::const_iterator it = m_utToGwMap.find(ut);
    if (it == m_utToGwMap.end())
    {
        NS_FATAL_ERROR("UT " << ut << " not in GW to UT map.");
    }
    return it->second;
}

void
SatTopology::ConnectGwToBeam(uint32_t beamId, Ptr<Node> gw)
{
    NS_LOG_FUNCTION(this << beamId << gw);

    if (m_beamToGwMap.count(beamId) > 0)
    {
        NS_FATAL_ERROR("Beam " << beamId << " already in GW to beam map. Connected to GW "
                               << m_beamToGwMap[beamId]);
    }

    m_beamToGwMap[beamId] = gw;
}

Ptr<Node>
SatTopology::GetGwFromBeam(uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << beamId);

    const std::map<uint32_t, Ptr<Node>>::const_iterator it = m_beamToGwMap.find(beamId);
    if (it == m_beamToGwMap.end())
    {
        NS_FATAL_ERROR("Beam " << beamId << " not in GW to beam map.");
    }
    return it->second;
}

void
SatTopology::AddOrbiterFeederMacPair(Ptr<SatOrbiterFeederMac> mac, Ptr<SatOrbiterFeederMac> usedMac)
{
    NS_LOG_FUNCTION(this << mac << usedMac);

    if (m_orbiterFeederMacMap.count(mac) > 0)
    {
        NS_FATAL_ERROR("Feeder MAC " << mac << " already associated to "
                                     << m_orbiterFeederMacMap[mac]);
    }

    m_orbiterFeederMacMap[mac] = usedMac;
}

Ptr<SatOrbiterFeederMac>
SatTopology::GetOrbiterFeederMacUsed(Ptr<SatOrbiterFeederMac> mac) const
{
    NS_LOG_FUNCTION(this << mac);

    const std::map<Ptr<SatOrbiterFeederMac>, Ptr<SatOrbiterFeederMac>>::const_iterator it =
        m_orbiterFeederMacMap.find(mac);
    if (it == m_orbiterFeederMacMap.end())
    {
        NS_FATAL_ERROR("Feeder MAC " << mac << " not found");
    }

    return it->second;
}

Mac48Address
SatTopology::GetGwAddressInUt(uint32_t utId)
{
    NS_LOG_FUNCTION(this << utId);

    Ptr<Node> utNode = GetNodeFromId(utId);
    uint32_t utBeamId = GetUtBeamId(utNode);
    Ptr<Node> gwNode = GetGwFromBeam(utBeamId);

    uint32_t gwSatId =
        GetClosestSat(GeoCoordinate(gwNode->GetObject<SatMobilityModel>()->GetPosition()));
    Ptr<Node> gwOrbiter = m_orbiters.Get(gwSatId);

    Ptr<SatOrbiterFeederMac> mac = GetOrbiterFeederMac(gwOrbiter, utBeamId);
    uint32_t usedBeamId = GetOrbiterFeederMacUsed(mac)->GetBeamId();

    Ptr<SatGwMac> gwMac = GetDvbGwMac(gwNode, gwSatId, usedBeamId);

    NS_ASSERT_MSG(gwMac != nullptr, "Got a null SatGwMac");

    return Mac48Address::ConvertFrom(gwMac->GetAddress());
}

NodeContainer
SatTopology::GetGwNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_gws;
}

NodeContainer
SatTopology::GetUtNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_uts;
}

NodeContainer
SatTopology::GetOrbiterNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_orbiters;
}

NodeContainer
SatTopology::GetGwUserNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_gwUsers;
}

NodeContainer
SatTopology::GetUtUserNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_utUsers;
}

NodeContainer
SatTopology::GetUtUserNodes(NodeContainer uts) const
{
    NS_LOG_FUNCTION(this);

    NodeContainer total;
    for (NodeContainer::Iterator i = uts.Begin(); i != uts.End(); i++)
    {
        total.Add(GetUtUserNodes(*i));
    }
    return total;
}

NodeContainer
SatTopology::GetUtUserNodes(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    std::map<Ptr<Node>, NodeContainer>::const_iterator it = m_detailledUtUsers.find(ut);

    if (it == m_detailledUtUsers.end())
    {
        NS_FATAL_ERROR("UT which users are requested in not installed!!!");
    }

    return it->second;
}

Ptr<Node>
SatTopology::GetUtNode(Ptr<Node> utUser) const
{
    for (std::map<Ptr<Node>, NodeContainer>::const_iterator it = m_detailledUtUsers.begin();
         it != m_detailledUtUsers.end();
         it++)
    {
        for (NodeContainer::Iterator it2 = it->second.Begin(); it2 != it->second.End(); it2++)
        {
            if (*it2 == utUser)
            {
                return it->first;
            }
        }
    }

    return nullptr;
}

uint32_t
SatTopology::GetNGwNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_gws.GetN();
}

uint32_t
SatTopology::GetNUtNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_uts.GetN();
}

uint32_t
SatTopology::GetNOrbiterNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_orbiters.GetN();
}

uint32_t
SatTopology::GetNGwUserNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_gwUsers.GetN();
}

uint32_t
SatTopology::GetNUtUserNodes() const
{
    NS_LOG_FUNCTION(this);

    return m_utUsers.GetN();
}

Ptr<Node>
SatTopology::GetGwNode(uint32_t nodeId) const
{
    NS_LOG_FUNCTION(this << nodeId);

    return m_gws.Get(nodeId);
}

Ptr<Node>
SatTopology::GetUtNode(uint32_t nodeId) const
{
    NS_LOG_FUNCTION(this << nodeId);

    return m_uts.Get(nodeId);
}

Ptr<Node>
SatTopology::GetOrbiterNode(uint32_t nodeId) const
{
    NS_LOG_FUNCTION(this << nodeId);

    return m_orbiters.Get(nodeId);
}

Ptr<Node>
SatTopology::GetGwUserNode(uint32_t nodeId) const
{
    NS_LOG_FUNCTION(this << nodeId);

    return m_gwUsers.Get(nodeId);
}

Ptr<Node>
SatTopology::GetUtUserNode(uint32_t nodeId) const
{
    NS_LOG_FUNCTION(this << nodeId);

    return m_utUsers.Get(nodeId);
}

Ptr<Node>
SatTopology::GetNodeFromId(uint32_t nodeId) const
{
    NS_LOG_FUNCTION(this << nodeId);

    NodeContainer::Iterator it;
    for (it = m_gws.Begin(); it != m_gws.End(); it++)
    {
        if ((*it)->GetId() == nodeId)
        {
            return *it;
        }
    }

    for (it = m_uts.Begin(); it != m_uts.End(); it++)
    {
        if ((*it)->GetId() == nodeId)
        {
            return *it;
        }
    }

    for (it = m_orbiters.Begin(); it != m_orbiters.End(); it++)
    {
        if ((*it)->GetId() == nodeId)
        {
            return *it;
        }
    }

    for (it = m_gwUsers.Begin(); it != m_gwUsers.End(); it++)
    {
        if ((*it)->GetId() == nodeId)
        {
            return *it;
        }
    }

    for (it = m_utUsers.Begin(); it != m_utUsers.End(); it++)
    {
        if ((*it)->GetId() == nodeId)
        {
            return *it;
        }
    }

    NS_FATAL_ERROR("Node " << nodeId << " not found");

    return nullptr;
}

void
SatTopology::AddGwLayersDvb(Ptr<Node> gw,
                            uint32_t gwSatId,
                            uint32_t gwBeamId,
                            uint32_t utSatId,
                            uint32_t utBeamId,
                            Ptr<SatNetDevice> netDevice,
                            Ptr<SatGwLlc> llc,
                            Ptr<SatGwMac> mac,
                            Ptr<SatGwPhy> phy)
{
    NS_LOG_FUNCTION(this << gw << gwSatId << gwBeamId << utSatId << utBeamId << netDevice << llc
                         << mac << phy);

    std::map<Ptr<Node>, GwLayers_s>::iterator it = m_gwLayers.find(gw);
    GwLayers_s layers;
    if (it != m_gwLayers.end())
    {
        layers = it->second;
        NS_ASSERT_MSG(
            layers.m_satId == gwSatId,
            "Gw has already a different GW satellite ID that the one in argument of this method");
        NS_ASSERT_MSG(
            layers.m_beamId == gwBeamId,
            "Gw has already a different GW beam ID that the one in argument of this method");
        NS_ASSERT_MSG(layers.m_netDevice.find(std::make_pair(utSatId, utBeamId)) ==
                          layers.m_netDevice.end(),
                      "Net device already stored for this GW + UT satellite and beam");
        NS_ASSERT_MSG(layers.m_llc.find(std::make_pair(utSatId, utBeamId)) == layers.m_llc.end(),
                      "LLC already stored for this GW + UT satellite and beam");
        NS_ASSERT_MSG(layers.m_dvbMac.find(std::make_pair(utSatId, utBeamId)) ==
                          layers.m_dvbMac.end(),
                      "MAC already stored for this GW + UT satellite and beam");
        NS_ASSERT_MSG(layers.m_phy.find(std::make_pair(utSatId, utBeamId)) == layers.m_phy.end(),
                      "Physical layer already stored for this GW + UT satellite and beam");
    }
    else
    {
        layers.m_satId = gwSatId;
        layers.m_beamId = gwBeamId;
    }

    layers.m_netDevice.insert(std::make_pair(std::make_pair(utSatId, utBeamId), netDevice));
    layers.m_llc.insert(std::make_pair(std::make_pair(utSatId, utBeamId), llc));
    layers.m_dvbMac.insert(std::make_pair(std::make_pair(utSatId, utBeamId), mac));
    layers.m_phy.insert(std::make_pair(std::make_pair(utSatId, utBeamId), phy));

    m_gwLayers[gw] = layers;
}

void
SatTopology::AddGwLayersLora(Ptr<Node> gw,
                             uint32_t gwSatId,
                             uint32_t gwBeamId,
                             uint32_t utSatId,
                             uint32_t utBeamId,
                             Ptr<SatNetDevice> netDevice,
                             Ptr<LorawanGroundMacGateway> mac,
                             Ptr<SatGwPhy> phy)
{
    NS_LOG_FUNCTION(this << gw << gwSatId << gwBeamId << utSatId << utBeamId << netDevice << mac
                         << phy);

    std::map<Ptr<Node>, GwLayers_s>::iterator it = m_gwLayers.find(gw);
    GwLayers_s layers;
    if (it != m_gwLayers.end())
    {
        layers = it->second;
        NS_ASSERT_MSG(
            layers.m_satId == gwSatId,
            "Gw has already a different GW satellite ID that the one in argument of this method");
        NS_ASSERT_MSG(
            layers.m_beamId == gwBeamId,
            "Gw has already a different GW beam ID that the one in argument of this method");
        NS_ASSERT_MSG(layers.m_netDevice.find(std::make_pair(utSatId, utBeamId)) ==
                          layers.m_netDevice.end(),
                      "Net device already stored for this GW + UT satellite and beam");
        NS_ASSERT_MSG(layers.m_loraMac.find(std::make_pair(utSatId, utBeamId)) ==
                          layers.m_loraMac.end(),
                      "MAC already stored for this GW + UT satellite and beam");
        NS_ASSERT_MSG(layers.m_phy.find(std::make_pair(utSatId, utBeamId)) == layers.m_phy.end(),
                      "Physical layer already stored for this GW + UT satellite and beam");
    }
    else
    {
        layers.m_satId = gwSatId;
        layers.m_beamId = gwBeamId;
    }

    layers.m_netDevice.insert(std::make_pair(std::make_pair(utSatId, utBeamId), netDevice));
    layers.m_loraMac.insert(std::make_pair(std::make_pair(utSatId, utBeamId), mac));
    layers.m_phy.insert(std::make_pair(std::make_pair(utSatId, utBeamId), phy));

    m_gwLayers[gw] = layers;
}

void
SatTopology::UpdateGwSatAndBeam(Ptr<Node> gw, uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << gw << satId << beamId);

    NS_ASSERT_MSG(m_gwLayers.find(gw) != m_gwLayers.end(), "Layers do not exist for this GW");

    m_gwLayers[gw].m_satId = satId;
    m_gwLayers[gw].m_beamId = beamId;
}

uint32_t
SatTopology::GetGwSatId(Ptr<Node> gw) const
{
    NS_LOG_FUNCTION(this << gw);

    NS_ASSERT_MSG(m_gwLayers.find(gw) != m_gwLayers.end(), "Layers do not exist for this GW");

    return m_gwLayers.at(gw).m_satId;
}

uint32_t
SatTopology::GetGwBeamId(Ptr<Node> gw) const
{
    NS_LOG_FUNCTION(this << gw);

    NS_ASSERT_MSG(m_gwLayers.find(gw) != m_gwLayers.end(), "Layers do not exist for this GW");

    return m_gwLayers.at(gw).m_beamId;
}

Ptr<SatNetDevice>
SatTopology::GetGwNetDevice(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << gw << utSatId << utBeamId);

    std::map<Ptr<Node>, GwLayers_s>::const_iterator it = m_gwLayers.find(gw);
    NS_ASSERT_MSG(it != m_gwLayers.end(),
                  "Layers do not exist for this GW and pair UT satellite+beam");

    GwLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_netDevice.find(std::make_pair(utSatId, utBeamId)) !=
                      layers.m_netDevice.end(),
                  "Net device not stored for this GW + UT satellite and beam");

    return layers.m_netDevice.at(std::make_pair(utSatId, utBeamId));
}

Ptr<SatGwLlc>
SatTopology::GetGwLlc(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << gw << utSatId << utBeamId);

    std::map<Ptr<Node>, GwLayers_s>::const_iterator it = m_gwLayers.find(gw);
    NS_ASSERT_MSG(it != m_gwLayers.end(),
                  "Layers do not exist for this GW and pair UT satellite+beam");

    GwLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_llc.find(std::make_pair(utSatId, utBeamId)) != layers.m_llc.end(),
                  "LLC not stored for this GW + UT satellite and beam");

    return layers.m_llc.at(std::make_pair(utSatId, utBeamId));
}

Ptr<SatGwMac>
SatTopology::GetDvbGwMac(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << gw << utSatId << utBeamId);

    std::map<Ptr<Node>, GwLayers_s>::const_iterator it = m_gwLayers.find(gw);
    NS_ASSERT_MSG(it != m_gwLayers.end(),
                  "Layers do not exist for this GW and pair UT satellite+beam");

    GwLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_dvbMac.find(std::make_pair(utSatId, utBeamId)) != layers.m_dvbMac.end(),
                  "MAC not stored for this GW + UT satellite and beam");

    return layers.m_dvbMac.at(std::make_pair(utSatId, utBeamId));
}

Ptr<LorawanGroundMacGateway>
SatTopology::GetLoraGwMac(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << gw << utSatId << utBeamId);

    std::map<Ptr<Node>, GwLayers_s>::const_iterator it = m_gwLayers.find(gw);
    NS_ASSERT_MSG(it != m_gwLayers.end(),
                  "Layers do not exist for this GW and pair UT satellite+beam");

    GwLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_loraMac.find(std::make_pair(utSatId, utBeamId)) !=
                      layers.m_loraMac.end(),
                  "MAC not stored for this GW + UT satellite and beam");

    return layers.m_loraMac.at(std::make_pair(utSatId, utBeamId));
}

Ptr<SatGwPhy>
SatTopology::GetGwPhy(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << gw << utSatId << utBeamId);

    std::map<Ptr<Node>, GwLayers_s>::const_iterator it = m_gwLayers.find(gw);
    NS_ASSERT_MSG(it != m_gwLayers.end(),
                  "Layers do not exist for this GW and pair UT satellite+beam");

    GwLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_phy.find(std::make_pair(utSatId, utBeamId)) != layers.m_phy.end(),
                  "Physical layer not stored for this GW + UT satellite and beam");

    return layers.m_phy.at(std::make_pair(utSatId, utBeamId));
}

void
SatTopology::AddUtLayersDvb(Ptr<Node> ut,
                            uint32_t satId,
                            uint32_t beamId,
                            uint32_t groupId,
                            Ptr<SatNetDevice> netDevice,
                            Ptr<SatUtLlc> llc,
                            Ptr<SatUtMac> mac,
                            Ptr<SatUtPhy> phy)
{
    NS_LOG_FUNCTION(this << ut << satId << beamId << groupId << netDevice << llc << mac << phy);

    NS_ASSERT_MSG(m_utLayers.find(ut) == m_utLayers.end(), "Layers already added to this UT node");

    UtLayers_s layers;
    layers.m_satId = satId;
    layers.m_beamId = beamId;
    layers.m_groupId = groupId;
    layers.m_netDevice = netDevice;
    layers.m_llc = llc;
    layers.m_dvbMac = mac;
    layers.m_phy = phy;

    m_utLayers.insert(std::make_pair(ut, layers));
}

void
SatTopology::AddUtLayersLora(Ptr<Node> ut,
                             uint32_t satId,
                             uint32_t beamId,
                             uint32_t groupId,
                             Ptr<SatNetDevice> netDevice,
                             Ptr<LorawanMacEndDevice> mac,
                             Ptr<SatUtPhy> phy)
{
    NS_LOG_FUNCTION(this << ut << satId << beamId << groupId << netDevice << mac << phy);

    NS_ASSERT_MSG(m_utLayers.find(ut) == m_utLayers.end(), "Layers already added to this UT node");

    UtLayers_s layers;
    layers.m_satId = satId;
    layers.m_beamId = beamId;
    layers.m_groupId = groupId;
    layers.m_netDevice = netDevice;
    layers.m_loraMac = mac;
    layers.m_phy = phy;

    m_utLayers.insert(std::make_pair(ut, layers));
}

void
SatTopology::UpdateUtSatAndBeam(Ptr<Node> ut, uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << ut << satId << beamId);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    m_utLayers[ut].m_satId = satId;
    m_utLayers[ut].m_beamId = beamId;
}

void
SatTopology::UpdateUtGroup(Ptr<Node> ut, uint32_t groupId)
{
    NS_LOG_FUNCTION(this << ut << groupId);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    m_utLayers[ut].m_groupId = groupId;
}

uint32_t
SatTopology::GetUtSatId(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_satId;
}

uint32_t
SatTopology::GetUtBeamId(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_beamId;
}

uint32_t
SatTopology::GetUtGroupId(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_groupId;
}

Ptr<SatNetDevice>
SatTopology::GetUtNetDevice(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_netDevice;
}

Ptr<SatUtLlc>
SatTopology::GetUtLlc(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_llc;
}

Ptr<SatUtMac>
SatTopology::GetDvbUtMac(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_dvbMac;
}

Ptr<LorawanMacEndDevice>
SatTopology::GetLoraUtMac(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_loraMac;
}

Ptr<SatUtPhy>
SatTopology::GetUtPhy(Ptr<Node> ut) const
{
    NS_LOG_FUNCTION(this << ut);

    NS_ASSERT_MSG(m_utLayers.find(ut) != m_utLayers.end(), "Layers do not exist for this UT");

    return m_utLayers.at(ut).m_phy;
}

void
SatTopology::AddOrbiterFeederLayers(Ptr<Node> orbiter,
                                    uint32_t satId,
                                    uint32_t utBeamId,
                                    Ptr<SatOrbiterNetDevice> netDevice,
                                    Ptr<SatOrbiterFeederLlc> llc,
                                    Ptr<SatOrbiterFeederMac> mac,
                                    Ptr<SatOrbiterFeederPhy> phy)
{
    NS_LOG_FUNCTION(this << orbiter << satId << utBeamId << netDevice << llc << mac << phy);

    std::map<Ptr<Node>, OrbiterLayers_s>::iterator it = m_orbiterLayers.find(orbiter);
    OrbiterLayers_s layers;
    if (it != m_orbiterLayers.end())
    {
        layers = it->second;
        NS_ASSERT_MSG(
            layers.m_satId == satId,
            "Orbiter has already a different satellite ID that the one in argument of this method");
        NS_ASSERT_MSG(layers.m_netDevice == netDevice,
                      "Orbiter has already a different SatOrbiterNetDevice that the one in "
                      "argument of this method");
        NS_ASSERT_MSG(layers.m_feederLlc.find(utBeamId) == layers.m_feederLlc.end(),
                      "Feeder LLC already stored for this pair orbiter/beam");
        NS_ASSERT_MSG(layers.m_feederMac.find(utBeamId) == layers.m_feederMac.end(),
                      "Feeder MAC already stored for this pair orbiter/beam");
        NS_ASSERT_MSG(layers.m_feederPhy.find(utBeamId) == layers.m_feederPhy.end(),
                      "Feeder physical layer already stored for this pair orbiter/beam");
    }
    else
    {
        layers.m_satId = satId;
        layers.m_netDevice = netDevice;
    }

    layers.m_feederLlc.insert(std::make_pair(utBeamId, llc));
    layers.m_feederMac.insert(std::make_pair(utBeamId, mac));
    layers.m_feederPhy.insert(std::make_pair(utBeamId, phy));

    m_orbiterLayers[orbiter] = layers;
}

void
SatTopology::AddOrbiterUserLayersDvb(Ptr<Node> orbiter,
                                     uint32_t satId,
                                     uint32_t beamId,
                                     Ptr<SatOrbiterNetDevice> netDevice,
                                     Ptr<SatOrbiterUserLlc> llc,
                                     Ptr<SatOrbiterUserMac> mac,
                                     Ptr<SatOrbiterUserPhy> phy)
{
    NS_LOG_FUNCTION(this << orbiter << satId << beamId << netDevice << llc << mac << phy);

    std::map<Ptr<Node>, OrbiterLayers_s>::iterator it = m_orbiterLayers.find(orbiter);
    OrbiterLayers_s layers;
    if (it != m_orbiterLayers.end())
    {
        layers = it->second;
        NS_ASSERT_MSG(
            layers.m_satId == satId,
            "Orbiter has already a different satellite ID that the one in argument of this method");
        NS_ASSERT_MSG(layers.m_netDevice == netDevice,
                      "Orbiter has already a different SatOrbiterNetDevice that the one in "
                      "argument of this method");
        NS_ASSERT_MSG(layers.m_userLlc.find(beamId) == layers.m_userLlc.end(),
                      "User LLC already stored for this pair orbiter/beam");
        NS_ASSERT_MSG(layers.m_dvbUserMac.find(beamId) == layers.m_dvbUserMac.end(),
                      "User MAC already stored for this pair orbiter/beam");
        NS_ASSERT_MSG(layers.m_userPhy.find(beamId) == layers.m_userPhy.end(),
                      "User physical layer already stored for this pair orbiter/beam");
    }
    else
    {
        layers.m_satId = satId;
        layers.m_netDevice = netDevice;
    }

    layers.m_userLlc.insert(std::make_pair(beamId, llc));
    layers.m_dvbUserMac.insert(std::make_pair(beamId, mac));
    layers.m_userPhy.insert(std::make_pair(beamId, phy));

    m_orbiterLayers[orbiter] = layers;
}

void
SatTopology::AddOrbiterUserLayersLora(Ptr<Node> orbiter,
                                      uint32_t satId,
                                      uint32_t beamId,
                                      Ptr<SatOrbiterNetDevice> netDevice,
                                      Ptr<LorawanOrbiterMacGateway> mac,
                                      Ptr<SatOrbiterUserPhy> phy)
{
    NS_LOG_FUNCTION(this << orbiter << satId << beamId << netDevice << mac << phy);

    std::map<Ptr<Node>, OrbiterLayers_s>::iterator it = m_orbiterLayers.find(orbiter);
    OrbiterLayers_s layers;
    if (it != m_orbiterLayers.end())
    {
        layers = it->second;
        NS_ASSERT_MSG(
            layers.m_satId == satId,
            "Orbiter has already a different satellite ID that the one in argument of this method");
        NS_ASSERT_MSG(layers.m_netDevice == netDevice,
                      "Orbiter has already a different SatOrbiterNetDevice that the one in "
                      "argument of this method");
        NS_ASSERT_MSG(layers.m_loraUserMac.find(beamId) == layers.m_loraUserMac.end(),
                      "User MAC already stored for this pair orbiter/beam");
        NS_ASSERT_MSG(layers.m_userPhy.find(beamId) == layers.m_userPhy.end(),
                      "User physical layer already stored for this pair orbiter/beam");
    }
    else
    {
        layers.m_satId = satId;
        layers.m_netDevice = netDevice;
    }

    layers.m_loraUserMac.insert(std::make_pair(beamId, mac));
    layers.m_userPhy.insert(std::make_pair(beamId, phy));

    m_orbiterLayers[orbiter] = layers;
}

uint32_t
SatTopology::GetOrbiterSatId(Ptr<Node> orbiter) const
{
    NS_LOG_FUNCTION(this << orbiter);

    NS_ASSERT_MSG(m_orbiterLayers.find(orbiter) != m_orbiterLayers.end(),
                  "Layers do not exist for this UT");

    return m_orbiterLayers.at(orbiter).m_satId;
}

Ptr<SatOrbiterNetDevice>
SatTopology::GetOrbiterNetDevice(Ptr<Node> orbiter) const
{
    NS_LOG_FUNCTION(this << orbiter);

    NS_ASSERT_MSG(m_orbiterLayers.find(orbiter) != m_orbiterLayers.end(),
                  "Layers do not exist for this UT");

    return m_orbiterLayers.at(orbiter).m_netDevice;
}

Ptr<SatOrbiterFeederLlc>
SatTopology::GetOrbiterFeederLlc(Ptr<Node> orbiter, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << orbiter << utBeamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_feederLlc.find(utBeamId) != layers.m_feederLlc.end(),
                  "Feeder LLC not stored for this pair orbiter/beam");

    return layers.m_feederLlc.at(utBeamId);
}

Ptr<SatOrbiterUserLlc>
SatTopology::GetOrbiterUserLlc(Ptr<Node> orbiter, uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << orbiter << beamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_userLlc.find(beamId) != layers.m_userLlc.end(),
                  "Feeder LLC not stored for this pair orbiter/beam");

    return layers.m_userLlc.at(beamId);
}

Ptr<SatOrbiterFeederMac>
SatTopology::GetOrbiterFeederMac(Ptr<Node> orbiter, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << orbiter << utBeamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;

    NS_ASSERT_MSG(layers.m_feederMac.find(utBeamId) != layers.m_feederMac.end(),
                  "Feeder MAC not stored for this pair orbiter/beam");

    return layers.m_feederMac.at(utBeamId);
}

Ptr<SatOrbiterUserMac>
SatTopology::GetDvbOrbiterUserMac(Ptr<Node> orbiter, uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << orbiter << beamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_dvbUserMac.find(beamId) != layers.m_dvbUserMac.end(),
                  "Feeder MAC not stored for this pair orbiter/beam");

    return layers.m_dvbUserMac.at(beamId);
}

Ptr<LorawanOrbiterMacGateway>
SatTopology::GetLoraOrbiterUserMac(Ptr<Node> orbiter, uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << orbiter << beamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_loraUserMac.find(beamId) != layers.m_loraUserMac.end(),
                  "Feeder MAC not stored for this pair orbiter/beam");

    return layers.m_loraUserMac.at(beamId);
}

Ptr<SatOrbiterFeederPhy>
SatTopology::GetOrbiterFeederPhy(Ptr<Node> orbiter, uint32_t utBeamId) const
{
    NS_LOG_FUNCTION(this << orbiter << utBeamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_feederPhy.find(utBeamId) != layers.m_feederPhy.end(),
                  "Feeder PHY not stored for this pair orbiter/beam");

    return layers.m_feederPhy.at(utBeamId);
}

Ptr<SatOrbiterUserPhy>
SatTopology::GetOrbiterUserPhy(Ptr<Node> orbiter, uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << orbiter << beamId);

    std::map<Ptr<Node>, OrbiterLayers_s>::const_iterator it = m_orbiterLayers.find(orbiter);
    NS_ASSERT_MSG(it != m_orbiterLayers.end(), "Layers do not exist for this UT");

    OrbiterLayers_s layers = it->second;
    NS_ASSERT_MSG(layers.m_userPhy.find(beamId) != layers.m_userPhy.end(),
                  "Feeder PHY not stored for this pair orbiter/beam");

    return layers.m_userPhy.at(beamId);
}

void
SatTopology::PrintTopology(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);

    os << "Satellite topology" << std::endl;
    os << "==================" << std::endl;

    os << "Satellites" << std::endl;
    Ptr<Node> orbiter;
    for (NodeContainer::Iterator itOrbiter = m_orbiters.Begin(); itOrbiter != m_orbiters.End();
         itOrbiter++)
    {
        orbiter = *itOrbiter;
        const OrbiterLayers_s layers = m_orbiterLayers.at(orbiter);

        os << "  SAT: ID = " << layers.m_satId;
        os << ", at " << GeoCoordinate(orbiter->GetObject<SatMobilityModel>()->GetPosition())
           << std::endl;
        os << "    Devices to ground stations " << std::endl;

        os << "      " << layers.m_netDevice->GetAddress() << std::endl;
        for (std::pair<uint32_t, Ptr<SatOrbiterFeederMac>> feederMac : layers.m_feederMac)
        {
            os << "        Feeder at " << feederMac.second->GetAddress() << ", beam "
               << feederMac.first << std::endl;
        }

        os << "      Feeder connected to" << std::endl;
        std::set<Mac48Address> gwConnected = layers.m_netDevice->GetGwConnected();
        for (std::set<Mac48Address>::iterator it = gwConnected.begin(); it != gwConnected.end();
             it++)
        {
            os << "        " << *it << std::endl;
        }

        if (m_standard == SatEnums::LORA &&
            m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
        {
            for (std::pair<uint32_t, Ptr<LorawanOrbiterMacGateway>> userMac : layers.m_loraUserMac)
            {
                os << "        User at " << userMac.second->GetAddress() << ", beam "
                   << userMac.first << std::endl;
            }
        }
        else
        {
            for (std::pair<uint32_t, Ptr<SatOrbiterUserMac>> userMac : layers.m_dvbUserMac)
            {
                os << "        User at " << userMac.second->GetAddress() << ", beam "
                   << userMac.first << std::endl;
            }
        }

        os << "      User connected to" << std::endl;
        std::set<Mac48Address> utConnected = layers.m_netDevice->GetUtConnected();
        for (std::set<Mac48Address>::iterator it = utConnected.begin(); it != utConnected.end();
             it++)
        {
            os << "        " << *it << std::endl;
        }

        os << "    ISLs " << std::endl;
        for (uint32_t j = 0; j < orbiter->GetNDevices(); j++)
        {
            Ptr<PointToPointIslNetDevice> islNetDevice =
                DynamicCast<PointToPointIslNetDevice>(orbiter->GetDevice(j));
            if (islNetDevice)
            {
                os << "      " << islNetDevice->GetAddress() << " to SAT "
                   << islNetDevice->GetDestinationNode()->GetId() << std::endl;
            }
        }
    }

    os << "GWs" << std::endl;
    Ptr<Node> gwNode;
    for (NodeContainer::Iterator itGw = m_gws.Begin(); itGw != m_gws.End(); itGw++)
    {
        gwNode = *itGw;
        const GwLayers_s layers = m_gwLayers.at(gwNode);
        os << "  GW: ID = " << gwNode->GetId();
        os << ", at " << GeoCoordinate(gwNode->GetObject<SatMobilityModel>()->GetPosition())
           << std::endl;
        os << "  Devices " << std::endl;

        if (m_standard == SatEnums::LORA && m_forwardLinkRegenerationMode == SatEnums::TRANSPARENT)
        {
            for (std::pair<std::pair<uint32_t, uint32_t>, Ptr<LorawanGroundMacGateway>> mac :
                 layers.m_loraMac)
            {
                uint32_t satId = mac.first.first;
                uint32_t beamId = mac.first.second;
                os << "    " << mac.second->GetAddress() << ", sat: " << satId
                   << ", beam: " << beamId << std::endl;
            }
        }
        else
        {
            for (std::pair<std::pair<uint32_t, uint32_t>, Ptr<SatGwMac>> mac : layers.m_dvbMac)
            {
                uint32_t satId = mac.first.first;
                uint32_t beamId = mac.first.second;
                os << "    " << mac.second->GetAddress() << ", sat: " << satId
                   << ", beam: " << beamId << std::endl;
            }
        }
    }

    os << "UTs" << std::endl;
    Ptr<Node> utNode;
    for (NodeContainer::Iterator itUt = m_uts.Begin(); itUt != m_uts.End(); itUt++)
    {
        utNode = *itUt;
        const UtLayers_s utLayers = m_utLayers.at(utNode);
        os << "  UT: ID = " << utNode->GetId();
        os << ", at " << GeoCoordinate(utNode->GetObject<SatMobilityModel>()->GetPosition())
           << std::endl;
        os << "  Devices " << std::endl;
        const GwLayers_s gwLayers = m_gwLayers.at(m_utToGwMap.at(utNode));
        uint32_t utSatId = utLayers.m_satId;
        uint32_t utBeamId = utLayers.m_beamId;
        uint32_t gwSatId = gwLayers.m_satId;

        if (m_standard == SatEnums::DVB)
        {
            os << "    " << utLayers.m_dvbMac->GetAddress() << ", sat: " << utSatId
               << ", beam: " << utBeamId;
            os << ". Linked to GW "
               << gwLayers.m_dvbMac.at(std::make_pair(gwSatId, utBeamId))->GetAddress()
               << std::endl;
        }
        else
        {
            os << "    " << utLayers.m_loraMac->GetAddress() << ", sat: " << utSatId
               << ", beam: " << utBeamId;
            if (m_forwardLinkRegenerationMode == SatEnums::TRANSPARENT)
            {
                os << ". Linked to GW "
                   << gwLayers.m_loraMac.at(std::make_pair(gwSatId, utBeamId))->GetAddress()
                   << std::endl;
            }
            else
            {
                os << ". Linked to GW "
                   << gwLayers.m_dvbMac.at(std::make_pair(gwSatId, utBeamId))->GetAddress()
                   << std::endl;
            }
        }
    }

    os << "GW users" << std::endl;
    for (NodeContainer::Iterator it = m_gwUsers.Begin(); it != m_gwUsers.End(); it++)
    {
        Ptr<Node> gwUserNode = *it;
        os << "  GW user: ID = " << gwUserNode->GetId() << std::endl;
    }

    os << "UT users" << std::endl;
    for (NodeContainer::Iterator it = m_utUsers.Begin(); it != m_utUsers.End(); it++)
    {
        Ptr<Node> utUserNode = *it;
        os << "  GW user: ID = " << utUserNode->GetId() << std::endl;
    }

    os << "==================" << std::endl;
    os << std::endl;
}

uint32_t
SatTopology::GetClosestSat(GeoCoordinate position)
{
    NS_LOG_FUNCTION(this << position);

    double distanceMin = std::numeric_limits<double>::max();
    uint32_t indexDistanceMin = 0;

    for (uint32_t i = 0; i < m_orbiters.GetN(); i++)
    {
        GeoCoordinate satPos = m_orbiters.Get(i)->GetObject<SatMobilityModel>()->GetGeoPosition();
        double distance = CalculateDistance(position.ToVector(), satPos.ToVector());
        if (distance < distanceMin)
        {
            distanceMin = distance;
            indexDistanceMin = i;
        }
    }
    return indexDistanceMin;
}

} // namespace ns3
