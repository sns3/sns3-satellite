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
#ifndef SATELLITE_TOPOLOGY_H
#define SATELLITE_TOPOLOGY_H

#include "lorawan-ground-mac-gateway.h"
#include "lorawan-mac-end-device.h"
#include "lorawan-mac-gateway.h"
#include "lorawan-orbiter-mac-gateway.h"
#include "satellite-gw-llc.h"
#include "satellite-gw-mac.h"
#include "satellite-gw-phy.h"
#include "satellite-net-device.h"
#include "satellite-orbiter-feeder-llc.h"
#include "satellite-orbiter-feeder-mac.h"
#include "satellite-orbiter-feeder-phy.h"
#include "satellite-orbiter-net-device.h"
#include "satellite-orbiter-user-llc.h"
#include "satellite-orbiter-user-mac.h"
#include "satellite-orbiter-user-phy.h"
#include "satellite-ut-llc.h"
#include "satellite-ut-mac.h"
#include "satellite-ut-phy.h"

#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object.h"

#include <map>
#include <ostream>
#include <stdint.h>
#include <utility>

namespace ns3
{

/**
 * @ingroup satellite
 *
 * @brief Class to store topology of the whole system. It has the information of all the nodes of
 * the scenario, and all the links between them.
 */
class SatTopology : public Object
{
  public:
    typedef struct
    {
        uint32_t m_satId;
        uint32_t m_beamId;
        std::map<std::pair<uint32_t, uint32_t>, Ptr<SatNetDevice>> m_netDevice;
        std::map<std::pair<uint32_t, uint32_t>, Ptr<SatGwLlc>> m_llc;
        std::map<std::pair<uint32_t, uint32_t>, Ptr<SatGwMac>> m_dvbMac;
        std::map<std::pair<uint32_t, uint32_t>, Ptr<LorawanGroundMacGateway>> m_loraMac;
        std::map<std::pair<uint32_t, uint32_t>, Ptr<SatGwPhy>> m_phy;
    } GwLayers_s;

    typedef struct
    {
        uint32_t m_satId;
        uint32_t m_beamId;
        uint32_t m_groupId;
        Ptr<SatNetDevice> m_netDevice;
        Ptr<SatUtLlc> m_llc;
        Ptr<SatUtMac> m_dvbMac;
        Ptr<LorawanMacEndDevice> m_loraMac;
        Ptr<SatUtPhy> m_phy;
    } UtLayers_s;

    typedef struct
    {
        uint32_t m_satId;
        Ptr<SatOrbiterNetDevice> m_netDevice;
        std::map<uint32_t, Ptr<SatOrbiterFeederLlc>> m_feederLlc;
        std::map<uint32_t, Ptr<SatOrbiterUserLlc>> m_userLlc;
        std::map<uint32_t, Ptr<SatOrbiterFeederMac>> m_feederMac;
        std::map<uint32_t, Ptr<SatOrbiterUserMac>> m_dvbUserMac;
        std::map<uint32_t, Ptr<LorawanOrbiterMacGateway>> m_loraUserMac;
        std::map<uint32_t, Ptr<SatOrbiterFeederPhy>> m_feederPhy;
        std::map<uint32_t, Ptr<SatOrbiterUserPhy>> m_userPhy;
    } OrbiterLayers_s;

    /**
     * @brief Constructor
     */
    SatTopology();

    /**
     * @brief Destructor
     */
    ~SatTopology();

    /**
     * @brief NS-3 type id function
     * @return type id
     */
    static TypeId GetTypeId(void);

    /**
     *  @brief Do needed dispose actions.
     */
    void DoDispose();

    /**
     * @brief Function for resetting the variables
     */
    void Reset();

    /**
     * Set standard used (DVB or LORA)
     *
     * @param standard Standard to use
     */
    void SetStandard(SatEnums::Standard_t standard);

    /**
     * Get standard used (DVB or LORA)
     *
     * @return Standard to use
     */
    SatEnums::Standard_t GetStandard();

