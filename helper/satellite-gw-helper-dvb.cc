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

#include "ns3/satellite-gw-helper-dvb.h"

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-fwd-link-scheduler-default.h>
#include <ns3/satellite-fwd-link-scheduler-time-slicing.h>
#include <ns3/satellite-fwd-link-scheduler.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-gw-llc.h>
#include <ns3/satellite-gw-mac.h>
#include <ns3/satellite-gw-phy.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-link-results.h>
#include <ns3/satellite-lower-layer-service.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-packet-classifier.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/singleton.h>

NS_LOG_COMPONENT_DEFINE("SatGwHelperDvb");

namespace ns3
{

void
logonCallbackHelper(Ptr<SatNcc> ncc,
                    Ptr<SatLowerLayerServiceConf> llsConf,
                    Address utId,
                    uint32_t satId,
                    uint32_t beamId,
                    Callback<void, uint32_t> setRaChannelCallback)
{
    ncc->AddUt(llsConf, utId, satId, beamId, setRaChannelCallback, true);
}

NS_OBJECT_ENSURE_REGISTERED(SatGwHelperDvb);

TypeId
SatGwHelperDvb::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatGwHelperDvb").SetParent<SatGwHelper>().AddConstructor<SatGwHelperDvb>();
    return tid;
}

TypeId
SatGwHelperDvb::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

SatGwHelperDvb::SatGwHelperDvb()
{
    // this default constructor should be never called
    NS_FATAL_ERROR("Default constructor not supported!!!");
}

