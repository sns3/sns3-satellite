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

#include "satellite-ut-helper-lora.h"

#include <ns3/callback.h>
#include <ns3/log.h>
#include <ns3/lorawan-mac-end-device-class-a.h>
#include <ns3/pointer.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-lora-conf.h>
#include <ns3/satellite-lorawan-net-device.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-tx.h>
#include <ns3/satellite-queue.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/satellite-ut-phy.h>
#include <ns3/singleton.h>

NS_LOG_COMPONENT_DEFINE("SatUtHelperLora");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatUtHelperLora);

TypeId
SatUtHelperLora::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatUtHelperLora").SetParent<SatUtHelper>().AddConstructor<SatUtHelperLora>();
    return tid;
}

TypeId
SatUtHelperLora::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatUtHelperLora::SatUtHelperLora()
{
    NS_LOG_FUNCTION(this);

    // this default constructor should be never called
    NS_FATAL_ERROR("SatUtHelperLora::SatUtHelperLora - Constructor not in use");
}

SatUtHelperLora::SatUtHelperLora(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                                 uint32_t fwdLinkCarrierCount,
                                 Ptr<SatSuperframeSeq> seq,
                                 SatMac::ReadCtrlMsgCallback readCb,
                                 SatMac::ReserveCtrlMsgCallback reserveCb,
                                 SatMac::SendCtrlMsgCallback sendCb,
                                 RandomAccessSettings_s randomAccessSettings)
    : SatUtHelper(carrierBandwidthConverter,
                  fwdLinkCarrierCount,
                  seq,
                  readCb,
                  reserveCb,
                  sendCb,
                  randomAccessSettings)
{
    NS_LOG_FUNCTION(this << fwdLinkCarrierCount << seq);
}

Ptr<NetDevice>
SatUtHelperLora::Install(Ptr<Node> n,
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
    NS_LOG_FUNCTION(this << n << satId << beamId << fCh << rCh << gwNd << ncc << satUserAddress
                         << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    NetDeviceContainer container;

    // Create SatNetDevice
    m_deviceFactory.SetTypeId("ns3::SatLorawanNetDevice");
    Ptr<SatLorawanNetDevice> dev = m_deviceFactory.Create<SatLorawanNetDevice>();

    // Attach the SatNetDevice to node
    n->AddDevice(dev);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = beamId;
    params.m_device = dev;
    params.m_txCh = rCh;
    params.m_rxCh = fCh;
    params.m_standard = SatEnums::LORA_UT;

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
        cec = Create<SatFwdLinkChannelEstimationErrorContainer>();
    }

    SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters =
        SatPhyRxCarrierConf::RxCarrierCreateParams_s();
    parameters.m_errorModel = m_errorModel;
    parameters.m_daConstantErrorRate = m_daConstantErrorRate;
    parameters.m_daIfModel = m_daInterferenceModel;
    parameters.m_raIfModel = m_raSettings.m_raInterferenceModel;
    parameters.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
    parameters.m_linkRegenerationMode = forwardLinkRegenerationMode;
    parameters.m_bwConverter = m_carrierBandwidthConverter;
    parameters.m_carrierCount = m_fwdLinkCarrierCount;
    parameters.m_cec = cec;
    parameters.m_raCollisionModel = m_raSettings.m_raCollisionModel;
    parameters.m_randomAccessModel = m_raSettings.m_randomAccessModel;

    Ptr<SatUtPhy> phy = CreateObject<SatUtPhy>(
        params,
        m_linkResults,
        parameters,
        m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE),
        forwardLinkRegenerationMode);
    phy->SetChannelPairGetterCallback(cbChannel);

    // Set fading
    phy->SetTxFadingContainer(n->GetObject<SatBaseFading>());
    phy->SetRxFadingContainer(n->GetObject<SatBaseFading>());

    Ptr<LorawanMacEndDeviceClassA> mac =
        CreateObject<LorawanMacEndDeviceClassA>(satId, beamId, m_superframeSeq);

    // TODO configuration for EU only
    mac->SetTxDbmForTxPower(std::vector<double>{16, 14, 12, 10, 8, 6, 4, 2});

    SatLoraConf satLoraConf;
    satLoraConf.SetConf(mac);

    // Attach the Mac layer receiver to Phy
    SatPhy::ReceiveCallback recCb = MakeCallback(&LorawanMac::Receive, mac);

    phy->SetAttribute("ReceiveCb", CallbackValue(recCb));

    // Attach the PHY layer to SatNetDevice
    dev->SetPhy(phy);

    // Attach the Mac layer to SatNetDevice
    dev->SetLorawanMac(mac);
    mac->SetDevice(dev);

    mac->SetPhy(phy);
    mac->SetPhyRx(DynamicCast<SatLoraPhyRx>(phy->GetPhyRx()));
    mac->SetRaModel(m_raSettings.m_randomAccessModel);

    // Set the device address and pass it to MAC as well
    Mac48Address addr = Mac48Address::Allocate();
    dev->SetAddress(addr);

    Singleton<SatIdMapper>::Get()->AttachMacToTraceId(dev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToUtId(dev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToBeamId(dev->GetAddress(), beamId);
    Singleton<SatIdMapper>::Get()->AttachMacToSatId(dev->GetAddress(), satId + 1);

    // Create encapsulator and add it to UT's LLC
    Mac48Address gwAddr = Mac48Address::ConvertFrom(gwNd->GetAddress());

    // set serving GW MAC address to RM
    mac->SetRoutingUpdateCallback(cbRouting);
    mac->SetGwAddress(gwAddr);

    // Add UT to NCC
    ncc->AddUt(m_llsConf,
               dev->GetAddress(),
               satId,
               beamId,
               MakeCallback(&LorawanMacEndDevice::SetRaChannel, mac));

    phy->Initialize();

    // Create a node info to all the protocol layers
    Ptr<SatNodeInfo> nodeInfo = Create<SatNodeInfo>(SatEnums::NT_UT, n->GetId(), addr);
    dev->SetNodeInfo(nodeInfo);
    mac->SetNodeInfo(nodeInfo);
    phy->SetNodeInfo(nodeInfo);

    return dev;
}

} // namespace ns3