    /**
     * Set forward link regeneration mode
     *
     * @param forwardLinkRegenerationMode Forward link regeneration mode
     */
    void SetForwardLinkRegenerationMode(SatEnums::RegenerationMode_t forwardLinkRegenerationMode);

    /**
     * Get forward link regeneration mode
     *
     * @return Forward link regeneration mode
     */
    SatEnums::RegenerationMode_t GetForwardLinkRegenerationMode();

    /**
     * Set return link regeneration mode
     *
     * @param returnLinkRegenerationMode Return link regeneration mode
     */
    void SetReturnLinkRegenerationMode(SatEnums::RegenerationMode_t returnLinkRegenerationMode);

    /**
     * Get return link regeneration mode
     *
     * @return Return link regeneration mode
     */
    SatEnums::RegenerationMode_t GetReturnLinkRegenerationMode();

    /**
     * Add a GW node to the topology
     *
     * @param gwId ID of the GW
     * @param gw The GW node to add
     */
    void AddGwNode(uint32_t gwId, Ptr<Node> gw);

    /**
     * Add a UT node to the topology
     *
     * @param ut The UT node to add
     */
    void AddUtNode(Ptr<Node> ut);

    /**
     * Add an orbiter node to the topology
     *
     * @param orbiter The orbiter node to add
     */
    void AddOrbiterNode(Ptr<Node> orbiter);

    /**
     * Add a GW user node to the topology
     *
     * @param gwUser The GW user node to add
     */
    void AddGwUserNode(Ptr<Node> gwUser);

    /**
     * Add a UT user node to the topology
     *
     * @param utUser The UT user node to add
     * @param ut Associated UT node
     */
    void AddUtUserNode(Ptr<Node> utUser, Ptr<Node> ut);

    /**
     * Connect a GW to a UT. UT must not have an associated GW yet
     *
     * @param ut The UT to consider
     * @param gw The GW connected to this UT
     */
    void ConnectGwToUt(Ptr<Node> ut, Ptr<Node> gw);

    /**
     * Connect a new GW to a UT. UT must already have an associated GW
     *
     * @param ut The UT to consider
     * @param gw The GW connected to this UT
     */
    void UpdateGwConnectedToUt(Ptr<Node> ut, Ptr<Node> gw);

    /**
     * Disconnect a GW from a UT. UT must already have an associated GW
     *
     * @param ut The UT to consider
     */
    void DisconnectGwFromUt(Ptr<Node> ut);

    /**
     * Get GW connected to a given UT. UT must already have an associated GW
     *
     * @param ut The UT to consider
     *
     * @return The associated GW
     */
    Ptr<Node> GetGwFromUt(Ptr<Node> ut) const;

    /**
     * Connect a GW to a Beam. Beam must not have an associated GW yet
     *
     * @param beamId The beam ID to consider
     * @param gw The GW connected to this beam
     */
    void ConnectGwToBeam(uint32_t beamId, Ptr<Node> gw);

    /**
     * Get GW connected to a given beam ID. Beam must already have an associated GW
     *
     * @param beamId The beam ID to consider
     *
     * @return The associated GW
     */
    Ptr<Node> GetGwFromBeam(uint32_t beamId) const;

    /**
     * Create an association between a feeder MAC layer, and the entity really used
     *
     * @param mac The Feeder MAC to pair
     * @param usedMac The Feeder MAC that is really linked to a GW
     */
    void AddOrbiterFeederMacPair(Ptr<SatOrbiterFeederMac> mac, Ptr<SatOrbiterFeederMac> usedMac);

    /**
     * Get feeder MAC really used to connect to a GW
     *
     * @param mac The Feeder MAC wanted
     *
     * @return The Feeder MAC that is really linked to a GW
     */
    Ptr<SatOrbiterFeederMac> GetOrbiterFeederMacUsed(Ptr<SatOrbiterFeederMac> mac) const;

