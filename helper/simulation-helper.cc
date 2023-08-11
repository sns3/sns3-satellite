/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
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
 * Author: Lauri Sormunen <lauri.sormunen@magister.fi>
 * Author: Mathias Ettinger <mettinger@viveris.toulouse.fr>
 * Modified by: Patrice Raveneau <patrice.raveneau@cnes.fr>
 *
 */

#include "simulation-helper.h"

#include <ns3/address.h>
#include <ns3/cbr-helper.h>
#include <ns3/config-store.h>
#include <ns3/config.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/lora-periodic-sender.h>
#include <ns3/nrtv-helper.h>
#include <ns3/packet-sink-helper.h>
#include <ns3/packet-sink.h>
#include <ns3/pointer.h>
#include <ns3/random-variable-stream.h>
#include <ns3/satellite-env-variables.h>
#include <ns3/satellite-on-off-helper.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/three-gpp-http-satellite-helper.h>
#include <ns3/uinteger.h>

NS_LOG_COMPONENT_DEFINE("SimulationHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SimulationHelperConf);

/**
 * SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES macro helps defining specific attribute
 * for traffic models in method GetTypeId.
 *
 * \param index Name of the traffic model which attributes are added to the configuration.
 * \param a1    'Percentage of users' attribute value
 * \param a2    'Transport layer protocol' attribute value
 * \param a3    'Traffic direction' attribute value
 * \param a4    'Start time' attribute value
 * \param a5    'Stop time' attribute value
 * \param a6    'Start delay' attribute value
 *
 * \return TypeId
 */
#define SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES(index, a1, a2, a3, a4, a5, a6)                            \
    AddAttribute("Traffic" TOSTRING(index) "Percentage",                                           \
                 "Percentage of final users that will use this traffic model",                     \
                 DoubleValue(a1),                                                                  \
                 MakeDoubleAccessor(&SimulationHelperConf::SetTraffic##index##Percentage,          \
                                    &SimulationHelperConf::GetTraffic##index##Percentage),         \
                 MakeDoubleChecker<double>(0, 1))                                                  \
        .AddAttribute("Traffic" TOSTRING(index) "Protocol",                                        \
                      "Network protocol that this traffic model will use",                         \
                      EnumValue(a2),                                                               \
                      MakeEnumAccessor(&SimulationHelperConf::SetTraffic##index##Protocol,         \
                                       &SimulationHelperConf::GetTraffic##index##Protocol),        \
                      MakeEnumChecker(SimulationHelperConf::PROTOCOL_UDP,                          \
                                      "UDP",                                                       \
                                      SimulationHelperConf::PROTOCOL_TCP,                          \
                                      "TCP",                                                       \
                                      SimulationHelperConf::PROTOCOL_BOTH,                         \
                                      "BOTH"))                                                     \
        .AddAttribute("Traffic" TOSTRING(index) "Direction",                                       \
                      "Satellite link direction that this traffic model will use",                 \
                      EnumValue(a3),                                                               \
                      MakeEnumAccessor(&SimulationHelperConf::SetTraffic##index##Direction,        \
                                       &SimulationHelperConf::GetTraffic##index##Direction),       \
                      MakeEnumChecker(SimulationHelperConf::RTN_LINK,                              \
                                      "ReturnLink",                                                \
                                      SimulationHelperConf::FWD_LINK,                              \
                                      "ForwardLink",                                               \
                                      SimulationHelperConf::BOTH_LINK,                             \
                                      "BothLinks"))                                                \
        .AddAttribute(                                                                             \
            "Traffic" TOSTRING(index) "StartTime",                                                 \
            "Time into the simulation when this traffic model will be started on each user",       \
            TimeValue(a4),                                                                         \
            MakeTimeAccessor(&SimulationHelperConf::SetTraffic##index##StartTime,                  \
                             &SimulationHelperConf::GetTraffic##index##StartTime),                 \
            MakeTimeChecker(Seconds(0)))                                                           \
        .AddAttribute("Traffic" TOSTRING(index) "StopTime",                                        \
                      "Time into the simulation when this traffic model will be stopped "          \
                      "on each user. 0 means endless traffic generation.",                         \
                      TimeValue(a5),                                                               \
                      MakeTimeAccessor(&SimulationHelperConf::SetTraffic##index##StopTime,         \
                                       &SimulationHelperConf::GetTraffic##index##StopTime),        \
                      MakeTimeChecker(Seconds(0)))                                                 \
        .AddAttribute("Traffic" TOSTRING(index) "StartDelay",                                      \
                      "Cummulative delay for each user before starting this traffic model",        \
                      TimeValue(a6),                                                               \
                      MakeTimeAccessor(&SimulationHelperConf::SetTraffic##index##StartDelay,       \
                                       &SimulationHelperConf::GetTraffic##index##StartDelay),      \
                      MakeTimeChecker(Seconds(0)))

TypeId
SimulationHelperConf::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SimulationHelperConf")
            .SetParent<Object>()
            .AddConstructor<SimulationHelperConf>()
            .AddAttribute("SimTime",
                          "Simulation time",
                          TimeValue(Seconds(100)),
                          MakeTimeAccessor(&SimulationHelperConf::m_simTime),
                          MakeTimeChecker(Seconds(1)))
            .AddAttribute("BeamsIDs",
                          "Enabled Beams IDs",
                          StringValue("10 11 12 23 24 25"),
                          MakeStringAccessor(&SimulationHelperConf::m_enabledBeams),
                          MakeStringChecker())
            .AddAttribute("UserCountPerUt",
                          "Amount of user per User Terminal",
                          StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                          MakePointerAccessor(&SimulationHelperConf::m_utUserCount),
                          MakePointerChecker<RandomVariableStream>())
            .AddAttribute("UserCountPerMobileUt",
                          "Amount of user per mobile User Terminal",
                          StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                          MakePointerAccessor(&SimulationHelperConf::m_utMobileUserCount),
                          MakePointerChecker<RandomVariableStream>())
            .AddAttribute("UtCountPerBeam",
                          "Amount of User Terminal associated to each Beam",
                          StringValue("ns3::ConstantRandomVariable[Constant=1]"),
                          MakePointerAccessor(&SimulationHelperConf::m_utCount),
                          MakePointerChecker<RandomVariableStream>())
            .AddAttribute("ActivateStatistics",
                          "Enable outputing values from stats helpers",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SimulationHelperConf::m_activateStatistics),
                          MakeBooleanChecker())
            .AddAttribute("ActivateProgressLogs",
                          "Enable outputing progress of the simulation",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SimulationHelperConf::m_activateProgressLogging),
                          MakeBooleanChecker())
            .AddAttribute("MobileUtsFolder",
                          "Select the folder where mobile UTs traces should be found",
                          StringValue(Singleton<SatEnvVariables>::Get()->LocateDataDirectory() +
                                      "/utpositions/mobiles/"),
                          MakeStringAccessor(&SimulationHelperConf::m_mobileUtsFolder),
                          MakeStringChecker())
            .SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES(Cbr,
                                              1.0,
                                              PROTOCOL_UDP,
                                              RTN_LINK,
                                              Seconds(0.1),
                                              Seconds(0),
                                              MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES(Http,
                                              0,
                                              PROTOCOL_TCP,
                                              RTN_LINK,
                                              Seconds(0.1),
                                              Seconds(0),
                                              MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES(OnOff,
                                              0,
                                              PROTOCOL_UDP,
                                              RTN_LINK,
                                              Seconds(0.1),
                                              Seconds(0),
                                              MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES(Nrtv,
                                              0,
                                              PROTOCOL_TCP,
                                              RTN_LINK,
                                              Seconds(0.1),
                                              Seconds(0),
                                              MilliSeconds(10));
    return tid;
}

TypeId
SimulationHelperConf::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SimulationHelperConf::SimulationHelperConf()
    : m_simTime(0),
      m_enabledBeams(""),
      m_utCount(0),
      m_utUserCount(0),
      m_utMobileUserCount(0),
      m_activateStatistics(false),
      m_activateProgressLogging(false)
{
    NS_LOG_FUNCTION(this);
}

SimulationHelperConf::~SimulationHelperConf()
{
    NS_LOG_FUNCTION(this);
}

NS_OBJECT_ENSURE_REGISTERED(SimulationHelper);

TypeId
SimulationHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SimulationHelper").SetParent<Object>().AddConstructor<SimulationHelper>();
    return tid;
}

TypeId
SimulationHelper::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SimulationHelper::SimulationHelper()
    : m_satHelper(NULL),
      m_statContainer(NULL),
      m_commonUtPositions(),
      m_utPositionsByBeam(),
      m_simulationName(""),
      m_enabledBeamsStr(""),
      m_enabledBeams(),
      m_outputPath(""),
      m_utCount(),
      m_utUserCount(0),
      m_utMobileUserCount(0),
      m_simTime(0),
      m_numberOfConfiguredFrames(0),
      m_randomAccessConfigured(false),
      m_enableInputFileUtListPositions(false),
      m_inputFileUtPositionsCheckBeams(true),
      m_gwUserId(0),
      m_progressLoggingEnabled(false),
      m_progressUpdateInterval(Seconds(0.5))
{
    NS_FATAL_ERROR(
        "SimulationHelper: Default constructor not in use. Please create with simulation name. ");
}

SimulationHelper::SimulationHelper(std::string simulationName)
    : m_satHelper(NULL),
      m_statContainer(NULL),
      m_commonUtPositions(),
      m_utPositionsByBeam(),
      m_simulationName(""),
      m_enabledBeamsStr(""),
      m_enabledBeams(),
      m_outputPath(""),
      m_utCount(),
      m_utUserCount(0),
      m_utMobileUserCount(0),
      m_simTime(0),
      m_numberOfConfiguredFrames(0),
      m_randomAccessConfigured(false),
      m_enableInputFileUtListPositions(false),
      m_inputFileUtPositionsCheckBeams(true),
      m_gwUserId(0),
      m_progressLoggingEnabled(false),
      m_progressUpdateInterval(Seconds(0.5))
{
    NS_LOG_FUNCTION(this);

    ObjectBase::ConstructSelf(AttributeConstructionList());

    m_simulationName = simulationName;
    Config::SetDefault("ns3::SatEnvVariables::SimulationCampaignName",
                       StringValue(m_simulationName));
    Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue(true));

    // NCC configuration
    Config::SetDefault("ns3::SatSuperframeConf0::FrameConfigType", StringValue("ConfigType_2"));
    Config::SetDefault("ns3::SatWaveformConf::AcmEnabled", BooleanValue(true));

    // RBDC
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided",
                       BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue(true));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_MinimumServiceRate",
                       UintegerValue(16));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed",
                       BooleanValue(false));
}

SimulationHelper::~SimulationHelper()
{
    NS_LOG_FUNCTION(this);
}

void
SimulationHelper::DoDispose(void)
{
    NS_LOG_FUNCTION(this);

    m_commonUtPositions = NULL;
    m_utPositionsByBeam.clear();
}

void
SimulationHelper::SetUtCountPerBeam(uint32_t count)
{
    NS_LOG_FUNCTION(this << count);

    Ptr<RandomVariableStream> utCount = CreateObject<ConstantRandomVariable>();
    utCount->SetAttribute("Constant", DoubleValue(count));

    m_utCount.insert(std::make_pair(0, utCount));
}

void
SimulationHelper::SetUtCountPerBeam(Ptr<RandomVariableStream> rs)
{
    NS_LOG_FUNCTION(this << &rs);

    m_utCount.insert(std::make_pair(0, rs));
}

void
SimulationHelper::SetUtCountPerBeam(uint32_t beamId, uint32_t count)
{
    NS_LOG_FUNCTION(this << beamId << count);

    Ptr<RandomVariableStream> utCount = CreateObject<ConstantRandomVariable>();
    utCount->SetAttribute("Constant", DoubleValue(count));

    m_utCount.insert(std::make_pair(beamId, utCount));
}

void
SimulationHelper::SetUtCountPerBeam(uint32_t beamId, Ptr<RandomVariableStream> rs)
{
    NS_LOG_FUNCTION(this << &rs);

    m_utCount.insert(std::make_pair(beamId, rs));
}

void
SimulationHelper::SetUserCountPerUt(uint32_t count)
{
    NS_LOG_FUNCTION(this << count);

    m_utUserCount = CreateObject<ConstantRandomVariable>();
    m_utUserCount->SetAttribute("Constant", DoubleValue(count));
}

void
SimulationHelper::SetUserCountPerUt(Ptr<RandomVariableStream> rs)
{
    NS_LOG_FUNCTION(this << &rs);

    m_utUserCount = rs;
}

void
SimulationHelper::SetUserCountPerMobileUt(uint32_t count)
{
    NS_LOG_FUNCTION(this << count);

    m_utMobileUserCount = CreateObject<ConstantRandomVariable>();
    m_utMobileUserCount->SetAttribute("Constant", DoubleValue(count));
}

void
SimulationHelper::SetUserCountPerMobileUt(Ptr<RandomVariableStream> rs)
{
    NS_LOG_FUNCTION(this << &rs);

    m_utMobileUserCount = rs;
}

void
SimulationHelper::SetGwUserCount(uint32_t gwUserCount)
{
    NS_LOG_FUNCTION(this << gwUserCount);

    Config::SetDefault("ns3::SatHelper::GwUsers", UintegerValue(gwUserCount));
}

void
SimulationHelper::SetSimulationTime(double seconds)
{
    NS_LOG_FUNCTION(this << seconds);

    m_simTime = Seconds(seconds);
}

void
SimulationHelper::SetOutputTag(std::string tag)
{
    NS_LOG_FUNCTION(this << tag);

    // Set simulation output details
    m_simulationTag = tag;
}

void
SimulationHelper::SetOutputPath(std::string path)
{
    NS_LOG_FUNCTION(this << path);

    // Set simulation output details
    m_outputPath = path;
}

void
SimulationHelper::AddDefaultUiArguments(CommandLine& cmd, std::string& xmlInputFile)
{
    NS_LOG_FUNCTION(this);

    AddDefaultUiArguments(cmd);
    cmd.AddValue("InputXml", "Input attributes in XML file", xmlInputFile);
}

void
SimulationHelper::AddDefaultUiArguments(CommandLine& cmd)
{
    NS_LOG_FUNCTION(this);

    // Create a customizable output path
    cmd.AddValue("OutputPath", "Output path for storing the simulation statistics", m_outputPath);
}

void
SimulationHelper::SetDefaultValues()
{
    NS_LOG_FUNCTION(this);

    DisableRandomAccess();
    Config::SetDefault("ns3::SatBeamHelper::FadingModel", EnumValue(SatEnums::FADING_OFF));

    Config::SetDefault("ns3::SatConf::SuperFrameConfForSeq0", StringValue("Configuration_0"));
    Config::SetDefault("ns3::SatSuperframeConf0::FrameConfigType", StringValue("ConfigType_2"));
    Config::SetDefault("ns3::SatSuperframeSeq::TargetDuration", TimeValue(MilliSeconds(100)));

    Config::SetDefault("ns3::SatRequestManager::EvaluationInterval", TimeValue(MilliSeconds(100)));
    Config::SetDefault("ns3::SatRequestManager::EnableOnDemandEvaluation", BooleanValue(false));

    Config::SetDefault("ns3::SatBbFrameConf::BBFrameUsageMode", StringValue("NormalFrames"));

    ConfigureFrequencyBands();
    ConfigureFrame(0, 20e5, 5e5, 0.2, 0.3, false);

    SetErrorModel(SatPhyRxCarrierConf::EM_AVI);
    SetInterferenceModel(SatPhyRxCarrierConf::IF_PER_PACKET);

    // ACM enabled
    EnableAcm(SatEnums::LD_FORWARD);
    EnableAcm(SatEnums::LD_RETURN);

    DisableAllCapacityAssignmentCategories();
    EnableOnlyVbdc(3);

    EnablePeriodicalControlSlots(MilliSeconds(100));

    Config::SetDefault("ns3::SatUtHelper::EnableChannelEstimationError", BooleanValue(true));
    Config::SetDefault("ns3::SatGwHelper::EnableChannelEstimationError", BooleanValue(true));

    Config::SetDefault("ns3::SatFwdLinkScheduler::DummyFrameSendingEnabled", BooleanValue(false));

    Config::SetDefault("ns3::SatQueue::MaxPackets", UintegerValue(10000));
}

void
SimulationHelper::DisableAllCapacityAssignmentCategories()
{
    NS_LOG_FUNCTION(this);

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
}

void
SimulationHelper::EnableOnlyConstantRate(uint32_t rcIndex, double rateKbps)
{
    NS_LOG_FUNCTION(this << rcIndex << rateKbps);

    std::stringstream ss;
    ss << rcIndex;
    std::string attributeDefault("ns3::SatLowerLayerServiceConf::DaService" + ss.str());

    Config::SetDefault(attributeDefault + "_ConstantAssignmentProvided", BooleanValue(true));
    Config::SetDefault(attributeDefault + "_RbdcAllowed", BooleanValue(false));
    Config::SetDefault(attributeDefault + "_VolumeAllowed", BooleanValue(false));
}

void
SimulationHelper::EnableOnlyRbdc(uint32_t rcIndex)
{
    NS_LOG_FUNCTION(this << rcIndex);

    std::stringstream ss;
    ss << rcIndex;
    std::string attributeDefault("ns3::SatLowerLayerServiceConf::DaService" + ss.str());

    Config::SetDefault(attributeDefault + "_ConstantAssignmentProvided", BooleanValue(false));
    Config::SetDefault(attributeDefault + "_RbdcAllowed", BooleanValue(true));
    Config::SetDefault(attributeDefault + "_VolumeAllowed", BooleanValue(false));

    Config::SetDefault("ns3::SatLowerLayerServiceConf::DynamicRatePersistence", UintegerValue(10));
}

void
SimulationHelper::EnableOnlyVbdc(uint32_t rcIndex)
{
    NS_LOG_FUNCTION(this << rcIndex);

    std::stringstream ss;
    ss << rcIndex;
    std::string attributeDefault("ns3::SatLowerLayerServiceConf::DaService" + ss.str());

    Config::SetDefault(attributeDefault + "_ConstantAssignmentProvided", BooleanValue(false));
    Config::SetDefault(attributeDefault + "_RbdcAllowed", BooleanValue(false));
    Config::SetDefault(attributeDefault + "_VolumeAllowed", BooleanValue(true));

    Config::SetDefault("ns3::SatLowerLayerServiceConf::VolumeBacklogPersistence",
                       UintegerValue(10));
}

void
SimulationHelper::EnableFca()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue(true));
}

