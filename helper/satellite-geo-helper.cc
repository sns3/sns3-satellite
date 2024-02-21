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

#include "ns3/satellite-geo-helper.h"

#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/satellite-channel-estimation-error-container.h"
#include "ns3/satellite-const-variables.h"
#include "ns3/satellite-geo-feeder-llc.h"
#include "ns3/satellite-geo-feeder-mac.h"
#include "ns3/satellite-geo-feeder-phy.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-geo-user-llc.h"
#include "ns3/satellite-geo-user-mac.h"
#include "ns3/satellite-geo-user-phy.h"
#include "ns3/satellite-helper.h"
#include "ns3/satellite-id-mapper.h"
#include "ns3/satellite-isl-arbiter-unicast-helper.h"
#include "ns3/satellite-phy-rx-carrier-conf.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-typedefs.h"
#include "ns3/satellite-utils.h"
#include "ns3/singleton.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE("SatGeoHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatGeoHelper);

TypeId
SatGeoHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatGeoHelper")
            .SetParent<Object>()
            .AddConstructor<SatGeoHelper>()
            .AddAttribute("DaFwdLinkInterferenceModel",
                          "Forward link interference model for dedicated access",
                          EnumValue(SatPhyRxCarrierConf::IF_CONSTANT),
                          MakeEnumAccessor<SatPhyRxCarrierConf::InterferenceModel>(&SatGeoHelper::m_daFwdLinkInterferenceModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::IF_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::IF_TRACE,
                                          "Trace",
                                          SatPhyRxCarrierConf::IF_PER_PACKET,
                                          "PerPacket",
                                          SatPhyRxCarrierConf::IF_PER_FRAGMENT,
                                          "PerFragment"))
            .AddAttribute("DaRtnLinkInterferenceModel",
                          "Return link interference model for dedicated access",
                          EnumValue(SatPhyRxCarrierConf::IF_PER_PACKET),
                          MakeEnumAccessor<SatPhyRxCarrierConf::InterferenceModel>(&SatGeoHelper::m_daRtnLinkInterferenceModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::IF_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::IF_TRACE,
                                          "Trace",
                                          SatPhyRxCarrierConf::IF_PER_PACKET,
                                          "PerPacket",
                                          SatPhyRxCarrierConf::IF_PER_FRAGMENT,
                                          "PerFragment"))
            .AddAttribute("FwdLinkErrorModel",
                          "Forward feeder link error model",
                          EnumValue(SatPhyRxCarrierConf::EM_NONE),
                          MakeEnumAccessor<SatPhyRxCarrierConf::ErrorModel>(&SatGeoHelper::m_fwdErrorModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::EM_NONE,
                                          "None",
                                          SatPhyRxCarrierConf::EM_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::EM_AVI,
                                          "AVI"))
            .AddAttribute("FwdLinkConstantErrorRate",
                          "Constant error rate on forward feeder link",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&SatGeoHelper::m_fwdDaConstantErrorRate),
                          MakeDoubleChecker<double>())
            .AddAttribute("RtnLinkErrorModel",
                          "Return user link error model",
                          EnumValue(SatPhyRxCarrierConf::EM_NONE),
                          MakeEnumAccessor<SatPhyRxCarrierConf::ErrorModel>(&SatGeoHelper::m_rtnErrorModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::EM_NONE,
                                          "None",
                                          SatPhyRxCarrierConf::EM_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::EM_AVI,
                                          "AVI"))
            .AddAttribute("RtnLinkConstantErrorRate",
                          "Constant error rate on return user link",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&SatGeoHelper::m_rtnDaConstantErrorRate),
                          MakeDoubleChecker<double>())
            .AddAttribute("IslArbiterType",
                          "Arbiter in use to route packets on ISLs",
                          EnumValue(SatEnums::UNICAST),
                          MakeEnumAccessor<SatEnums::IslArbiterType_t>(&SatGeoHelper::m_islArbiterType),
                          MakeEnumChecker(SatEnums::UNICAST, "Unicast", SatEnums::ECMP, "ECMP"))
            .AddTraceSource("Creation",
                            "Creation traces",
                            MakeTraceSourceAccessor(&SatGeoHelper::m_creationTrace),
                            "ns3::SatTypedefs::CreationCallback");
    return tid;
}