    /**
     * Set the value of GW address for a UT.
     * This method is called when using constellations, and can be called via callbacks after
     * handovers
     *
     * @param utId ID of UT to
     * @return MAC address of GW
     */
    Mac48Address GetGwAddressInUt(uint32_t utId);

    /**
     * Get the list of GW nodes
     *
     * @return The list of GW nodes
     */
    NodeContainer GetGwNodes() const;

    /**
     * Get the list of UT nodes
     *
     * @return The list of UT nodes
     */
    NodeContainer GetUtNodes() const;

    /**
     * Get the list of orbiter nodes
     *
     * @return The list of orbiter nodes
     */
    NodeContainer GetOrbiterNodes() const;

    /**
     * Get the list of GW user nodes
     *
     * @return The list of GW user nodes
     */
    NodeContainer GetGwUserNodes() const;

    /**
     * Get the list of UT user nodes
     *
     * @return The list of UT user nodes
     */
    NodeContainer GetUtUserNodes() const;

    /**
     * Get the list of UT user nodes connected to UTs
     *
     * @param uts List of UT nodes
     *
     * @return The list of UT user nodes
     */
    NodeContainer GetUtUserNodes(NodeContainer uts) const;

    /**
     * Get the list of UT user nodes connected to on UT
     *
     * @param ut UT node
     *
     * @return The list of UT user nodes
     */
    NodeContainer GetUtUserNodes(Ptr<Node> ut) const;

    /**
     * Get UT node linked to some UT user
     *
     * @param utUser Pointer to the UT user node
     *
     * @return a pointer to the UT node which serves the specified UT user node,
     *         or nullptr if the UT user node is invalid
     */
    Ptr<Node> GetUtNode(Ptr<Node> utUser) const;

    /**
     * Get the number of GW nodes
     *
     * @return The number of GW nodes
     */
    uint32_t GetNGwNodes() const;

    /**
     * Get the number of UT nodes
     *
     * @return The number of UT nodes
     */
    uint32_t GetNUtNodes() const;

    /**
     * Get the number of orbiter nodes
     *
     * @return The number of orbiter nodes
     */
    uint32_t GetNOrbiterNodes() const;

    /**
     * Get the number of GW user nodes
     *
     * @return The number of GW user nodes
     */
    uint32_t GetNGwUserNodes() const;

    /**
     * Get the number of UT user nodes
     *
     * @return The number of UT user nodes
     */
    uint32_t GetNUtUserNodes() const;

    /**
     * Get the wanted GW node
     *
     * @param nodeId ID of the node needed (index in vector)
     *
     * @return The GW node
     */
    Ptr<Node> GetGwNode(uint32_t nodeId) const;

    /**
     * Get the wanted UT node
     *
     * @param nodeId ID of the node needed
     *
     * @return The UT node
     */
    Ptr<Node> GetUtNode(uint32_t nodeId) const;

    /**
     * Get the wanted orbiter node
     *
     * @param nodeId ID of the node needed
     *
     * @return The orbiter node
     */
    Ptr<Node> GetOrbiterNode(uint32_t nodeId) const;

    /**
     * Get the wanted GW user node
     *
     * @param nodeId ID of the node needed
     *
     * @return The GW user node
     */
    Ptr<Node> GetGwUserNode(uint32_t nodeId) const;

    /**
     * Get the wanted UT user node
     *
     * @param nodeId ID of the node needed
     *
     * @return The UT user node
     */
    Ptr<Node> GetUtUserNode(uint32_t nodeId) const;

    /**
     * Get the wanted node from its node ID
     *
     * @param nodeId ID of the node needed
     *
     * @return The node
     */
    Ptr<Node> GetNodeFromId(uint32_t nodeId) const;

