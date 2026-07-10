/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#ifndef __SATELLITE_TRAFFIC_HELPER_H__
#define __SATELLITE_TRAFFIC_HELPER_H__

#include "satellite-helper.h"

#include "ns3/application-container.h"
#include "ns3/cbr-application.h"
#include "ns3/config.h"
#include "ns3/object.h"
#include "ns3/satellite-stats-helper-container.h"
#include "ns3/string.h"

#include <map>
#include <stdint.h>
#include <string>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace ns3
{
/**
 * @brief Creates pre-defined trafics.
 */
class SatTrafficHelper : public Object
{
  public:
    /**
     * @brief List of available traffics
     */
    typedef enum
    {
        LORA_PERIODIC, // implemented
        LORA_CBR,      // implemented
        CBR,           // implemented
        ONOFF,         // implemented
        HTTP,          // implemented
        NRTV,          // implemented
        POISSON,       // implemented
        VOIP,          // implemented
        CUSTOM         // implemented
    } TrafficType_t;

    typedef enum
    {
        RTN_LINK,
        FWD_LINK
    } TrafficDirection_t;

    typedef enum
    {
        UDP,
        TCP
    } TransportLayerProtocol_t;

    typedef enum
    {
        G_711_1,
        G_711_2,
        G_723_1,
        G_729_2,
        G_729_3
    } VoipCodec_t;

    /**
     * @brief Get the type ID
     * @return the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * @brief Default constructor. Not used.
     */
    SatTrafficHelper();

    /**
     * @brief Create a base SatTrafficHelper for creating customized traffics.
     */
    SatTrafficHelper(Ptr<SatHelper> satHelper,
                     Ptr<SatStatsHelperContainer> satStatsHelperContainer);

    /**
     * Destructor for SatTrafficHelper
     */
    virtual ~SatTrafficHelper()
    {
    }

    /**
     * Add Lora periodic traffic between chosen GWs and UTs
     * @param interval Wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param uts The User Terminals
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddLoraPeriodicTraffic(Time interval,
                                uint32_t packetSize,
                                NodeContainer uts,
                                Time startTime,
                                Time stopTime,
                                Time startDelay);

    /**
     * Add Lora periodic traffic between chosen GWs and UTs
     * @param interval Wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param uts The User Terminals
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddLoraPeriodicTraffic(Time interval,
                                uint32_t packetSize,
                                NodeContainer uts,
                                Time startTime,
                                Time stopTime,
                                Time startDelay,
                                double percentage);

    /**
     * Add Lora CBR traffic between chosen GWs and UTs
     * @param interval Wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddLoraCbrTraffic(Time interval,
                           uint32_t packetSize,
                           NodeContainer gwUsers,
                           NodeContainer utUsers,
                           Time startTime,
                           Time stopTime,
                           Time startDelay);

    /**
     * Add Lora CBR traffic between chosen GWs and UTs
     * @param interval Wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddLoraCbrTraffic(Time interval,
                           uint32_t packetSize,
                           NodeContainer gwUsers,
                           NodeContainer utUsers,
                           Time startTime,
                           Time stopTime,
                           Time startDelay,
                           double percentage);

    /**
     * Add a new CBR traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param protocol Transport layer protocol
     * @param interval Wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddCbrTraffic(TrafficDirection_t direction,
                       TransportLayerProtocol_t protocol,
                       Time interval,
                       uint32_t packetSize,
                       NodeContainer gwUsers,
                       NodeContainer utUsers,
                       Time startTime,
                       Time stopTime,
                       Time startDelay);

    /**
     * Add a new CBR traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param protocol Transport layer protocol
     * @param interval Wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddCbrTraffic(TrafficDirection_t direction,
                       TransportLayerProtocol_t protocol,
                       Time interval,
                       uint32_t packetSize,
                       NodeContainer gwUsers,
                       NodeContainer utUsers,
                       Time startTime,
                       Time stopTime,
                       Time startDelay,
                       double percentage);

    /**
     * Add a new ONOFF traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param protocol Transport layer protocol
     * @param dataRate Data rate in ON state
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param onTimePattern Pattern for ON state duration
     * @param offTimePattern Pattern for OFF state duration
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddOnOffTraffic(TrafficDirection_t direction,
                         TransportLayerProtocol_t protocol,
                         DataRate dataRate,
                         uint32_t packetSize,
                         NodeContainer gwUsers,
                         NodeContainer utUsers,
                         std::string onTimePattern,
                         std::string offTimePattern,
                         Time startTime,
                         Time stopTime,
                         Time startDelay);

    /**
     * Add a new ONOFF traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param protocol Transport layer protocol
     * @param dataRate Data rate in ON state
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param onTimePattern Pattern for ON state duration
     * @param offTimePattern Pattern for OFF state duration
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddOnOffTraffic(TrafficDirection_t direction,
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
                         double percentage);

    /**
     * Add a new TCP/HTTP traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddHttpTraffic(TrafficDirection_t direction,
                        NodeContainer gwUsers,
                        NodeContainer utUsers,
                        Time startTime,
                        Time stopTime,
                        Time startDelay);

    /**
     * Add a new TCP/HTTP traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddHttpTraffic(TrafficDirection_t direction,
                        NodeContainer gwUsers,
                        NodeContainer utUsers,
                        Time startTime,
                        Time stopTime,
                        Time startDelay,
                        double percentage);

    /**
     * Add a new TCP/NRTV traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddNrtvTraffic(TrafficDirection_t direction,
                        NodeContainer gwUsers,
                        NodeContainer utUsers,
                        Time startTime,
                        Time stopTime,
                        Time startDelay);

    /**
     * Add a new TCP/NRTV traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddNrtvTraffic(TrafficDirection_t direction,
                        NodeContainer gwUsers,
                        NodeContainer utUsers,
                        Time startTime,
                        Time stopTime,
                        Time startDelay,
                        double percentage);

    /**
     * Add a new Poisson traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param onTime On time duration in seconds
     * @param offTimeExpMean Off time mean in seconds. The off time follows an exponential law of
     * mean offTimeExpMean \param rate The rate with the unit \param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddPoissonTraffic(TrafficDirection_t direction,
                           Time onTime,
                           Time offTimeExpMean,
                           DataRate rate,
                           uint32_t packetSize,
                           NodeContainer gwUsers,
                           NodeContainer utUsers,
                           Time startTime,
                           Time stopTime,
                           Time startDelay);

    /**
     * Add a new Poisson traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param onTime On time duration in seconds
     * @param offTimeExpMean Off time mean in seconds. The off time follows an exponential law of
     * mean offTimeExpMean \param rate The rate with the unit \param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddPoissonTraffic(TrafficDirection_t direction,
                           Time onTime,
                           Time offTimeExpMean,
                           DataRate rate,
                           uint32_t packetSize,
                           NodeContainer gwUsers,
                           NodeContainer utUsers,
                           Time startTime,
                           Time stopTime,
                           Time startDelay,
                           double percentage);

    /**
     * Add a new Poisson traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param codec the Codec used
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddVoipTraffic(TrafficDirection_t direction,
                        VoipCodec_t codec,
                        NodeContainer gwUsers,
                        NodeContainer utUsers,
                        Time startTime,
                        Time stopTime,
                        Time startDelay);

    /**
     * Add a new Poisson traffic between chosen GWs and UTs
     * @param direction Direction of traffic
     * @param codec the Codec used
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     * @param percentage Percentage of UT users having the traffic installed
     */
    void AddVoipTraffic(TrafficDirection_t direction,
                        VoipCodec_t codec,
                        NodeContainer gwUsers,
                        NodeContainer utUsers,
                        Time startTime,
                        Time stopTime,
                        Time startDelay,
                        double percentage);

    /**
     * Add a new CBR traffic between chosen GWs and UTs that can be customized
     * @param direction Direction of traffic
     * @param interval Initial wait time between transmission of two packets
     * @param packetSize Packet size in bytes
     * @param gwUsers The Gateway Users
     * @param utUsers The UT Users
     * @param startTime Application Start time
     * @param stopTime Application stop time
     * @param startDelay application start delay between each user
     */
    void AddCustomTraffic(TrafficDirection_t direction,
                          std::string interval,
                          uint32_t packetSize,
                          NodeContainer gwUsers,
                          NodeContainer utUsers,
                          Time startTime,
                          Time stopTime,
                          Time startDelay);

    /**
     * Change the parameters of the last custom traffic created
     * @param delay Delay after traffic launch to apply the changes
     * @param interval New wait time between transmission of two packets
     * @param packetSize New packet size in bytes
     */
    void ChangeCustomTraffic(Time delay, std::string interval, uint32_t packetSize);

  private:
    /**
     * @brief Struct for info on last custom trafic created
     */
    typedef struct
    {
        ApplicationContainer application;
        Time start;
        Time stop;
        bool created;
    } CustomTrafficInfo_s;

    Ptr<SatHelper> m_satHelper; // Pointer to the SatHelper objet
    Ptr<SatStatsHelperContainer>
        m_satStatsHelperContainer; // Pointer to the SatStatsHelperContainer objet

    CustomTrafficInfo_s m_last_custom_application; // Last application container of custom traffic

    bool m_enableDefaultStatistics;

    /**
     * Update the chosen attribute of a custom traffic
     * @param application The CBR application to update
     * @param interval The new interval
     * @param packetSize the new packet size
     */
    void UpdateAttribute(Ptr<CbrApplication> application,
                         std::string interval,
                         uint32_t packetSize);

    /**
     * @brief Check if node has a PacketSink installed at certain port.
     */
    bool HasSinkInstalled(Ptr<Node> node, uint16_t port);
};

class SatTrafficHelperConf : public Object
{
  public:
    /**
     * Default constructor.
     */
    SatTrafficHelperConf();

    /**
     * Custom constructor.
     *
     * @param trafficHelper Pointer to the traffic helper
     * @param simTime Simulation time
     */
    SatTrafficHelperConf(Ptr<SatTrafficHelper> trafficHelper, Time simTime);

    /**
     * @brief Destructor.
     */
    virtual ~SatTrafficHelperConf();

    /**
     * @brief Derived from Object.
     */
    static TypeId GetTypeId(void);

    /**
     * @brief Install traffic models from configuration files
     */
    void InstallTrafficModels();

    typedef enum
    {
        PROTOCOL_UDP,
        PROTOCOL_TCP,
        PROTOCOL_BOTH
    } TransportLayerProtocol_t;

    typedef enum
    {
        RTN_LINK,
        FWD_LINK,
        BOTH_LINK
    } TrafficDirection_t;

    typedef struct TrafficConfiguration_t
    {
        TransportLayerProtocol_t m_protocol;
        TrafficDirection_t m_direction;
        Time m_interval;
        DataRate m_dataRate;
        uint32_t m_packetSize;
        std::string m_onTimePattern;
        std::string m_offTimePattern;
        Time m_onTime;
        Time m_offTime;
        SatTrafficHelper::VoipCodec_t m_codec;
        Time m_startTime;
        Time m_stopTime;
        Time m_startDelay;
        double m_percentage;

        TrafficConfiguration_t()
            : m_protocol(SatTrafficHelperConf::PROTOCOL_UDP),
              m_direction(SatTrafficHelperConf::RTN_LINK),
              m_interval(Seconds(1.0)),
              m_dataRate(DataRate("500kb/s")),
              m_packetSize(512),
              m_onTimePattern("ns3::ConstantRandomVariable[Constant=1000]"),
              m_offTimePattern("ns3::ConstantRandomVariable[Constant=0]"),
              m_onTime(Seconds(1)),
              m_offTime(MilliSeconds(100)),
              m_codec(SatTrafficHelper::VoipCodec_t::G_711_1),
              m_startTime(0),
              m_stopTime(0),
              m_startDelay(0),
              m_percentage(0.0)
        {
            // do nothing
        }
    } TrafficConfiguration_t;

    std::map<std::string, TrafficConfiguration_t> m_trafficModel;

  private:
    void SetTrafficProtocol(std::string trafficModel, TransportLayerProtocol_t protocol)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_protocol = protocol;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_protocol = protocol;
        }
    }

    TransportLayerProtocol_t GetTrafficProtocol(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_protocol;
    }

    void SetTrafficDirection(std::string trafficModel, TrafficDirection_t direction)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_direction = direction;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_direction = direction;
        }
    }

    TrafficDirection_t GetTrafficDirection(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_direction;
    }

    void SetTrafficInterval(std::string trafficModel, Time interval)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_interval = interval;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_interval = interval;
        }
    }

    Time GetTrafficInterval(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_interval;
    }

    void SetTrafficDataRate(std::string trafficModel, DataRate dataRate)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_dataRate = dataRate;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_dataRate = dataRate;
        }
    }

    DataRate GetTrafficDataRate(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_dataRate;
    }

    void SetTrafficPacketSize(std::string trafficModel, uint32_t packetSize)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_packetSize = packetSize;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_packetSize = packetSize;
        }
    }

    uint32_t GetTrafficPacketSize(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_packetSize;
    }

    void SetTrafficOnTimePattern(std::string trafficModel, std::string onTimePattern)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_onTimePattern = onTimePattern;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_onTimePattern = onTimePattern;
        }
    }

    std::string GetTrafficOnTimePattern(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_onTimePattern;
    }

    void SetTrafficOffTimePattern(std::string trafficModel, std::string offTimePattern)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_offTimePattern = offTimePattern;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_offTimePattern = offTimePattern;
        }
    }

    std::string GetTrafficOffTimePattern(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_offTimePattern;
    }

    void SetTrafficOnTime(std::string trafficModel, Time onTime)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_onTime = onTime;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_onTime = onTime;
        }
    }

    Time GetTrafficOnTime(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_onTime;
    }

    void SetTrafficOffTime(std::string trafficModel, Time offTime)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_offTime = offTime;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_offTime = offTime;
        }
    }

    Time GetTrafficOffTime(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_offTime;
    }

    void SetTrafficCodec(std::string trafficModel, SatTrafficHelper::VoipCodec_t codec)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_codec = codec;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_codec = codec;
        }
    }

    SatTrafficHelper::VoipCodec_t GetTrafficCodec(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_codec;
    }

    void SetTrafficStartTime(std::string trafficModel, Time startTime)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_startTime = startTime;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_startTime = startTime;
        }
    }

    Time GetTrafficStartTime(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_startTime;
    }

    void SetTrafficStopTime(std::string trafficModel, Time stopTime)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_startTime = stopTime;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_stopTime = stopTime;
        }
    }

    Time GetTrafficStopTime(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_stopTime;
    }

    void SetTrafficStartDelay(std::string trafficModel, Time startDelay)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_startDelay = startDelay;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_startDelay = startDelay;
        }
    }

    Time GetTrafficStartDelay(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_startDelay;
    }

    void SetTrafficPercentage(std::string trafficModel, double percentage)
    {
        std::map<std::string, TrafficConfiguration_t>::iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            TrafficConfiguration_t traffic;
            traffic.m_percentage = percentage;
            m_trafficModel.emplace(trafficModel, traffic);
        }
        else
        {
            it->second.m_percentage = percentage;
        }
    }

    double GetTrafficPercentage(std::string trafficModel) const
    {
        std::map<std::string, TrafficConfiguration_t>::const_iterator it =
            m_trafficModel.find(trafficModel);
        if (it == m_trafficModel.end())
        {
            NS_FATAL_ERROR("Traffic model " << trafficModel << " has not been configured");
        }

        return it->second.m_percentage;
    }