SatGwHelperDvb::SatGwHelperDvb(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
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
SatGwHelperDvb::Install(Ptr<Node> n,
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
    m_deviceFactory.SetTypeId("ns3::SatNetDevice");
    Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice>();

    // Attach the SatNetDevices to nodes
    n->AddDevice(dev);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = beamId;
    params.m_device = dev;
    params.m_txCh = fCh;
    params.m_rxCh = rCh;
    params.m_standard = SatEnums::DVB_GW;

    // Create a packet classifier
    Ptr<SatPacketClassifier> classifier = Create<SatPacketClassifier>();

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

    ncc->SetUseLogon(m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE)
                         ->IsLogonEnabled());

    // Set fading
    phy->SetTxFadingContainer(n->GetObject<SatBaseFading>());
    phy->SetRxFadingContainer(n->GetObject<SatBaseFading>());

    Ptr<SatGwMac> mac = CreateObject<SatGwMac>(satId,
                                               beamId,
                                               forwardLinkRegenerationMode,
                                               returnLinkRegenerationMode);

    // Set the control message container callbacks
    mac->SetReadCtrlCallback(m_readCtrlCb);
    mac->SetReserveCtrlCallback(m_reserveCtrlCb);
    mac->SetSendCtrlCallback(m_sendCtrlCb);

    mac->SetCrReceiveCallback(MakeCallback(&SatNcc::UtCrReceived, ncc));

    mac->SetHandoverCallback(MakeCallback(&SatNcc::MoveUtBetweenBeams, ncc));

    // Attach the Mac layer receiver to Phy
    SatPhy::ReceiveCallback recCb = MakeCallback(&SatGwMac::Receive, mac);

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
    dev->SetMac(mac);

    // Create Logical Link Control (LLC) layer
    Ptr<SatGwLlc> llc =
        CreateObject<SatGwLlc>(forwardLinkRegenerationMode, returnLinkRegenerationMode);

    // Set the control msg read callback to LLC due to ARQ ACKs
    llc->SetReadCtrlCallback(m_readCtrlCb);

    if (forwardLinkRegenerationMode != SatEnums::TRANSPARENT &&
        forwardLinkRegenerationMode != SatEnums::REGENERATION_PHY)
    {
        llc->SetAdditionalHeaderSize(SatAddressE2ETag::SIZE);
    }

    // Attach the LLC layer to SatNetDevice
    dev->SetLlc(llc);

    // Attach the packet classifier
    dev->SetPacketClassifier(classifier);

    // Attach the device receive callback to SatNetDevice
    llc->SetReceiveCallback(MakeCallback(&SatNetDevice::Receive, dev));

    // Attach the transmit callback to PHY
    mac->SetTransmitCallback(MakeCallback(&SatPhy::SendPdu, phy));

    // Attach the device receive callback to SatLlc
    mac->SetReceiveCallback(MakeCallback(&SatLlc::Receive, llc));

    // Attach the logon receive callback to SatNcc
    mac->SetLogonCallback(MakeBoundCallback(&logonCallbackHelper, ncc, llsConf));

    // Attach the control burst receive callback to SatNcc
    mac->SetControlMessageReceivedCallback(MakeCallback(&SatNcc::ReceiveControlBurst, ncc));

    // Attach the remove UT to SatNcc
    mac->SetRemoveUtCallback(MakeCallback(&SatNcc::RemoveUt, ncc));

    // Set the device address and pass it to MAC as well
    Mac48Address addr = Mac48Address::Allocate();
    dev->SetAddress(addr);

    Singleton<SatIdMapper>::Get()->AttachMacToTraceId(dev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToGwId(dev->GetAddress(), gwId);
    Singleton<SatIdMapper>::Get()->AttachMacToBeamId(dev->GetAddress(), beamId);
    Singleton<SatIdMapper>::Get()->AttachMacToSatId(dev->GetAddress(), satId + 1);

    // Create an encapsulator for control messages.
    // Source = GW address
    // Destination = broadcast address
    // Flow id = by default 0
    Ptr<SatQueue> queue = CreateObject<SatQueue>(SatEnums::CONTROL_FID);
    Ptr<SatBaseEncapsulator> gwEncap =
        CreateObject<SatBaseEncapsulator>(addr,
                                          Mac48Address::GetBroadcast(),
                                          addr,
                                          Mac48Address::GetBroadcast(),
                                          SatEnums::CONTROL_FID);
    gwEncap->SetQueue(queue);
    llc->AddEncap(addr, Mac48Address::GetBroadcast(), SatEnums::CONTROL_FID, gwEncap);
    llc->SetCtrlMsgCallback(MakeCallback(&SatNetDevice::SendControlMsg, dev));

    phy->Initialize();

    // Create a node info to all the protocol layers
    Ptr<SatNodeInfo> nodeInfo = Create<SatNodeInfo>(SatEnums::NT_GW, n->GetId(), addr);
    dev->SetNodeInfo(nodeInfo);
    llc->SetNodeInfo(nodeInfo);
    mac->SetNodeInfo(nodeInfo);
    phy->SetNodeInfo(nodeInfo);

    // Begin frame end scheduling for processes utilizing frame length as interval
    // Node info needs to be set before the start in order to get the scheduling context correctly
    // set
    if (returnLinkRegenerationMode == SatEnums::TRANSPARENT)
    {
        phy->BeginEndScheduling();
    }

    // TODO: When multiple carriers are supported. Multiple scheduler are needed too.
    double carrierBandwidth =
        m_carrierBandwidthConverter(SatEnums::FORWARD_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);

    Ptr<SatFwdLinkScheduler> fwdLinkScheduler;
    switch (m_fwdSchedulingAlgorithm)
    {
    case SatEnums::NO_TIME_SLICING:
        fwdLinkScheduler =
            CreateObject<SatFwdLinkSchedulerDefault>(m_bbFrameConf, addr, carrierBandwidth);
        break;
    case SatEnums::TIME_SLICING:
        fwdLinkScheduler =
            CreateObject<SatFwdLinkSchedulerTimeSlicing>(m_bbFrameConf, addr, carrierBandwidth);
        break;
    default:
        NS_FATAL_ERROR("Forward scheduling algorithm is not implemented");
    }

    fwdLinkScheduler->SetSendControlMsgCallback(MakeCallback(&SatNetDevice::SendControlMsg, dev));

    // Attach the LLC Tx opportunity and scheduling context getter callbacks to SatFwdLinkScheduler
    fwdLinkScheduler->SetTxOpportunityCallback(MakeCallback(&SatGwLlc::NotifyTxOpportunity, llc));
    fwdLinkScheduler->SetSchedContextCallback(MakeCallback(&SatLlc::GetSchedulingContexts, llc));

    // set scheduler to Mac
    mac->SetFwdScheduler(fwdLinkScheduler);

    mac->StartPeriodicTransmissions();

    return dev;
}

} // namespace ns3
