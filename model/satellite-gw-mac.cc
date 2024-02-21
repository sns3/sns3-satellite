/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "satellite-gw-mac.h"

#include "satellite-bbframe.h"
#include "satellite-beam-scheduler.h"
#include "satellite-control-message.h"
#include "satellite-fwd-link-scheduler.h"
#include "satellite-log.h"
#include "satellite-mac-tag.h"
#include "satellite-rtn-link-time.h"
#include "satellite-signal-parameters.h"
#include "satellite-time-tag.h"
#include "satellite-uplink-info-tag.h"
#include "satellite-utils.h"

#include <ns3/address.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/mac48-address.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/singleton.h>
#include <ns3/uinteger.h>

NS_LOG_COMPONENT_DEFINE("SatGwMac");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatGwMac);

TypeId
SatGwMac::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatGwMac")
            .SetParent<SatMac>()
            .AddConstructor<SatGwMac>()
            .AddAttribute("Scheduler",
                          "Forward link scheduler used by this Sat GW MAC.",
                          PointerValue(),
                          MakePointerAccessor(&SatGwMac::m_fwdScheduler),
                          MakePointerChecker<SatFwdLinkScheduler>())
            .AddAttribute("GuardTime",
                          "Guard time in forward link",
                          TimeValue(MicroSeconds(1)),
                          MakeTimeAccessor(&SatGwMac::m_guardTime),
                          MakeTimeChecker())
            .AddAttribute("NcrBroadcastPeriod",
                          "Interval between two broadcast of NCR dates",
                          TimeValue(MilliSeconds(100)),
                          MakeTimeAccessor(&SatGwMac::m_ncrInterval),
                          MakeTimeChecker())
            .AddAttribute("UseCmt",
                          "Use CMT control messages to correct time on the UTs",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatGwMac::m_useCmt),
                          MakeBooleanChecker())
            .AddAttribute("CmtPeriodMin",
                          "Minimum interval between two CMT control messages for a same UT",
                          TimeValue(MilliSeconds(550)),
                          MakeTimeAccessor(&SatGwMac::m_cmtPeriodMin),
                          MakeTimeChecker())
            .AddAttribute("SendNcrBroadcast",
                          "Broadcast NCR messages to all UTs",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SatGwMac::m_broadcastNcr),
                          MakeBooleanChecker())
            .AddAttribute("DisableSchedulingIfNoDeviceConnected",
                          "If true, the periodic calls of StartTransmission are not called when no "
                          "devices are connected to this MAC",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatGwMac::m_disableSchedulingIfNoDeviceConnected),
                          MakeBooleanChecker())
            .AddTraceSource("BBFrameTxTrace",
                            "Trace for transmitted BB Frames.",
                            MakeTraceSourceAccessor(&SatGwMac::m_bbFrameTxTrace),
                            "ns3::SatBbFrame::BbFrameCallback");
    return tid;
}

TypeId
SatGwMac::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatGwMac::SatGwMac()
    : SatMac(),
      m_fwdScheduler(),
      m_guardTime(MicroSeconds(1)),
      m_ncrInterval(MilliSeconds(100)),
      m_useCmt(false),
      m_lastCmtSent(),
      m_cmtPeriodMin(MilliSeconds(550)),
      m_broadcastNcr(true),
      m_disableSchedulingIfNoDeviceConnected(false),
      m_periodicTransmissionEnabled(false)
{
    NS_LOG_FUNCTION(this);

    NS_FATAL_ERROR("SatUtMac::SatGwMac - Constructor not in use");
}