TypeId
SatGeoHelper::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatGeoHelper::SatGeoHelper()
    : m_carrierBandwidthConverter(),
      m_fwdLinkCarrierCount(),
      m_rtnLinkCarrierCount(),
      m_deviceCount(),
      m_deviceFactory(),
      m_daFwdLinkInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_daRtnLinkInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_raSettings(),
      m_fwdLinkResults(),
      m_rtnLinkResults(),
      m_islArbiterType(SatEnums::UNICAST),
      m_fwdReadCtrlCb(),
      m_rtnReadCtrlCb()
{
    NS_LOG_FUNCTION(this);

    // this default constructor should be never called
    NS_ASSERT(false);
}

SatGeoHelper::SatGeoHelper(SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                           uint32_t rtnLinkCarrierCount,
                           uint32_t fwdLinkCarrierCount,
                           Ptr<SatSuperframeSeq> seq,
                           SatMac::ReadCtrlMsgCallback fwdReadCb,
                           SatMac::ReadCtrlMsgCallback rtnReadCb,
                           RandomAccessSettings_s randomAccessSettings)
    : m_carrierBandwidthConverter(bandwidthConverterCb),
      m_fwdLinkCarrierCount(fwdLinkCarrierCount),
      m_rtnLinkCarrierCount(rtnLinkCarrierCount),
      m_deviceCount(),
      m_deviceFactory(),
      m_daFwdLinkInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_daRtnLinkInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_superframeSeq(seq),
      m_raSettings(randomAccessSettings),
      m_fwdLinkResults(),
      m_rtnLinkResults(),
      m_fwdReadCtrlCb(fwdReadCb),
      m_rtnReadCtrlCb(rtnReadCb)
{
    NS_LOG_FUNCTION(this << rtnLinkCarrierCount << fwdLinkCarrierCount);

    m_deviceFactory.SetTypeId("ns3::SatGeoNetDevice");
}

void
SatGeoHelper::Initialize(Ptr<SatLinkResultsFwd> lrFwd, Ptr<SatLinkResultsRtn> lrRcs2)
{
    NS_LOG_FUNCTION(this);

    /*
     * Forward channel link results (DVB-S2 or DVB-S2X).
     */
    if (lrFwd && m_fwdErrorModel == SatPhyRxCarrierConf::EM_AVI)
    {
        m_fwdLinkResults = lrFwd;
    }

    /*
     * Return channel link results (DVB-RCS2).
     */
    if (lrRcs2 && m_rtnErrorModel == SatPhyRxCarrierConf::EM_AVI)
    {
        m_rtnLinkResults = lrRcs2;
    }

    m_symbolRateRtn =
        m_carrierBandwidthConverter(SatEnums::RETURN_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);
    m_bbFrameConfRtn =
        CreateObject<SatBbFrameConf>(m_symbolRateRtn,
                                     SatEnums::DVB_S2); // TODO We should be able to switch to S2X ?
    m_bbFrameConfRtn->InitializeCNoRequirements(lrFwd);

    m_symbolRateFwd =
        m_carrierBandwidthConverter(SatEnums::FORWARD_USER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);
    m_bbFrameConfFwd =
        CreateObject<SatBbFrameConf>(m_symbolRateFwd,
                                     SatEnums::DVB_S2); // TODO We should be able to switch to S2X ?
    m_bbFrameConfFwd->InitializeCNoRequirements(lrFwd);
}

void
SatGeoHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    m_deviceFactory.Set(n1, v1);
}

void
SatGeoHelper::SetUserPhyAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    Config::SetDefault("ns3::SatGeoUserPhy::" + n1, v1);
}

void
SatGeoHelper::SetFeederPhyAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    Config::SetDefault("ns3::SatGeoFeederPhy::" + n1, v1);
}