void
SimulationHelper::DisableFca()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue(false));
}

void
SimulationHelper::EnablePeriodicalControlSlots(Time periodicity)
{
    NS_LOG_FUNCTION(this << periodicity.GetSeconds());

    // Controls slots
    Config::SetDefault("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue(true));
    Config::SetDefault("ns3::SatBeamScheduler::ControlSlotInterval", TimeValue(periodicity));
}

void
SimulationHelper::EnableArq(SatEnums::SatLinkDir_t dir)
{
    NS_LOG_FUNCTION(this << dir);

    switch (dir)
    {
    case SatEnums::LD_FORWARD: {
        Config::SetDefault("ns3::SatLlc::FwdLinkArqEnabled", BooleanValue(true));

        Config::SetDefault("ns3::SatGenericStreamEncapsulatorArq::MaxNoOfRetransmissions",
                           UintegerValue(2));
        Config::SetDefault("ns3::SatGenericStreamEncapsulatorArq::RetransmissionTimer",
                           TimeValue(MilliSeconds(600)));
        Config::SetDefault("ns3::SatGenericStreamEncapsulatorArq::WindowSize", UintegerValue(10));
        Config::SetDefault("ns3::SatGenericStreamEncapsulatorArq::ArqHeaderSize", UintegerValue(1));
        Config::SetDefault("ns3::SatGenericStreamEncapsulatorArq::RxWaitingTime",
                           TimeValue(Seconds(1.8)));
        break;
    }
    case SatEnums::LD_RETURN: {
        Config::SetDefault("ns3::SatLlc::RtnLinkArqEnabled", BooleanValue(true));

        Config::SetDefault("ns3::SatReturnLinkEncapsulatorArq::MaxRtnArqSegmentSize",
                           UintegerValue(38));
        Config::SetDefault("ns3::SatReturnLinkEncapsulatorArq::MaxNoOfRetransmissions",
                           UintegerValue(2));
        Config::SetDefault("ns3::SatReturnLinkEncapsulatorArq::RetransmissionTimer",
                           TimeValue(MilliSeconds(600)));
        Config::SetDefault("ns3::SatReturnLinkEncapsulatorArq::WindowSize", UintegerValue(10));
        Config::SetDefault("ns3::SatReturnLinkEncapsulatorArq::ArqHeaderSize", UintegerValue(1));
        Config::SetDefault("ns3::SatReturnLinkEncapsulatorArq::RxWaitingTime",
                           TimeValue(Seconds(1.8)));
        break;
    }
    default: {
        NS_FATAL_ERROR("Unsupported SatLinkDir_t!");
        break;
    }
    }
}

void
SimulationHelper::DisableRandomAccess()
{
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel", EnumValue(SatEnums::RA_MODEL_OFF));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_CONSTANT));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_PERFECT));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_NOT_DEFINED));
    Config::SetDefault("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl",
                       BooleanValue(false));
}

void
SimulationHelper::EnableSlottedAloha()
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessConfigured == true)
    {
        NS_FATAL_ERROR("Random access already configured!");
    }

    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances",
                       UintegerValue(1));
    EnableRandomAccess();
}