SatGwMac::SatGwMac(uint32_t satId,
                   uint32_t beamId,
                   uint32_t feederSatId,
                   uint32_t feederBeamId,
                   SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                   SatEnums::RegenerationMode_t returnLinkRegenerationMode)
    : SatMac(satId, beamId, forwardLinkRegenerationMode, returnLinkRegenerationMode),
      m_feederSatId(feederSatId),
      m_feederBeamId(feederBeamId),
      m_fwdScheduler(),
      m_guardTime(MicroSeconds(1)),
      m_ncrInterval(MilliSeconds(100)),
      m_useCmt(false),
      m_lastCmtSent(),
      m_cmtPeriodMin(MilliSeconds(550)),
      m_broadcastNcr(true),
      m_disableSchedulingIfNoDeviceConnected(false),
      m_periodicTransmissionEnabled(false)
{
    NS_LOG_FUNCTION(this);
}

SatGwMac::~SatGwMac()
{
    NS_LOG_FUNCTION(this);
}

void
SatGwMac::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_txOpportunityCallback.Nullify();
    m_crReceiveCallback.Nullify();
    m_handoverCallback.Nullify();
    m_logonCallback.Nullify();
    m_controlMessageReceivedCallback.Nullify();

    SatMac::DoDispose();
}

void
SatGwMac::StartPeriodicTransmissions()
{
    NS_LOG_FUNCTION(this);

    if (m_disableSchedulingIfNoDeviceConnected && !HasPeer())
    {
        NS_LOG_INFO("Do not start beam " << m_beamId << " because no device is connected");
        return;
    }

    if (m_periodicTransmissionEnabled == true)
    {
        NS_LOG_INFO("Beam " << m_beamId << " already enabled");
        return;
    }

    m_periodicTransmissionEnabled = true;

    if (!m_fwdScheduler)
    {
        NS_FATAL_ERROR("Scheduler not set for GW MAC!!!");
    }

    m_clearQueuesCallback();
    m_fwdScheduler->ClearAllPackets();

    /**
     * It is currently assumed that there is only one carrier in FWD link. This
     * carrier has a default index of 0.
     * TODO: When enabling multi-carrier support for FWD link, we need to
     * modify the FWD link scheduler to schedule separately each FWD link
     * carrier.
     */
    Simulator::Schedule(Seconds(0), &SatGwMac::StartTransmission, this, 0);

    if (m_broadcastNcr)
    {
        Simulator::Schedule(MilliSeconds(50), &SatGwMac::StartNcrTransmission, this);
    }
}

