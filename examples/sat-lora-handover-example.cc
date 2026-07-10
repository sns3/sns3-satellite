/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 *
 */

#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * @file sat-lora-handover-example.cc
 * @ingroup satellite
 *
 * @brief  Test handover for both GW and UT when using regenerative LORA
 *
 */

NS_LOG_COMPONENT_DEFINE("sat-lora-handover-example");

int
main(int argc, char* argv[])
{
    Time appStartTime = Seconds(0.001);
    Time simLength = Seconds(100);

    uint32_t packetSize = 24;
    Time loraInterval = Seconds(10);
    std::string interval = "10s";

    double frameAllocatedBandwidthHz = 15000;
    double frameCarrierAllocatedBandwidthHz = 15000;
    double frameCarrierRollOff = 0.22;
    double frameCarrierSpacing = 0;
    uint32_t frameSpreadingFactor = 256;

    Time firstWindowDelay = MilliSeconds(1000);
    Time secondWindowDelay = Seconds(2);
    Time firstWindowDuration = MilliSeconds(900);
    Time secondWindowDuration = MilliSeconds(400);
    Time firstWindowAnswerDelay = Seconds(1);
    Time secondWindowAnswerDelay = Seconds(2);

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("example-lora-handover");

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    // Enable Lora
    Config::SetDefault("ns3::LorawanMacEndDevice::DataRate", UintegerValue(5));
    Config::SetDefault("ns3::LorawanMacEndDevice::MType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));

    // Config::SetDefault ("ns3::SatLoraConf::Standard", EnumValue (SatLoraConf::EU863_870));
    Config::SetDefault("ns3::SatLoraConf::Standard", EnumValue(SatLoraConf::SATELLITE));

    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDelay",
                       TimeValue(firstWindowDelay));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDelay",
                       TimeValue(secondWindowDelay));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::FirstWindowDuration",
                       TimeValue(firstWindowDuration));
    Config::SetDefault("ns3::LorawanMacEndDeviceClassA::SecondWindowDuration",
                       TimeValue(secondWindowDuration));
    Config::SetDefault("ns3::LoraNetworkScheduler::FirstWindowAnswerDelay",
                       TimeValue(firstWindowAnswerDelay));
    Config::SetDefault("ns3::LoraNetworkScheduler::SecondWindowAnswerDelay",
                       TimeValue(secondWindowAnswerDelay));

    // Defaults
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));
    Config::SetDefault("ns3::SatHelper::PacketTraceEnabled", BooleanValue(true));

    // Superframe configuration
    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0",
                       EnumValue(SatSuperframeConf::SUPER_FRAME_CONFIG_4));
    Config::SetDefault("ns3::SatSuperframeConf4::FrameConfigType",
                       EnumValue(SatSuperframeConf::CONFIG_TYPE_4));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_AllocatedBandwidthHz",
                       DoubleValue(frameAllocatedBandwidthHz));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierAllocatedBandwidthHz",
                       DoubleValue(frameCarrierAllocatedBandwidthHz));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierRollOff",
                       DoubleValue(frameCarrierRollOff));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_CarrierSpacing",
                       DoubleValue(frameCarrierSpacing));
    Config::SetDefault("ns3::SatSuperframeConf4::Frame0_SpreadingFactor",
                       UintegerValue(frameSpreadingFactor));

    // CRDSA only
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaServiceCount", UintegerValue(4));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed",
                       BooleanValue(false));

    // Configure RA
    Config::SetDefault("ns3::SatOrbiterHelper::FwdLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatOrbiterHelper::RtnLinkErrorModel",
                       EnumValue(SatPhyRxCarrierConf::EM_AVI));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_ESSA));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::ReturnLinkLinkResults", EnumValue(SatEnums::LR_LORA));

    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDuration", StringValue("600ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowStep", StringValue("200ms"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowDelay", StringValue("0s"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::FirstWindow", StringValue("0s"));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::WindowSICIterations", UintegerValue(5));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::SpreadingFactor", UintegerValue(1));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::DetectionThreshold", DoubleValue(0));
    Config::SetDefault("ns3::SatPhyRxCarrierPerWindow::EnableSIC", BooleanValue(false));

    // Set random access parameters
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_MaximumUniquePayloadPerBlock",
                       UintegerValue(3));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_MaximumConsecutiveBlockAccessed",
                       UintegerValue(6));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_MinimumIdleBlock",
                       UintegerValue(2));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_BackOffTimeInMilliSeconds",
                       UintegerValue(50));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_BackOffProbability",
                       UintegerValue(1));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_HighLoadBackOffProbability",
                       UintegerValue(1));
    Config::SetDefault(
        "ns3::SatLowerLayerServiceConf::RaService0_AverageNormalizedOfferedLoadThreshold",
        DoubleValue(0.99));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances",
                       UintegerValue(3));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_SlottedAlohaAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_CrdsaAllowed",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_EssaAllowed", BooleanValue(true));

    /// Set regeneration mode
    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Config::SetDefault("ns3::SatHelper::HandoversEnabled", BooleanValue(true));
    Config::SetDefault("ns3::SatHandoverModule::NumberClosestSats", UintegerValue(2));

    Config::SetDefault("ns3::SatGwMac::DisableSchedulingIfNoDeviceConnected", BooleanValue(true));
    Config::SetDefault("ns3::SatOrbiterMac::DisableSchedulingIfNoDeviceConnected",
                       BooleanValue(true));

    /// Set simulation output details
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    /// Enable packet trace
    Config::SetDefault("ns3::SatHelper::PacketTraceEnabled", BooleanValue(true));

    Ptr<SimulationHelperConf> simulationConf = CreateObject<SimulationHelperConf>();
    simulationHelper->SetSimulationTime(simLength);
    simulationHelper->SetGwUserCount(1);
    simulationHelper->SetUserCountPerUt(1);
    std::set<uint32_t> beamSetAll = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                                     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                                     31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
                                     46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
                                     61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72};
    simulationHelper->SetBeamSet(beamSetAll);
    simulationHelper->SetUserCountPerMobileUt(simulationConf->m_utMobileUserCount);

    simulationHelper->LoadScenario("constellation-leo-2-satellites-lora");

    simulationHelper->CreateSatScenario(SatHelper::NONE);

    simulationHelper->GetTrafficHelper()->AddLoraCbrTraffic(
        loraInterval,
        packetSize,
        Singleton<SatTopology>::Get()->GetGwUserNode(0),
        Singleton<SatTopology>::Get()->GetUtUserNodes(),
        appStartTime,
        simLength,
        Seconds(3));

    /*simulationHelper->GetTrafficHelper()->AddLoraPeriodicTraffic(
        loraInterval,
        packetSize,
        Singleton<SatTopology>::Get()->GetUtNodes(),
        appStartTime,
        simLength,
        Seconds(3));*/

    // Outputs
    std::string outputPath = SatEnvVariables::GetInstance()->LocateDirectory(
        "contrib/satellite/data/sims/example-lora-handover");
    Config::SetDefault("ns3::ConfigStore::Filename",
                       StringValue(outputPath + "/output-attributes.xml"));
    Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("Xml"));
    Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Save"));
    ConfigStore outputConfig;
    outputConfig.ConfigureDefaults();

    Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer();

    s->AddGlobalFeederEssaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalFeederEssaPacketError(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtFeederEssaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtFeederEssaPacketError(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalFeederEssaPacketCollision(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalFeederEssaPacketCollision(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtFeederEssaPacketCollision(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtFeederEssaPacketCollision(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalRtnFeederWindowLoad(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnFeederWindowLoad(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerBeamRtnFeederWindowLoad(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerBeamRtnFeederWindowLoad(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddGlobalRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddPerUtRtnAppDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtRtnMacDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtRtnAppDelay(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtRtnMacDelay(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalRtnCompositeSinr(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnCompositeSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtRtnCompositeSinr(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtRtnCompositeSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalRtnFeederLinkSinr(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnFeederLinkSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddGlobalRtnUserLinkSinr(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnUserLinkSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalRtnFeederLinkRxPower(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnFeederLinkRxPower(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddGlobalRtnUserLinkRxPower(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalRtnUserLinkRxPower(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddGlobalFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddGlobalFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddGlobalFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    s->AddPerUtFwdAppDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtFwdMacDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    s->AddPerUtFwdAppDelay(SatStatsHelper::OUTPUT_SCATTER_FILE);
    s->AddPerUtFwdMacDelay(SatStatsHelper::OUTPUT_SCATTER_FILE);

    simulationHelper->EnableProgressLogs();
    simulationHelper->RunSimulation();

    return 0;
}
