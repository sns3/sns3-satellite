/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#include "satellite-scpc-scheduler.h"

#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("SatScpcScheduler");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatScpcScheduler);

TypeId
SatScpcScheduler::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatScpcScheduler")
            .SetParent<SatFwdLinkScheduler>()
            .AddConstructor<SatScpcScheduler>()
            .AddAttribute("SchedulingStartThresholdTime",
                          "Threshold time of total transmissions in BB Frame container to trigger "
                          "a scheduling round.",
                          TimeValue(MilliSeconds(5)),
                          MakeTimeAccessor(&SatScpcScheduler::m_schedulingStartThresholdTime),
                          MakeTimeChecker())
            .AddAttribute("SchedulingStopThresholdTime",
                          "Threshold time of total transmissions in BB Frame container to stop a "
                          "scheduling round.",
                          TimeValue(MilliSeconds(15)),
                          MakeTimeAccessor(&SatScpcScheduler::m_schedulingStopThresholdTime),
                          MakeTimeChecker())
            .AddAttribute("BBFrameContainer",
                          "BB frame container of this scheduler.",
                          PointerValue(),
                          MakePointerAccessor(&SatScpcScheduler::m_bbFrameContainer),
                          MakePointerChecker<SatBbFrameContainer>());
    return tid;
}

TypeId
SatScpcScheduler::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatScpcScheduler::SatScpcScheduler()
    : SatFwdLinkScheduler()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("Default constructor for SatScpcScheduler not supported");
}

SatScpcScheduler::SatScpcScheduler(Ptr<SatBbFrameConf> conf,
                                   Mac48Address address,
                                   double carrierBandwidthInHz)
    : SatFwdLinkScheduler(conf, address, carrierBandwidthInHz),
      m_symbolsSent(0)
{
    NS_LOG_FUNCTION(this);

    ObjectBase::ConstructSelf(AttributeConstructionList());

    std::vector<SatEnums::SatModcod_t> modCods = conf->GetModCodsUsed();

    m_bbFrameContainer = CreateObject<SatBbFrameContainer>(modCods, m_bbFrameConf);

    Simulator::Schedule(m_periodicInterval, &SatScpcScheduler::PeriodicTimerExpired, this);
}

SatScpcScheduler::~SatScpcScheduler()
{
    NS_LOG_FUNCTION(this);
}

void
SatScpcScheduler::DoDispose()
{
    NS_LOG_FUNCTION(this);
    SatFwdLinkScheduler::DoDispose();
    m_bbFrameContainer = NULL;
}

std::pair<Ptr<SatBbFrame>, const Time>
SatScpcScheduler::GetNextFrame()
{
    NS_LOG_FUNCTION(this);

    if (m_bbFrameContainer->GetTotalDuration() < m_schedulingStartThresholdTime)
    {
        ScheduleBbFrames();
    }

    Ptr<SatBbFrame> frame = m_bbFrameContainer->GetNextFrame();
    Time frameDuration;

    if (frame)
    {
        m_symbolsSent += ceil(frame->GetDuration().GetSeconds() * m_carrierBandwidthInHz);
    }

    // create dummy frame
    if (m_dummyFrameSendingEnabled && !frame)
    {
        frame = Create<SatBbFrame>(m_bbFrameConf->GetDefaultModCod(),
                                   SatEnums::DUMMY_FRAME,
                                   m_bbFrameConf);

        // create dummy packet
        Ptr<Packet> dummyPacket = Create<Packet>(1);

        // Add MAC tag
        SatMacTag mTag;
        mTag.SetDestAddress(Mac48Address::GetBroadcast());
        mTag.SetSourceAddress(m_macAddress);
        dummyPacket->AddPacketTag(mTag);

        // Add E2E address tag
        SatAddressE2ETag addressE2ETag;
        addressE2ETag.SetE2EDestAddress(Mac48Address::GetBroadcast());
        addressE2ETag.SetE2ESourceAddress(m_macAddress);
        dummyPacket->AddPacketTag(addressE2ETag);

        // Add dummy packet to dummy frame
        frame->AddPayload(dummyPacket);

        frameDuration = frame->GetDuration();
    }
    // If no bb frame available and dummy frames disabled
    else if (!frame)
    {
        frameDuration = m_bbFrameConf->GetDummyBbFrameDuration();
    }

    if (frame)
    {
        frameDuration = frame->GetDuration();
        frame->SetSliceId(0);
    }

    return std::make_pair(frame, frameDuration);
}

