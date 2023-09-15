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

#include "satellite-ut-helper-dvb.h"

#include <ns3/callback.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/satellite-base-encapsulator.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-generic-stream-encapsulator-arq.h>
#include <ns3/satellite-generic-stream-encapsulator.h>
#include <ns3/satellite-gw-llc.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-mobility-observer.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-packet-classifier.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-tx.h>
#include <ns3/satellite-queue.h>
#include <ns3/satellite-request-manager.h>
#include <ns3/satellite-return-link-encapsulator-arq.h>
#include <ns3/satellite-return-link-encapsulator.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/satellite-ut-handover-module.h>
#include <ns3/satellite-ut-llc.h>
#include <ns3/satellite-ut-mac.h>
#include <ns3/satellite-ut-phy.h>
#include <ns3/satellite-ut-scheduler.h>
#include <ns3/satellite-utils.h>
#include <ns3/singleton.h>

NS_LOG_COMPONENT_DEFINE("SatUtHelperDvb");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatUtHelperDvb);

TypeId
SatUtHelperDvb::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatUtHelperDvb")
            .SetParent<SatUtHelper>()
            .AddConstructor<SatUtHelperDvb>();
    return tid;
}

TypeId
SatUtHelperDvb::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatUtHelperDvb::SatUtHelperDvb()
{
    NS_LOG_FUNCTION(this);

    // this default constructor should be never called
    NS_FATAL_ERROR("SatUtHelperDvb::SatUtHelperDvb - Constructor not in use");
}

SatUtHelperDvb::SatUtHelperDvb(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                               uint32_t fwdLinkCarrierCount,
                               Ptr<SatSuperframeSeq> seq,
                               SatMac::ReadCtrlMsgCallback readCb,
                               SatMac::ReserveCtrlMsgCallback reserveCb,
                               SatMac::SendCtrlMsgCallback sendCb,
                               RandomAccessSettings_s randomAccessSettings)
    : SatUtHelper(carrierBandwidthConverter, fwdLinkCarrierCount, seq, readCb, reserveCb, sendCb, randomAccessSettings)
{
    NS_LOG_FUNCTION(this << fwdLinkCarrierCount << seq);
}