#define TRAFFIC_MODEL_PROTOCOL_ATTRIBUTE_ACCESSOR_DEFINE(index)                                    \
    inline void SetTraffic##index##Protocol(TransportLayerProtocol_t value)                        \
    {                                                                                              \
        return SetTrafficProtocol(TOSTRING(index), value);                                         \
    }                                                                                              \
    inline TransportLayerProtocol_t GetTraffic##index##Protocol() const                            \
    {                                                                                              \
        return GetTrafficProtocol(TOSTRING(index));                                                \
    }

#define TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(index)                                   \
    inline void SetTraffic##index##Direction(TrafficDirection_t value)                             \
    {                                                                                              \
        return SetTrafficDirection(TOSTRING(index), value);                                        \
    }                                                                                              \
    inline TrafficDirection_t GetTraffic##index##Direction() const                                 \
    {                                                                                              \
        return GetTrafficDirection(TOSTRING(index));                                               \
    }

#define TRAFFIC_MODEL_INTERVAL_ATTRIBUTE_ACCESSOR_DEFINE(index)                                    \
    inline void SetTraffic##index##Interval(Time value)                                            \
    {                                                                                              \
        return SetTrafficInterval(TOSTRING(index), value);                                         \
    }                                                                                              \
    inline Time GetTraffic##index##Interval() const                                                \
    {                                                                                              \
        return GetTrafficInterval(TOSTRING(index));                                                \
    }