void
SimulationHelper::EnableCrdsa()
{
    NS_LOG_FUNCTION(this);

    if (m_randomAccessConfigured == true)
    {
        NS_FATAL_ERROR("Random access already configured!");
    }

    Config::SetDefault("ns3::SatUtHelper::UseCrdsaOnlyForControlPackets", BooleanValue(false));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaService0_NumberOfInstances",
                       UintegerValue(3));
    EnableRandomAccess();
}

void
SimulationHelper::EnableRandomAccess()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue(false));

    Config::SetDefault("ns3::SatLowerLayerServiceConf::DefaultControlRandomizationInterval",
                       TimeValue(MilliSeconds(100)));
    Config::SetDefault("ns3::SatLowerLayerServiceConf::RaServiceCount", UintegerValue(1));
    Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel",
                       EnumValue(SatEnums::RA_MODEL_RCS2_SPECIFICATION));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceModel",
                       EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       EnumValue(SatPhyRxCarrierConf::SIC_RESIDUAL));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));
    Config::SetDefault("ns3::SatBeamHelper::RaConstantErrorRate", DoubleValue(0.0));

    Config::SetDefault("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl",
                       BooleanValue(false));
    Config::SetDefault(
        "ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize",
        UintegerValue(10));

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

    m_randomAccessConfigured = true;

    ConfigureFrame(0, 1.25e6, 1.25e6, 0.2, 0.3, true);
}

void
SimulationHelper::SetIdealPhyParameterization()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue(false));
    Config::SetDefault("ns3::SatChannel::ForwardingMode", StringValue("OnlyDestNode"));
    Config::SetDefault("ns3::SatChannel::RxPowerCalculationMode",
                       StringValue("RxPowerCalculation"));
    Config::SetDefault("ns3::SatBeamHelper::FadingModel", StringValue("FadingOff"));
    Config::SetDefault("ns3::SatUtHelper::EnableChannelEstimationError", BooleanValue(false));
    Config::SetDefault("ns3::SatGwHelper::EnableChannelEstimationError", BooleanValue(false));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceModel", StringValue("Constant"));
    Config::SetDefault("ns3::SatBeamHelper::RaInterferenceEliminationModel",
                       StringValue("Perfect"));
    Config::SetDefault("ns3::SatBeamHelper::RaCollisionModel",
                       StringValue("RaCollisionNotDefined"));

    SetErrorModel(SatPhyRxCarrierConf::EM_NONE);
    SetInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT);
}

void
SimulationHelper::EnableAcm(SatEnums::SatLinkDir_t dir)
{
    NS_LOG_FUNCTION(this << dir);

    switch (dir)
    {
    case SatEnums::LD_FORWARD: {
        Config::SetDefault("ns3::SatBbFrameConf::AcmEnabled", BooleanValue(true));
        Config::SetDefault("ns3::SatBbFrameConf::DefaultModCod", StringValue("QPSK_1_TO_2"));
        Config::SetDefault("ns3::SatRequestManager::CnoReportInterval", TimeValue(Seconds(0.1)));
        Config::SetDefault("ns3::SatFwdLinkScheduler::CnoEstimationMode",
                           StringValue("AverageValueInWindow"));
        Config::SetDefault("ns3::SatFwdLinkScheduler::CnoEstimationWindow", TimeValue(Seconds(2)));
        break;
    }
    case SatEnums::LD_RETURN: {
        Config::SetDefault("ns3::SatBeamScheduler::CnoEstimationMode",
                           StringValue("MinimumValueInWindow"));
        Config::SetDefault("ns3::SatBeamScheduler::CnoEstimationWindow", TimeValue(Seconds(2)));
        Config::SetDefault("ns3::SatWaveformConf::AcmEnabled", BooleanValue(true));
        Config::SetDefault("ns3::SatWaveformConf::DefaultWfId", UintegerValue(3));
        break;
    }
    default: {
        NS_FATAL_ERROR("Unsupported SatLinkDir_t!");
        break;
    }
    }
}

void
SimulationHelper::DisableAcm(SatEnums::SatLinkDir_t dir)
{
    NS_LOG_FUNCTION(this << dir);

    switch (dir)
    {
    case SatEnums::LD_FORWARD: {
        Config::SetDefault("ns3::SatBbFrameConf::AcmEnabled", BooleanValue(false));
        Config::SetDefault("ns3::SatBbFrameConf::DefaultModCod", StringValue("QPSK_1_TO_2"));
        Config::SetDefault("ns3::SatRequestManager::CnoReportInterval", TimeValue(Seconds(100)));
        break;
    }
    case SatEnums::LD_RETURN: {
        Config::SetDefault("ns3::SatWaveformConf::AcmEnabled", BooleanValue(false));
        Config::SetDefault("ns3::SatWaveformConf::DefaultWfId", UintegerValue(3));
        break;
    }
    default: {
        NS_FATAL_ERROR("Unsupported SatLinkDir_t!");
        break;
    }
    }
}

void
SimulationHelper::ProgressCb()
{
    std::cout << "Progress: " << Simulator::Now().GetSeconds() << "/" << GetSimTime().GetSeconds()
              << std::endl;
    m_progressReportEvent =
        Simulator::Schedule(m_progressUpdateInterval, &SimulationHelper::ProgressCb, this);
}

void
SimulationHelper::CreateDefaultStats()
{
    NS_ASSERT_MSG(m_satHelper != nullptr, "Satellite scenario not created yet!");

    if (!m_statContainer)
    {
        Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite",
                           BooleanValue(true));
        m_statContainer = CreateObject<SatStatsHelperContainer>(m_satHelper);
    }

    CreateDefaultFwdLinkStats();
    CreateDefaultRtnLinkStats();
}