    /**
     * Add DVB GW layers for given node, associated to chosen satellite and beam
     *
     * @param gw GW node to consider
     * @param gwSatId ID of satellite linked to this stack
     * @param gwBeamId ID of beam linked to this stack
     * @param utSatId ID of satellite serving UTs for this stack
     * @param utBeamId ID of beam serving UTs for this stack
     * @param netDevice SatNetDevice for pair UT satellite/UT beam
     * @param llc LLC layer for pair UT satellite/UT beam
     * @param mac MAC layer for pair UT satellite/UT beam
     * @param phy PHY layer for pair UT satellite/UT beam
     */
    void AddGwLayersDvb(Ptr<Node> gw,
                        uint32_t gwSatId,
                        uint32_t gwBeamId,
                        uint32_t utSatId,
                        uint32_t utBeamId,
                        Ptr<SatNetDevice> netDevice,
                        Ptr<SatGwLlc> llc,
                        Ptr<SatGwMac> mac,
                        Ptr<SatGwPhy> phy);

    /**
     * Add LORA GW layers for given node, associated to chosen satellite and beam
     *
     * @param gw GW node to consider
     * @param gwSatId ID of satellite linked to this stack
     * @param gwBeamId ID of beam linked to this stack
     * @param utSatId ID of satellite serving UTs for this stack
     * @param utBeamId ID of beam serving UTs for this stack
     * @param netDevice SatNetDevice for pair UT satellite/UT beam
     * @param llc LLC layer for pair UT satellite/UT beam
     * @param mac MAC layer for pair UT satellite/UT beam
     * @param phy PHY layer for pair UT satellite/UT beam
     */
    void AddGwLayersLora(Ptr<Node> gw,
                         uint32_t gwSatId,
                         uint32_t gwBeamId,
                         uint32_t utSatId,
                         uint32_t utBeamId,
                         Ptr<SatNetDevice> netDevice,
                         Ptr<LorawanGroundMacGateway> mac,
                         Ptr<SatGwPhy> phy);

    /**
     * Update satellite and beam associated to a GW
     *
     * @param gw GW node to consider
     * @param satId ID of satellite linked to this stack
     * @param beamId ID of beam linked to this stack
     */
    void UpdateGwSatAndBeam(Ptr<Node> gw, uint32_t satId, uint32_t beamId);

    /**
     * Get ID of satellite linked to a GW
     *
     * @param gw GW node to consider
     *
     * @return ID of satellite linked to this node
     */
    uint32_t GetGwSatId(Ptr<Node> gw) const;

    /**
     * Get ID of beam linked to a GW
     *
     * @param gw GW node to consider
     *
     * @return ID of beam linked to this node
     */
    uint32_t GetGwBeamId(Ptr<Node> gw) const;

