/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
 * Copyright (c) 2020 CNES
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

#include "satellite-traffic-helper.h"

#include "satellite-on-off-helper.h"
#include "simulation-helper.h"

#include "ns3/cbr-helper.h"
#include "ns3/log.h"
#include "ns3/lora-periodic-sender.h"
#include "ns3/nrtv-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/satellite-topology.h"
#include "ns3/singleton.h"
#include "ns3/three-gpp-http-satellite-helper.h"
#include "ns3/type-id.h"

#include <string>
#include <utility>
#include <vector>

NS_LOG_COMPONENT_DEFINE("SatelliteTrafficHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatTrafficHelperConf);

/**
 * .SIM_ADD_TRAFFIC_MODEL_ATTRIBUTES macro helps defining specific attribute
 * for traffic models in method GetTypeId.
 *
 * @param index Name of the traffic model which attributes are added to the configuration.
 * @param value Attribute value
 *
 * @return TypeId
 */
#define SIM_ADD_TRAFFIC_MODEL_PROTOCOL_ATTRIBUTE(index, value)                                     \
    AddAttribute("Traffic" TOSTRING(index) "Protocol",                                             \
                 "Network protocol that this traffic model will use",                              \
                 EnumValue(value),                                                                 \
                 MakeEnumAccessor<SatTrafficHelperConf::TransportLayerProtocol_t>(                 \
                     &SatTrafficHelperConf::SetTraffic##index##Protocol,                           \
                     &SatTrafficHelperConf::GetTraffic##index##Protocol),                          \
                 MakeEnumChecker(SatTrafficHelperConf::PROTOCOL_UDP,                               \
                                 "UDP",                                                            \
                                 SatTrafficHelperConf::PROTOCOL_TCP,                               \
                                 "TCP",                                                            \
                                 SatTrafficHelperConf::PROTOCOL_BOTH,                              \
                                 "BOTH"))

#define SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(index, value)                                    \
    AddAttribute("Traffic" TOSTRING(index) "Direction",                                            \
                 "Satellite link direction that this traffic model will use",                      \
                 EnumValue(value),                                                                 \
                 MakeEnumAccessor<SatTrafficHelperConf::TrafficDirection_t>(                       \
                     &SatTrafficHelperConf::SetTraffic##index##Direction,                          \
                     &SatTrafficHelperConf::GetTraffic##index##Direction),                         \
                 MakeEnumChecker(SatTrafficHelperConf::RTN_LINK,                                   \
                                 "ReturnLink",                                                     \
                                 SatTrafficHelperConf::FWD_LINK,                                   \
                                 "ForwardLink",                                                    \
                                 SatTrafficHelperConf::BOTH_LINK,                                  \
                                 "BothLinks"))

#define SIM_ADD_TRAFFIC_MODEL_INTERVAL_ATTRIBUTE(index, value)                                     \
    AddAttribute("Traffic" TOSTRING(index) "Interval",                                             \
                 "Interval between packets",                                                       \
                 TimeValue(value),                                                                 \
                 MakeTimeAccessor(&SatTrafficHelperConf::SetTraffic##index##Interval,              \
                                  &SatTrafficHelperConf::GetTraffic##index##Interval),             \
                 MakeTimeChecker())

#define SIM_ADD_TRAFFIC_MODEL_DATA_RATE_ATTRIBUTE(index, value)                                    \
    AddAttribute("Traffic" TOSTRING(index) "DataRate",                                             \
                 "Data rate of traffic",                                                           \
                 DataRateValue(value),                                                             \
                 MakeDataRateAccessor(&SatTrafficHelperConf::SetTraffic##index##DataRate,          \
                                      &SatTrafficHelperConf::GetTraffic##index##DataRate),         \
                 MakeDataRateChecker())

#define SIM_ADD_TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE(index, value)                                  \
    AddAttribute("Traffic" TOSTRING(index) "PacketSize",                                           \
                 "Packet size in bytes",                                                           \
                 UintegerValue(value),                                                             \
                 MakeUintegerAccessor(&SatTrafficHelperConf::SetTraffic##index##PacketSize,        \
                                      &SatTrafficHelperConf::GetTraffic##index##PacketSize),       \
                 MakeUintegerChecker<uint32_t>())

#define SIM_ADD_TRAFFIC_MODEL_ON_TIME_PATTERN_ATTRIBUTE(index, value)                              \
    AddAttribute("Traffic" TOSTRING(index) "OnTimePattern",                                        \
                 "On time patter for on/off traffic",                                              \
                 StringValue(value),                                                               \
                 MakeStringAccessor(&SatTrafficHelperConf::SetTraffic##index##OnTimePattern,       \
                                    &SatTrafficHelperConf::GetTraffic##index##OnTimePattern),      \
                 MakeStringChecker())

#define SIM_ADD_TRAFFIC_MODEL_OFF_TIME_PATTERN_ATTRIBUTE(index, value)                             \
    AddAttribute("Traffic" TOSTRING(index) "OffTimePattern",                                       \
                 "Off time patter for on/off traffic",                                             \
                 StringValue(value),                                                               \
                 MakeStringAccessor(&SatTrafficHelperConf::SetTraffic##index##OffTimePattern,      \
                                    &SatTrafficHelperConf::GetTraffic##index##OffTimePattern),     \
                 MakeStringChecker())

#define SIM_ADD_TRAFFIC_MODEL_ON_TIME_ATTRIBUTE(index, value)                                      \
    AddAttribute("Traffic" TOSTRING(index) "OnTime",                                               \
                 "On time value for Poisson traffic",                                              \
                 TimeValue(value),                                                                 \
                 MakeTimeAccessor(&SatTrafficHelperConf::SetTraffic##index##OnTime,                \
                                  &SatTrafficHelperConf::GetTraffic##index##OnTime),               \
                 MakeTimeChecker())

#define SIM_ADD_TRAFFIC_MODEL_OFF_TIME_ATTRIBUTE(index, value)                                     \
    AddAttribute("Traffic" TOSTRING(index) "OffTime",                                              \
                 "Off time value for Poisson traffic",                                             \
                 TimeValue(value),                                                                 \
                 MakeTimeAccessor(&SatTrafficHelperConf::SetTraffic##index##OffTime,               \
                                  &SatTrafficHelperConf::GetTraffic##index##OffTime),              \
                 MakeTimeChecker())

#define SIM_ADD_TRAFFIC_MODEL_CODEC_ATTRIBUTE(index, value)                                        \
    AddAttribute("Traffic" TOSTRING(index) "Codec",                                                \
                 "Codec used for VoIP traffic",                                                    \
                 EnumValue(value),                                                                 \
                 MakeEnumAccessor<SatTrafficHelper::VoipCodec_t>(                                  \
                     &SatTrafficHelperConf::SetTraffic##index##Codec,                              \
                     &SatTrafficHelperConf::GetTraffic##index##Codec),                             \
                 MakeEnumChecker(SatTrafficHelper::G_711_1,                                        \
                                 "G_711_1",                                                        \
                                 SatTrafficHelper::G_711_2,                                        \
                                 "G_711_2",                                                        \
                                 SatTrafficHelper::G_723_1,                                        \
                                 "G_723_1",                                                        \
                                 SatTrafficHelper::G_729_2,                                        \
                                 "G_729_2",                                                        \
                                 SatTrafficHelper::G_729_3,                                        \
                                 "G_729_3"))

#define SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(index, value)                                   \
    AddAttribute("Traffic" TOSTRING(index) "StartTime",                                            \
                 "Time into the simulation when this traffic model will be started on each user",  \
                 TimeValue(value),                                                                 \
                 MakeTimeAccessor(&SatTrafficHelperConf::SetTraffic##index##StartTime,             \
                                  &SatTrafficHelperConf::GetTraffic##index##StartTime),            \
                 MakeTimeChecker())

#define SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(index, value)                                    \
    AddAttribute("Traffic" TOSTRING(index) "StopTime",                                             \
                 "Time into the simulation when this traffic model will be stopped "               \
                 "on each user. 0 means endless traffic generation.",                              \
                 TimeValue(value),                                                                 \
                 MakeTimeAccessor(&SatTrafficHelperConf::SetTraffic##index##StopTime,              \
                                  &SatTrafficHelperConf::GetTraffic##index##StopTime),             \
                 MakeTimeChecker())

#define SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(index, value)                                  \
    AddAttribute("Traffic" TOSTRING(index) "StartDelay",                                           \
                 "Cummulative delay for each user before starting this traffic model",             \
                 TimeValue(value),                                                                 \
                 MakeTimeAccessor(&SatTrafficHelperConf::SetTraffic##index##StartDelay,            \
                                  &SatTrafficHelperConf::GetTraffic##index##StartDelay),           \
                 MakeTimeChecker())

#define SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(index, value)                                   \
    AddAttribute("Traffic" TOSTRING(index) "Percentage",                                           \
                 "Percentage of final users that will use this traffic model",                     \
                 DoubleValue(value),                                                               \
                 MakeDoubleAccessor(&SatTrafficHelperConf::SetTraffic##index##Percentage,          \
                                    &SatTrafficHelperConf::GetTraffic##index##Percentage),         \
                 MakeDoubleChecker<double>(0, 1))

TypeId
SatTrafficHelperConf::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatTrafficHelperConf")
            .SetParent<Object>()
            .AddConstructor<SatTrafficHelperConf>()
            .SIM_ADD_TRAFFIC_MODEL_INTERVAL_ATTRIBUTE(LoraPeriodic, Seconds(1))
            .SIM_ADD_TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE(LoraPeriodic, 512)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(LoraPeriodic, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(LoraPeriodic, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(LoraPeriodic, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(LoraPeriodic, 0)

            .SIM_ADD_TRAFFIC_MODEL_INTERVAL_ATTRIBUTE(LoraCbr, Seconds(1))
            .SIM_ADD_TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE(LoraCbr, 512)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(LoraCbr, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(LoraCbr, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(LoraCbr, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(LoraCbr, 0)

            .SIM_ADD_TRAFFIC_MODEL_PROTOCOL_ATTRIBUTE(Cbr, SatTrafficHelperConf::PROTOCOL_UDP)
            .SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(Cbr, SatTrafficHelperConf::RTN_LINK)
            .SIM_ADD_TRAFFIC_MODEL_INTERVAL_ATTRIBUTE(Cbr, Seconds(1))
            .SIM_ADD_TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE(Cbr, 512)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(Cbr, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(Cbr, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(Cbr, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(Cbr, 1)

            .SIM_ADD_TRAFFIC_MODEL_PROTOCOL_ATTRIBUTE(OnOff, SatTrafficHelperConf::PROTOCOL_UDP)
            .SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(OnOff, SatTrafficHelperConf::RTN_LINK)
            .SIM_ADD_TRAFFIC_MODEL_DATA_RATE_ATTRIBUTE(OnOff, DataRate("500kb/s"))
            .SIM_ADD_TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE(OnOff, 512)
            .SIM_ADD_TRAFFIC_MODEL_ON_TIME_PATTERN_ATTRIBUTE(
                OnOff,
                "ns3::ConstantRandomVariable[Constant=1000]")
            .SIM_ADD_TRAFFIC_MODEL_OFF_TIME_PATTERN_ATTRIBUTE(
                OnOff,
                "ns3::ConstantRandomVariable[Constant=0]")
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(OnOff, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(OnOff, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(OnOff, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(OnOff, 0)

            .SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(Http, SatTrafficHelperConf::RTN_LINK)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(Http, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(Http, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(Http, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(Http, 0)

            .SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(Nrtv, SatTrafficHelperConf::RTN_LINK)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(Nrtv, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(Nrtv, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(Nrtv, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(Nrtv, 0)

            .SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(Poisson, SatTrafficHelperConf::RTN_LINK)
            .SIM_ADD_TRAFFIC_MODEL_ON_TIME_ATTRIBUTE(Poisson, Seconds(1))
            .SIM_ADD_TRAFFIC_MODEL_OFF_TIME_ATTRIBUTE(Poisson, MilliSeconds(100))
            .SIM_ADD_TRAFFIC_MODEL_DATA_RATE_ATTRIBUTE(Poisson, DataRate("500kb/s"))
            .SIM_ADD_TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE(Poisson, 512)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(Poisson, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(Poisson, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(Poisson, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(Poisson, 0)

            .SIM_ADD_TRAFFIC_MODEL_DIRECTION_ATTRIBUTE(Voip, SatTrafficHelperConf::RTN_LINK)
            .SIM_ADD_TRAFFIC_MODEL_CODEC_ATTRIBUTE(Voip, SatTrafficHelper::VoipCodec_t::G_711_1)
            .SIM_ADD_TRAFFIC_MODEL_START_TIME_ATTRIBUTE(Voip, Seconds(0.1))
            .SIM_ADD_TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE(Voip, Seconds(0))
            .SIM_ADD_TRAFFIC_MODEL_START_DELAY_ATTRIBUTE(Voip, MilliSeconds(10))
            .SIM_ADD_TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE(Voip, 0);

    return tid;
}

SatTrafficHelperConf::SatTrafficHelperConf()
    : m_simTime(Seconds(0)),
      m_trafficHelper(nullptr)
{
    NS_LOG_FUNCTION(this);

    NS_FATAL_ERROR("Default constructor not in use");
}

SatTrafficHelperConf::SatTrafficHelperConf(Ptr<SatTrafficHelper> trafficHelper, Time simTime)
    : m_simTime(simTime),
      m_trafficHelper(trafficHelper)
{
    NS_LOG_FUNCTION(this << simTime << trafficHelper);
}

SatTrafficHelperConf::~SatTrafficHelperConf()
{
    NS_LOG_FUNCTION(this);
}

NS_OBJECT_ENSURE_REGISTERED(SatTrafficHelper);

TypeId
SatTrafficHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatTrafficHelper")
            .SetParent<Object>()
            .AddConstructor<SatTrafficHelper>()
            .AddAttribute("EnableDefaultStatistics",
                          "Enable outputing values from stats helpers",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SatTrafficHelper::m_enableDefaultStatistics),
                          MakeBooleanChecker());
    return tid;
}

SatTrafficHelper::SatTrafficHelper()
    : m_satHelper(nullptr),
      m_satStatsHelperContainer(nullptr)
{
    NS_FATAL_ERROR("Default constructor of SatTrafficHelper cannot be used.");
}

SatTrafficHelper::SatTrafficHelper(Ptr<SatHelper> satHelper,
                                   Ptr<SatStatsHelperContainer> satStatsHelperContainer)
    : m_satHelper(satHelper),
      m_satStatsHelperContainer(satStatsHelperContainer),
      m_enableDefaultStatistics(false)
{
    m_last_custom_application.created = false;
}

void
SatTrafficHelper::AddLoraPeriodicTraffic(Time interval,
                                         uint32_t packetSize,
                                         NodeContainer uts,
                                         Time startTime,
                                         Time stopTime,
                                         Time startDelay)
{
    NS_LOG_FUNCTION(this << interval << packetSize << startTime << stopTime << startDelay);

    if (uts.GetN() == 0)
    {
        NS_LOG_WARN("UT container is empty");
        return;
    }

    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", Address());
    CbrHelper cbrHelper("ns3::UdpSocketFactory", Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer cbrContainer;

    Ptr<Node> node;

    // create Lora Periodic sender applications from UT users to GW users
    for (uint32_t i = 0; i < uts.GetN(); i++)
    {
        node = uts.Get(i);
        Ptr<LoraPeriodicSender> app = CreateObject<LoraPeriodicSender>();

        app->SetInterval(interval);
        NS_LOG_DEBUG("Created an application with interval = " << interval.GetHours() << " hours");

        app->SetStartTime(startTime + (i + 1) * startDelay);
        app->SetStopTime(stopTime);
        app->SetPacketSize(packetSize);

        app->SetNode(node);
        node->AddApplication(app);
    }
}

void
SatTrafficHelper::AddLoraPeriodicTraffic(Time interval,
                                         uint32_t packetSize,
                                         NodeContainer uts,
                                         Time startTime,
                                         Time stopTime,
                                         Time startDelay,
                                         double percentage)
{
    NS_LOG_FUNCTION(this << interval << packetSize << startTime << stopTime << startDelay
                         << percentage);

    // Filter UTs to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utsUpdated;
    for (uint32_t i = 0; i < uts.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utsUpdated.Add(uts.Get(i));
        }
    }

    AddLoraPeriodicTraffic(interval, packetSize, utsUpdated, startTime, stopTime, startDelay);
}

void
SatTrafficHelper::AddLoraCbrTraffic(Time interval,
                                    uint32_t packetSize,
                                    NodeContainer gwUsers,
                                    NodeContainer utUsers,
                                    Time startTime,
                                    Time stopTime,
                                    Time startDelay)
{
    NS_LOG_FUNCTION(this << interval << packetSize << startTime << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    uint16_t port = 9;

    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", Address());
    CbrHelper cbrHelper("ns3::UdpSocketFactory", Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer cbrContainer;

    // create CBR applications from UT users to GW users
    for (uint32_t j = 0; j < gwUsers.GetN(); j++)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            InetSocketAddress gwUserAddr =
                InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(j)), port);
            if (!HasSinkInstalled(gwUsers.Get(j), port))
            {
                sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                sinkContainer.Add(sinkHelper.Install(gwUsers.Get(j)));
            }

            cbrHelper.SetConstantTraffic(interval, packetSize);
            cbrHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));
            auto app = cbrHelper.Install(utUsers.Get(i)).Get(0);
            app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
            cbrContainer.Add(app);
        }
    }

    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime);
}

void
SatTrafficHelper::AddLoraCbrTraffic(Time interval,
                                    uint32_t packetSize,
                                    NodeContainer gwUsers,
                                    NodeContainer utUsers,
                                    Time startTime,
                                    Time stopTime,
                                    Time startDelay,
                                    double percentage)
{
    NS_LOG_FUNCTION(this << interval << packetSize << startTime << stopTime << startDelay
                         << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddLoraCbrTraffic(interval,
                      packetSize,
                      gwUsers,
                      utUsersUpdated,
                      startTime,
                      stopTime,
                      startDelay);
}

void
SatTrafficHelper::AddCbrTraffic(TrafficDirection_t direction,
                                TransportLayerProtocol_t protocol,
                                Time interval,
                                uint32_t packetSize,
                                NodeContainer gwUsers,
                                NodeContainer utUsers,
                                Time startTime,
                                Time stopTime,
                                Time startDelay)
{
    NS_LOG_FUNCTION(this << interval << packetSize << startTime << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    std::string socketFactory =
        (protocol == SatTrafficHelper::TCP ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory");
    uint16_t port = 9;

    PacketSinkHelper sinkHelper(socketFactory, Address());
    CbrHelper cbrHelper(socketFactory, Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer cbrContainer;

    // create CBR applications from GWs to UT users
    for (uint32_t j = 0; j < gwUsers.GetN(); j++)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            if (direction == RTN_LINK)
            {
                InetSocketAddress gwUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(j)), port);
                if (!HasSinkInstalled(gwUsers.Get(j), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(gwUsers.Get(j)));
                }

                cbrHelper.SetConstantTraffic(interval, packetSize);
                cbrHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));
                auto app = cbrHelper.Install(utUsers.Get(i)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                cbrContainer.Add(app);
            }
            else if (direction == FWD_LINK)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);
                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                cbrHelper.SetConstantTraffic(interval, packetSize);
                cbrHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));
                auto app = cbrHelper.Install(gwUsers.Get(j)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                cbrContainer.Add(app);
            }
        }
    }

    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime);

    if (m_enableDefaultStatistics)
    {
        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::AddCbrTraffic(TrafficDirection_t direction,
                                TransportLayerProtocol_t protocol,
                                Time interval,
                                uint32_t packetSize,
                                NodeContainer gwUsers,
                                NodeContainer utUsers,
                                Time startTime,
                                Time stopTime,
                                Time startDelay,
                                double percentage)
{
    NS_LOG_FUNCTION(this << interval << packetSize << startTime << stopTime << startDelay
                         << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddCbrTraffic(direction,
                  protocol,
                  interval,
                  packetSize,
                  gwUsers,
                  utUsersUpdated,
                  startTime,
                  stopTime,
                  startDelay);
}

void
SatTrafficHelper::AddOnOffTraffic(TrafficDirection_t direction,
                                  TransportLayerProtocol_t protocol,
                                  DataRate dataRate,
                                  uint32_t packetSize,
                                  NodeContainer gwUsers,
                                  NodeContainer utUsers,
                                  std::string onTimePattern,
                                  std::string offTimePattern,
                                  Time startTime,
                                  Time stopTime,
                                  Time startDelay)
{
    NS_LOG_FUNCTION(this << dataRate << packetSize << onTimePattern << offTimePattern << startTime
                         << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    std::string socketFactory =
        (protocol == SatTrafficHelper::TCP ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory");
    uint16_t port = 9;

    PacketSinkHelper sinkHelper(socketFactory, Address());
    SatOnOffHelper onOffHelper(socketFactory, Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer onOffContainer;

    onOffHelper.SetAttribute("OnTime", StringValue(onTimePattern));
    onOffHelper.SetAttribute("OffTime", StringValue(offTimePattern));
    onOffHelper.SetAttribute("DataRate", DataRateValue(dataRate));
    onOffHelper.SetAttribute("PacketSize", UintegerValue(packetSize));

    // create ONOFF applications from GWs to UT users
    for (uint32_t j = 0; j < gwUsers.GetN(); j++)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            if (direction == RTN_LINK)
            {
                InetSocketAddress gwUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(j)), port);
                if (!HasSinkInstalled(gwUsers.Get(j), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(gwUsers.Get(j)));
                }

                onOffHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));
                auto app = onOffHelper.Install(utUsers.Get(i)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                onOffContainer.Add(app);
            }
            else if (direction == FWD_LINK)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);
                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                onOffHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));
                auto app = onOffHelper.Install(gwUsers.Get(j)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                onOffContainer.Add(app);
            }
        }
    }

    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime);

    if (m_enableDefaultStatistics)
    {
        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::AddOnOffTraffic(TrafficDirection_t direction,
                                  TransportLayerProtocol_t protocol,
                                  DataRate dataRate,
                                  uint32_t packetSize,
                                  NodeContainer gwUsers,
                                  NodeContainer utUsers,
                                  std::string onTimePattern,
                                  std::string offTimePattern,
                                  Time startTime,
                                  Time stopTime,
                                  Time startDelay,
                                  double percentage)
{
    NS_LOG_FUNCTION(this << dataRate << packetSize << onTimePattern << offTimePattern << startTime
                         << stopTime << startDelay << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddOnOffTraffic(direction,
                    protocol,
                    dataRate,
                    packetSize,
                    gwUsers,
                    utUsersUpdated,
                    onTimePattern,
                    offTimePattern,
                    startTime,
                    stopTime,
                    startDelay);
}

void
SatTrafficHelper::AddHttpTraffic(TrafficDirection_t direction,
                                 NodeContainer gwUsers,
                                 NodeContainer utUsers,
                                 Time startTime,
                                 Time stopTime,
                                 Time startDelay)
{
    NS_LOG_FUNCTION(this << direction << startTime << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    ThreeGppHttpHelper httpHelper;
    if (direction == FWD_LINK)
    {
        for (uint32_t j = 0; j < gwUsers.GetN(); j++)
        {
            auto app = httpHelper.InstallUsingIpv4(gwUsers.Get(j), utUsers).Get(1);
            app->SetStartTime(startTime + (j + 1) * startDelay);
            httpHelper.GetServer().Start(startTime);
            httpHelper.GetServer().Stop(stopTime);
        }
    }
    else if (direction == RTN_LINK)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            auto app = httpHelper.InstallUsingIpv4(utUsers.Get(i), gwUsers).Get(1);
            app->SetStartTime(startTime + (i + 1) * startDelay);
            httpHelper.GetServer().Start(startTime);
            httpHelper.GetServer().Stop(stopTime);
        }
    }

    if (m_enableDefaultStatistics)
    {
        // Add PLT statistics
        if (direction == FWD_LINK)
        {
            m_satStatsHelperContainer->AddGlobalFwdAppPlt(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdAppPlt(SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdAppPlt(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdAppPlt(SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdAppPlt(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdAppPlt(SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            m_satStatsHelperContainer->AddGlobalRtnAppPlt(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnAppPlt(SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnAppPlt(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnAppPlt(SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnAppPlt(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnAppPlt(SatStatsHelper::OUTPUT_SCATTER_FILE);
        }

        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::AddHttpTraffic(TrafficDirection_t direction,
                                 NodeContainer gwUsers,
                                 NodeContainer utUsers,
                                 Time startTime,
                                 Time stopTime,
                                 Time startDelay,
                                 double percentage)
{
    NS_LOG_FUNCTION(this << direction << startTime << stopTime << startDelay << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddHttpTraffic(direction, gwUsers, utUsersUpdated, startTime, stopTime, startDelay);
}

void
SatTrafficHelper::AddNrtvTraffic(TrafficDirection_t direction,
                                 NodeContainer gwUsers,
                                 NodeContainer utUsers,
                                 Time startTime,
                                 Time stopTime,
                                 Time startDelay)
{
    NS_LOG_FUNCTION(this << direction << startTime << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    std::string socketFactory = "ns3::TcpSocketFactory";

    NrtvHelper nrtvHelper(TypeId::LookupByName(socketFactory));
    if (direction == FWD_LINK)
    {
        for (uint32_t j = 0; j < gwUsers.GetN(); j++)
        {
            auto app = nrtvHelper.InstallUsingIpv4(gwUsers.Get(j), utUsers).Get(1);
            app->SetStartTime(startTime + (j + 1) * startDelay);
            nrtvHelper.GetServer().Start(startTime);
            nrtvHelper.GetServer().Stop(stopTime);
        }
    }
    else if (direction == RTN_LINK)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            auto app = nrtvHelper.InstallUsingIpv4(utUsers.Get(i), gwUsers).Get(1);
            app->SetStartTime(startTime + (i + 1) * startDelay);
            nrtvHelper.GetServer().Start(startTime);
            nrtvHelper.GetServer().Stop(stopTime);
        }
    }

    if (m_enableDefaultStatistics)
    {
        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::AddNrtvTraffic(TrafficDirection_t direction,
                                 NodeContainer gwUsers,
                                 NodeContainer utUsers,
                                 Time startTime,
                                 Time stopTime,
                                 Time startDelay,
                                 double percentage)
{
    NS_LOG_FUNCTION(this << direction << startTime << stopTime << startDelay << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddNrtvTraffic(direction, gwUsers, utUsersUpdated, startTime, stopTime, startDelay);
}

void
SatTrafficHelper::AddPoissonTraffic(TrafficDirection_t direction,
                                    Time onTime,
                                    Time offTimeExpMean,
                                    DataRate rate,
                                    uint32_t packetSize,
                                    NodeContainer gwUsers,
                                    NodeContainer utUsers,
                                    Time startTime,
                                    Time stopTime,
                                    Time startDelay)
{
    NS_LOG_FUNCTION(this << direction << onTime << offTimeExpMean << rate << packetSize << startTime
                         << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    std::string socketFactory = "ns3::UdpSocketFactory";

    uint16_t port = 9;

    PacketSinkHelper sinkHelper(socketFactory, Address());
    SatOnOffHelper onOffHelper(socketFactory, Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer onOffContainer;

    // create CBR applications from GWs to UT users
    for (uint32_t j = 0; j < gwUsers.GetN(); j++)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            if (direction == RTN_LINK)
            {
                InetSocketAddress gwUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(j)), port);

                if (!HasSinkInstalled(gwUsers.Get(j), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(gwUsers.Get(j)));
                }

                onOffHelper.SetAttribute("OnTime",
                                         StringValue("ns3::ConstantRandomVariable[Constant=" +
                                                     std::to_string(onTime.GetSeconds()) + "]"));
                onOffHelper.SetAttribute("OffTime",
                                         StringValue("ns3::ExponentialRandomVariable[Mean=" +
                                                     std::to_string(offTimeExpMean.GetSeconds()) +
                                                     "]"));
                onOffHelper.SetAttribute("DataRate", DataRateValue(rate));
                onOffHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
                onOffHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));

                auto app = onOffHelper.Install(utUsers.Get(i)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                onOffContainer.Add(app);
            }
            else if (direction == FWD_LINK)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);

                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                onOffHelper.SetAttribute("OnTime",
                                         StringValue("ns3::ConstantRandomVariable[Constant=" +
                                                     std::to_string(onTime.GetSeconds()) + "]"));
                onOffHelper.SetAttribute("OffTime",
                                         StringValue("ns3::ExponentialRandomVariable[Mean=" +
                                                     std::to_string(offTimeExpMean.GetSeconds()) +
                                                     "]"));
                onOffHelper.SetAttribute("DataRate", DataRateValue(rate));
                onOffHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
                onOffHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));

                auto app = onOffHelper.Install(gwUsers.Get(j)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                onOffContainer.Add(app);
            }
        }
    }
    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime);

    if (m_enableDefaultStatistics)
    {
        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::AddPoissonTraffic(TrafficDirection_t direction,
                                    Time onTime,
                                    Time offTimeExpMean,
                                    DataRate rate,
                                    uint32_t packetSize,
                                    NodeContainer gwUsers,
                                    NodeContainer utUsers,
                                    Time startTime,
                                    Time stopTime,
                                    Time startDelay,
                                    double percentage)
{
    NS_LOG_FUNCTION(this << direction << onTime << offTimeExpMean << rate << packetSize << startTime
                         << stopTime << startDelay << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddPoissonTraffic(direction,
                      onTime,
                      offTimeExpMean,
                      rate,
                      packetSize,
                      gwUsers,
                      utUsersUpdated,
                      startTime,
                      stopTime,
                      startDelay);
}

void
SatTrafficHelper::AddVoipTraffic(TrafficDirection_t direction,
                                 VoipCodec_t codec,
                                 NodeContainer gwUsers,
                                 NodeContainer utUsers,
                                 Time startTime,
                                 Time stopTime,
                                 Time startDelay)
{
    NS_LOG_FUNCTION(this << direction << codec << startTime << stopTime << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    std::string socketFactory = "ns3::UdpSocketFactory";
    uint16_t port = 9;

    double onTime;
    double offTime;
    std::string rate;
    uint32_t packetSize;

    switch (codec)
    {
    case G_711_1:
        onTime = 0.5;
        offTime = 0.05;
        rate = "70kbps"; // 64kbps globally
        packetSize = 80;
        break;
    case G_711_2:
        onTime = 0.5;
        offTime = 0.05;
        rate = "70kbps"; // 64kbps globally
        packetSize = 160;
        break;
    case G_723_1:
        onTime = 0.5;
        offTime = 0.05;
        rate = "6864bps"; // 6240bps globally
        packetSize = 30;
        break;
    case G_729_2:
        onTime = 0.5;
        offTime = 0.05;
        rate = "8800bps"; // 8kbps globally
        packetSize = 20;
        break;
    case G_729_3:
        onTime = 0.5;
        offTime = 0.05;
        rate = "7920bps"; // 7200bps globally
        packetSize = 30;
        break;
    default:
        NS_FATAL_ERROR("VoIP codec does not exist or is not implemented");
    }

    PacketSinkHelper sinkHelper(socketFactory, Address());
    SatOnOffHelper onOffHelper(socketFactory, Address());
    ApplicationContainer sinkContainer;
    ApplicationContainer onOffContainer;

    // create CBR applications from GWs to UT users
    for (uint32_t j = 0; j < gwUsers.GetN(); j++)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            if (direction == RTN_LINK)
            {
                InetSocketAddress gwUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(j)), port);

                if (!HasSinkInstalled(gwUsers.Get(j), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(gwUsers.Get(j)));
                }

                onOffHelper.SetAttribute("OnTime",
                                         StringValue("ns3::ConstantRandomVariable[Constant=" +
                                                     std::to_string(onTime) + "]"));
                onOffHelper.SetAttribute("OffTime",
                                         StringValue("ns3::ConstantRandomVariable[Constant=" +
                                                     std::to_string(offTime) + "]"));
                onOffHelper.SetAttribute("DataRate", DataRateValue(rate));
                onOffHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
                onOffHelper.SetAttribute("Remote", AddressValue(Address(gwUserAddr)));

                auto app = onOffHelper.Install(utUsers.Get(i)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                onOffContainer.Add(app);
            }
            else if (direction == FWD_LINK)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);

                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                onOffHelper.SetAttribute("OnTime",
                                         StringValue("ns3::ConstantRandomVariable[Constant=" +
                                                     std::to_string(onTime) + "]"));
                onOffHelper.SetAttribute("OffTime",
                                         StringValue("ns3::ConstantRandomVariable[Constant=" +
                                                     std::to_string(offTime) + "]"));
                onOffHelper.SetAttribute("DataRate", DataRateValue(rate));
                onOffHelper.SetAttribute("PacketSize", UintegerValue(packetSize));
                onOffHelper.SetAttribute("Remote", AddressValue(Address(utUserAddr)));

                auto app = onOffHelper.Install(gwUsers.Get(j)).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                onOffContainer.Add(app);
            }
        }
    }
    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime);

    if (m_enableDefaultStatistics)
    {
        // Add jitter statistics
        if (direction == FWD_LINK)
        {
            // Global
            m_satStatsHelperContainer->AddGlobalFwdAppJitter(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdAppJitter(SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT
            m_satStatsHelperContainer->AddPerUtFwdAppJitter(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdAppJitter(SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW
            m_satStatsHelperContainer->AddPerGwFwdAppJitter(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdAppJitter(SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global
            m_satStatsHelperContainer->AddGlobalRtnAppJitter(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnAppJitter(SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT
            m_satStatsHelperContainer->AddPerUtRtnAppJitter(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnAppJitter(SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW
            m_satStatsHelperContainer->AddPerGwRtnAppJitter(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnAppJitter(SatStatsHelper::OUTPUT_SCATTER_FILE);
        }

        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::AddVoipTraffic(TrafficDirection_t direction,
                                 VoipCodec_t codec,
                                 NodeContainer gwUsers,
                                 NodeContainer utUsers,
                                 Time startTime,
                                 Time stopTime,
                                 Time startDelay,
                                 double percentage)
{
    NS_LOG_FUNCTION(this << direction << codec << startTime << stopTime << startDelay
                         << percentage);

    // Filter UT users to keep only a given percentage on which installing the application
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    NodeContainer utUsersUpdated;
    for (uint32_t i = 0; i < utUsers.GetN(); ++i)
    {
        if (rng->GetValue(0.0, 1.0) < percentage)
        {
            utUsersUpdated.Add(utUsers.Get(i));
        }
    }

    AddVoipTraffic(direction, codec, gwUsers, utUsersUpdated, startTime, stopTime, startDelay);
}

void
SatTrafficHelper::AddCustomTraffic(TrafficDirection_t direction,
                                   std::string interval,
                                   uint32_t packetSize,
                                   NodeContainer gwUsers,
                                   NodeContainer utUsers,
                                   Time startTime,
                                   Time stopTime,
                                   Time startDelay)
{
    NS_LOG_FUNCTION(this << direction << interval << packetSize << startTime << stopTime
                         << startDelay);

    if (gwUsers.GetN() == 0)
    {
        NS_LOG_WARN("Gateway users container is empty");
        return;
    }
    if (utUsers.GetN() == 0)
    {
        NS_LOG_WARN("UT users container is empty");
        return;
    }

    std::string socketFactory = "ns3::UdpSocketFactory";
    uint16_t port = 9;

    PacketSinkHelper sinkHelper(socketFactory, Address());

    ObjectFactory factory;
    factory.SetTypeId("ns3::CbrApplication");
    factory.Set("Protocol", StringValue(socketFactory));
    ApplicationContainer sinkContainer;
    ApplicationContainer cbrContainer;

    // create CBR applications from GWs to UT users
    for (uint32_t j = 0; j < gwUsers.GetN(); j++)
    {
        for (uint32_t i = 0; i < utUsers.GetN(); i++)
        {
            if (direction == RTN_LINK)
            {
                InetSocketAddress gwUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(gwUsers.Get(j)), port);
                if (!HasSinkInstalled(gwUsers.Get(j), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(gwUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(gwUsers.Get(j)));
                }

                factory.Set("Interval", TimeValue(Time(interval)));
                factory.Set("PacketSize", UintegerValue(packetSize));
                factory.Set("Remote", AddressValue(Address(gwUserAddr)));
                Ptr<CbrApplication> p_app = factory.Create<CbrApplication>();
                utUsers.Get(i)->AddApplication(p_app);
                auto app = ApplicationContainer(p_app).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                cbrContainer.Add(app);
            }
            else if (direction == FWD_LINK)
            {
                InetSocketAddress utUserAddr =
                    InetSocketAddress(m_satHelper->GetUserAddress(utUsers.Get(i)), port);
                if (!HasSinkInstalled(utUsers.Get(i), port))
                {
                    sinkHelper.SetAttribute("Local", AddressValue(Address(utUserAddr)));
                    sinkContainer.Add(sinkHelper.Install(utUsers.Get(i)));
                }

                factory.Set("Interval", TimeValue(Time(interval)));
                factory.Set("PacketSize", UintegerValue(packetSize));
                factory.Set("Remote", AddressValue(Address(utUserAddr)));
                Ptr<CbrApplication> p_app = factory.Create<CbrApplication>();
                gwUsers.Get(j)->AddApplication(p_app);
                auto app = ApplicationContainer(p_app).Get(0);
                app->SetStartTime(startTime + (i + j * gwUsers.GetN() + 1) * startDelay);
                cbrContainer.Add(app);
            }
        }
    }

    sinkContainer.Start(startTime);
    sinkContainer.Stop(stopTime);

    m_last_custom_application.application = cbrContainer;
    m_last_custom_application.start = startTime;
    m_last_custom_application.stop = stopTime;
    m_last_custom_application.created = true;

    if (m_enableDefaultStatistics)
    {
        // Add throuhgput statistics
        if (direction == FWD_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwFwdAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwFwdUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
        else if (direction == RTN_LINK)
        {
            // Global scalar
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Global scatter
            m_satStatsHelperContainer->AddGlobalRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddGlobalRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per UT scalar
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per UT scatter
            m_satStatsHelperContainer->AddPerUtRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerUtRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);

            // Per GW scalar
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCALAR_FILE);

            // Per GW scatter
            m_satStatsHelperContainer->AddPerGwRtnAppThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnFeederMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
            m_satStatsHelperContainer->AddPerGwRtnUserMacThroughput(
                SatStatsHelper::OUTPUT_SCATTER_FILE);
        }
    }
}

void
SatTrafficHelper::ChangeCustomTraffic(Time delay, std::string interval, uint32_t packetSize)
{
    NS_LOG_FUNCTION(this << delay << interval << packetSize);

    if (!m_last_custom_application.created)
    {
        NS_FATAL_ERROR("No custom traffic created when calling the method "
                       "SatTrafficHelper::ChangeCustomTraffic for the first time.");
    }
    if (m_last_custom_application.start + delay > m_last_custom_application.stop)
    {
        NS_FATAL_ERROR("Custom traffic updated after its stop time.");
    }
    for (auto i = m_last_custom_application.application.Begin();
         i != m_last_custom_application.application.End();
         ++i)
    {
        Ptr<CbrApplication> app = (dynamic_cast<CbrApplication*>(PeekPointer(*i)));
        Simulator::Schedule(m_last_custom_application.start + delay,
                            &SatTrafficHelper::UpdateAttribute,
                            this,
                            app,
                            interval,
                            packetSize);
    }
}

void
SatTrafficHelper::UpdateAttribute(Ptr<CbrApplication> application,
                                  std::string interval,
                                  uint32_t packetSize)
{
    NS_LOG_FUNCTION(this << application << interval << packetSize);

    application->SetInterval(Time(interval));
    application->SetPacketSize(packetSize);
}

bool
SatTrafficHelper::HasSinkInstalled(Ptr<Node> node, uint16_t port)
{
    NS_LOG_FUNCTION(this << node->GetId() << port);

    for (uint32_t i = 0; i < node->GetNApplications(); i++)
    {
        auto sink = DynamicCast<PacketSink>(node->GetApplication(i));
        if (sink != nullptr)
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
SatTrafficHelperConf::InstallTrafficModels()
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT_MSG(m_trafficHelper != nullptr, "Traffic helper must be defined");

    for (const std::pair<const std::string, SatTrafficHelperConf::TrafficConfiguration_t>&
             trafficModel : m_trafficModel)
    {
        SatTrafficHelper::TrafficType_t modelName;
        if (trafficModel.first == "LoraCbr")
        {
            modelName = SatTrafficHelper::LORA_PERIODIC;
        }
        else if (trafficModel.first == "LoraPeriodic")
        {
            modelName = SatTrafficHelper::LORA_CBR;
        }
        else if (trafficModel.first == "Cbr")
        {
            modelName = SatTrafficHelper::CBR;
        }
        else if (trafficModel.first == "OnOff")
        {
            modelName = SatTrafficHelper::ONOFF;
        }
        else if (trafficModel.first == "Http")
        {
            modelName = SatTrafficHelper::HTTP;
        }
        else if (trafficModel.first == "Nrtv")
        {
            modelName = SatTrafficHelper::NRTV;
        }
        else if (trafficModel.first == "Poisson")
        {
            modelName = SatTrafficHelper::POISSON;
        }
        else if (trafficModel.first == "Voip")
        {
            modelName = SatTrafficHelper::VOIP;
        }
        else
        {
            NS_FATAL_ERROR("Unknown traffic model has been configured: " << trafficModel.first);
        }

        std::vector<SatTrafficHelper::TransportLayerProtocol_t> protocols;
        switch (trafficModel.second.m_protocol)
        {
        case SatTrafficHelperConf::PROTOCOL_UDP: {
            protocols.push_back(SatTrafficHelper::UDP);
            break;
        }
        case SatTrafficHelperConf::PROTOCOL_TCP: {
            protocols.push_back(SatTrafficHelper::TCP);
            break;
        }
        case SatTrafficHelperConf::PROTOCOL_BOTH: {
            protocols.push_back(SatTrafficHelper::TCP);
            protocols.push_back(SatTrafficHelper::UDP);
            break;
        }
        default: {
            NS_FATAL_ERROR("Unknown traffic protocol");
        }
        }

        std::vector<SatTrafficHelper::TrafficDirection_t> directions;
        switch (trafficModel.second.m_direction)
        {
        case SatTrafficHelperConf::RTN_LINK: {
            directions.push_back(SatTrafficHelper::RTN_LINK);
            break;
        }
        case SatTrafficHelperConf::FWD_LINK: {
            directions.push_back(SatTrafficHelper::FWD_LINK);
            break;
        }
        case SatTrafficHelperConf::BOTH_LINK: {
            directions.push_back(SatTrafficHelper::FWD_LINK);
            directions.push_back(SatTrafficHelper::RTN_LINK);
            break;
        }
        default: {
            NS_FATAL_ERROR("Unknown traffic protocol");
        }
        }

        if (trafficModel.second.m_percentage > 0.0)
        {
            SatTrafficHelperConf::TrafficConfiguration_t conf = trafficModel.second;
            Time startTime = conf.m_startTime;
            if (startTime > m_simTime)
            {
                NS_FATAL_ERROR("Traffic model "
                               << trafficModel.first
                               << " configured to start after the simulation ended");
            }

            Time stopTime = conf.m_stopTime;
            if (stopTime == Seconds(0))
            {
                stopTime = m_simTime + Seconds(1);
            }
            if (stopTime < startTime)
            {
                NS_FATAL_ERROR("Traffic model " << trafficModel.first
                                                << " configured to stop before it is started");
            }

            NodeContainer gws = Singleton<SatTopology>::Get()->GetGwUserNodes();
            NodeContainer uts;
            if (modelName == SatTrafficHelper::LORA_PERIODIC)
            {
                uts = Singleton<SatTopology>::Get()->GetUtNodes();
            }
            else
            {
                uts = Singleton<SatTopology>::Get()->GetUtUserNodes();
            }

            for (SatTrafficHelper::TransportLayerProtocol_t& protocol : protocols)
            {
                for (SatTrafficHelper::TrafficDirection_t& direction : directions)
                {
                    switch (modelName)
                    {
                    case SatTrafficHelper::LORA_PERIODIC: {
                        m_trafficHelper->AddLoraPeriodicTraffic(conf.m_interval,
                                                                conf.m_packetSize,
                                                                uts,
                                                                startTime,
                                                                stopTime,
                                                                conf.m_startDelay,
                                                                conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::LORA_CBR: {
                        m_trafficHelper->AddLoraCbrTraffic(conf.m_interval,
                                                           conf.m_packetSize,
                                                           gws,
                                                           uts,
                                                           startTime,
                                                           stopTime,
                                                           conf.m_startDelay,
                                                           conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::CBR: {
                        m_trafficHelper->AddCbrTraffic(direction,
                                                       protocol,
                                                       conf.m_interval,
                                                       conf.m_packetSize,
                                                       gws,
                                                       uts,
                                                       startTime,
                                                       stopTime,
                                                       conf.m_startDelay,
                                                       conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::ONOFF: {
                        m_trafficHelper->AddOnOffTraffic(direction,
                                                         protocol,
                                                         conf.m_dataRate,
                                                         conf.m_packetSize,
                                                         gws,
                                                         uts,
                                                         conf.m_onTimePattern,
                                                         conf.m_offTimePattern,
                                                         startTime,
                                                         stopTime,
                                                         conf.m_startDelay,
                                                         conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::HTTP: {
                        m_trafficHelper->AddHttpTraffic(direction,
                                                        gws,
                                                        uts,
                                                        startTime,
                                                        stopTime,
                                                        conf.m_startDelay,
                                                        conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::NRTV: {
                        m_trafficHelper->AddNrtvTraffic(direction,
                                                        gws,
                                                        uts,
                                                        startTime,
                                                        stopTime,
                                                        conf.m_startDelay,
                                                        conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::POISSON: {
                        m_trafficHelper->AddPoissonTraffic(direction,
                                                           conf.m_onTime,
                                                           conf.m_offTime,
                                                           conf.m_dataRate,
                                                           conf.m_packetSize,
                                                           gws,
                                                           uts,
                                                           startTime,
                                                           stopTime,
                                                           conf.m_startDelay,
                                                           conf.m_percentage);
                        break;
                    }
                    case SatTrafficHelper::VOIP: {
                        m_trafficHelper->AddVoipTraffic(direction,
                                                        conf.m_codec,
                                                        gws,
                                                        uts,
                                                        startTime,
                                                        stopTime,
                                                        conf.m_startDelay,
                                                        conf.m_percentage);
                        break;
                    }
                    default:
                        NS_FATAL_ERROR("Unknown traffic model has been configured: " << modelName);
                    }
                }
            }
        }
    }
}

} // namespace ns3
