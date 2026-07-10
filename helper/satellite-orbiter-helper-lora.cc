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

#include "satellite-orbiter-helper-lora.h"

#include "satellite-helper.h"
#include "satellite-isl-arbiter-unicast-helper.h"
#include "satellite-lora-conf.h"

#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/lorawan-mac-gateway.h"
#include "ns3/lorawan-orbiter-mac-gateway.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/satellite-channel-estimation-error-container.h"
#include "ns3/satellite-const-variables.h"
#include "ns3/satellite-id-mapper.h"
#include "ns3/satellite-orbiter-feeder-llc.h"
#include "ns3/satellite-orbiter-feeder-mac.h"
#include "ns3/satellite-orbiter-feeder-phy.h"
#include "ns3/satellite-orbiter-net-device-lora.h"
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

NS_LOG_COMPONENT_DEFINE("SatOrbiterHelperLora");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatOrbiterHelperLora);

TypeId
SatOrbiterHelperLora::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatOrbiterHelperLora")
                            .SetParent<SatOrbiterHelper>()
                            .AddConstructor<SatOrbiterHelperLora>();
    return tid;
}

SatOrbiterHelperLora::SatOrbiterHelperLora()
    : SatOrbiterHelper()
{
    NS_LOG_FUNCTION(this);

    // this default constructor should be never called
    NS_ASSERT(false);
}

SatOrbiterHelperLora::SatOrbiterHelperLora(
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

    m_deviceFactory.SetTypeId("ns3::SatOrbiterNetDeviceLora");
}

Ptr<SatOrbiterNetDevice>
SatOrbiterHelperLora::CreateOrbiterNetDevice()
{
    NS_LOG_FUNCTION(this);

    return m_deviceFactory.Create<SatOrbiterNetDeviceLora>();
}

void
SatOrbiterHelperLora::AttachChannelsUser(Ptr<SatOrbiterNetDevice> dev,
                                         Ptr<SatChannel> uf,
                                         Ptr<SatChannel> ur,
                                         Ptr<SatAntennaGainPattern> userAgp,
                                         Ptr<SatNcc> ncc,
                                         uint32_t satId,
                                         uint32_t userBeamId)
{
    NS_LOG_FUNCTION(this << dev << uf << ur << userAgp << satId << userBeamId);

    NS_ASSERT_MSG(
        Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode() ==
            Singleton<SatTopology>::Get()->GetReturnLinkRegenerationMode(),
        "Regeneration level must be the same on forward and return in Lora configuration");
    NS_ASSERT_MSG(
        Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode() == SatEnums::TRANSPARENT ||
            Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode() ==
                SatEnums::REGENERATION_NETWORK,
        "Satellite can only be transparent or with network regenration in Lora configuration");

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = userBeamId;
    params.m_device = dev;
    params.m_standard =
        Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode() == SatEnums::TRANSPARENT
            ? SatEnums::DVB_ORBITER
            : SatEnums::LORA_ORBITER;

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

    ncc->SetUseLora(true);

    Mac48Address userAddress;

    // Connect callbacks on forward link
    switch (Singleton<SatTopology>::Get()->GetForwardLinkRegenerationMode())
    {
    case SatEnums::TRANSPARENT: {
        // Create layers
        Ptr<SatOrbiterUserMac> uTransparentMac = CreateObject<SatOrbiterUserMac>(satId, userBeamId);

        Ptr<SatNodeInfo> niPhyUser =
            Create<SatNodeInfo>(SatEnums::NT_SAT,
                                m_nodeIds[satId],
                                Mac48Address::ConvertFrom(dev->GetAddress()));
        uPhy->SetNodeInfo(niPhyUser);
        uTransparentMac->SetNodeInfo(niPhyUser);

        // Return link
        SatPhy::ReceiveCallback uCb = MakeCallback(&SatOrbiterNetDevice::ReceiveUser, dev);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(uCb));

        uTransparentMac->SetReceiveNetDeviceCallback(
            MakeCallback(&SatOrbiterNetDevice::ReceiveUser, dev));

        Singleton<SatTopology>::Get()->AddOrbiterUserLayersDvb(dev->GetNode(),
                                                               satId,
                                                               userBeamId,
                                                               dev,
                                                               nullptr,
                                                               uTransparentMac,
                                                               uPhy);
        break;
    }
    case SatEnums::REGENERATION_NETWORK: {
        // Create layers
        Ptr<LorawanMacGateway> uRegenerationMac =
            CreateObject<LorawanOrbiterMacGateway>(satId, userBeamId);

        dev->AddUserMac(uRegenerationMac, userBeamId);
        uRegenerationMac->SetDevice(dev);

        uRegenerationMac->SetReadCtrlCallback(m_rtnReadCtrlCb);

        Ptr<SatLoraConf> satLoraConf = CreateObject<SatLoraConf>();
        satLoraConf->SetConf(uRegenerationMac);

        // Attach the Mac layer receiver to Phy
        SatPhy::ReceiveCallback recCb = MakeCallback(&LorawanMac::Receive, uRegenerationMac);
        uPhy->SetAttribute("ReceiveCb", CallbackValue(recCb));

        // Create a node info to PHY and MAC layers
        userAddress = Mac48Address::Allocate();
        Ptr<SatNodeInfo> niUser =
            Create<SatNodeInfo>(SatEnums::NT_SAT, m_nodeIds[satId], userAddress);
        uPhy->SetNodeInfo(niUser);
        uRegenerationMac->SetNodeInfo(niUser);

        dev->AddUserPair(userBeamId, userAddress);

        uRegenerationMac->SetPhy(uPhy);

        Singleton<SatTopology>::Get()->AddOrbiterUserLayersLora(
            dev->GetNode(),
            satId,
            userBeamId,
            dev,
            DynamicCast<LorawanOrbiterMacGateway>(uRegenerationMac),
            uPhy);

        break;
    }
    default:
        NS_FATAL_ERROR("Regeneration mode unknown");
    }

    if (Singleton<SatTopology>::Get()->GetReturnLinkRegenerationMode() != SatEnums::TRANSPARENT)
    {
        uPhy->BeginEndScheduling();
        uPhy->SetSendControlMsgToFeederCallback(
            MakeCallback(&SatOrbiterNetDevice::SendControlMsgToFeeder, dev));
    }
}

} // namespace ns3
