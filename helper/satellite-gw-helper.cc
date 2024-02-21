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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Mathias Ettinger <mettinger@viveris.toulouse.fr>
 */

#include "ns3/satellite-gw-helper.h"

#include <ns3/boolean.h>
#include <ns3/config.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-link-results.h>
#include <ns3/satellite-lower-layer-service.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-typedefs.h>

NS_LOG_COMPONENT_DEFINE("SatGwHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatGwHelper);

TypeId
SatGwHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatGwHelper")
            .SetParent<Object>()
            .AddAttribute("DaRtnLinkInterferenceModel",
                          "Return link interference model for dedicated access",
                          EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET),
                          MakeEnumAccessor<SatPhyRxCarrierConf::InterferenceModel>(&SatGwHelper::m_daInterferenceModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::IF_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::IF_TRACE,
                                          "Trace",
                                          SatPhyRxCarrierConf::IF_PER_PACKET,
                                          "PerPacket",
                                          SatPhyRxCarrierConf::IF_PER_FRAGMENT,
                                          "PerFragment"))
            .AddAttribute("RtnLinkErrorModel",
                          "Return link error model for",
                          EnumValue(SatPhyRxCarrierConf::EM_AVI),
                          MakeEnumAccessor<SatPhyRxCarrierConf::ErrorModel>(&SatGwHelper::m_errorModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::EM_NONE,
                                          "None",
                                          SatPhyRxCarrierConf::EM_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::EM_AVI,
                                          "AVI"))
            .AddAttribute("FwdSchedulingAlgorithm",
                          "The scheduling algorithm used to fill the BBFrames",
                          EnumValue(SatEnums::NO_TIME_SLICING),
                          MakeEnumAccessor<SatEnums::FwdSchedulingAlgorithm_t>(&SatGwHelper::m_fwdSchedulingAlgorithm),
                          MakeEnumChecker(SatEnums::NO_TIME_SLICING,
                                          "NoTimeSlicing",
                                          SatEnums::TIME_SLICING,
                                          "TimeSlicing"))
            .AddAttribute("RtnLinkConstantErrorRate",
                          "Constant error rate",
                          DoubleValue(0.01),
                          MakeDoubleAccessor(&SatGwHelper::m_daConstantErrorRate),
                          MakeDoubleChecker<double>())
            .AddAttribute("EnableChannelEstimationError",
                          "Enable channel estimation error in return link receiver at GW.",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SatGwHelper::m_enableChannelEstimationError),
                          MakeBooleanChecker())
            .AddTraceSource("Creation",
                            "Creation traces",
                            MakeTraceSourceAccessor(&SatGwHelper::m_creationTrace),
                            "ns3::SatTypedefs::CreationCallback");
    return tid;
}

TypeId
SatGwHelper::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

SatGwHelper::SatGwHelper()
    : m_rtnLinkCarrierCount(0),
      m_daInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_errorModel(SatPhyRxCarrierConf::EM_AVI),
      m_daConstantErrorRate(0.0),
      m_symbolRate(0.0),
      m_enableChannelEstimationError(false),
      m_raSettings()
{
    // this default constructor should be never called
    NS_FATAL_ERROR("Default constructor not supported!!!");
}

SatGwHelper::SatGwHelper(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                         uint32_t rtnLinkCarrierCount,
                         Ptr<SatSuperframeSeq> seq,
                         SatMac::ReadCtrlMsgCallback readCb,
                         SatMac::ReserveCtrlMsgCallback reserveCb,
                         SatMac::SendCtrlMsgCallback sendCb,
                         RandomAccessSettings_s randomAccessSettings)
    : m_carrierBandwidthConverter(carrierBandwidthConverter),
      m_rtnLinkCarrierCount(rtnLinkCarrierCount),
      m_superframeSeq(seq),
      m_readCtrlCb(readCb),
      m_reserveCtrlCb(reserveCb),
      m_sendCtrlCb(sendCb),
      m_daInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_errorModel(SatPhyRxCarrierConf::EM_AVI),
      m_daConstantErrorRate(0.0),
      m_symbolRate(0.0),
      m_enableChannelEstimationError(false),
      m_raSettings(randomAccessSettings)
{
    NS_LOG_FUNCTION(this << rtnLinkCarrierCount << seq);

    m_deviceFactory.SetTypeId("ns3::SatNetDevice");
    m_channelFactory.SetTypeId("ns3::SatChannel");
}

void
SatGwHelper::Initialize(Ptr<SatLinkResultsRtn> lrRcs2,
                        Ptr<SatLinkResultsFwd> lrFwd,
                        SatEnums::DvbVersion_t dvbVersion,
                        bool useScpc)
{
    NS_LOG_FUNCTION(this);

    switch (m_fwdSchedulingAlgorithm)
    {
    case SatEnums::NO_TIME_SLICING:
        Config::SetDefault("ns3::SatBbFrameConf::PlHeaderInSlots", UintegerValue(1));
        break;
    case SatEnums::TIME_SLICING:
        Config::SetDefault("ns3::SatBbFrameConf::PlHeaderInSlots", UintegerValue(2));
        break;
    default:
        NS_FATAL_ERROR("Forward scheduling algorithm is not implemented");
    }

    // TODO: Usage of multiple carriers needed to take into account, now only one carrier assumed to
    // be used.
    // TODO: Symbol rate needed to check.
    m_symbolRate =
        m_carrierBandwidthConverter(SatEnums::FORWARD_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);

    /*
     * Return channel link results (DVB-RCS2) are created for GWs.
     */
    if (lrRcs2 && m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
        m_linkResults = lrRcs2;
    }
    if (useScpc)
    {
        m_linkResults = lrFwd;
    }

    m_bbFrameConf = CreateObject<SatBbFrameConf>(m_symbolRate, dvbVersion);
    m_bbFrameConf->InitializeCNoRequirements(lrFwd);

    // m_bbFrameConf->DumpWaveforms ();
}

Ptr<SatBbFrameConf>
SatGwHelper::GetBbFrameConf() const
{
    NS_LOG_FUNCTION(this);

    return m_bbFrameConf;
}

void
SatGwHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    m_deviceFactory.Set(n1, v1);
}

void
SatGwHelper::SetChannelAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    m_channelFactory.Set(n1, v1);
}

void
SatGwHelper::SetPhyAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    Config::SetDefault("ns3::SatGwPhy::" + n1, v1);
}

NetDeviceContainer
SatGwHelper::Install(NodeContainer c,
                     uint32_t gwId,
                     uint32_t satId,
                     uint32_t beamId,
                     uint32_t feederSatId,
                     uint32_t feederBeamId,
                     Ptr<SatChannel> fCh,
                     Ptr<SatChannel> rCh,
                     SatPhy::ChannelPairGetterCallback cbChannel,
                     Ptr<SatNcc> ncc,
                     Ptr<SatLowerLayerServiceConf> llsConf,
                     SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                     SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << gwId << satId << beamId << fCh << rCh << ncc << llsConf
                         << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    NetDeviceContainer devs;

    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); i++)
    {
        devs.Add(Install(*i,
                         gwId,
                         satId,
                         beamId,
                         feederSatId,
                         feederBeamId,
                         fCh,
                         rCh,
                         cbChannel,
                         ncc,
                         llsConf,
                         forwardLinkRegenerationMode,
                         returnLinkRegenerationMode));
    }

    return devs;
}

void
SatGwHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase& cb)
{
    NS_LOG_FUNCTION(this);

    TraceConnect("Creation", "SatGwHelper", cb);
}

} // namespace ns3