#define TRAFFIC_MODEL_DATA_RATE_ATTRIBUTE_ACCESSOR_DEFINE(index)                                   \
    inline void SetTraffic##index##DataRate(DataRate value)                                        \
    {                                                                                              \
        return SetTrafficDataRate(TOSTRING(index), value);                                         \
    }                                                                                              \
    inline DataRate GetTraffic##index##DataRate() const                                            \
    {                                                                                              \
        return GetTrafficDataRate(TOSTRING(index));                                                \
    }

#define TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE_ACCESSOR_DEFINE(index)                                 \
    inline void SetTraffic##index##PacketSize(uint32_t value)                                      \
    {                                                                                              \
        return SetTrafficPacketSize(TOSTRING(index), value);                                       \
    }                                                                                              \
    inline uint32_t GetTraffic##index##PacketSize() const                                          \
    {                                                                                              \
        return GetTrafficPacketSize(TOSTRING(index));                                              \
    }

#define TRAFFIC_MODEL_ON_TIME_PATTERN_ATTRIBUTE_ACCESSOR_DEFINE(index)                             \
    inline void SetTraffic##index##OnTimePattern(std::string value)                                \
    {                                                                                              \
        return SetTrafficOnTimePattern(TOSTRING(index), value);                                    \
    }                                                                                              \
    inline std::string GetTraffic##index##OnTimePattern() const                                    \
    {                                                                                              \
        return GetTrafficOnTimePattern(TOSTRING(index));                                           \
    }