void
SatGwMac::Receive(SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
    NS_LOG_FUNCTION(this);

    // Add packet trace entry:
    m_packetTrace(Simulator::Now(),
                  SatEnums::PACKET_RECV,
                  m_nodeInfo->GetNodeType(),
                  m_nodeInfo->GetNodeId(),
                  m_nodeInfo->GetMacAddress(),
                  SatEnums::LL_MAC,
                  SatEnums::LD_RETURN,
                  SatUtils::GetPacketInfo(packets));

    // Invoke the `Rx` and `RxDelay` trace sources.
    RxTraces(packets);

    Address utId;

    for (SatPhy::PacketContainer_t::iterator i = packets.begin(); i != packets.end(); i++)
    {
        // Remove packet tag
        SatMacTag macTag;
        bool mSuccess = (*i)->PeekPacketTag(macTag);
        if (!mSuccess)
        {
            NS_FATAL_ERROR("MAC tag was not found from the packet!");
        }
        SatAddressE2ETag addressE2ETag;
        mSuccess = (*i)->PeekPacketTag(addressE2ETag);
        if (!mSuccess)
        {
            NS_FATAL_ERROR("Address E2E tag was not found from the packet!");
        }

        NS_LOG_INFO("Packet from " << macTag.GetSourceAddress() << " to "
                                   << macTag.GetDestAddress());
        NS_LOG_INFO("Receiver " << m_nodeInfo->GetMacAddress());

        // If the packet is intended for this receiver
        Mac48Address destAddress = macTag.GetDestAddress();
        utId = addressE2ETag.GetE2ESourceAddress();

        if (destAddress == m_nodeInfo->GetMacAddress() || destAddress.IsBroadcast())
        {
            // Peek control msg tag
            SatControlMsgTag ctrlTag;
            bool cSuccess = (*i)->PeekPacketTag(ctrlTag);

            if (cSuccess)
            {
                SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType();

                if (cType != SatControlMsgTag::SAT_NON_CTRL_MSG)
                {
                    uint32_t beamId;
                    uint32_t satId;
                    switch (m_returnLinkRegenerationMode)
                    {
                    case SatEnums::TRANSPARENT:
                    case SatEnums::REGENERATION_PHY: {
                        beamId = rxParams->m_beamId;
                        satId = rxParams->m_satId;
                        break;
                    }
                    case SatEnums::REGENERATION_LINK:
                    case SatEnums::REGENERATION_NETWORK: {
                        SatUplinkInfoTag satUplinkInfoTag;
                        if (!(*i)->PeekPacketTag(satUplinkInfoTag))
                        {
                            NS_FATAL_ERROR("SatUplinkInfoTag not found!");
                        }
                        beamId = satUplinkInfoTag.GetBeamId();
                        satId = satUplinkInfoTag.GetSatId();
                        break;
                    }
                    default:
                        NS_FATAL_ERROR("Unknown regeneration mode");
                    }
                    ReceiveSignalingPacket(*i, satId, beamId);
                }
                else
                {
                    NS_FATAL_ERROR("A control message received with not valid msg type!");
                }
            }
            else
            {
                // Pass the source address to LLC
                m_rxCallback(*i,
                             addressE2ETag.GetE2ESourceAddress(),
                             addressE2ETag.GetE2EDestAddress());
            }
        }
        else
        {
            NS_LOG_INFO(
                "Packet intended for others received by MAC: " << m_nodeInfo->GetMacAddress());
        }
    }

    if (m_useCmt)
    {
        switch (m_returnLinkRegenerationMode)
        {
        case SatEnums::TRANSPARENT: {
            if (rxParams->m_txInfo.waveformId == 2)
            {
                SendCmtMessage(utId,
                               rxParams->m_duration,
                               Seconds(0),
                               rxParams->m_satId,
                               rxParams->m_beamId);
                m_controlMessageReceivedCallback(utId, rxParams->m_satId, rxParams->m_beamId);
            }
            break;
        }
        case SatEnums::REGENERATION_PHY: {
            if (rxParams->m_txInfo.waveformId == 2)
            {
                for (SatPhy::PacketContainer_t::iterator i = packets.begin(); i != packets.end();
                     i++)
                {
                    SatControlMsgTag ctrlTag;
                    if ((*i)->PeekPacketTag(ctrlTag))
                    {
                        // This is a NCR CTRL packet, do not use it
                        continue;
                    }
                    SatUplinkInfoTag satUplinkInfoTag;
                    if (!(*i)->PeekPacketTag(satUplinkInfoTag))
                    {
                        NS_FATAL_ERROR("SatUplinkInfoTag not found !");
                    }
                    Time satelliteReceptionTime = satUplinkInfoTag.GetSatelliteReceptionTime();
                    uint32_t beamId = satUplinkInfoTag.GetBeamId();
                    uint32_t satId = satUplinkInfoTag.GetSatId();

                    SendCmtMessage(utId, Seconds(0), satelliteReceptionTime, satId, beamId);
                    m_controlMessageReceivedCallback(utId, satId, beamId);
                }
            }
            break;
        }
        case SatEnums::REGENERATION_LINK:
        case SatEnums::REGENERATION_NETWORK: {
            for (SatPhy::PacketContainer_t::iterator i = packets.begin(); i != packets.end(); i++)
            {
                SatUplinkInfoTag satUplinkInfoTag;
                if (!(*i)->PeekPacketTag(satUplinkInfoTag))
                {
                    NS_FATAL_ERROR("SatUplinkInfoTag not found !");
                }
                Time satelliteReceptionTime = satUplinkInfoTag.GetSatelliteReceptionTime();
                uint32_t beamId = satUplinkInfoTag.GetBeamId();
                uint32_t satId = satUplinkInfoTag.GetSatId();
                bool isControl = satUplinkInfoTag.IsControl();

                if (isControl)
                {
                    SendCmtMessage(utId, Seconds(0), satelliteReceptionTime, satId, beamId);
                    m_controlMessageReceivedCallback(utId, satId, beamId);
                }
            }
            break;
        }
        default:
            NS_FATAL_ERROR("Unknown regeneration mode, or received");
        }
    }
}