Ptr<NetDevice>
SatUtHelperDvb::Install(Ptr<Node> n,
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
    NS_LOG_FUNCTION(this << n << satId << beamId << fCh << rCh << gwNd << ncc << satUserAddress << forwardLinkRegenerationMode << returnLinkRegenerationMode);

    NetDeviceContainer container;

    // Create SatNetDevice
    m_deviceFactory.SetTypeId("ns3::SatNetDevice");
    Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice>();

    // Attach the SatNetDevice to node
    n->AddDevice(dev);

    SatPhy::CreateParam_t params;
    params.m_satId = satId;
    params.m_beamId = beamId;
    params.m_device = dev;
    params.m_txCh = rCh;
    params.m_rxCh = fCh;
    params.m_standard = SatEnums::DVB_UT;

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

    Ptr<SatUtMac> mac = CreateObject<SatUtMac>(satId,
                                               beamId,
                                               m_superframeSeq,
                                               forwardLinkRegenerationMode,
                                               returnLinkRegenerationMode,
                                               m_crdsaOnlyForControl);

    // Set the control message container callbacks
    mac->SetReadCtrlCallback(m_readCtrlCb);
    mac->SetReserveCtrlCallback(m_reserveCtrlCb);
    mac->SetSendCtrlCallback(m_sendCtrlCb);

    // Set timing advance callback to mac (if not asynchronous access)
    if (m_raSettings.m_randomAccessModel != SatEnums::RA_MODEL_ESSA)
    {
        Ptr<SatMobilityObserver> observer = n->GetObject<SatMobilityObserver>();
        NS_ASSERT(observer != NULL);

        SatUtMac::TimingAdvanceCallback timingCb =
            MakeCallback(&SatMobilityObserver::GetTimingAdvance, observer);
        mac->SetTimingAdvanceCallback(timingCb);
    }

    // Attach the Mac layer receiver to Phy
    SatPhy::ReceiveCallback recCb = MakeCallback(&SatUtMac::Receive, mac);

    phy->SetAttribute("ReceiveCb", CallbackValue(recCb));
    mac->SetTxCheckCallback(MakeCallback(&SatUtPhy::IsTxPossible, phy));

    // Create callback to inform phy layer slices subscription
    mac->SetSliceSubscriptionCallback(MakeCallback(&SatUtPhy::UpdateSliceSubscription, phy));

    // Create Logical Link Control (LLC) layer
    Ptr<SatUtLlc> llc =
        CreateObject<SatUtLlc>(forwardLinkRegenerationMode, returnLinkRegenerationMode);

    // Set the control msg read callback to LLC due to ARQ ACKs
    llc->SetReadCtrlCallback(m_readCtrlCb);

    // Create a request manager and attach it to LLC, and set control message callback to RM
    Ptr<SatRequestManager> rm = CreateObject<SatRequestManager>();
    llc->SetRequestManager(rm);
    rm->SetCtrlMsgCallback(MakeCallback(&SatNetDevice::SendControlMsg, dev));

    if (returnLinkRegenerationMode != SatEnums::TRANSPARENT &&
        returnLinkRegenerationMode != SatEnums::REGENERATION_PHY)
    {
        llc->SetAdditionalHeaderSize(SatAddressE2ETag::SIZE);
        rm->SetHeaderOffsetVbdc(38.0 / (38 - 2 - SatAddressE2ETag::SIZE));
    }

    // Set the callback to check whether control msg transmissions are possible
    rm->SetCtrlMsgTxPossibleCallback(MakeCallback(&SatUtMac::ControlMsgTransmissionPossible, mac));

    // Set the callback to check whether logon msg transmissions are possible
    rm->SetLogonMsgTxPossibleCallback(MakeCallback(&SatUtMac::LogonMsgTransmissionPossible, mac));

    // Set TBTP callback to UT MAC
    mac->SetAssignedDaResourcesCallback(MakeCallback(&SatRequestManager::AssignedDaResources, rm));

    // Set Send Logon callback to UT MAC
    mac->SetSendLogonCallback(MakeCallback(&SatRequestManager::SendLogonMessage, rm));

    // Attach the PHY layer to SatNetDevice
    dev->SetPhy(phy);

    // Attach the Mac layer to SatNetDevice
    dev->SetMac(mac);

    // Attach the LLC layer to SatNetDevice
    dev->SetLlc(llc);

    // Attach the packet classifier
    dev->SetPacketClassifier(classifier);

    // Attach the Mac layer C/N0 updates receiver to Phy
    SatPhy::CnoCallback cnoCb = MakeCallback(&SatRequestManager::CnoUpdated, rm);
    phy->SetAttribute("CnoCb", CallbackValue(cnoCb));

    // Set the device address and pass it to MAC as well
    Mac48Address addr = Mac48Address::Allocate();
    dev->SetAddress(addr);

    Singleton<SatIdMapper>::Get()->AttachMacToTraceId(dev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToUtId(dev->GetAddress());
    Singleton<SatIdMapper>::Get()->AttachMacToBeamId(dev->GetAddress(), beamId);
    Singleton<SatIdMapper>::Get()->AttachMacToSatId(dev->GetAddress(), satId + 1);

    // Create encapsulator and add it to UT's LLC
    Mac48Address gwAddr = Mac48Address::ConvertFrom(gwNd->GetAddress());

    // Create an encapsulator for control messages.
    // Source = UT MAC address
    // Destination = GW MAC address (or SAT user MAC address if regenerative)
    // Flow id = by default 0
    Ptr<SatBaseEncapsulator> utEncap;
    if (returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        utEncap = CreateObject<SatBaseEncapsulator>(addr,
                                                    Mac48Address::ConvertFrom(satUserAddress),
                                                    addr,
                                                    gwAddr,
                                                    SatEnums::CONTROL_FID);
    }
    else
    {
        utEncap =
            CreateObject<SatBaseEncapsulator>(addr, gwAddr, addr, gwAddr, SatEnums::CONTROL_FID);
    }

    // Create queue event callbacks to MAC (for random access) and RM (for on-demand DAMA)
    SatQueue::QueueEventCallback macCb;
    if (m_raSettings.m_randomAccessModel == SatEnums::RA_MODEL_ESSA)
    {
        macCb = MakeCallback(&SatUtMac::ReceiveQueueEventEssa, mac);
    }
    else
    {
        macCb = MakeCallback(&SatUtMac::ReceiveQueueEvent, mac);
    }
    SatQueue::LogonCallback logonCb = MakeCallback(&SatUtMac::SendLogon, mac);
    SatQueue::QueueEventCallback rmCb = MakeCallback(&SatRequestManager::ReceiveQueueEvent, rm);

    // Create a queue
    Ptr<SatQueue> queue = CreateObject<SatQueue>(SatEnums::CONTROL_FID);
    queue->AddLogonCallback(logonCb);
    queue->AddQueueEventCallback(macCb);
    queue->AddQueueEventCallback(rmCb);
    utEncap->SetQueue(queue);
    if (returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        llc->AddEncap(addr,
                      Mac48Address::ConvertFrom(satUserAddress),
                      SatEnums::CONTROL_FID,
                      utEncap);
    }
    else
    {
        llc->AddEncap(addr, gwAddr, SatEnums::CONTROL_FID, utEncap);
    }
    rm->AddQueueCallback(SatEnums::CONTROL_FID, MakeCallback(&SatQueue::GetQueueStatistics, queue));

    // Add callbacks to LLC for future need. LLC creates encapsulators and
    // decapsulators dynamically 'on-a-need-basis'.
    llc->SetCtrlMsgCallback(MakeCallback(&SatNetDevice::SendControlMsg, dev));
    llc->SetMacQueueEventCallback(macCb);

    // set serving GW MAC address to RM
    mac->SetRoutingUpdateCallback(cbRouting);
    mac->SetGatewayUpdateCallback(MakeCallback(&SatUtLlc::SetGwAddress, llc));
    mac->SetGwAddress(gwAddr);

    // Attach the transmit callback to PHY
    mac->SetTransmitCallback(MakeCallback(&SatPhy::SendPdu, phy));

    // Attach the PHY handover callback to SatMac
    mac->SetHandoverCallback(MakeCallback(&SatUtPhy::PerformHandover, phy));

    // Attach the LLC receive callback to SatMac
    mac->SetReceiveCallback(MakeCallback(&SatLlc::Receive, llc));

    llc->SetReceiveCallback(MakeCallback(&SatNetDevice::Receive, dev));

    Ptr<SatSuperframeConf> superFrameConf =
        m_superframeSeq->GetSuperframeConf(SatConstVariables::SUPERFRAME_SEQUENCE);
    bool enableLogon = superFrameConf->IsLogonEnabled();
    uint32_t logonChannelId = superFrameConf->GetLogonChannelIndex();

    // Add UT to NCC
    if (enableLogon)
    {
        ncc->ReserveLogonChannel(logonChannelId);
    }
    else
    {
        ncc->AddUt(m_llsConf,
                   dev->GetAddress(),
                   satId,
                   beamId,
                   MakeCallback(&SatUtMac::SetRaChannel, mac));
    }

    phy->Initialize();

    // Create UT scheduler for MAC and connect callbacks to LLC
    Ptr<SatUtScheduler> utScheduler = CreateObject<SatUtScheduler>(m_llsConf);
    utScheduler->SetTxOpportunityCallback(MakeCallback(&SatUtLlc::NotifyTxOpportunity, llc));
    utScheduler->SetSchedContextCallback(MakeCallback(&SatLlc::GetSchedulingContexts, llc));
    mac->SetAttribute("Scheduler", PointerValue(utScheduler));

    // Create a node info to all the protocol layers
    Ptr<SatNodeInfo> nodeInfo = Create<SatNodeInfo>(SatEnums::NT_UT, n->GetId(), addr);
    dev->SetNodeInfo(nodeInfo);
    llc->SetNodeInfo(nodeInfo);
    mac->SetNodeInfo(nodeInfo);
    phy->SetNodeInfo(nodeInfo);

    rm->Initialize(m_llsConf, m_superframeSeq->GetDuration(0));

    if (m_raSettings.m_randomAccessModel != SatEnums::RA_MODEL_OFF)
    {
        Ptr<SatRandomAccessConf> randomAccessConf =
            CreateObject<SatRandomAccessConf>(m_llsConf, m_superframeSeq);

        /// create RA module with defaults
        Ptr<SatRandomAccess> randomAccess =
            CreateObject<SatRandomAccess>(randomAccessConf, m_raSettings.m_randomAccessModel);

        /// attach callbacks
        if (m_crdsaOnlyForControl)
        {
            randomAccess->SetAreBuffersEmptyCallback(
                MakeCallback(&SatLlc::ControlBuffersEmpty, llc));
        }
        else
        {
            randomAccess->SetAreBuffersEmptyCallback(MakeCallback(&SatLlc::BuffersEmpty, llc));
        }

        /// attach the RA module
        mac->SetRandomAccess(randomAccess);
        if (enableLogon)
        {
            mac->SetLogonChannel(logonChannelId);
            mac->LogOff();
        }
    }
    else if (enableLogon)
    {
        NS_FATAL_ERROR("Cannot simulate logon without a RA frame");
    }

    Ptr<SatUtHandoverModule> utHandoverModule = n->GetObject<SatUtHandoverModule>();
    if (utHandoverModule != NULL)
    {
        utHandoverModule->SetHandoverRequestCallback(
            MakeCallback(&SatRequestManager::SendHandoverRecommendation, rm));
        mac->SetBeamCheckerCallback(
            MakeCallback(&SatUtHandoverModule::CheckForHandoverRecommendation, utHandoverModule));
        mac->SetAskedBeamCallback(
            MakeCallback(&SatUtHandoverModule::GetAskedBeamId, utHandoverModule));
        mac->SetBeamScheculerCallback(MakeCallback(&SatNcc::GetBeamScheduler, ncc));
        mac->SetUpdateGwAddressCallback(MakeCallback(&SatRequestManager::SetGwAddress, rm));
    }

    return dev;
}

} // namespace ns3