#define TRAFFIC_MODEL_OFF_TIME_PATTERN_ATTRIBUTE_ACCESSOR_DEFINE(index)                            \
    inline void SetTraffic##index##OffTimePattern(std::string value)                               \
    {                                                                                              \
        return SetTrafficOffTimePattern(TOSTRING(index), value);                                   \
    }                                                                                              \
    inline std::string GetTraffic##index##OffTimePattern() const                                   \
    {                                                                                              \
        return GetTrafficOffTimePattern(TOSTRING(index));                                          \
    }

#define TRAFFIC_MODEL_ON_TIME_ATTRIBUTE_ACCESSOR_DEFINE(index)                                     \
    inline void SetTraffic##index##OnTime(Time value)                                              \
    {                                                                                              \
        return SetTrafficOnTime(TOSTRING(index), value);                                           \
    }                                                                                              \
    inline Time GetTraffic##index##OnTime() const                                                  \
    {                                                                                              \
        return GetTrafficOnTime(TOSTRING(index));                                                  \
    }

#define TRAFFIC_MODEL_OFF_TIME_ATTRIBUTE_ACCESSOR_DEFINE(index)                                    \
    inline void SetTraffic##index##OffTime(Time value)                                             \
    {                                                                                              \
        return SetTrafficOffTime(TOSTRING(index), value);                                          \
    }                                                                                              \
    inline Time GetTraffic##index##OffTime() const                                                 \
    {                                                                                              \
        return GetTrafficOffTime(TOSTRING(index));                                                 \
    }