void
SimulationHelper::CreateDefaultFwdLinkStats()
{
    NS_LOG_FUNCTION(this);

    if (!m_statContainer)
    {
        Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite",
                           BooleanValue(true));
        m_statContainer = CreateObject<SatStatsHelperContainer>(m_satHelper);
    }

    // Throughput
    m_statContainer->AddAverageUtUserFwdAppThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddAverageUtFwdUserPhyThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddAverageBeamFwdAppThroughput(SatStatsHelper::OUTPUT_CDF_FILE);

    m_statContainer->AddGlobalFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalFwdAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalFwdFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalFwdFeederMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalFwdFeederPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalFwdUserPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalFwdFeederPhyThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalFwdUserPhyThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    m_statContainer->AddPerBeamFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFwdFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFwdUserMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFwdFeederPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFwdUserPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    Config::SetDefault("ns3::SatStatsThroughputHelper::AveragingMode", BooleanValue(true));
    m_statContainer->AddPerBeamFwdAppThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamFwdFeederMacThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamFwdUserMacThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamFwdFeederPhyThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamFwdUserPhyThroughput(SatStatsHelper::OUTPUT_CDF_FILE);

    // SINR
    m_statContainer->AddGlobalFwdCompositeSinr(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddGlobalFwdCompositeSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);

    // Delay
    m_statContainer->AddGlobalFwdAppDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalFwdAppDelay(SatStatsHelper::OUTPUT_CDF_FILE);

    // Packet error
    m_statContainer->AddGlobalFwdUserDaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFwdUserDaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnFeederDaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederCrdsaPacketCollision(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederCrdsaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederSlottedAlohaPacketCollision(
        SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederSlottedAlohaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);

    // Marsala
    m_statContainer->AddPerBeamMarsalaCorrelation(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamMarsalaCorrelation(SatStatsHelper::OUTPUT_SCATTER_FILE);

    // Frame type usage
    Config::SetDefault("ns3::SatStatsFrameTypeUsageHelper::Percentage", BooleanValue(true));
    m_statContainer->AddGlobalFrameTypeUsage(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerGwFrameTypeUsage(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFrameTypeUsage(SatStatsHelper::OUTPUT_SCALAR_FILE);

    // Beam service time
    m_statContainer->AddPerBeamBeamServiceTime(SatStatsHelper::OUTPUT_SCALAR_FILE);
}

void
SimulationHelper::CreateDefaultRtnLinkStats()
{
    NS_LOG_FUNCTION(this);

    if (!m_statContainer)
    {
        Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite",
                           BooleanValue(true));
        m_statContainer = CreateObject<SatStatsHelperContainer>(m_satHelper);
    }

    // Throughput
    m_statContainer->AddAverageUtUserRtnAppThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddAverageUtRtnFeederPhyThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddAverageBeamRtnAppThroughput(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerUtUserRtnAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    m_statContainer->AddGlobalRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalRtnAppThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalRtnUserMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalRtnUserMacThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalRtnFeederPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalRtnUserPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalRtnFeederPhyThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalRtnUserPhyThroughput(SatStatsHelper::OUTPUT_SCATTER_FILE);

    m_statContainer->AddPerBeamRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnFeederDevThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnUserDevThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnFeederMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnUserMacThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnFeederPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnUserPhyThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);

    // Granted resources
    m_statContainer->AddGlobalResourcesGranted(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddGlobalResourcesGranted(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalResourcesGranted(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamResourcesGranted(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamResourcesGranted(SatStatsHelper::OUTPUT_CDF_PLOT);
    m_statContainer->AddPerUtResourcesGranted(SatStatsHelper::OUTPUT_SCATTER_FILE);

    // Frame load
    m_statContainer->AddGlobalFrameSymbolLoad(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFrameSymbolLoad(SatStatsHelper::OUTPUT_SCALAR_FILE);

    m_statContainer->AddGlobalRtnFeederLinkSinr(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddGlobalRtnUserLinkSinr(SatStatsHelper::OUTPUT_CDF_FILE);

    // SINR
    m_statContainer->AddGlobalRtnCompositeSinr(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddGlobalRtnCompositeSinr(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddPerBeamRtnCompositeSinr(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamRtnCompositeSinr(SatStatsHelper::OUTPUT_CDF_PLOT);

    // Delay
    m_statContainer->AddGlobalRtnAppDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddGlobalRtnAppDelay(SatStatsHelper::OUTPUT_CDF_FILE);

    m_statContainer->AddPerBeamRtnAppDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnDevDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnPhyDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnMacDelay(SatStatsHelper::OUTPUT_SCALAR_FILE);

    m_statContainer->AddPerBeamRtnAppDelay(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamRtnDevDelay(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamRtnPhyDelay(SatStatsHelper::OUTPUT_CDF_FILE);
    m_statContainer->AddPerBeamRtnMacDelay(SatStatsHelper::OUTPUT_CDF_FILE);

    // Packet error
    m_statContainer->AddGlobalRtnFeederDaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamRtnFeederDaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederCrdsaPacketCollision(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederCrdsaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederSlottedAlohaPacketCollision(
        SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFeederSlottedAlohaPacketError(SatStatsHelper::OUTPUT_SCALAR_FILE);

    // Waveform
    m_statContainer->AddPerBeamWaveformUsage(SatStatsHelper::OUTPUT_SCALAR_FILE);
    m_statContainer->AddPerBeamFrameSymbolLoad(SatStatsHelper::OUTPUT_SCALAR_FILE);

    // Capacity request
    m_statContainer->AddPerUtCapacityRequest(SatStatsHelper::OUTPUT_SCATTER_FILE);
    m_statContainer->AddPerBeamCapacityRequest(SatStatsHelper::OUTPUT_SCATTER_FILE);
}

void
SimulationHelper::SetErrorModel(SatPhyRxCarrierConf::ErrorModel em, double errorRate)
{
    NS_LOG_FUNCTION(this << em << errorRate);

    Config::SetDefault("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue(em));
    Config::SetDefault("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue(em));

    if (errorRate == SatPhyRxCarrierConf::EM_CONSTANT)
    {
        Config::SetDefault("ns3::SatGwHelper::RtnLinkConstantErrorRate", DoubleValue(errorRate));
        Config::SetDefault("ns3::SatUtHelper::FwdLinkConstantErrorRate", DoubleValue(errorRate));
    }
}

void
SimulationHelper::SetInterferenceModel(SatPhyRxCarrierConf::InterferenceModel ifModel,
                                       double constantIf)
{
    NS_LOG_FUNCTION(this << ifModel << constantIf);

    Config::SetDefault("ns3::SatUtHelper::DaFwdLinkInterferenceModel", EnumValue(ifModel));
    Config::SetDefault("ns3::SatGwHelper::DaRtnLinkInterferenceModel", EnumValue(ifModel));

    if (ifModel == SatPhyRxCarrierConf::IF_CONSTANT)
    {
        Config::SetDefault("ns3::SatConstantInterference::ConstantInterferencePower",
                           DoubleValue(constantIf));
    }
}

void
SimulationHelper::ConfigureFrame(uint32_t superFrameId,
                                 double bw,
                                 double carrierBw,
                                 double rollOff,
                                 double carrierSpacing,
                                 bool isRandomAccess)
{
    NS_LOG_FUNCTION(this << bw << carrierBw << rollOff << carrierSpacing << isRandomAccess);

    std::stringstream sfId, fId;
    sfId << superFrameId;
    fId << m_numberOfConfiguredFrames;
    std::string attributeDefault("ns3::SatSuperframeConf" + sfId.str() + "::Frame" + fId.str());

    Config::SetDefault(attributeDefault + "_AllocatedBandwidthHz", DoubleValue(bw));
    Config::SetDefault(attributeDefault + "_CarrierAllocatedBandwidthHz", DoubleValue(carrierBw));
    Config::SetDefault(attributeDefault + "_CarrierRollOff", DoubleValue(rollOff));
    Config::SetDefault(attributeDefault + "_CarrierSpacing", DoubleValue(carrierSpacing));
    Config::SetDefault(attributeDefault + "_RandomAccessFrame", BooleanValue(isRandomAccess));

    m_numberOfConfiguredFrames++;

    Config::SetDefault("ns3::SatSuperframeConf" + sfId.str() + "::FrameCount",
                       UintegerValue(m_numberOfConfiguredFrames));
}

void
SimulationHelper::ConfigureFrequencyBands()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue(2e+09));
    Config::SetDefault("ns3::SatConf::FwdFeederLinkBaseFrequency", DoubleValue(2.75e+10));
    Config::SetDefault("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue(5e+08));
    Config::SetDefault("ns3::SatConf::FwdUserLinkBaseFrequency", DoubleValue(1.97e+10));

    Config::SetDefault("ns3::SatConf::RtnFeederLinkBandwidth", DoubleValue(2e+09));
    Config::SetDefault("ns3::SatConf::RtnFeederLinkBaseFrequency", DoubleValue(1.77e+10));
    Config::SetDefault("ns3::SatConf::RtnUserLinkBandwidth", DoubleValue(5e+08));
    Config::SetDefault("ns3::SatConf::RtnUserLinkBaseFrequency", DoubleValue(2.95e+10));

    Config::SetDefault("ns3::SatConf::FwdUserLinkChannels", UintegerValue(4));
    Config::SetDefault("ns3::SatConf::FwdFeederLinkChannels", UintegerValue(16));
    Config::SetDefault("ns3::SatConf::RtnUserLinkChannels", UintegerValue(4));
    Config::SetDefault("ns3::SatConf::RtnFeederLinkChannels", UintegerValue(16));

    Config::SetDefault("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue(1.25e+08));
    Config::SetDefault("ns3::SatConf::FwdCarrierRollOff", DoubleValue(0.2));
    Config::SetDefault("ns3::SatConf::FwdCarrierSpacing", DoubleValue(0.0));
}

void
SimulationHelper::ConfigureFwdLinkBeamHopping()
{
    NS_LOG_FUNCTION(this);

    // Enable flag
    Config::SetDefault("ns3::SatBeamHelper::EnableFwdLinkBeamHopping", BooleanValue(true));

    // Channel configuration for 500 MHz user link bandwidth
    Config::SetDefault("ns3::SatHelper::SatFwdConfFileName",
                       StringValue("beamhopping/Scenario72FwdConf_BH.txt"));

    Config::SetDefault("ns3::SatBstpController::BeamHoppingMode",
                       EnumValue(SatBstpController::BH_STATIC));
    Config::SetDefault("ns3::SatBstpController::StaticBeamHoppingConfigFileName",
                       StringValue("beamhopping/SatBstpConf_GW1.txt"));
    Config::SetDefault("ns3::SatBstpController::SuperframeDuration", TimeValue(MilliSeconds(1)));

    // Frequency configuration for 500 MHz user link bandwidth
    Config::SetDefault("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue(2e+09));
    Config::SetDefault("ns3::SatConf::FwdFeederLinkBaseFrequency", DoubleValue(2.75e+10));
    Config::SetDefault("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue(5e+08));
    Config::SetDefault("ns3::SatConf::FwdUserLinkBaseFrequency", DoubleValue(1.97e+10));

    Config::SetDefault("ns3::SatConf::FwdUserLinkChannels", UintegerValue(1));
    Config::SetDefault("ns3::SatConf::FwdFeederLinkChannels", UintegerValue(4));

    Config::SetDefault("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue(5e+08));
    Config::SetDefault("ns3::SatConf::FwdCarrierRollOff", DoubleValue(0.2));
    Config::SetDefault("ns3::SatConf::FwdCarrierSpacing", DoubleValue(0.0));
}

void
SimulationHelper::EnableOutputTraces()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatPhyRxCarrier::EnableCompositeSinrOutputTrace", BooleanValue(true));
    Config::SetDefault("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace", BooleanValue(true));
    Config::SetDefault("ns3::SatChannel::EnableRxPowerOutputTrace", BooleanValue(true));
    Config::SetDefault("ns3::SatChannel::EnableFadingOutputTrace", BooleanValue(true));
}

void
SimulationHelper::ConfigureLinkBudget()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatGeoFeederPhy::RxTemperatureDbk", DoubleValue(28.4));
    Config::SetDefault("ns3::SatGeoFeederPhy::RxMaxAntennaGainDb", DoubleValue(54));
    Config::SetDefault("ns3::SatGeoFeederPhy::TxMaxAntennaGainDb", DoubleValue(54));
    Config::SetDefault("ns3::SatGeoFeederPhy::TxMaxPowerDbw", DoubleValue(-4.38));
    Config::SetDefault("ns3::SatGeoFeederPhy::TxOutputLossDb", DoubleValue(1.75));
    Config::SetDefault("ns3::SatGeoFeederPhy::TxPointingLossDb", DoubleValue(0));
    Config::SetDefault("ns3::SatGeoFeederPhy::TxOboLossDb", DoubleValue(4));
    Config::SetDefault("ns3::SatGeoFeederPhy::TxAntennaLossDb", DoubleValue(1));
    Config::SetDefault("ns3::SatGeoFeederPhy::RxAntennaLossDb", DoubleValue(1));
    Config::SetDefault("ns3::SatGeoFeederPhy::DefaultFadingValue", DoubleValue(1));
    Config::SetDefault("ns3::SatGeoFeederPhy::ExtNoisePowerDensityDbwhz", DoubleValue(-207));
    Config::SetDefault("ns3::SatGeoFeederPhy::ImIfCOverIDb", DoubleValue(27));
    Config::SetDefault("ns3::SatGeoFeederPhy::FixedAmplificationGainDb", DoubleValue(82));
    Config::SetDefault("ns3::SatGeoUserPhy::RxTemperatureDbk", DoubleValue(28.4));
    Config::SetDefault("ns3::SatGeoUserPhy::RxMaxAntennaGainDb", DoubleValue(54));
    Config::SetDefault("ns3::SatGeoUserPhy::TxMaxAntennaGainDb", DoubleValue(54));
    Config::SetDefault("ns3::SatGeoUserPhy::TxMaxPowerDbw", DoubleValue(15));
    Config::SetDefault("ns3::SatGeoUserPhy::TxOutputLossDb", DoubleValue(2.85));
    Config::SetDefault("ns3::SatGeoUserPhy::TxPointingLossDb", DoubleValue(0));
    Config::SetDefault("ns3::SatGeoUserPhy::TxOboLossDb", DoubleValue(0));
    Config::SetDefault("ns3::SatGeoUserPhy::TxAntennaLossDb", DoubleValue(1));
    Config::SetDefault("ns3::SatGeoUserPhy::RxAntennaLossDb", DoubleValue(1));
    Config::SetDefault("ns3::SatGeoUserPhy::DefaultFadingValue", DoubleValue(1));
    Config::SetDefault("ns3::SatGeoUserPhy::OtherSysIfCOverIDb", DoubleValue(27.5));
    Config::SetDefault("ns3::SatGeoUserPhy::AciIfCOverIDb", DoubleValue(17));
    Config::SetDefault("ns3::SatGwPhy::RxTemperatureDbk", DoubleValue(24.62));
    Config::SetDefault("ns3::SatGwPhy::RxMaxAntennaGainDb", DoubleValue(61.5));
    Config::SetDefault("ns3::SatGwPhy::TxMaxAntennaGainDb", DoubleValue(65.2));
    Config::SetDefault("ns3::SatGwPhy::TxMaxPowerDbw", DoubleValue(8.97));
    Config::SetDefault("ns3::SatGwPhy::TxOutputLossDb", DoubleValue(2));
    Config::SetDefault("ns3::SatGwPhy::TxPointingLossDb", DoubleValue(1.1));
    Config::SetDefault("ns3::SatGwPhy::TxOboLossDb", DoubleValue(6));
    Config::SetDefault("ns3::SatGwPhy::TxAntennaLossDb", DoubleValue(0));
    Config::SetDefault("ns3::SatGwPhy::RxAntennaLossDb", DoubleValue(0));
    Config::SetDefault("ns3::SatGwPhy::DefaultFadingValue", DoubleValue(1));
    Config::SetDefault("ns3::SatGwPhy::ImIfCOverIDb", DoubleValue(22));
    Config::SetDefault("ns3::SatGwPhy::AciIfWrtNoisePercent", DoubleValue(10));
    Config::SetDefault("ns3::SatUtPhy::RxTemperatureDbk", DoubleValue(24.6));
    Config::SetDefault("ns3::SatUtPhy::RxMaxAntennaGainDb", DoubleValue(44.6));
    Config::SetDefault("ns3::SatUtPhy::TxMaxAntennaGainDb", DoubleValue(45.2));
    Config::SetDefault("ns3::SatUtPhy::TxMaxPowerDbw", DoubleValue(4));
    Config::SetDefault("ns3::SatUtPhy::TxOutputLossDb", DoubleValue(0.5));
    Config::SetDefault("ns3::SatUtPhy::TxPointingLossDb", DoubleValue(1));
    Config::SetDefault("ns3::SatUtPhy::TxOboLossDb", DoubleValue(0.5));
    Config::SetDefault("ns3::SatUtPhy::TxAntennaLossDb", DoubleValue(1));
    Config::SetDefault("ns3::SatUtPhy::RxAntennaLossDb", DoubleValue(0));
    Config::SetDefault("ns3::SatUtPhy::DefaultFadingValue", DoubleValue(1));
    Config::SetDefault("ns3::SatUtPhy::OtherSysIfCOverIDb", DoubleValue(24.7));
}

void
SimulationHelper::EnableExternalFadingInputTrace()
{
    NS_LOG_FUNCTION(this);

    Config::SetDefault("ns3::SatBeamHelper::FadingModel", EnumValue(SatEnums::FADING_OFF));

    // Set index files defining external tracing input files for UTs
    // Given index files must locate in /satellite/data/ext-fadingtraces/input folder
    Config::SetDefault("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName",
                       StringValue("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));
    Config::SetDefault("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName",
                       StringValue("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));

    // Set external fading input trace container mode as list mode
    // Now external fading input file used for UT1 is input file defined in row one in set index
    // file, for UT2 second input file defined in row two in set index file etc. Position info in
    // index file is ignored by list mode
    Config::SetDefault("ns3::SatFadingExternalInputTraceContainer::UtInputMode",
                       StringValue("ListMode"));

    // enable/disable external fading input on SatChannel as user requests
    Config::SetDefault("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue(true));
}

void
SimulationHelper::SetCommonUtPositionAllocator(Ptr<SatListPositionAllocator> posAllocator)
{
    NS_LOG_FUNCTION(this);
    m_commonUtPositions = posAllocator;
}

void
SimulationHelper::SetUtPositionAllocatorForBeam(uint32_t beamId,
                                                Ptr<SatListPositionAllocator> posAllocator)
{
    NS_LOG_FUNCTION(this << beamId);
    m_utPositionsByBeam[beamId] = posAllocator;
}

void
SimulationHelper::EnableUtListPositionsFromInputFile(std::string inputFile, bool checkBeams)
{
    NS_LOG_FUNCTION(this << inputFile);

    // Set user specific UT position file (UserDefinedUtPos.txt) to be utilized by SatConf.
    // Given file must locate in /satellite/data folder
    //
    // This enables user defined positions used instead of default positions (default position file
    // UtPos.txt replaced),
    Config::SetDefault("ns3::SatConf::UtPositionInputFileName", StringValue(inputFile));
    m_enableInputFileUtListPositions = true;
    m_inputFileUtPositionsCheckBeams = checkBeams;
}

Ptr<SatStatsHelperContainer>
SimulationHelper::GetStatisticsContainer()
{
    NS_LOG_FUNCTION(this);

    if (!m_statContainer)
    {
        Config::SetDefault("ns3::SatEnvVariables::EnableSimulationOutputOverwrite",
                           BooleanValue(true));
        m_statContainer = CreateObject<SatStatsHelperContainer>(m_satHelper);
    }

    return m_statContainer;
}

Ptr<SatTrafficHelper>
SimulationHelper::GetTrafficHelper()
{
    NS_LOG_FUNCTION(this);

    if (!m_trafficHelper)
    {
        m_trafficHelper =
            CreateObject<SatTrafficHelper>(GetSatelliteHelper(), GetStatisticsContainer());
    }

    return m_trafficHelper;
}

Ptr<SatGroupHelper>
SimulationHelper::GetGroupHelper()
{
    NS_LOG_FUNCTION(this);

    if (!m_groupHelper)
    {
        m_groupHelper = CreateObject<SatGroupHelper>();
    }

    return m_groupHelper;
}

Ptr<SatCnoHelper>
SimulationHelper::GetCnoHelper()
{
    NS_LOG_FUNCTION(this);

    if (!m_cnoHelper)
    {
        m_cnoHelper = CreateObject<SatCnoHelper>(m_satHelper);
    }

    return m_cnoHelper;
}

void
SimulationHelper::SetupOutputPath()
{
    NS_LOG_FUNCTION(this);
    if (m_outputPath == "")
    {
        m_outputPath = Singleton<SatEnvVariables>::Get()->LocateDataDirectory() + "/sims/" +
                       m_simulationName + "/";
        // Create the simulation campaign output directory in data/sims/
        if (!Singleton<SatEnvVariables>::Get()->IsValidDirectory(m_outputPath))
        {
            Singleton<SatEnvVariables>::Get()->CreateDirectory(m_outputPath);
        }
        if (m_simulationTag != "")
        {
            m_outputPath += m_simulationTag + "/";
            Config::SetDefault("ns3::SatEnvVariables::SimulationTag", StringValue(m_simulationTag));

            // Create the simulation output directory by tag name in
            // data/sims/simulation-campaign-directory/
            if (!Singleton<SatEnvVariables>::Get()->IsValidDirectory(m_outputPath))
            {
                Singleton<SatEnvVariables>::Get()->CreateDirectory(m_outputPath);
            }
        }
    }
    Singleton<SatEnvVariables>::Get()->SetOutputPath(m_outputPath);
}

Ptr<SatHelper>
SimulationHelper::CreateSatScenario(SatHelper::PreDefinedScenario_t scenario,
                                    const std::string& mobileUtsFolder)
{
    NS_LOG_FUNCTION(this);

    std::stringstream ss;
    ss << "Created scenario: " << std::endl;

    // Set final output path
    SetupOutputPath();

    m_satHelper = CreateObject<SatHelper>();

    m_satHelper->SetGroupHelper(
        GetGroupHelper()); // If not done in user scenario, group helper is created here
    Ptr<SatAntennaGainPatternContainer> antennaGainPatterns = m_satHelper->GetAntennaGainPatterns();
    m_satHelper->GetBeamHelper()->SetAntennaGainPatterns(antennaGainPatterns);

    // Set UT position allocators, if any
    if (!m_enableInputFileUtListPositions && !m_satHelper->IsSatConstellationEnabled())
    {
        if (m_commonUtPositions)
        {
            m_satHelper->SetCustomUtPositionAllocator(m_commonUtPositions);
        }
        for (auto it : m_utPositionsByBeam)
        {
            m_satHelper->SetUtPositionAllocatorForBeam(it.first, it.second);
        }
    }

    if (m_satHelper->IsSatConstellationEnabled())
    {
        SatHelper::BeamUserInfoMap_t beamInfo;
        for (uint32_t satId = 0; satId < m_satHelper->GeoSatNodes().GetN(); satId++)
        {
            // Set beamInfo to indicate enabled beams
            for (uint32_t i = 1; i <= m_satHelper->GetBeamCount(); i++)
            {
                if (IsBeamEnabled(i))
                {
                    SatBeamUserInfo info;
                    beamInfo.insert(std::make_pair(std::make_pair(satId, i), info));
                }
            }
        }

        m_satHelper->CreateConstellationScenario(
            beamInfo,
            MakeCallback(&SimulationHelper::GetNextUtUserCount, this));
    }

    // Determine scenario
    else if (scenario == SatHelper::NONE)
    {
        // Create beam scenario
        SatHelper::BeamUserInfoMap_t beamInfo;

        for (uint32_t i = 1; i <= m_satHelper->GetBeamCount(); i++)
        {
            if (IsBeamEnabled(i))
            {
                SatBeamUserInfo info;
                uint32_t utCount = GetNextUtCount(i);

                ss << "  Beam " << i << ": UT count= " << utCount;

                for (uint32_t j = 1; j < utCount + 1; j++)
                {
                    uint32_t utUserCount = GetNextUtUserCount();
                    info.AppendUt(utUserCount);
                    ss << ", " << j << ". UT user count= " << utUserCount;
                }

                beamInfo.insert(std::make_pair(std::make_pair(0, i), info));

                ss << std::endl;
            }
        }

        std::vector<std::pair<GeoCoordinate, uint32_t>> additionalNodesVector =
            m_groupHelper->GetAdditionalNodesPerBeam();
        std::map<uint32_t, std::vector<std::pair<GeoCoordinate, uint32_t>>> additionalNodes;
        for (std::vector<std::pair<GeoCoordinate, uint32_t>>::iterator it =
                 additionalNodesVector.begin();
             it != additionalNodesVector.end();
             it++)
        {
            uint32_t bestBeamId = antennaGainPatterns->GetBestBeamId(0, it->first, false);
            additionalNodes[bestBeamId].push_back(*it);
        }

        for (std::map<uint32_t, std::vector<std::pair<GeoCoordinate, uint32_t>>>::iterator it =
                 additionalNodes.begin();
             it != additionalNodes.end();
             it++)
        {
            if (!IsBeamEnabled(it->first))
            {
                NS_LOG_WARN("Beam ID " << it->first << " is not enabled, cannot add "
                                       << it->second.size() << " UTs from SatGroupHelper");
                std::cout << "Beam ID " << it->first << " is not enabled, cannot add "
                          << it->second.size() << " UTs from SatGroupHelper" << std::endl;
                continue;
            }
            beamInfo[std::make_pair(0, it->first)].SetPositions(it->second);
            for (uint32_t i = 0; i < it->second.size(); i++)
            {
                beamInfo[std::make_pair(0, it->first)].AppendUt(GetNextUtUserCount());
            }
        }

        if (mobileUtsFolder != "")
        {
            m_satHelper->LoadMobileUTsFromFolder(0, mobileUtsFolder, m_utMobileUserCount);
        }

        // Now, create either a scenario based on list positions in input file
        // or create a generic scenario with UT positions configured by other ways..
        if (m_enableInputFileUtListPositions)
        {
            m_satHelper->CreateUserDefinedScenarioFromListPositions(
                0,
                beamInfo,
                m_inputFileUtPositionsCheckBeams);
        }
        else
        {
            m_satHelper->CreateUserDefinedScenario(beamInfo);
        }
    }
    else
    {
        m_satHelper->CreatePredefinedScenario(scenario);
    }

    NS_LOG_INFO(ss.str());

    m_groupHelper->Init(m_satHelper->UtNodes());

    return m_satHelper;
}

bool
SimulationHelper::HasSinkInstalled(Ptr<Node> node, uint16_t port)
{
    NS_LOG_FUNCTION(this << node->GetId() << port);

    for (uint32_t i = 0; i < node->GetNApplications(); i++)
    {
        auto sink = DynamicCast<PacketSink>(node->GetApplication(i));
        if (sink != NULL)
        {
            AddressValue av;
            sink->GetAttribute("Local", av);
            if (InetSocketAddress::ConvertFrom(av.Get()).GetPort() == port)
            {
                return true;
            }
        }
    }
    return false;
}

void
SimulationHelper::InstallTrafficModel(TrafficModel_t trafficModel,
                                      TransportLayerProtocol_t protocol,
                                      TrafficDirection_t direction,
                                      Time startTime,
                                      Time stopTime,
                                      Time startDelay,
                                      double percentage)
{
    NS_LOG_FUNCTION(this);

    std::string socketFactory =
        protocol == SimulationHelper::TCP ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";

    // get users
    NodeContainer utAllUsers = m_satHelper->GetUtUsers();
    NodeContainer gwUsers = m_satHelper->GetGwUsers();
    NS_ASSERT_MSG(m_gwUserId < gwUsers.GetN(),
                  "The number of GW users configured was too low. " << m_gwUserId << " "
                                                                    << gwUsers.GetN());

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsers;
    for (uint32_t i = 0; i < utAllUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsers.Add(utAllUsers.Get(i));
        }
    }

    std::cout << "Installing traffic model on " << utUsers.GetN() << "/" << utAllUsers.GetN()
              << " UT users" << std::endl;

    switch (trafficModel)
    {
    case SimulationHelper::CBR: {
        uint16_t port = 9;
        InetSocketAddress gwUserAddr =
            InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(m_gwUserId)), port);

        PacketSinkHelper sinkHelper(socketFactory, Address());
        CbrHelper cbrHelper(socketFactory, Address());
        ApplicationContainer sinkContainer;
        ApplicationContainer cbrContainer;
        if (direction == SimulationHelper::RTN_LINK)
        {
            // create sink application on GW user
            if (!HasSinkInstalled(gwUsers.Get(m_gwUserId), port))
            {
                sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                sinkContainer.Add(sinkHelper.Install(gwUsers.Get(m_gwUserId)));
            }

            cbrHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));

            // create CBR applications on UT users
            for (uint32_t i = 0; i < utUsers.GetN(); i++)
            {
                auto app = cbrHelper.Install(utUsers.Get(i)).Get(0);
                app->SetStartTime(startTime + (i + 1) * startDelay);
                cbrContainer.Add(app);
            }
        }
        else if (direction == SimulationHelper::FWD_LINK)
        {
            // create CBR applications on UT users
            for (uint32_t i = 0; i < utUsers.GetN(); i++)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);
                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                cbrHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));
                auto app = cbrHelper.Install(gwUsers.Get(m_gwUserId)).Get(0);
                app->SetStartTime(startTime + (i + 1) * startDelay);
                cbrContainer.Add(app);
            }
        }
        sinkContainer.Start(startTime);
        sinkContainer.Stop(stopTime);
    }
    break;

    case SimulationHelper::ONOFF: {
        uint16_t port = 9;
        InetSocketAddress gwUserAddr =
            InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(m_gwUserId)), port);

        PacketSinkHelper sinkHelper(socketFactory, Address());
        SatOnOffHelper onOffHelper(socketFactory, Address());
        ApplicationContainer sinkContainer;
        ApplicationContainer onOffContainer;
        if (direction == SimulationHelper::RTN_LINK)
        {
            // create sink application on GW user
            if (!HasSinkInstalled(gwUsers.Get(m_gwUserId), port))
            {
                sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                sinkContainer.Add(sinkHelper.Install(gwUsers.Get(m_gwUserId)));
            }

            onOffHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));

            // create OnOff applications on UT users
            for (uint32_t i = 0; i < utUsers.GetN(); i++)
            {
                auto app = onOffHelper.Install(utUsers.Get(i)).Get(0);
                app->SetStartTime(startTime + (i + 1) * startDelay);
                onOffContainer.Add(app);
            }
        }
        else if (direction == SimulationHelper::FWD_LINK)
        {
            // create OnOff applications on UT users
            for (uint32_t i = 0; i < utUsers.GetN(); i++)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);

                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                onOffHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));
                auto app = onOffHelper.Install(gwUsers.Get(m_gwUserId)).Get(0);
                app->SetStartTime(startTime + (i + 1) * startDelay);
                onOffContainer.Add(app);
            }
        }
        sinkContainer.Start(startTime);
        sinkContainer.Stop(stopTime);
    }
    break;

    case SimulationHelper::HTTP: {
        ThreeGppHttpHelper httpHelper;
        // Since more content should be transferred from server to clients,
        // we call server behind GW and clients behind UTs scenario DOWNLINK
        if (direction == SimulationHelper::FWD_LINK)
        {
            auto apps = httpHelper.InstallUsingIpv4(gwUsers.Get(m_gwUserId), utUsers);
            for (uint32_t i = 1; i < apps.GetN(); i++)
            {
                apps.Get(i)->SetStartTime(startTime + (i + 1) * startDelay);
            }
        }
        // An unlikely, but possible scenario where a HTTP server
        // (e.g. web user interface of a device) is reachable only by satellite.
        // Note that parameters should be defined by user.
        // We also assume that a single gateway user accesses all HTTP servers.
        // Modify this if other scenarios are required.
        else if (direction == SimulationHelper::RTN_LINK)
        {
            for (uint32_t i = 0; i < utUsers.GetN(); i++)
            {
                auto apps = httpHelper.InstallUsingIpv4(utUsers.Get(i), gwUsers.Get(m_gwUserId));
                apps.Get(1)->SetStartTime(startTime + (i + 1) * startDelay);
            }
        }
        httpHelper.GetServer().Start(startTime);
        httpHelper.GetServer().Stop(stopTime);
    }
    break;

    case SimulationHelper::NRTV: {
        NrtvHelper nrtvHelper(TypeId::LookupByName(socketFactory));
        // Since more content should be transferred from server to clients,
        // we call server behind GW and clients behind UTs scenario DOWNLINK
        if (direction == SimulationHelper::FWD_LINK)
        {
            auto apps = nrtvHelper.InstallUsingIpv4(gwUsers.Get(m_gwUserId), utUsers);
            for (uint32_t i = 1; i < apps.GetN(); i++)
            {
                apps.Get(i)->SetStartTime(startTime + (i + 1) * startDelay);
            }
        }
        // An unlikely, but possible scenario where an NRTV server
        // (e.g. video surveillance feed) is reachable only by satellite.
        // Note that parameters should be defined by user.
        // We also assume that a single gateway user accesses all NRTV servers.
        // Modify this if other scenarios are required.
        else if (direction == SimulationHelper::RTN_LINK)
        {
            for (uint32_t i = 0; i < utUsers.GetN(); i++)
            {
                auto apps = nrtvHelper.InstallUsingIpv4(utUsers.Get(i), gwUsers.Get(m_gwUserId));
                apps.Get(1)->SetStartTime(startTime + (i + 1) * startDelay);
            }
        }
        nrtvHelper.GetServer().Start(startTime);
        nrtvHelper.GetServer().Stop(stopTime);
    }
    break;

    default:
        NS_FATAL_ERROR("Invalid traffic model");
        break;
    }
}

void
SimulationHelper::InstallLoraTrafficModel(LoraTrafficModel_t trafficModel,
                                          Time interval,
                                          uint32_t packetSize,
                                          Time startTime,
                                          Time stopTime,
                                          Time startDelay)
{
    NS_LOG_FUNCTION(this << trafficModel << interval << packetSize << startTime << stopTime);

    NodeContainer nodes = GetSatelliteHelper()->UtNodes();
    NodeContainer utUsers = m_satHelper->GetUtUsers();
    Ptr<Node> node;

    std::cout << "Installing Lora traffic model on " << nodes.GetN() << " UTs" << std::endl;

    switch (trafficModel)
    {
    case SimulationHelper::PERIODIC: {
        for (uint32_t i = 0; i < nodes.GetN(); i++)
        {
            node = nodes.Get(i);
            Ptr<LoraPeriodicSender> app = Create<LoraPeriodicSender>();

            app->SetInterval(interval);
            NS_LOG_DEBUG("Created an application with interval = " << interval.GetHours()
                                                                   << " hours");

            app->SetStartTime(startTime + (i + 1) * startDelay);
            app->SetStopTime(stopTime);
            app->SetPacketSize(packetSize);

            app->SetNode(node);
            node->AddApplication(app);
        }
        break;
    }
    case SimulationHelper::LORA_CBR: {
        NodeContainer gwUsers = m_satHelper->GetGwUsers();

        uint16_t port = 9;
        InetSocketAddress gwUserAddr =
            InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(m_gwUserId)), port);

        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", Address());
        CbrHelper cbrHelper("ns3::UdpSocketFactory", Address());
        ApplicationContainer sinkContainer;
        ApplicationContainer cbrContainer;

        // create sink application on GW user
        if (!HasSinkInstalled(gwUsers.Get(m_gwUserId), port))
        {
            sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
            sinkContainer.Add(sinkHelper.Install(gwUsers.Get(m_gwUserId)));
        }

        cbrHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));

        // create CBR applications on UT users
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            auto app = cbrHelper.Install(utUsers.Get(i)).Get(0);
            app->SetStartTime(startTime + (i + 1) * startDelay);
            cbrContainer.Add(app);
        }

        sinkContainer.Start(startTime);
        sinkContainer.Stop(stopTime);
        break;
    }
    case SimulationHelper::ONE_SHOT:
    default:
        NS_FATAL_ERROR("Traffic Model for Lora not implemented yet");
    }
}

void
SimulationHelper::SetCrTxConf(CrTxConf_t crTxConf)
{
    switch (crTxConf)
    {
    case CR_PERIODIC_CONTROL: {
        Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel",
                           EnumValue(SatEnums::RA_MODEL_OFF));
        Config::SetDefault("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue(true));
        break;
    }
    case CR_SLOTTED_ALOHA: {
        Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel",
                           EnumValue(SatEnums::RA_MODEL_SLOTTED_ALOHA));
        Config::SetDefault("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue(false));
        break;
    }
    case CR_CRDSA_LOOSE_RC_0: {
        Config::SetDefault("ns3::SatBeamHelper::RandomAccessModel",
                           EnumValue(SatEnums::RA_MODEL_CRDSA));
        Config::SetDefault("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue(false));
        Config::SetDefault("ns3::SatUtHelper::UseCrdsaOnlyForControlPackets", BooleanValue(false));
        break;
    }
    default: {
        NS_FATAL_ERROR("Unsupported crTxConf: " << crTxConf);
        break;
    }
    }
}

void
SimulationHelper::SetBeams(const std::string& enabledBeams)
{
    NS_LOG_FUNCTION(this << enabledBeams);

    m_enabledBeamsStr = enabledBeams;
    m_enabledBeams.clear();
    std::stringstream bss(enabledBeams);

    while (!bss.eof())
    {
        uint32_t beamId;
        bss >> beamId;
        if (bss.fail())
        {
            bss.clear();
            std::string garbage;
            bss >> garbage;
        }
        else
        {
            m_enabledBeams.insert(beamId);
        }
    }
}

void
SimulationHelper::SetBeamSet(std::set<uint32_t> beamSet)
{
    NS_LOG_FUNCTION(this);

    m_enabledBeams = beamSet;
    std::stringstream bss;
    for (auto beamId : beamSet)
    {
        bss << beamId << " ";
    }
    m_enabledBeamsStr = bss.str();
}

const std::set<uint32_t>&
SimulationHelper::GetBeams()
{
    NS_LOG_FUNCTION(this);

    return m_enabledBeams;
}

bool
SimulationHelper::IsBeamEnabled(uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << beamId);

    bool beamEnabled = false;

    if (m_enabledBeams.find(beamId) != m_enabledBeams.end())
    {
        beamEnabled = true;
    }

    return beamEnabled;
}

uint32_t
SimulationHelper::GetNextUtCount(uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << beamId);

    auto iter = m_utCount.find(beamId);
    if (iter != m_utCount.end())
    {
        return m_utCount.at(beamId)->GetInteger();
    }

    return m_utCount.at(0)->GetInteger();
}