void
SatGwMac::StartTransmission(uint32_t carrierId)
{
    NS_LOG_FUNCTION(this << carrierId);

    if (m_handoverModule != nullptr)
    {
        if (m_handoverModule->CheckForHandoverRecommendation(m_feederSatId, m_feederBeamId))
        {
            NS_LOG_INFO("GW handover, old satellite is " << m_feederSatId << ", old beam is "
                                                         << m_feederBeamId);

            Ptr<SatBeamScheduler> srcScheduler =
                m_beamSchedulerCallback(m_feederSatId, m_feederBeamId);

            m_feederSatId = m_handoverModule->GetAskedSatId();
            m_feederBeamId = m_handoverModule->GetAskedBeamId();

            NS_LOG_INFO("GW handover, new satellite is " << m_feederSatId << ", new beam is "
                                                         << m_feederBeamId);

            Ptr<SatBeamScheduler> dstScheduler =
                m_beamSchedulerCallback(m_feederSatId, m_feederBeamId);
            srcScheduler->DisconnectGw(m_nodeInfo->GetMacAddress());
            dstScheduler->ConnectGw(m_nodeInfo->GetMacAddress());

            m_updateIslCallback();

            Ptr<SatGeoNetDevice> geoNetDevice =
                DynamicCast<SatGeoNetDevice>(m_geoNodesCallback().Get(m_feederSatId)->GetDevice(0));
            Mac48Address satFeederAddress = geoNetDevice->GetSatelliteFeederAddress(m_beamId);
            SetSatelliteAddress(satFeederAddress);
            if (m_gwLlcSetSatelliteAddress.IsNull() == false)
            {
                m_gwLlcSetSatelliteAddress(satFeederAddress);
            }

            m_beamCallback(m_feederSatId, m_beamId);

            m_handoverModule->HandoverFinished();
        }
    }

    if (m_nodeInfo->GetNodeType() == SatEnums::NT_GW)
    {
        m_lastSOF.push(Simulator::Now());
        uint8_t lastSOFSize = m_ncrV2 ? 3 : 1;
        if (m_lastSOF.size() > lastSOFSize)
        {
            m_lastSOF.pop();
        }
    }

    Time txDuration;

    if (m_txEnabled && (!m_disableSchedulingIfNoDeviceConnected || m_periodicTransmissionEnabled))
    {
        std::pair<Ptr<SatBbFrame>, const Time> bbFrameInfo = m_fwdScheduler->GetNextFrame();
        Ptr<SatBbFrame> bbFrame = bbFrameInfo.first;
        txDuration = bbFrameInfo.second;

        // trace out BB frames sent.
        m_bbFrameTxTrace(bbFrame);

        // Handle both dummy frames and normal frames
        if (bbFrame)
        {
            // Add packet trace entry:
            m_packetTrace(Simulator::Now(),
                          SatEnums::PACKET_SENT,
                          m_nodeInfo->GetNodeType(),
                          m_nodeInfo->GetNodeId(),
                          m_nodeInfo->GetMacAddress(),
                          SatEnums::LL_MAC,
                          SatEnums::LD_FORWARD,
                          SatUtils::GetPacketInfo(bbFrame->GetPayload()));

            SatSignalParameters::txInfo_s txInfo;
            txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
            txInfo.modCod = bbFrame->GetModcod();
            txInfo.sliceId = bbFrame->GetSliceId();
            txInfo.frameType = bbFrame->GetFrameType();
            txInfo.waveformId = 0;

            /**
             * Decrease a guard time from BB frame duration.
             */
            SendPacket(bbFrame->GetPayload(), carrierId, txDuration - m_guardTime, txInfo);
        }
    }
    else
    {
        /**
         * GW MAC is disabled, thus get the duration of the default BB frame
         * and try again then.
         */

        NS_LOG_INFO("TX is disabled, thus nothing is transmitted!");
        txDuration = m_fwdScheduler->GetDefaultFrameDuration();
    }

    if (m_periodicTransmissionEnabled)
    {
        /**
         * It is currently assumed that there is only one carrier in FWD link. This
         * carrier has a default index of 0.
         * TODO: When enabling multi-carrier support for FWD link, we need to
         * modify the FWD link scheduler to schedule separately each FWD link
         * carrier.
         */
        Simulator::Schedule(txDuration, &SatGwMac::StartTransmission, this, 0);
    }
}