NetDeviceContainer
SatGeoHelper::Install(NodeContainer c)
{
    NS_LOG_FUNCTION(this);

    NetDeviceContainer devs;

    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); i++)
    {
        devs.Add(Install(*i));
    }

    return devs;
}

Ptr<NetDevice>
SatGeoHelper::Install(Ptr<Node> n)
{
    NS_LOG_FUNCTION(this << n);

    NS_ASSERT(m_deviceCount[n->GetId()] == 0);

    // Create SatGeoNetDevice
    Ptr<SatGeoNetDevice> satDev = m_deviceFactory.Create<SatGeoNetDevice>();

    satDev->SetAddress(Mac48Address::Allocate());
    n->AddDevice(satDev);
    m_deviceCount[n->GetId()]++;
    m_nodeIds.push_back(n->GetId());

    Singleton<SatIdMapper>::Get()->AttachMacToTraceId(satDev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToSatId(satDev->GetAddress(), m_nodeIds.size());

    return satDev;
}

Ptr<NetDevice>
SatGeoHelper::Install(std::string aName)
{
    NS_LOG_FUNCTION(this << aName);

    Ptr<Node> n = Names::Find<Node>(aName);

    return Install(n);
}

void
SatGeoHelper::AttachChannels(Ptr<NetDevice> d,
                             Ptr<SatChannel> ff,
                             Ptr<SatChannel> fr,
                             Ptr<SatChannel> uf,
                             Ptr<SatChannel> ur,
                             Ptr<SatAntennaGainPattern> userAgp,
                             Ptr<SatAntennaGainPattern> feederAgp,
                             Ptr<SatNcc> ncc,
                             uint32_t satId,
                             uint32_t gwId,
                             uint32_t userBeamId,
                             SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                             SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << d << ff << fr << uf << ur << userAgp << feederAgp << satId << gwId
                         << userBeamId);

    Ptr<SatGeoNetDevice> dev = DynamicCast<SatGeoNetDevice>(d);

    dev->SetForwardLinkRegenerationMode(forwardLinkRegenerationMode);
    dev->SetReturnLinkRegenerationMode(returnLinkRegenerationMode);
    dev->SetNodeId(satId);

    AttachChannelsFeeder(dev,
                         ff,
                         fr,
                         feederAgp,
                         ncc,
                         satId,
                         gwId,
                         userBeamId,
                         forwardLinkRegenerationMode,
                         returnLinkRegenerationMode);
    AttachChannelsUser(dev,
                       uf,
                       ur,
                       userAgp,
                       ncc,
                       satId,
                       userBeamId,
                       forwardLinkRegenerationMode,
                       returnLinkRegenerationMode);
}