#define TRAFFIC_MODEL_CODEC_ATTRIBUTE_ACCESSOR_DEFINE(index)                                       \
    inline void SetTraffic##index##Codec(SatTrafficHelper::VoipCodec_t value)                      \
    {                                                                                              \
        return SetTrafficCodec(TOSTRING(index), value);                                            \
    }                                                                                              \
    inline SatTrafficHelper::VoipCodec_t GetTraffic##index##Codec() const                          \
    {                                                                                              \
        return GetTrafficCodec(TOSTRING(index));                                                   \
    }

#define TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(index)                                  \
    inline void SetTraffic##index##StartTime(Time value)                                           \
    {                                                                                              \
        return SetTrafficStartTime(TOSTRING(index), value);                                        \
    }                                                                                              \
    inline Time GetTraffic##index##StartTime() const                                               \
    {                                                                                              \
        return GetTrafficStartTime(TOSTRING(index));                                               \
    }

#define TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(index)                                   \
    inline void SetTraffic##index##StopTime(Time value)                                            \
    {                                                                                              \
        return SetTrafficStopTime(TOSTRING(index), value);                                         \
    }                                                                                              \
    inline Time GetTraffic##index##StopTime() const                                                \
    {                                                                                              \
        return GetTrafficStopTime(TOSTRING(index));                                                \
    }