void
SatScpcScheduler::PeriodicTimerExpired()
{
    NS_LOG_FUNCTION(this);

    SendAndClearSymbolsSentStat();
    ScheduleBbFrames();

    Simulator::Schedule(m_periodicInterval, &SatScpcScheduler::PeriodicTimerExpired, this);
}

void
SatScpcScheduler::SendAndClearSymbolsSentStat()
{
    NS_LOG_FUNCTION(this);

    m_schedulingSymbolRateTrace(0, m_symbolsSent / Seconds(1).GetSeconds());

    m_symbolsSent = 0;
}

void
SatScpcScheduler::ScheduleBbFrames()
{
    NS_LOG_FUNCTION(this);

    // Get scheduling objects from LLC
    std::vector<Ptr<SatSchedulingObject>> so;
    GetSchedulingObjects(so);

    for (std::vector<Ptr<SatSchedulingObject>>::const_iterator it = so.begin();
         (it != so.end()) &&
         (m_bbFrameContainer->GetTotalDuration() < m_schedulingStopThresholdTime);
         it++)
    {
        uint32_t currentObBytes = (*it)->GetBufferedBytes();
        uint32_t currentObMinReqBytes = (*it)->GetMinTxOpportunityInBytes();
        uint8_t flowId = (*it)->GetFlowId();
        SatEnums::SatModcod_t modcod =
            m_bbFrameContainer->GetModcod(flowId, GetSchedulingObjectCno(*it));

        uint32_t frameBytes = m_bbFrameContainer->GetBytesLeftInTailFrame(flowId, modcod);

        while (((m_bbFrameContainer->GetTotalDuration() < m_schedulingStopThresholdTime)) &&
               (currentObBytes > 0))
        {
            if (frameBytes < currentObMinReqBytes)
            {
                frameBytes = m_bbFrameContainer->GetMaxFramePayloadInBytes(flowId, modcod) -
                             m_bbFrameConf->GetBbFrameHeaderSizeInBytes();

                // if frame bytes still too small, we must have too long control message, so let's
                // crash
                if (frameBytes < currentObMinReqBytes)
                {
                    NS_FATAL_ERROR("Control package probably too long!!!");
                }
            }

            Ptr<Packet> p = m_txOpportunityCallback(frameBytes,
                                                    (*it)->GetMacAddress(),
                                                    flowId,
                                                    currentObBytes,
                                                    currentObMinReqBytes);

            if (p)
            {
                m_bbFrameContainer->AddData(flowId, modcod, p);
                frameBytes = m_bbFrameContainer->GetBytesLeftInTailFrame(flowId, modcod);
            }
            else if (m_bbFrameContainer->GetMaxFramePayloadInBytes(flowId, modcod) !=
                     m_bbFrameContainer->GetBytesLeftInTailFrame(flowId, modcod))
            {
                frameBytes = m_bbFrameContainer->GetMaxFramePayloadInBytes(flowId, modcod);
            }
            else
            {
                NS_FATAL_ERROR("Packet does not fit in empty BB Frame. Control package too long or "
                               "fragmentation problem in user package!!!");
            }
        }

        m_bbFrameContainer->MergeBbFrames(m_carrierBandwidthInHz);
    }
}

void
SatScpcScheduler::GetSchedulingObjects(std::vector<Ptr<SatSchedulingObject>>& output)
{
    NS_LOG_FUNCTION(this);

    if (m_bbFrameContainer->GetTotalDuration() < m_schedulingStopThresholdTime)
    {
        // Get scheduling objects from LLC
        m_schedContextCallback(output);

        SortSchedulingObjects(output);
    }
}

} // namespace ns3