void
SatGeoHelper::AttachChannelsFeeder(Ptr<SatGeoNetDevice> dev,
                                   Ptr<SatChannel> ff,
                                   Ptr<SatChannel> fr,
                                   Ptr<SatAntennaGainPattern> feederAgp,
                                   Ptr<SatNcc> ncc,
                                   uint32_t satId,
                                   uint32_t gwId,
                                   uint32_t userBeamId,
                                   SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                   SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << dev << ff << fr << feederAgp << satId << gwId << userBeamId
                         << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = userBeamId;
    params.m_device = dev;
    params.m_standard = SatEnums::GEO;

    /**
     * Simple channel estimation, which does not do actually anything
     */
    Ptr<SatChannelEstimationErrorContainer> cec =
        Create<SatSimpleChannelEstimationErrorContainer>();

    SatPhyRxCarrierConf::RxCarrierCreateParams_s parametersFeeder =
        SatPhyRxCarrierConf::RxCarrierCreateParams_s();
    parametersFeeder.m_errorModel = m_fwdErrorModel;
    parametersFeeder.m_daConstantErrorRate = m_fwdDaConstantErrorRate;
    parametersFeeder.m_daIfModel = m_daFwdLinkInterferenceModel;
    parametersFeeder.m_raIfModel = m_raSettings.m_raFwdInterferenceModel;
    parametersFeeder.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
    parametersFeeder.m_linkRegenerationMode = forwardLinkRegenerationMode;
    parametersFeeder.m_bwConverter = m_carrierBandwidthConverter;
    parametersFeeder.m_carrierCount = m_fwdLinkCarrierCount;
    parametersFeeder.m_cec = cec;
    parametersFeeder.m_raCollisionModel = m_raSettings.m_raCollisionModel;
    parametersFeeder.m_randomAccessModel = m_raSettings.m_randomAccessModel;

    params.m_txCh = fr;
    params.m_rxCh = ff;

    Ptr<SatGeoFeederPhy> fPhy = CreateObject<SatGeoFeederPhy>(
        params,
        m_fwdLinkResults,
        parametersFeeder,
        m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE),
        forwardLinkRegenerationMode,
        returnLinkRegenerationMode);

    // Note, that currently we have only one set of antenna patterns,
    // which are utilized in both in user link and feeder link, and
    // in both uplink and downlink directions.
    fPhy->SetTxAntennaGainPattern(feederAgp, dev->GetNode()->GetObject<SatMobilityModel>());
    fPhy->SetRxAntennaGainPattern(feederAgp, dev->GetNode()->GetObject<SatMobilityModel>());

    dev->AddFeederPhy(fPhy, userBeamId);

    fPhy->Initialize();

    Ptr<SatGeoFeederMac> fMac;
    Ptr<SatGeoLlc> fLlc;
    bool startScheduler = false;

    // Create MAC layer
    fMac = CreateObject<SatGeoFeederMac>(satId,
                                         userBeamId,
                                         forwardLinkRegenerationMode,
                                         returnLinkRegenerationMode);

    Mac48Address feederAddress;

    // Create layers needed depending on max regeneration mode
    switch (std::max(forwardLinkRegenerationMode, returnLinkRegenerationMode))
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        // Create a node info to PHY layers
        Ptr<SatNodeInfo> niPhyFeeder =
            Create<SatNodeInfo>(SatEnums::NT_SAT,
                                m_nodeIds[satId],
                                Mac48Address::ConvertFrom(dev->GetAddress()));
        fPhy->SetNodeInfo(niPhyFeeder);
        fMac->SetNodeInfo(niPhyFeeder);

        break;
    }
    case SatEnums::REGENERATION_LINK: {
        // Create LLC layer
        fLlc = CreateObject<SatGeoLlc>(forwardLinkRegenerationMode, returnLinkRegenerationMode);

        if (m_gwMacMap.count(std::make_pair(satId, gwId)))
        {
            // MAC already exists for this GW ID, reusing it, and disabling the other
            dev->AddFeederMac(fMac, m_gwMacMap[std::make_pair(satId, gwId)], userBeamId);
        }
        else
        {
            // First MAC for this GW ID, storing it to the map
            dev->AddFeederMac(fMac, fMac, userBeamId);
            m_gwMacMap[std::make_pair(satId, gwId)] = fMac;
            startScheduler = true;
        }

        fMac->SetReadCtrlCallback(m_fwdReadCtrlCb);
        fLlc->SetReadCtrlCallback(m_fwdReadCtrlCb);

        // Create a node info to PHY and MAC layers
        feederAddress = Mac48Address::Allocate();
        Ptr<SatNodeInfo> niFeeder =
            Create<SatNodeInfo>(SatEnums::NT_SAT, m_nodeIds[satId], feederAddress);
        fPhy->SetNodeInfo(niFeeder);
        fMac->SetNodeInfo(niFeeder);
        fLlc->SetNodeInfo(niFeeder);

        dev->AddFeederPair(userBeamId, feederAddress);

        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        // Create LLC layer
        fLlc =
            CreateObject<SatGeoFeederLlc>(forwardLinkRegenerationMode, returnLinkRegenerationMode);

        if (m_gwMacMap.count(std::make_pair(satId, gwId)))
        {
            // MAC already exists for this GW ID, reusing it, and disabling the other
            dev->AddFeederMac(fMac, m_gwMacMap[std::make_pair(satId, gwId)], userBeamId);
        }
        else
        {
            // First MAC for this GW ID, storing it to the map
            dev->AddFeederMac(fMac, fMac, userBeamId);
            m_gwMacMap[std::make_pair(satId, gwId)] = fMac;
            startScheduler = true;
        }

        fMac->SetReadCtrlCallback(m_fwdReadCtrlCb);
        fLlc->SetReadCtrlCallback(m_fwdReadCtrlCb);

        // Create a node info to PHY and MAC layers
        feederAddress = Mac48Address::Allocate();
        Ptr<SatNodeInfo> niFeeder =
            Create<SatNodeInfo>(SatEnums::NT_SAT, m_nodeIds[satId], feederAddress);
        fPhy->SetNodeInfo(niFeeder);
        fMac->SetNodeInfo(niFeeder);
        fLlc->SetNodeInfo(niFeeder);

        dev->AddFeederPair(userBeamId, feederAddress);

        break;
    }
    default:
        NS_FATAL_ERROR("Forward or return link regeneration mode unknown");
    }

    // Connect callbacks on forward link
    switch (forwardLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        SatPhy::ReceiveCallback fCb = MakeCallback(&SatGeoFeederMac::Receive, fMac);
        fPhy->SetAttribute("ReceiveCb", CallbackValue(fCb));

        fMac->SetReceiveNetDeviceCallback(MakeCallback(&SatGeoNetDevice::ReceiveFeeder, dev));

        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        SatPhy::ReceiveCallback fCb = MakeCallback(&SatGeoFeederMac::Receive, fMac);
        fPhy->SetAttribute("ReceiveCb", CallbackValue(fCb));

        fMac->SetReceiveCallback(MakeCallback(&SatGeoFeederLlc::Receive, fLlc));

        fLlc->SetReceiveSatelliteCallback(MakeCallback(&SatGeoNetDevice::ReceivePacketFeeder, dev));

        break;
    }
    default:
        NS_FATAL_ERROR("Forward link regeneration mode unknown");
    }

    // Connect callbacks on return link
    switch (returnLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        // Nothing to do on feeder side
        break;
    }
    case SatEnums::REGENERATION_LINK:
    case SatEnums::REGENERATION_NETWORK: {
        fMac->SetTransmitCallback(MakeCallback(&SatGeoFeederPhy::SendPduWithParams, fPhy));

        double carrierBandwidth = m_carrierBandwidthConverter(SatEnums::RETURN_FEEDER_CH,
                                                              0,
                                                              SatEnums::EFFECTIVE_BANDWIDTH);
        Ptr<SatScpcScheduler> scpcScheduler =
            CreateObject<SatScpcScheduler>(m_bbFrameConfRtn, feederAddress, carrierBandwidth);
        fMac->SetFwdScheduler(scpcScheduler);
        fMac->SetLlc(fLlc);
        if (startScheduler)
        {
            fMac->StartPeriodicTransmissions();
        }

        // Attach the LLC Tx opportunity and scheduling context getter callbacks to
        // SatFwdLinkScheduler
        scpcScheduler->SetTxOpportunityCallback(
            MakeCallback(&SatGeoLlc::NotifyTxOpportunity, fLlc));
        scpcScheduler->SetSchedContextCallback(MakeCallback(&SatLlc::GetSchedulingContexts, fLlc));

        break;
    }
    default:
        NS_FATAL_ERROR("Return link regeneration mode unknown");
    }
}