#define TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(index)                                 \
    inline void SetTraffic##index##StartDelay(Time value)                                          \
    {                                                                                              \
        return SetTrafficStartDelay(TOSTRING(index), value);                                       \
    }                                                                                              \
    inline Time GetTraffic##index##StartDelay() const                                              \
    {                                                                                              \
        return GetTrafficStartDelay(TOSTRING(index));                                              \
    }

#define TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(index)                                  \
    inline void SetTraffic##index##Percentage(double value)                                        \
    {                                                                                              \
        return SetTrafficPercentage(TOSTRING(index), value);                                       \
    }                                                                                              \
    inline double GetTraffic##index##Percentage() const                                            \
    {                                                                                              \
        return GetTrafficPercentage(TOSTRING(index));                                              \
    }

    TRAFFIC_MODEL_INTERVAL_ATTRIBUTE_ACCESSOR_DEFINE(LoraPeriodic);
    TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE_ACCESSOR_DEFINE(LoraPeriodic);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(LoraPeriodic);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(LoraPeriodic);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(LoraPeriodic);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(LoraPeriodic);

    TRAFFIC_MODEL_INTERVAL_ATTRIBUTE_ACCESSOR_DEFINE(LoraCbr);
    TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE_ACCESSOR_DEFINE(LoraCbr);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(LoraCbr);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(LoraCbr);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(LoraCbr);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(LoraCbr);

    TRAFFIC_MODEL_PROTOCOL_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_INTERVAL_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(Cbr);

    TRAFFIC_MODEL_PROTOCOL_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_DATA_RATE_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_ON_TIME_PATTERN_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_OFF_TIME_PATTERN_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(OnOff);

    TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(Http);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Http);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Http);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(Http);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(Http);

    TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(Nrtv);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Nrtv);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Nrtv);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(Nrtv);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(Nrtv);

    TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_ON_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_OFF_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_DATA_RATE_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_PACKET_SIZE_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(Poisson);

    TRAFFIC_MODEL_DIRECTION_ATTRIBUTE_ACCESSOR_DEFINE(Voip);
    TRAFFIC_MODEL_CODEC_ATTRIBUTE_ACCESSOR_DEFINE(Voip);
    TRAFFIC_MODEL_START_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Voip);
    TRAFFIC_MODEL_STOP_TIME_ATTRIBUTE_ACCESSOR_DEFINE(Voip);
    TRAFFIC_MODEL_START_DELAY_ATTRIBUTE_ACCESSOR_DEFINE(Voip);
    TRAFFIC_MODEL_PERCENTAGE_ATTRIBUTE_ACCESSOR_DEFINE(Voip);

    Time m_simTime;
    Ptr<SatTrafficHelper> m_trafficHelper;
};

} // namespace ns3

#endif /* __SATELLITE_TRAFFIC_HELPER_H__ */

// More generic
// call functons AddVoipTraffic, AddPoissonTraffic, etc.
// each one call a generic function (with for loops). Has a additionnal function -> subfunction, for
// traffic with parameters Subfunction (private) call GW*UT times
