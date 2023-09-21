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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "ns3/satellite-gw-helper-lora.h"

#include <ns3/log.h>
#include <ns3/lorawan-mac-gateway.h>
#include <ns3/pointer.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-gw-phy.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-link-results.h>
#include <ns3/satellite-lora-conf.h>
#include <ns3/satellite-lorawan-net-device.h>
#include <ns3/satellite-lower-layer-service.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/singleton.h>

NS_LOG_COMPONENT_DEFINE("SatGwHelperLora");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatGwHelperLora);

TypeId
SatGwHelperLora::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatGwHelperLora").SetParent<SatGwHelper>().AddConstructor<SatGwHelperLora>();
    return tid;
}

TypeId
SatGwHelperLora::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

SatGwHelperLora::SatGwHelperLora()
{
    // this default constructor should be never called
    NS_FATAL_ERROR("Default constructor not supported!!!");
}

SatGwHelperLora::SatGwHelperLora(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                                 uint32_t rtnLinkCarrierCount,
                                 Ptr<SatSuperframeSeq> seq,
                                 SatMac::ReadCtrlMsgCallback readCb,
                                 SatMac::ReserveCtrlMsgCallback reserveCb,
                                 SatMac::SendCtrlMsgCallback sendCb,
                                 RandomAccessSettings_s randomAccessSettings)
    : SatGwHelper(carrierBandwidthConverter,
                  rtnLinkCarrierCount,
                  seq,
                  readCb,
                  reserveCb,
                  sendCb,
                  randomAccessSettings)
{
    NS_LOG_FUNCTION(this << rtnLinkCarrierCount << seq);
}

Ptr<NetDevice>
SatGwHelperLora::Install(Ptr<Node> n,
                         uint32_t gwId,
                         uint32_t satId,
                         uint32_t beamId,
                         Ptr<SatChannel> fCh,
                         Ptr<SatChannel> rCh,
                         Ptr<SatNcc> ncc,
                         Ptr<SatLowerLayerServiceConf> llsConf,
                         SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                         SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
    NS_LOG_FUNCTION(this << n << gwId << satId << beamId << fCh << rCh << ncc << llsConf
                         << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    NetDeviceContainer container;

    // Create SatNetDevice
    m_deviceFactory.SetTypeId("ns3::SatLorawanNetDevice");
    Ptr<SatLorawanNetDevice> dev = m_deviceFactory.Create<SatLorawanNetDevice>();

    // Attach the SatNetDevices to nodes
    n->AddDevice(dev);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = beamId;
    params.m_device = dev;
    params.m_txCh = fCh;
    params.m_rxCh = rCh;
    params.m_standard = SatEnums::LORA_GW;

    /**
     * Channel estimation errors
     */
    Ptr<SatChannelEstimationErrorContainer> cec;
    // Not enabled, create only base class
    if (!m_enableChannelEstimationError)
    {
        cec = Create<SatSimpleChannelEstimationErrorContainer>();
    }
    // Create SatFwdLinkChannelEstimationErrorContainer
    else
    {
        uint32_t minWfId = m_superframeSeq->GetWaveformConf()->GetMinWfId();
        uint32_t maxWfId = m_superframeSeq->GetWaveformConf()->GetMaxWfId();
        if (returnLinkRegenerationMode == SatEnums::TRANSPARENT ||
            returnLinkRegenerationMode == SatEnums::REGENERATION_PHY)
        {
            cec = Create<SatRtnLinkChannelEstimationErrorContainer>(minWfId, maxWfId);
        }
        else
        {
            cec = Create<SatFwdLinkChannelEstimationErrorContainer>();
        }
    }

    SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters =
        SatPhyRxCarrierConf::RxCarrierCreateParams_s();
    parameters.m_errorModel = m_errorModel;
    parameters.m_daConstantErrorRate = m_daConstantErrorRate;
    parameters.m_daIfModel = m_daInterferenceModel;
    parameters.m_raIfModel = m_raSettings.m_raInterferenceModel;
    parameters.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
    parameters.m_linkRegenerationMode = returnLinkRegenerationMode;
    parameters.m_bwConverter = m_carrierBandwidthConverter;
    parameters.m_carrierCount = m_rtnLinkCarrierCount;
    parameters.m_cec = cec;
    parameters.m_raCollisionModel = m_raSettings.m_raCollisionModel;
    parameters.m_raConstantErrorRate = m_raSettings.m_raConstantErrorRate;
    parameters.m_randomAccessModel = m_raSettings.m_randomAccessModel;

    Ptr<SatGwPhy> phy = CreateObject<SatGwPhy>(
        params,
        m_linkResults,
        parameters,
        m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE),
        returnLinkRegenerationMode);

    ncc->SetUseLora(true);

    // Set fading
    phy->SetTxFadingContainer(n->GetObject<SatBaseFading>());
    phy->SetRxFadingContainer(n->GetObject<SatBaseFading>());

    Ptr<LorawanMacGateway> mac = CreateObject<LorawanMacGateway>(satId, beamId);

    SatLoraConf satLoraConf;
    satLoraConf.SetConf(mac);

    // Attach the Mac layer receiver to Phy
    SatPhy::ReceiveCallback recCb = MakeCallback(&LorawanMac::Receive, mac);

    // Attach the NCC C/N0 update to Phy
    SatPhy::CnoCallback cnoCb = MakeCallback(&SatNcc::UtCnoUpdated, ncc);

    // Attach the NCC random access load update to Phy
    SatPhy::AverageNormalizedOfferedLoadCallback avgNormalizedOfferedLoadCb =
        MakeCallback(&SatNcc::DoRandomAccessDynamicLoadControl, ncc);

    phy->SetAttribute("ReceiveCb", CallbackValue(recCb));
    phy->SetAttribute("CnoCb", CallbackValue(cnoCb));
    phy->SetAttribute("AverageNormalizedOfferedLoadCallback",
                      CallbackValue(avgNormalizedOfferedLoadCb));

    // Attach the PHY layer to SatNetDevice
    dev->SetPhy(phy);

    // Attach the Mac layer to SatNetDevice
    dev->SetLorawanMac(mac);
    mac->SetDevice(dev);

    mac->SetPhy(phy);

    // Set the device address and pass it to MAC as well
    Mac48Address addr = Mac48Address::Allocate();
    dev->SetAddress(addr);

    Singleton<SatIdMapper>::Get()->AttachMacToTraceId(dev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToGwId(dev->GetAddress(), gwId);
    Singleton<SatIdMapper>::Get()->AttachMacToBeamId(dev->GetAddress(), beamId);
    Singleton<SatIdMapper>::Get()->AttachMacToSatId(dev->GetAddress(), satId + 1);

    phy->Initialize();

    // Create a node info to all the protocol layers
    Ptr<SatNodeInfo> nodeInfo = Create<SatNodeInfo>(SatEnums::NT_GW, n->GetId(), addr);
    dev->SetNodeInfo(nodeInfo);
    mac->SetNodeInfo(nodeInfo);
    phy->SetNodeInfo(nodeInfo);

    // Begin frame end scheduling for processes utilizing frame length as interval
    // Node info needs to be set before the start in order to get the scheduling context correctly
    // set
    if (returnLinkRegenerationMode == SatEnums::TRANSPARENT)
    {
        phy->BeginEndScheduling();
    }

    return dev;
}

} // namespace ns3