void
SimulationHelper::RunSimulation()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("--- " << m_simulationName << "---");
    NS_LOG_INFO("  Simulation length: " << m_simTime.GetSeconds());
    NS_LOG_INFO("  Enabled beams: " << m_enabledBeamsStr);
    NS_LOG_INFO("  Number of UTs: " << m_satHelper->GetGwUsers().GetN());
    NS_LOG_INFO("  Number of end users: " << m_satHelper->GetUtUsers().GetN());

    Simulator::Stop(m_simTime);
    Simulator::Run();

    Simulator::Destroy();
}

void
SimulationHelper::EnableProgressLogs()
{
    NS_LOG_FUNCTION(this);

    if (!m_progressLoggingEnabled)
    {
        if (GetSimTime().GetSeconds() > 20)
        {
            m_progressUpdateInterval = Seconds(GetSimTime().GetSeconds() / 100);
        }
        else
        {
            m_progressUpdateInterval = Seconds(0.2);
        }
        m_progressReportEvent =
            Simulator::Schedule(m_progressUpdateInterval, &SimulationHelper::ProgressCb, this);
        m_progressLoggingEnabled = true;
    }
}

void
SimulationHelper::DisableProgressLogs()
{
    NS_LOG_FUNCTION(this);

    m_progressLoggingEnabled = false;
    m_progressReportEvent.Cancel();
}

