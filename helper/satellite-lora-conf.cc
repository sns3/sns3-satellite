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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-lora-conf.h"

#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-wave-form-conf.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/string.h"

NS_LOG_COMPONENT_DEFINE("SatLoraConf");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatLoraConf);

TypeId
SatLoraConf::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatLoraConf")
                            .SetParent<Object>()
                            .AddConstructor<SatLoraConf>()
                            .AddAttribute("Standard",
                                          "Standard used for phy layer configuration",
                                          EnumValue(SatLoraConf::SATELLITE),
                                          MakeEnumAccessor<SatLoraConf::PhyLayerStandard_t>(&SatLoraConf::m_phyLayerStandard),
                                          MakeEnumChecker(SatLoraConf::SATELLITE,
                                                          "Satellite",
                                                          SatLoraConf::EU863_870,
                                                          "EU863-870"));
    return tid;
}

TypeId
SatLoraConf::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatLoraConf::SatLoraConf()
{
    NS_LOG_FUNCTION(this);

    ObjectBase::ConstructSelf(AttributeConstructionList());
}

void
SatLoraConf::SetConf(Ptr<LorawanMacGateway> gatewayMac)
{
    NS_LOG_FUNCTION(this << gatewayMac);

    switch (m_phyLayerStandard)
    {
    case SatLoraConf::SATELLITE: {
        SetSatelliteConf(gatewayMac);
        break;
    }
    case SatLoraConf::EU863_870: {
        SetEu863_870Conf(gatewayMac);
        break;
    }
    }
}

void
SatLoraConf::SetConf(Ptr<LorawanMacEndDeviceClassA> endDeviceMac)
{
    NS_LOG_FUNCTION(this << endDeviceMac);

    switch (m_phyLayerStandard)
    {
    case SatLoraConf::SATELLITE: {
        SetSatelliteConf(endDeviceMac);
        break;
    }
    case SatLoraConf::EU863_870: {
        SetEu863_870Conf(endDeviceMac);
        break;
    }
    }
}

void
SatLoraConf::setSatConfAttributes(Ptr<SatConf> satConf)
{
    NS_LOG_FUNCTION(this << satConf);

    double baseFrequency;
    double bandwidth;
    uint32_t channels;
    double allocatedBandwidth;

    switch (m_phyLayerStandard)
    {
    case SatLoraConf::SATELLITE: {
        baseFrequency = 1.5e9;
        bandwidth = 125000;
        channels = 1;
        allocatedBandwidth = 125000;
        break;
    }
    case SatLoraConf::EU863_870: {
        baseFrequency = 0.868e9;
        bandwidth = 375000;
        channels = 3;
        allocatedBandwidth = 125000;
        break;
    }
    default: {
        NS_FATAL_ERROR("Unknown physical layer standard");
    }
    }

    satConf->SetAttribute("FwdFeederLinkBandwidth", DoubleValue(bandwidth));
    satConf->SetAttribute("FwdFeederLinkBaseFrequency", DoubleValue(baseFrequency));
    satConf->SetAttribute("RtnFeederLinkBandwidth", DoubleValue(bandwidth));
    satConf->SetAttribute("RtnFeederLinkBaseFrequency", DoubleValue(baseFrequency));
    satConf->SetAttribute("FwdUserLinkBandwidth", DoubleValue(bandwidth));
    satConf->SetAttribute("FwdUserLinkBaseFrequency", DoubleValue(baseFrequency));
    satConf->SetAttribute("RtnUserLinkBandwidth", DoubleValue(bandwidth));
    satConf->SetAttribute("RtnUserLinkBaseFrequency", DoubleValue(baseFrequency));
    satConf->SetAttribute("FwdUserLinkChannels", UintegerValue(channels));
    satConf->SetAttribute("RtnUserLinkChannels", UintegerValue(channels));
    satConf->SetAttribute("FwdFeederLinkChannels", UintegerValue(channels));
    satConf->SetAttribute("RtnFeederLinkChannels", UintegerValue(channels));
    satConf->SetAttribute("FwdCarrierAllocatedBandwidth", DoubleValue(allocatedBandwidth));
}

