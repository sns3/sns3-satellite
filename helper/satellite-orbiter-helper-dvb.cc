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

#include "satellite-orbiter-helper-dvb.h"

#include "satellite-helper.h"
#include "satellite-isl-arbiter-unicast-helper.h"

#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/satellite-channel-estimation-error-container.h"
#include "ns3/satellite-const-variables.h"
#include "ns3/satellite-id-mapper.h"
#include "ns3/satellite-orbiter-feeder-llc.h"
#include "ns3/satellite-orbiter-feeder-mac.h"
#include "ns3/satellite-orbiter-feeder-phy.h"
#include "ns3/satellite-orbiter-net-device-dvb.h"
#include "ns3/satellite-orbiter-user-llc.h"
#include "ns3/satellite-orbiter-user-mac.h"
#include "ns3/satellite-orbiter-user-phy.h"
#include "ns3/satellite-phy-rx-carrier-conf.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-topology.h"
#include "ns3/satellite-typedefs.h"
#include "ns3/satellite-utils.h"
#include "ns3/singleton.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

NS_LOG_COMPONENT_DEFINE("SatOrbiterHelperDvb");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatOrbiterHelperDvb);

TypeId
SatOrbiterHelperDvb::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatOrbiterHelperDvb")
                            .SetParent<SatOrbiterHelper>()
                            .AddConstructor<SatOrbiterHelperDvb>();
    return tid;
}

SatOrbiterHelperDvb::SatOrbiterHelperDvb()
    : SatOrbiterHelper()
{
    NS_LOG_FUNCTION(this);

    // this default constructor should be never called
    NS_ASSERT(false);
}

SatOrbiterHelperDvb::SatOrbiterHelperDvb(
    SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
    uint32_t rtnLinkCarrierCount,
    uint32_t fwdLinkCarrierCount,
    Ptr<SatSuperframeSeq> seq,
    SatMac::ReadCtrlMsgCallback fwdReadCb,
    SatMac::ReadCtrlMsgCallback rtnReadCb,
    RandomAccessSettings_s randomAccessSettings)
    : SatOrbiterHelper(bandwidthConverterCb,
                       rtnLinkCarrierCount,
                       fwdLinkCarrierCount,
                       seq,
                       fwdReadCb,
                       rtnReadCb,
                       randomAccessSettings)
{
    NS_LOG_FUNCTION(this << rtnLinkCarrierCount << fwdLinkCarrierCount);

    m_deviceFactory.SetTypeId("ns3::SatOrbiterNetDeviceDvb");
}

Ptr<SatOrbiterNetDevice>
SatOrbiterHelperDvb::CreateOrbiterNetDevice()
{
    NS_LOG_FUNCTION(this);

    return m_deviceFactory.Create<SatOrbiterNetDeviceDvb>();
}

void
SatOrbiterHelperDvb::AttachChannelsUser(Ptr<SatOrbiterNetDevice> dev,
                                        Ptr<SatChannel> uf,
                                        Ptr<SatChannel> ur,
                                        Ptr<SatAntennaGainPattern> userAgp,
                                        Ptr<SatNcc> ncc,
                                        uint32_t satId,
                                        uint32_t userBeamId)
{
    NS_LOG_FUNCTION(this << dev << uf << ur << userAgp << satId << userBeamId);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = userBeamId;
    params.m_device = dev;
    params.m_standard = SatEnums::DVB_ORBITER;

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
    parametersUser.m_linkRegenerationMode =
        Singleton<SatTopology>::Get()->GetReturnLinkRegenerationMode();
    parametersUser.m_bwConverter = m_carrierBandwidthConverter;
    parametersUser.m_carrierCount = m_rtnLinkCarrierCount;
    parametersUser.m_cec = cec;
    parametersUser.m_raCollisionModel = m_raSettings.m_raCollisionModel;
    parametersUser.m_randomAccessModel = m_raSettings.m_randomAccessModel;

    params.m_txCh = uf;
    params.m_rxCh = ur;

    Ptr<SatOrbiterUserPhy> uPhy = CreateObject<SatOrbiterUserPhy>(
        params,
        m_rtnLinkResults,
        parametersUser,
        m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE));

    // Note, that currently we have only one set of antenna patterns,
    // which are utilized in both in user link and feeder link, and
    // in both uplink and downlink directions.
    uPhy->SetTxAntennaGainPattern(userAgp, dev->GetNode()->GetObject<SatMobilityModel>());
    uPhy->SetRxAntennaGainPattern(userAgp, dev->GetNode()->GetObject<SatMobilityModel>());

    dev->AddUserPhy(uPhy, userBeamId);

    uPhy->Initialize();

    Ptr<SatOrbiterUserMac> uMac;
    Ptr<SatOrbiterUserLlc> uLlc;

    uMac = CreateObject<SatOrbiterUserMac>(satId, userBeamId);

    Mac48Address userAddress;

    // Create layers needed depending on max regeneration mode
    switch (std::max(Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode(),
                     Singleton<SatTopology>::Get()->GetReturnLinkRegenerationMode()))
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
        uLlc = CreateObject<SatOrbiterUserLlc>();

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
        uLlc = CreateObject<SatOrbiterUserLlc>();

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
    switch (Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode())
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        // Nothing to do on user side
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        uMac->SetTransmitCallback(MakeCallback(&SatOrbiterUserPhy::SendPduWithParams, uPhy));

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
        fwdScheduler->SetTxOpportunityCallback(
            MakeCallback(&SatOrbiterLlc::NotifyTxOpportunity, uLlc));
        fwdScheduler->SetSchedContextCallback(MakeCallback(&SatLlc::GetSchedulingContexts, uLlc));

        break;
    }
    default:
        NS_FATAL_ERROR("Forward link regeneration mode unknown");
    }

    // Connect callbacks on return link
    switch (Singleton<SatTopology>::Get()->GetReturnLinkRegenerationMode())
    {
    case SatEnums::TRANSPARENT:
    case SatEnums::REGENERATION_PHY: {
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatOrbiterNetDevice::ReceiveUser, dev);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uMac->SetReceiveNetDeviceCallback(MakeCallback(&SatOrbiterNetDevice::ReceiveUser, dev));

        break;
    }
    case SatEnums::REGENERATION_LINK: {
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatOrbiterUserMac::Receive, uMac);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uMac->SetReceiveNetDeviceCallback(MakeCallback(&SatOrbiterNetDevice::ReceiveUser, dev));

        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatOrbiterUserMac::Receive, uMac);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uMac->SetReceiveCallback(MakeCallback(&SatOrbiterUserLlc::Receive, uLlc));

        uLlc->SetReceiveSatelliteCallback(
            MakeCallback(&SatOrbiterNetDevice::ReceivePacketUser, dev));

        break;
    }
    default:
        NS_FATAL_ERROR("Return link regeneration mode unknown");
    }

    if (Singleton<SatTopology>::Get()->GetReturnLinkRegenerationMode() != SatEnums::TRANSPARENT)
    {
        uPhy->BeginEndScheduling();
        uPhy->SetSendControlMsgToFeederCallback(
            MakeCallback(&SatOrbiterNetDevice::SendControlMsgToFeeder, dev));
    }

    Singleton<SatTopology>::Get()
        ->AddOrbiterUserLayersDvb(dev->GetNode(), satId, userBeamId, dev, uLlc, uMac, uPhy);
}

} // namespace ns3