void
SatGwMac::TbtpSent(Ptr<SatTbtpMessage> tbtp)
{
    NS_LOG_FUNCTION(this << tbtp);

    uint32_t superframeCounter = tbtp->GetSuperframeCounter();

    if (m_tbtps.find(superframeCounter) == m_tbtps.end())
    {
        m_tbtps[superframeCounter] = std::vector<Ptr<SatTbtpMessage>>();
    }
    m_tbtps[superframeCounter].push_back(tbtp);

    Simulator::Schedule(Seconds(10), &SatGwMac::RemoveTbtp, this, superframeCounter);
}

uint32_t
SatGwMac::GetFeederSatId()
{
    NS_LOG_FUNCTION(this);

    return m_feederSatId;
}

uint32_t
SatGwMac::GetFeederBeamId()
{
    NS_LOG_FUNCTION(this);

    return m_feederBeamId;
}

void
SatGwMac::RemoveTbtp(uint32_t superframeCounter)
{
    m_tbtps.erase(superframeCounter);
}

void
SatGwMac::StartNcrTransmission()
{
    NS_LOG_FUNCTION(this);

    SendNcrMessage();

    if (m_periodicTransmissionEnabled)
    {
        Simulator::Schedule(m_ncrInterval, &SatGwMac::StartNcrTransmission, this);
    }
}