    /**
     * Get SatNetDevice instance of a GW
     *
     * @param gw GW node to consider
     * @param utSatId ID of satellite serving the UTs
     * @param utBeamId ID of beam serving the UTs
     *
     * @return SatNetDevice instance of a GW
     */
    Ptr<SatNetDevice> GetGwNetDevice(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const;

    /**
     * Get SatGwLlc instance of a GW
     *
     * @param gw GW node to consider
     * @param utSatId ID of satellite serving the UTs
     * @param utBeamId ID of beam serving the UTs
     *
     * @return SatGwLlc instance of a GW
     */
    Ptr<SatGwLlc> GetGwLlc(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const;

    /**
     * Get SatGwMac instance of a DVB GW
     *
     * @param gw GW node to consider
     * @param utSatId ID of satellite serving the UTs
     * @param utBeamId ID of beam serving the UTs
     *
     * @return SatGwMac instance of a GW
     */
    Ptr<SatGwMac> GetDvbGwMac(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const;

    /**
     * Get SatGwMac instance of a LORA GW
     *
     * @param gw GW node to consider
     * @param utSatId ID of satellite serving the UTs
     * @param utBeamId ID of beam serving the UTs
     *
     * @return LorawanGroundMacGateway instance of a GW
     */
    Ptr<LorawanGroundMacGateway> GetLoraGwMac(Ptr<Node> gw,
                                              uint32_t utSatId,
                                              uint32_t utBeamId) const;

    /**
     * Get SatGwPhy instance of a GW
     *
     * @param gw GW node to consider
     * @param utSatId ID of satellite serving the UTs
     * @param utBeamId ID of beam serving the UTs
     *
     * @return SatGwPhy instance of a GW
     */
    Ptr<SatGwPhy> GetGwPhy(Ptr<Node> gw, uint32_t utSatId, uint32_t utBeamId) const;

    /**
     * Add DVB UT layers for given node, associated to chosen satellite and beam
     *
     * @param ut UT node to consider
     * @param satId ID of satellite linked to this stack
     * @param beamId ID of beam linked to this stack
     * @param groupId ID of group linked to this stack
     * @param netDevice SatNetDevice of this node
     * @param llc LLC layer of this node
     * @param mac MAC layer of this node
     * @param phy PHY layer of this node
     */
    void AddUtLayersDvb(Ptr<Node> ut,
                        uint32_t satId,
                        uint32_t beamId,
                        uint32_t groupId,
                        Ptr<SatNetDevice> netDevice,
                        Ptr<SatUtLlc> llc,
                        Ptr<SatUtMac> mac,
                        Ptr<SatUtPhy> phy);

    /**
     * Add LORA UT layers for given node, associated to chosen satellite and beam
     *
     * @param ut UT node to consider
     * @param satId ID of satellite linked to this stack
     * @param beamId ID of beam linked to this stack
     * @param groupId ID of group linked to this stack
     * @param netDevice SatNetDevice of this node
     * @param llc LLC layer of this node
     * @param mac MAC layer of this node
     * @param phy PHY layer of this node
     */
    void AddUtLayersLora(Ptr<Node> ut,
                         uint32_t satId,
                         uint32_t beamId,
                         uint32_t groupId,
                         Ptr<SatNetDevice> netDevice,
                         Ptr<LorawanMacEndDevice> mac,
                         Ptr<SatUtPhy> phy);

    /**
     * Update satellite and beam associated to a UT
     *
     * @param ut UT node to consider
     * @param satId ID of satellite linked to this stack
     * @param beamId ID of beam linked to this stack
     */
    void UpdateUtSatAndBeam(Ptr<Node> ut, uint32_t satId, uint32_t beamId);

    /**
     * Update satellite and beam associated to a UT
     *
     * @param ut UT node to consider
     * @param groupId ID of group linked to this stack
     */
    void UpdateUtGroup(Ptr<Node> ut, uint32_t groupId);

    /**
     * Get ID of satellite linked to a UT
     *
     * @param ut UT node to consider
     *
     * @return ID of satellite linked to this node
     */
    uint32_t GetUtSatId(Ptr<Node> ut) const;

    /**
     * Get ID of beam linked to a UT
     *
     * @param ut UT node to consider
     *
     * @return ID of beam linked to this node
     */
    uint32_t GetUtBeamId(Ptr<Node> ut) const;

    /**
     * Get ID of group linked to a UT
     *
     * @param ut UT node to consider
     *
     * @return ID of group linked to this node
     */
    uint32_t GetUtGroupId(Ptr<Node> ut) const;

    /**
     * Get SatNetDevice instance of a UT
     *
     * @param ut UT node to consider
     *
     * @return SatNetDevice instance of a UT
     */
    Ptr<SatNetDevice> GetUtNetDevice(Ptr<Node> ut) const;

    /**
     * Get SatUtLlc instance of a UT
     *
     * @param ut UT node to consider
     *
     * @return SatUtLlc instance of a UT
     */
    Ptr<SatUtLlc> GetUtLlc(Ptr<Node> ut) const;

    /**
     * Get SatUtMac instance of a DVB UT
     *
     * @param ut UT node to consider
     *
     * @return SatUtMac instance of a UT
     */
    Ptr<SatUtMac> GetDvbUtMac(Ptr<Node> ut) const;

    /**
     * Get SatUtMac instance of a LORA UT
     *
     * @param ut UT node to consider
     *
     * @return LorawanMacEndDevice instance of a UT
     */
    Ptr<LorawanMacEndDevice> GetLoraUtMac(Ptr<Node> ut) const;

    /**
     * Get SatUtPhy instance of a UT
     *
     * @param ut UT node to consider
     *
     * @return SatUtPhy instance of a UT
     */
    Ptr<SatUtPhy> GetUtPhy(Ptr<Node> ut) const;

    /**
     * Add orbiter feeder layers for given satellite and beam ID
     *
     * @param orbiter UT node to consider
     * @param satId ID of satellite linked to this stack
     * @param utSatId ID of UT satellite linked to this stack
     * @param utBeamId ID of UT beam linked to this stack
     * @param netDevice SatNetDevice of this node
     * @param mac LLC layer of this node
     * @param mac MAC layer of this node
     * @param phy PHY layer of this node
     */
    void AddOrbiterFeederLayers(Ptr<Node> orbiter,
                                uint32_t satId,
                                uint32_t utBeamId,
                                Ptr<SatOrbiterNetDevice> netDevice,
                                Ptr<SatOrbiterFeederLlc> llc,
                                Ptr<SatOrbiterFeederMac> mac,
                                Ptr<SatOrbiterFeederPhy> phy);

    /**
     * Add DVB orbiter user layers for given satellite and beam ID
     *
     * @param orbiter UT node to consider
     * @param satId ID of satellite linked to this stack
     * @param beamId ID of beam linked to this stack
     * @param netDevice SatNetDevice of this node
     * @param mac LLC layer of this node
     * @param mac MAC layer of this node
     * @param phy PHY layer of this node
     */
    void AddOrbiterUserLayersDvb(Ptr<Node> orbiter,
                                 uint32_t satId,
                                 uint32_t beamId,
                                 Ptr<SatOrbiterNetDevice> netDevice,
                                 Ptr<SatOrbiterUserLlc> llc,
                                 Ptr<SatOrbiterUserMac> mac,
                                 Ptr<SatOrbiterUserPhy> phy);

    /**
     * Add LORA orbiter user layers for given satellite and beam ID
     *
     * @param orbiter UT node to consider
     * @param satId ID of satellite linked to this stack
     * @param beamId ID of beam linked to this stack
     * @param netDevice SatNetDevice of this node
     * @param mac LLC layer of this node
     * @param mac MAC layer of this node
     * @param phy PHY layer of this node
     */
    void AddOrbiterUserLayersLora(Ptr<Node> orbiter,
                                  uint32_t satId,
                                  uint32_t beamId,
                                  Ptr<SatOrbiterNetDevice> netDevice,
                                  Ptr<LorawanOrbiterMacGateway> mac,
                                  Ptr<SatOrbiterUserPhy> phy);

    /**
     * Get ID of a given orbiter
     *
     * @param orbiter Orbiter node to consider
     *
     * @return ID of orbiter linked to this node
     */
    uint32_t GetOrbiterSatId(Ptr<Node> orbiter) const;

    /**
     * Get SatOrbiterNetDevice instance of an orbiter
     *
     * @param orbiter Orbiter node to consider
     *
     * @return SatOrbiterNetDevice instance of an orbiter
     */
    Ptr<SatOrbiterNetDevice> GetOrbiterNetDevice(Ptr<Node> orbiter) const;

    /**
     * Get SatOrbiterFeederLlc instance of an orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param utBeamId UT Beam ID served by this feeder LLC layer
     *
     * @return SatOrbiterFeederLlc instance of an orbiter
     */
    Ptr<SatOrbiterFeederLlc> GetOrbiterFeederLlc(Ptr<Node> orbiter, uint32_t utBeamId) const;

    /**
     * Get SatOrbiterUserLlc instance of an orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param beamId Beam ID served by this user LLC layer
     *
     * @return SatOrbiterUserLlc instance of an orbiter
     */
    Ptr<SatOrbiterUserLlc> GetOrbiterUserLlc(Ptr<Node> orbiter, uint32_t beamId) const;

    /**
     * Get SatOrbiterFeederMac instance of an orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param utBeamId UT Beam ID served by this feeder MAC layer
     *
     * @return SatOrbiterFeederMac instance of an orbiter
     */
    Ptr<SatOrbiterFeederMac> GetOrbiterFeederMac(Ptr<Node> orbiter, uint32_t utBeamId) const;

    /**
     * Get SatOrbiterUserMac instance of a DVB orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param beamId Beam ID served by this user MAC layer
     *
     * @return SatOrbiterUserMac instance of an orbiter
     */
    Ptr<SatOrbiterUserMac> GetDvbOrbiterUserMac(Ptr<Node> orbiter, uint32_t beamId) const;

    /**
     * Get LorawanOrbiterMacGateway instance of a LORA orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param beamId Beam ID served by this user MAC layer
     *
     * @return LorawanOrbiterMacGateway instance of an orbiter
     */
    Ptr<LorawanOrbiterMacGateway> GetLoraOrbiterUserMac(Ptr<Node> orbiter, uint32_t beamId) const;

    /**
     * Get SatOrbiterFeederPhy instance of an orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param utBeamId UT Beam ID served by this feeder PHY layer
     *
     * @return SatOrbiterFeederPhy instance of an orbiter
     */
    Ptr<SatOrbiterFeederPhy> GetOrbiterFeederPhy(Ptr<Node> orbiter, uint32_t utBeamId) const;

    /**
     * Get SatOrbiterUserPhy instance of an orbiter serving wanted beam ID
     *
     * @param orbiter Orbiter node to consider
     * @param beamId Beam ID served by this user physical layer
     *
     * @return SatOrbiterUserPhy instance of an orbiter
     */
    Ptr<SatOrbiterUserPhy> GetOrbiterUserPhy(Ptr<Node> orbiter, uint32_t beamId) const;

    /**
     * @brief Function for enabling the map prints
     */
    void EnableMapPrint(bool enableMapPrint)
    {
        m_enableMapPrint = enableMapPrint;
    }

    /**
     * Print all the satellite topology
     * @param os output stream in which the data should be printed
     */
    void PrintTopology(std::ostream& os) const;

    /**
     * Get closest satellite to a ground station
     * @param position The position of the ground station
     * @return The ID of the closest satellite
     */
    uint32_t GetClosestSat(GeoCoordinate position);

  private:
    SatEnums::Standard_t m_standard;                            // Standard used
    SatEnums::RegenerationMode_t m_forwardLinkRegenerationMode; // Regeneration mode on forward link
    SatEnums::RegenerationMode_t m_returnLinkRegenerationMode;  // Regeneration mode on return link

    std::map<uint32_t, Ptr<Node>> m_gwIds;                 // List of GW nodes
    NodeContainer m_gws;                                   // List of GW nodes
    NodeContainer m_uts;                                   // List of UT nodes
    NodeContainer m_orbiters;                              // List of orbiter nodes
    NodeContainer m_gwUsers;                               // List of GW user nodes
    NodeContainer m_utUsers;                               // List of UT user nodes
    std::map<Ptr<Node>, NodeContainer> m_detailledUtUsers; // Map associating UT to UT users

    std::map<Ptr<Node>, GwLayers_s> m_gwLayers; // Map giving protocol layers for all GWs
    std::map<Ptr<Node>, UtLayers_s> m_utLayers; // Map giving protocol layers for all UTs
    std::map<Ptr<Node>, OrbiterLayers_s>
        m_orbiterLayers; // Map giving protocol layers for all orbiters

    std::map<Ptr<Node>, Ptr<Node>> m_utToGwMap;  // Map of GW connected for each UT
    std::map<uint32_t, Ptr<Node>> m_beamToGwMap; // Map of GW connected for each satellite beam
    std::map<Ptr<SatOrbiterFeederMac>, Ptr<SatOrbiterFeederMac>>
        m_orbiterFeederMacMap; // Map giving which Feeder Mac is really used

    bool m_enableMapPrint; // Is map printing enabled or not
};

} // namespace ns3

#endif /* SATELLITE_TOPOLOGY_H */
