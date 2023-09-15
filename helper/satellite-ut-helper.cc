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

#include "satellite-ut-helper.h"

#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <ns3/satellite-typedefs.h>

NS_LOG_COMPONENT_DEFINE("SatUtHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatUtHelper);

TypeId
SatUtHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatUtHelper")
            .SetParent<Object>()
            .AddAttribute("DaFwdLinkInterferenceModel",
                          "Forward link interference model for dedicated access",
                          EnumValue(SatPhyRxCarrierConf::IF_CONSTANT),
                          MakeEnumAccessor(&SatUtHelper::m_daInterferenceModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::IF_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::IF_TRACE,
                                          "Trace",
                                          SatPhyRxCarrierConf::IF_PER_PACKET,
                                          "PerPacket",
                                          SatPhyRxCarrierConf::IF_PER_FRAGMENT,
                                          "PerFragment"))
            .AddAttribute("FwdLinkErrorModel",
                          "Forward link error model",
                          EnumValue(SatPhyRxCarrierConf::EM_AVI),
                          MakeEnumAccessor(&SatUtHelper::m_errorModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::EM_NONE,
                                          "None",
                                          SatPhyRxCarrierConf::EM_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::EM_AVI,
                                          "AVI"))
            .AddAttribute("FwdLinkConstantErrorRate",
                          "Constant error rate",
                          DoubleValue(0.01),
                          MakeDoubleAccessor(&SatUtHelper::m_daConstantErrorRate),
                          MakeDoubleChecker<double>())
            .AddAttribute("LowerLayerServiceConf",
                          "Pointer to lower layer service configuration.",
                          PointerValue(),
                          MakePointerAccessor(&SatUtHelper::m_llsConf),
                          MakePointerChecker<SatLowerLayerServiceConf>())
            .AddAttribute("EnableChannelEstimationError",
                          "Enable channel estimation error in forward link receiver at UT.",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SatUtHelper::m_enableChannelEstimationError),
                          MakeBooleanChecker())
            .AddAttribute("UseCrdsaOnlyForControlPackets",
                          "CRDSA utilized only for control packets or also for user data.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatUtHelper::m_crdsaOnlyForControl),
                          MakeBooleanChecker())
            .AddAttribute("AsynchronousReturnAccess",
                          "Use asynchronous access methods on the return channel.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatUtHelper::m_asyncAccess),
                          MakeBooleanChecker())
            .AddTraceSource("Creation",
                            "Creation traces",
                            MakeTraceSourceAccessor(&SatUtHelper::m_creationTrace),
                            "ns3::SatTypedefs::CreationCallback");
    return tid;
}

TypeId
SatUtHelper::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatUtHelper::SatUtHelper()
    : m_carrierBandwidthConverter(),
      m_fwdLinkCarrierCount(),
      m_superframeSeq(),
      m_daInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_errorModel(SatPhyRxCarrierConf::EM_AVI),
      m_daConstantErrorRate(0.0),
      m_linkResults(),
      m_llsConf(),
      m_enableChannelEstimationError(false),
      m_crdsaOnlyForControl(false),
      m_asyncAccess(false),
      m_raSettings()
{
    NS_LOG_FUNCTION(this);

    // this default constructor should be never called
    NS_FATAL_ERROR("SatUtHelper::SatUtHelper - Constructor not in use");
}

SatUtHelper::SatUtHelper(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                         uint32_t fwdLinkCarrierCount,
                         Ptr<SatSuperframeSeq> seq,
                         SatMac::ReadCtrlMsgCallback readCb,
                         SatMac::ReserveCtrlMsgCallback reserveCb,
                         SatMac::SendCtrlMsgCallback sendCb,
                         RandomAccessSettings_s randomAccessSettings)
    : m_carrierBandwidthConverter(carrierBandwidthConverter),
      m_fwdLinkCarrierCount(fwdLinkCarrierCount),
      m_superframeSeq(seq),
      m_readCtrlCb(readCb),
      m_reserveCtrlCb(reserveCb),
      m_sendCtrlCb(sendCb),
      m_daInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_errorModel(SatPhyRxCarrierConf::EM_AVI),
      m_daConstantErrorRate(0.0),
      m_linkResults(),
      m_llsConf(),
      m_enableChannelEstimationError(false),
      m_crdsaOnlyForControl(false),
      m_raSettings(randomAccessSettings)
{
    NS_LOG_FUNCTION(this << fwdLinkCarrierCount << seq);
    m_deviceFactory.SetTypeId("ns3::SatNetDevice");
    m_channelFactory.SetTypeId("ns3::SatChannel");

    m_llsConf = CreateObject<SatLowerLayerServiceConf>();
}

void
SatUtHelper::Initialize(Ptr<SatLinkResultsFwd> lrFwd)
{
    NS_LOG_FUNCTION(this);
    /*
     * Forward channel link results (DVB-S2 or DVB-S2X) are created for UTs.
     */
    if (lrFwd && m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
        m_linkResults = lrFwd;
    }
}

void
SatUtHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    m_deviceFactory.Set(n1, v1);
}

void
SatUtHelper::SetChannelAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    m_channelFactory.Set(n1, v1);
}

void
SatUtHelper::SetPhyAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    Config::SetDefault("ns3::SatUtPhy::" + n1, v1);
}

NetDeviceContainer
SatUtHelper::Install(NodeContainer c,
                     uint32_t satId,
                     uint32_t beamId,
                     Ptr<SatChannel> fCh,
                     Ptr<SatChannel> rCh,
                     Ptr<SatNetDevice> gwNd,
                     Ptr<SatNcc> ncc,
                     Address satUserAddress,
                     SatPhy::ChannelPairGetterCallback cbChannel,
                     SatMac::RoutingUpdateCallback cbRouting,
                     SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                     SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << satId << beamId << fCh << rCh << gwNd << ncc << satUserAddress << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    NetDeviceContainer devs;

    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); i++)
    {
        devs.Add(Install(*i,
                         satId,
                         beamId,
                         fCh,
                         rCh,
                         gwNd,
                         ncc,
                         satUserAddress,
                         cbChannel,
                         cbRouting,
                         forwardLinkRegenerationMode,
                         returnLinkRegenerationMode));
    }

    return devs;
}

void
SatUtHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase& cb)
{
    NS_LOG_FUNCTION(this);

    TraceConnect("Creation", "SatUtHelper", cb);
}

} // namespace ns3