void
SatGwMac::ReceiveSignalingPacket(Ptr<Packet> packet, uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << packet << beamId);

    // Remove the mac tag
    SatMacTag macTag;
    packet->PeekPacketTag(macTag);

    SatAddressE2ETag addressE2ETag;
    packet->PeekPacketTag(addressE2ETag);

    // Peek control msg tag
    SatControlMsgTag ctrlTag;
    bool cSuccess = packet->PeekPacketTag(ctrlTag);

    if (!cSuccess)
    {
        NS_FATAL_ERROR("SatControlMsgTag not found in the packet!");
    }

    switch (ctrlTag.GetMsgType())
    {
    case SatControlMsgTag::SAT_CR_CTRL_MSG: {
        uint32_t msgId = ctrlTag.GetMsgId();
        Ptr<SatCrMessage> crMsg = DynamicCast<SatCrMessage>(m_readCtrlCallback(msgId));

        if (crMsg)
        {
            Mac48Address sourceAddress;
            switch (m_forwardLinkRegenerationMode)
            {
            case SatEnums::TRANSPARENT:
            case SatEnums::REGENERATION_PHY: {
                sourceAddress = addressE2ETag.GetE2ESourceAddress();
                break;
            }
            case SatEnums::REGENERATION_NETWORK: {
                sourceAddress = Mac48Address::ConvertFrom(m_satelliteAddress);
                break;
            }
            default:
                NS_FATAL_ERROR("Unknown regeneration mode");
            }
            m_fwdScheduler->CnoInfoUpdated(sourceAddress, crMsg->GetCnoEstimate());

            if (m_crReceiveCallback.IsNull() == false)
            {
                m_crReceiveCallback(satId, beamId, addressE2ETag.GetE2ESourceAddress(), crMsg);
            }
        }
        else
        {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType()
                << " is not found from the RTN link control msg container!";
            msg << " at: " << Now().GetSeconds() << "s";
            Singleton<SatLog>::Get()->AddToLog(SatLog::LOG_WARNING, "", msg.str());
        }

        packet->RemovePacketTag(macTag);
        packet->RemovePacketTag(addressE2ETag);
        packet->RemovePacketTag(ctrlTag);

        break;
    }
    case SatControlMsgTag::SAT_CN0_REPORT: {
        uint32_t msgId = ctrlTag.GetMsgId();
        Ptr<SatCnoReportMessage> cnoReport =
            DynamicCast<SatCnoReportMessage>(m_readCtrlCallback(msgId));

        if (cnoReport)
        {
            m_fwdScheduler->CnoInfoUpdated(addressE2ETag.GetE2ESourceAddress(),
                                           cnoReport->GetCnoEstimate());
        }
        else
        {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType()
                << " is not found from the RTN link control msg container!";
            msg << " at: " << Now().GetSeconds() << "s";
            Singleton<SatLog>::Get()->AddToLog(SatLog::LOG_WARNING, "", msg.str());
        }

        packet->RemovePacketTag(macTag);
        packet->RemovePacketTag(addressE2ETag);
        packet->RemovePacketTag(ctrlTag);

        break;
    }
    case SatControlMsgTag::SAT_ARQ_ACK: {
        // ARQ ACK messages are forwarded to LLC, since they may be fragmented
        m_rxCallback(packet, addressE2ETag.GetE2ESourceAddress(), macTag.GetDestAddress());
        break;
    }
    case SatControlMsgTag::SAT_HR_CTRL_MSG: {
        uint32_t msgId = ctrlTag.GetMsgId();
        Ptr<SatHandoverRecommendationMessage> handoverRecommendation =
            DynamicCast<SatHandoverRecommendationMessage>(m_readCtrlCallback(msgId));

        if (handoverRecommendation)
        {
            uint32_t newSatId = handoverRecommendation->GetRecommendedSatId();
            uint32_t newBeamId = handoverRecommendation->GetRecommendedBeamId();
            m_handoverCallback(addressE2ETag.GetE2ESourceAddress(),
                               satId,
                               beamId,
                               newSatId,
                               newBeamId);
        }
        else
        {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType()
                << " is not found from the RTN link control msg container!";
            msg << " at: " << Now().GetSeconds() << "s";
            Singleton<SatLog>::Get()->AddToLog(SatLog::LOG_WARNING, "", msg.str());
        }

        break;
    }
    case SatControlMsgTag::SAT_LOGON_CTRL_MSG: {
        uint32_t msgId = ctrlTag.GetMsgId();
        Ptr<SatLogonMessage> logonMessage = DynamicCast<SatLogonMessage>(m_readCtrlCallback(msgId));

        if (logonMessage)
        {
            Address utId = addressE2ETag.GetE2ESourceAddress();
            Callback<void, uint32_t> raChannelCallback =
                MakeBoundCallback(&SatGwMac::SendLogonResponseHelper, this, utId);
            m_logonCallback(utId, satId, beamId, raChannelCallback);
        }
        else
        {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInRtnLink attribute may be set to too short value
             * or there are something wrong in the RTN link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType()
                << " is not found from the RTN link control msg container!";
            msg << " at: " << Now().GetSeconds() << "s";
            Singleton<SatLog>::Get()->AddToLog(SatLog::LOG_WARNING, "", msg.str());
        }
        break;
    }
    case SatControlMsgTag::SAT_CMT_CTRL_MSG: {
        // We do nothing, from SAT to GW, these messages only add load on link
        break;
    }
    default: {
        NS_FATAL_ERROR("SatGwMac received a non-supported control packet!");
        break;
    }
    }
}