void
SimulationHelper::ConfigureAttributesFromFile(std::string filePath,
                                              bool overrideManualConfiguration)
{
    ReadInputAttributesFromFile(filePath);
    Ptr<SimulationHelperConf> simulationConf = CreateObject<SimulationHelperConf>();

    if (overrideManualConfiguration)
    {
        SetBeams(simulationConf->m_enabledBeams);
        SetUtCountPerBeam(simulationConf->m_utCount);
        SetUserCountPerUt(simulationConf->m_utUserCount);
        SetUserCountPerMobileUt(simulationConf->m_utMobileUserCount);
        SetSimulationTime(simulationConf->m_simTime);
    }

    CreateSatScenario(SatHelper::NONE, simulationConf->m_mobileUtsFolder);
    if (simulationConf->m_activateProgressLogging)
    {
        EnableProgressLogs();
    }

    for (const auto& trafficModel : simulationConf->m_trafficModel)
    {
        TrafficModel_t modelName;
        if (trafficModel.first == "Cbr")
        {
            modelName = CBR;
        }
        else if (trafficModel.first == "OnOff")
        {
            modelName = ONOFF;
        }
        else if (trafficModel.first == "Http")
        {
            modelName = HTTP;
        }
        else if (trafficModel.first == "Nrtv")
        {
            modelName = NRTV;
        }
        else
        {
            NS_FATAL_ERROR("Unknown traffic model has been configured: " << trafficModel.first);
        }

        std::vector<SimulationHelper::TransportLayerProtocol_t> protocols;
        switch (trafficModel.second.m_protocol)
        {
        case SimulationHelperConf::PROTOCOL_UDP: {
            protocols.push_back(SimulationHelper::UDP);
            break;
        }
        case SimulationHelperConf::PROTOCOL_TCP: {
            protocols.push_back(SimulationHelper::TCP);
            break;
        }
        case SimulationHelperConf::PROTOCOL_BOTH: {
            protocols.push_back(SimulationHelper::TCP);
            protocols.push_back(SimulationHelper::UDP);
            break;
        }
        default: {
            NS_FATAL_ERROR("Unknown traffic protocol");
        }
        }

        std::vector<SimulationHelper::TrafficDirection_t> directions;
        switch (trafficModel.second.m_direction)
        {
        case SimulationHelperConf::RTN_LINK: {
            directions.push_back(SimulationHelper::RTN_LINK);
            break;
        }
        case SimulationHelperConf::FWD_LINK: {
            directions.push_back(SimulationHelper::FWD_LINK);
            break;
        }
        case SimulationHelperConf::BOTH_LINK: {
            directions.push_back(SimulationHelper::FWD_LINK);
            directions.push_back(SimulationHelper::RTN_LINK);
            break;
        }
        default: {
            NS_FATAL_ERROR("Unknown traffic protocol");
        }
        }

        if (trafficModel.second.m_percentage > 0.0)
        {
            Time startTime = trafficModel.second.m_startTime;
            if (startTime > m_simTime)
            {
                NS_FATAL_ERROR("Traffic model "
                               << trafficModel.first
                               << " configured to start after the simulation ended");
            }

            Time stopTime = trafficModel.second.m_stopTime;
            if (stopTime == Seconds(0))
            {
                stopTime = m_simTime + Seconds(1);
            }
            if (stopTime < startTime)
            {
                NS_FATAL_ERROR("Traffic model " << trafficModel.first
                                                << " configured to stop before it is started");
            }

            for (auto& protocol : protocols)
            {
                for (auto& direction : directions)
                {
                    InstallTrafficModel(modelName,
                                        protocol,
                                        direction,
                                        startTime,
                                        stopTime,
                                        trafficModel.second.m_startDelay);
                }
            }
        }
    }

    if (simulationConf->m_activateStatistics)
    {
        CreateDefaultStats();
    }
}

void
SimulationHelper::ReadInputAttributesFromFile(std::string filePath)
{
    NS_LOG_FUNCTION(this << filePath);

    // To read attributes from file
    Config::SetDefault("ns3::ConfigStore::Filename", StringValue(filePath));
    Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Load"));
    Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("Xml"));
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults();
}

std::string
SimulationHelper::StoreAttributesToFile(std::string fileName, bool outputAttributes)
{
    NS_LOG_FUNCTION(this);

    std::string outputPath;
    outputPath = Singleton<SatEnvVariables>::Get()->GetOutputPath();

    //  Store set attribute values to XML output file
    Config::SetDefault("ns3::ConfigStore::Filename", StringValue(outputPath + "/" + fileName));
    Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("Xml"));
    Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Save"));
    ConfigStore outputConfig;
    outputConfig.ConfigureDefaults();

    if (outputAttributes)
    {
        outputConfig.ConfigureAttributes();
    }

    return outputPath;
}

} // namespace ns3