void
SatLoraConf::SetEu863_870Conf(Ptr<LorawanMacGateway> gatewayMac)
{
    NS_LOG_FUNCTION(this << gatewayMac);

    LoraLogicalChannelHelper channelHelper;

    channelHelper.AddLoraSubBand(868, 868.6, 0.01, 14);
    channelHelper.AddLoraSubBand(868.7, 869.2, 0.001, 14);
    channelHelper.AddLoraSubBand(869.4, 869.65, 0.1, 27);

    Ptr<LoraLogicalChannel> lc1 = CreateObject<LoraLogicalChannel>(868.1, 0, 5);
    Ptr<LoraLogicalChannel> lc2 = CreateObject<LoraLogicalChannel>(868.3, 0, 5);
    Ptr<LoraLogicalChannel> lc3 = CreateObject<LoraLogicalChannel>(868.5, 0, 5);
    channelHelper.AddChannel(lc1);
    channelHelper.AddChannel(lc2);
    channelHelper.AddChannel(lc3);

    gatewayMac->SetLoraLogicalChannelHelper(channelHelper);

    gatewayMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    gatewayMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    gatewayMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

void
SatLoraConf::SetEu863_870Conf(Ptr<LorawanMacEndDeviceClassA> endDeviceMac)
{
    NS_LOG_FUNCTION(this << endDeviceMac);

    LoraLogicalChannelHelper channelHelper;

    channelHelper.AddLoraSubBand(868, 868.6, 0.01, 14);
    channelHelper.AddLoraSubBand(868.7, 869.2, 0.001, 14);
    channelHelper.AddLoraSubBand(869.4, 869.65, 0.1, 27);

    Ptr<LoraLogicalChannel> lc1 = CreateObject<LoraLogicalChannel>(868.1, 0, 5);
    Ptr<LoraLogicalChannel> lc2 = CreateObject<LoraLogicalChannel>(868.3, 0, 5);
    Ptr<LoraLogicalChannel> lc3 = CreateObject<LoraLogicalChannel>(868.5, 0, 5);
    channelHelper.AddChannel(lc1);
    channelHelper.AddChannel(lc2);
    channelHelper.AddChannel(lc3);

    endDeviceMac->SetLoraLogicalChannelHelper(channelHelper);

    endDeviceMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    endDeviceMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    endDeviceMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});

    LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                               {{1, 0, 0, 0, 0, 0}},
                                               {{2, 1, 0, 0, 0, 0}},
                                               {{3, 2, 1, 0, 0, 0}},
                                               {{4, 3, 2, 1, 0, 0}},
                                               {{5, 4, 3, 2, 1, 0}},
                                               {{6, 5, 4, 3, 2, 1}},
                                               {{7, 6, 5, 4, 3, 2}}}};
    endDeviceMac->SetReplyDataRateMatrix(matrix);

    endDeviceMac->SetNPreambleSymbols(8);

    // TODO should be zero but transmission time is higher than propagation time. This is not
    // possible right now. Revert to zero when possible.
    endDeviceMac->SetSecondReceiveWindowDataRate(4);
    endDeviceMac->SetSecondReceiveWindowFrequency(869.525);
}

void
SatLoraConf::SetSatelliteConf(Ptr<LorawanMacGateway> gatewayMac)
{
    NS_LOG_FUNCTION(this << gatewayMac);

    LoraLogicalChannelHelper channelHelper;

    // TODO check values

    // L -> 1.5GHz
    // S -> 3GHz

    // void AddLoraSubBand (double firstFrequency, double lastFrequency, double dutyCycle, double
    // maxTxPowerDbm) LoraLogicalChannel::LoraLogicalChannel (double frequency, uint8_t minDataRate,
    // uint8_t maxDataRate)

    channelHelper.AddLoraSubBand(1500, 1500.25, 0.0, 14);

    Ptr<LoraLogicalChannel> lc1 = CreateObject<LoraLogicalChannel>(1500.125, 0, 5);
    channelHelper.AddChannel(lc1);

    gatewayMac->SetLoraLogicalChannelHelper(channelHelper);

    gatewayMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    gatewayMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    gatewayMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

void
SatLoraConf::SetSatelliteConf(Ptr<LorawanMacEndDeviceClassA> endDeviceMac)
{
    NS_LOG_FUNCTION(this << endDeviceMac);

    LoraLogicalChannelHelper channelHelper;

    // TODO check values

    channelHelper.AddLoraSubBand(1500, 1500.25, 0.0, 14);

    Ptr<LoraLogicalChannel> lc1 = CreateObject<LoraLogicalChannel>(1500.125, 0, 5);
    channelHelper.AddChannel(lc1);

    endDeviceMac->SetLoraLogicalChannelHelper(channelHelper);

    endDeviceMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    endDeviceMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    endDeviceMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});

    LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                               {{1, 0, 0, 0, 0, 0}},
                                               {{2, 1, 0, 0, 0, 0}},
                                               {{3, 2, 1, 0, 0, 0}},
                                               {{4, 3, 2, 1, 0, 0}},
                                               {{5, 4, 3, 2, 1, 0}},
                                               {{6, 5, 4, 3, 2, 1}},
                                               {{7, 6, 5, 4, 3, 2}}}};
    endDeviceMac->SetReplyDataRateMatrix(matrix);

    endDeviceMac->SetNPreambleSymbols(8);

    // TODO should be zero but transmission time is higher than propagation time. This is not
    // possible right now. Revert to zero when possible.
    endDeviceMac->SetSecondReceiveWindowDataRate(4);
    endDeviceMac->SetSecondReceiveWindowFrequency(869.525);
}

} // namespace ns3