void
SatGwMac::SendNcrMessage()
{
    NS_LOG_FUNCTION(this);

    Ptr<SatNcrMessage> ncrMessage = CreateObject<SatNcrMessage>();
    m_fwdScheduler->SendControlMsg(ncrMessage, Mac48Address::GetBroadcast());
    m_ncrMessagesToSend.push(ncrMessage);
}

void
SatGwMac::SendCmtMessage(Address utId,
                         Time burstDuration,
                         Time satelliteReceptionTime,
                         uint32_t satId,
                         uint32_t beamId)
{
    NS_LOG_FUNCTION(this << utId);

    Time lastCmtSent = Seconds(0);
    if (m_lastCmtSent.find(utId) != m_lastCmtSent.end())
    {
        lastCmtSent = m_lastCmtSent[utId];
    }

    Time timeReceived = satelliteReceptionTime;
    if (satelliteReceptionTime == Seconds(0))
    {
        timeReceived = Simulator::Now();
    }

    if (timeReceived < lastCmtSent + m_cmtPeriodMin)
    {
        return;
    }

    uint32_t indexClosest = 0;
    uint32_t tbtpIndexClosest = 0;
    uint32_t timeSlotIndexClosest = 0;
    Time differenceClosest = Seconds(1000000);
    std::vector<Ptr<SatTbtpMessage>> tbtpsForCurrentSF;
    Ptr<SatTbtpMessage> tbtp;
    for (uint32_t i = 0; i < m_tbtps.size(); i++)
    {
        tbtpsForCurrentSF = m_tbtps[i];
        for (uint32_t tbtpIndex = 0; tbtpIndex < tbtpsForCurrentSF.size(); tbtpIndex++)
        {
            tbtp = tbtpsForCurrentSF[tbtpIndex];
            std::pair<uint8_t, std::vector<Ptr<SatTimeSlotConf>>> timeslots =
                tbtp->GetDaTimeslots(utId);
            for (uint32_t j = 0; j < timeslots.second.size(); j++)
            {
                Ptr<SatTimeSlotConf> tsConf = timeslots.second[j];
                if (tsConf->GetSlotType() == SatTimeSlotConf::SLOT_TYPE_C)
                {
                    Time frameStartTime = Singleton<SatRtnLinkTime>::Get()->GetSuperFrameTxTime(
                        SatConstVariables::SUPERFRAME_SEQUENCE,
                        i,
                        Seconds(0));
                    Time slotStartTime = tsConf->GetStartTime();
                    Time difference = timeReceived - frameStartTime - slotStartTime - burstDuration;
                    if (Abs(difference) < differenceClosest)
                    {
                        differenceClosest = Abs(difference);
                        indexClosest = i;
                        timeSlotIndexClosest = j;
                        tbtpIndexClosest = tbtpIndex;
                    }
                }
            }
        }
    }

    if (indexClosest == 0)
    {
        return;
    }

    tbtp = m_tbtps[indexClosest][tbtpIndexClosest];
    std::pair<uint8_t, std::vector<Ptr<SatTimeSlotConf>>> timeslots = tbtp->GetDaTimeslots(utId);

    if (timeslots.second[timeSlotIndexClosest]->GetSlotType() == 0)
    {
        Time frameStartTime = Singleton<SatRtnLinkTime>::Get()->GetSuperFrameTxTime(
            SatConstVariables::SUPERFRAME_SEQUENCE,
            indexClosest,
            Seconds(0));
        Time slotStartTime = timeslots.second[timeSlotIndexClosest]->GetStartTime();

        Time difference = frameStartTime + slotStartTime + burstDuration - timeReceived;
        int32_t differenceNcr = difference.GetMicroSeconds() * 27;

        if (differenceNcr > 16256 || differenceNcr < -16256)
        {
            NS_LOG_INFO("Burst Time Correction outside bounds, should be at least -16256 and at "
                        "most 16256, but got "
                        << differenceNcr << ". Forcing logoff of UT " << utId);
            Ptr<SatLogoffMessage> logoffMsg = CreateObject<SatLogoffMessage>();
            m_fwdScheduler->SendControlMsg(logoffMsg, utId);
            m_removeUtCallback(utId, satId, beamId);
        }
        else
        {
            Ptr<SatCmtMessage> cmt = CreateObject<SatCmtMessage>();
            cmt->SetBurstTimeCorrection(differenceNcr);
            m_fwdScheduler->SendControlMsg(cmt, utId);
            m_lastCmtSent[utId] = Simulator::Now();
        }

        return;
    }
}