void
SatGeoHelper::AttachChannelsUser(Ptr<SatGeoNetDevice> dev,
                                 Ptr<SatChannel> uf,
                                 Ptr<SatChannel> ur,
                                 Ptr<SatAntennaGainPattern> userAgp,
                                 Ptr<SatNcc> ncc,
                                 uint32_t satId,
                                 uint32_t userBeamId,
                                 SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                 SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << dev << uf << ur << userAgp << satId << userBeamId
                         << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = userBeamId;
    params.m_device = dev;
    params.m_standard = SatEnums::GEO;

    /**
     * Simple channel estimation, which does not do actually anything
     */
    Ptr<SatChannelEstimationErrorContainer> cec =
        Create<SatSimpleChannelEstimationErrorContainer>();

    SatPhyRxCarrierConf::RxCarrierCreateParams_s parametersUser =
        SatPhyRxCarrierConf::RxCarrierCreateParams_s();
    parametersUser.m_errorModel = m_rtnErrorModel;
    parametersUser.m_daConstantErrorRate = m_rtnDaConstantErrorRate;
    parametersUser.m_daIfModel = m_daRtnLinkInterferenceModel;
    parametersUser.m_raIfModel = m_raSettings.m_raRtnInterferenceModel;
    parametersUser.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
    parametersUser.m_linkRegenerationMode = returnLinkRegenerationMode;
    parametersUser.m_bwConverter = m_carrierBandwidthConverter;
    parametersUser.m_carrierCount = m_rtnLinkCarrierCount;
    parametersUser.m_cec = cec;
    parametersUser.m_raCollisionModel = m_raSettings.m_raCollisionModel;
    parametersUser.m_randomAccessModel = m_raSettings.m_randomAccessModel;

    params.m_txCh = uf;
    params.m_rxCh = ur;

    Ptr<SatGeoUserPhy> uPhy = CreateObject<SatGeoUserPhy>(
        params,
        m_rtnLinkResults,
        parametersUser,
        m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE),
        forwardLinkRegenerationMode,
        returnLinkRegenerationMode);

    // Note, that currently we have only one set of antenna patterns,
    // which are utilized in both in user link and feeder link, and
    // in both uplink and downlink directions.
    uPhy->SetTxAntennaGainPattern(userAgp, dev->GetNode()->GetObject<SatMobilityModel>());
    uPhy->SetRxAntennaGainPattern(userAgp, dev->GetNode()->GetObject<SatMobilityModel>());

    dev->AddUserPhy(uPhy, userBeamId);

    uPhy->Initialize();

    Ptr<SatGeoUserMac> uMac;
    Ptr<SatGeoLlc> uLlc;

    uMac = CreateObject<SatGeoUserMac>(satId,
                                       userBeamId,
                                       forwardLinkRegenerationMode,
                                       returnLinkRegenerationMode);

    Mac48Address userAddress;

    // Create layers needed depending on max regeneration mode
    switch (std::max(forwardLinkRegenerationMode, returnLinkRegenerationMode))
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        // Create a node info to PHY layers
        Ptr<SatNodeInfo> niPhyUser =
            Create<SatNodeInfo>(SatEnums::NT_SAT,
                                m_nodeIds[satId],
                                Mac48Address::ConvertFrom(dev->GetAddress()));
        uPhy->SetNodeInfo(niPhyUser);
        uMac->SetNodeInfo(niPhyUser);

        break;
    }
    case SatEnums::REGENERATION_LINK: {
        // Create LLC layer
        uLlc = CreateObject<SatGeoLlc>(forwardLinkRegenerationMode, returnLinkRegenerationMode);

        dev->AddUserMac(uMac, userBeamId);

        uMac->SetReadCtrlCallback(m_rtnReadCtrlCb);
        uLlc->SetReadCtrlCallback(m_rtnReadCtrlCb);

        // Create a node info to PHY and MAC layers
        userAddress = Mac48Address::Allocate();
        Ptr<SatNodeInfo> niUser =
            Create<SatNodeInfo>(SatEnums::NT_SAT, m_nodeIds[satId], userAddress);
        uPhy->SetNodeInfo(niUser);
        uMac->SetNodeInfo(niUser);
        uLlc->SetNodeInfo(niUser);

        dev->AddUserPair(userBeamId, userAddress);

        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        // Create LLC layer
        uLlc = CreateObject<SatGeoUserLlc>(forwardLinkRegenerationMode, returnLinkRegenerationMode);

        dev->AddUserMac(uMac, userBeamId);

        uMac->SetReadCtrlCallback(m_rtnReadCtrlCb);
        uLlc->SetReadCtrlCallback(m_rtnReadCtrlCb);

        // Create a node info to PHY and MAC layers
        userAddress = Mac48Address::Allocate();
        Ptr<SatNodeInfo> niUser =
            Create<SatNodeInfo>(SatEnums::NT_SAT, m_nodeIds[satId], userAddress);
        uPhy->SetNodeInfo(niUser);
        uMac->SetNodeInfo(niUser);
        uLlc->SetNodeInfo(niUser);

        dev->AddUserPair(userBeamId, userAddress);

        break;
    }
    default:
        NS_FATAL_ERROR("Forward or return link regeneration mode unknown");
    }

    // Connect callbacks on forward link
    switch (forwardLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        // Nothing to do on user side
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        uMac->SetTransmitCallback(MakeCallback(&SatGeoUserPhy::SendPduWithParams, uPhy));

        double carrierBandwidth = m_carrierBandwidthConverter(SatEnums::FORWARD_USER_CH,
                                                              0,
                                                              SatEnums::EFFECTIVE_BANDWIDTH);
        Ptr<SatFwdLinkScheduler> fwdScheduler =
            CreateObject<SatScpcScheduler>(m_bbFrameConfFwd, userAddress, carrierBandwidth);
        uMac->SetFwdScheduler(fwdScheduler);
        uMac->SetLlc(uLlc);
        uMac->StartPeriodicTransmissions();

        // Attach the LLC Tx opportunity and scheduling context getter callbacks to
        // SatFwdLinkScheduler
        fwdScheduler->SetTxOpportunityCallback(MakeCallback(&SatGeoLlc::NotifyTxOpportunity, uLlc));
        fwdScheduler->SetSchedContextCallback(MakeCallback(&SatLlc::GetSchedulingContexts, uLlc));

        break;
    }
    default:
        NS_FATAL_ERROR("Forward link regeneration mode unknown");
    }

    // Connect callbacks on return link
    switch (returnLinkRegenerationMode)
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatGeoNetDevice::ReceiveUser, dev);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uMac->SetReceiveNetDeviceCallback(MakeCallback(&SatGeoNetDevice::ReceiveUser, dev));

        break;
    }
    case SatEnums::REGENERATION_LINK: {
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatGeoUserMac::Receive, uMac);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uMac->SetReceiveNetDeviceCallback(MakeCallback(&SatGeoNetDevice::ReceiveUser, dev));

        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatGeoUserMac::Receive, uMac);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uMac->SetReceiveCallback(MakeCallback(&SatGeoUserLlc::Receive, uLlc));

        uLlc->SetReceiveSatelliteCallback(MakeCallback(&SatGeoNetDevice::ReceivePacketUser, dev));

        break;
    }
    default:
        NS_FATAL_ERROR("Return link regeneration mode unknown");
    }

    if (returnLinkRegenerationMode != SatEnums::TRANSPARENT)
    {
        uPhy->BeginEndScheduling();
        uPhy->SetSendControlMsgToFeederCallback(
            MakeCallback(&SatGeoNetDevice::SendControlMsgToFeeder, dev));
    }
}

void
SatGeoHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase& cb)
{
    NS_LOG_FUNCTION(this);

    TraceConnect("Creation", "SatGeoHelper", cb);
}

void
SatGeoHelper::SetIslRoutes(NodeContainer geoNodes, std::vector<std::pair<uint32_t, uint32_t>> isls)
{
    NS_LOG_FUNCTION(this);

    switch (m_islArbiterType)
    {
    case SatEnums::UNICAST: {
        Ptr<SatIslArbiterUnicastHelper> satIslArbiterHelper =
            CreateObject<SatIslArbiterUnicastHelper>(geoNodes, isls);
        satIslArbiterHelper->InstallArbiters();
        break;
    }
    case SatEnums::ECMP: {
        NS_FATAL_ERROR("ISL Arbiter ECMP not implemented yet");
    }
    default: {
        NS_FATAL_ERROR("Unknown ISL arbiter");
    }
    }
}

} // namespace ns3