void
SatGwMac::SendLogonResponse(Address utId, uint32_t raChannel)
{
    NS_LOG_FUNCTION(this << utId << raChannel);
    Ptr<SatLogonResponseMessage> logonResponse = CreateObject<SatLogonResponseMessage>();
    logonResponse->SetRaChannel(raChannel);
    m_fwdScheduler->SendControlMsg(logonResponse, utId);
}

void
SatGwMac::SendLogonResponseHelper(SatGwMac* self, Address utId, uint32_t raChannel)
{
    self->SendLogonResponse(utId, raChannel);
}

void
SatGwMac::SetCrReceiveCallback(SatGwMac::CrReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_crReceiveCallback = cb;
}

void
SatGwMac::SetHandoverCallback(SatGwMac::HandoverCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_handoverCallback = cb;
}

void
SatGwMac::SetLogonCallback(SatGwMac::LogonCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_logonCallback = cb;
}

void
SatGwMac::SetControlMessageReceivedCallback(SatGwMac::ControlMessageReceivedCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_controlMessageReceivedCallback = cb;
}

void
SatGwMac::SetRemoveUtCallback(SatGwMac::RemoveUtCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_removeUtCallback = cb;
}

void
SatGwMac::SetClearQueuesCallback(SatGwMac::ClearQueuesCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_clearQueuesCallback = cb;
}

void
SatGwMac::SetBeamCallback(SatGwMac::PhyBeamCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_beamCallback = cb;
}

void
SatGwMac::SetGeoNodesCallback(SatGwMac::GeoNodesCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_geoNodesCallback = cb;
}

void
SatGwMac::SetGwLlcSetSatelliteAddress(SatGwMac::GwLlcSetSatelliteAddress cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_gwLlcSetSatelliteAddress = cb;
}

void
SatGwMac::SetFwdScheduler(Ptr<SatFwdLinkScheduler> fwdScheduler)
{
    m_fwdScheduler = fwdScheduler;

    if (m_ncrV2)
    {
        m_fwdScheduler->SetDummyFrameSendingEnabled(true);
    }
}

void
SatGwMac::ChangeBeam(uint32_t satId, uint32_t beamId)
{
    NS_LOG_FUNCTION(this << satId << beamId);
}

void
SatGwMac::ConnectUt(Mac48Address utAddress)
{
    NS_LOG_FUNCTION(this << utAddress);

    NS_ASSERT(m_peers.find(utAddress) == m_peers.end());

    if (m_disableSchedulingIfNoDeviceConnected && !HasPeer())
    {
        NS_LOG_INFO("Start beam " << m_beamId);
        m_peers.insert(utAddress);
        StartPeriodicTransmissions();
    }
    else
    {
        m_peers.insert(utAddress);
    }
}

void
SatGwMac::DisconnectUt(Mac48Address utAddress)
{
    NS_LOG_FUNCTION(this << utAddress);

    NS_ASSERT(m_peers.find(utAddress) != m_peers.end());

    m_peers.erase(utAddress);

    if (m_disableSchedulingIfNoDeviceConnected && !HasPeer())
    {
        NS_LOG_INFO("Stop beam " << m_beamId);
        StopPeriodicTransmissions();
    }
}

void
SatGwMac::StopPeriodicTransmissions()
{
    NS_LOG_FUNCTION(this);

    m_periodicTransmissionEnabled = false;
}

bool
SatGwMac::HasPeer()
{
    NS_LOG_FUNCTION(this);

    return !m_peers.empty();
}

} // namespace ns3
