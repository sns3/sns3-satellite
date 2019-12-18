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

#include "satellite-fwd-link-scheduler-time-slicing.h"

#include "satellite-utils.h"


NS_LOG_COMPONENT_DEFINE ("SatFwdLinkSchedulerTimeSlicing");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFwdLinkSchedulerTimeSlicing);

TypeId
SatFwdLinkSchedulerTimeSlicing::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFwdLinkSchedulerTimeSlicing")
    .SetParent<SatFwdLinkScheduler> ()
    .AddConstructor<SatFwdLinkSchedulerTimeSlicing> ()
    .AddAttribute ( "NumberOfSlices",
                    "Number of slices used.",
                    UintegerValue (1),
                    MakeUintegerAccessor (&SatFwdLinkSchedulerTimeSlicing::m_numberOfSlices),
                    MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
SatFwdLinkSchedulerTimeSlicing::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SatFwdLinkSchedulerTimeSlicing::SatFwdLinkSchedulerTimeSlicing () : SatFwdLinkScheduler ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor for SatFwdLinkSchedulerTimeSlicing not supported");
}

SatFwdLinkSchedulerTimeSlicing::SatFwdLinkSchedulerTimeSlicing (Ptr<SatBbFrameConf> conf, Mac48Address address, double carrierBandwidthInHz) :
    SatFwdLinkScheduler (conf, address, carrierBandwidthInHz),
    m_lastSliceAssigned (1),
    m_lastSliceDequeued (1)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  std::vector<SatEnums::SatModcod_t> modCods;
  SatEnums::GetAvailableModcodsFwdLink (modCods);

  /* TODO compute number of slices depending on decoding capacity
  uint32_t maxModulatedBits = SatUtils::GetModulatedBits (modCods.back ());
  m_numberOfSlices = 1;*/

  m_bbFrameCtrlContainer = CreateObject<SatBbFrameContainer> (modCods, m_bbFrameConf);

  for(uint8_t i = 0; i < m_numberOfSlices; i++)
    {
      m_bbFrameContainers.insert (std::pair<uint8_t, Ptr<SatBbFrameContainer>> (i+1, CreateObject<SatBbFrameContainer> (modCods, m_bbFrameConf)));
    }

  Simulator::Schedule (m_periodicInterval, &SatFwdLinkSchedulerTimeSlicing::PeriodicTimerExpired, this);
}

SatFwdLinkSchedulerTimeSlicing::~SatFwdLinkSchedulerTimeSlicing ()
{
  NS_LOG_FUNCTION (this);
}

void
SatFwdLinkSchedulerTimeSlicing::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  SatFwdLinkScheduler::DoDispose ();
  m_bbFrameContainers.clear ();
}


Ptr<SatBbFrame>
SatFwdLinkSchedulerTimeSlicing::GetNextFrame ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SatBbFrame> frame;

  // Send slice control messages first if there is any.
  if (m_bbFrameCtrlContainer->GetTotalDuration () > 0)
    {
      std::cout << "Send control message" << std::endl;
      frame = m_bbFrameCtrlContainer->GetNextFrame ();
    }
  else
    {
      if ( GetTotalDuration () < m_schedulingStartThresholdTime )
        {
          ScheduleBbFrames ();
        }

      uint8_t lastDeque = m_lastSliceDequeued;
      do
        {
          frame = m_bbFrameContainers.at (m_lastSliceDequeued)->GetNextFrame ();
          if (m_lastSliceDequeued == m_numberOfSlices)
            {
              m_lastSliceDequeued = 0;
            }
          m_lastSliceDequeued++;
        }
      while (frame == NULL && m_lastSliceDequeued != lastDeque);
    }

  // create dummy frame
  if ( frame == NULL )
    {
      frame = Create<SatBbFrame> (m_bbFrameConf->GetDefaultModCod (), SatEnums::DUMMY_FRAME, m_bbFrameConf);

      // create dummy packet
      Ptr<Packet> dummyPacket = Create<Packet> (1);

      // Add MAC tag
      SatMacTag tag;
      tag.SetDestAddress (Mac48Address::GetBroadcast ());
      tag.SetSourceAddress (m_macAddress);
      dummyPacket->AddPacketTag (tag);

      // Add dummy packet to dummy frame
      frame->AddPayload (dummyPacket);
    }

  return frame;
}

void
SatFwdLinkSchedulerTimeSlicing::SetSendControlMsgCallback (SatFwdLinkSchedulerTimeSlicing::SendControlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_sendControlMsgCallback = cb;
}

void
SatFwdLinkSchedulerTimeSlicing::PeriodicTimerExpired ()
{
  NS_LOG_FUNCTION (this);

  ScheduleBbFrames ();

  Simulator::Schedule (m_periodicInterval, &SatFwdLinkSchedulerTimeSlicing::PeriodicTimerExpired, this);
}

void
SatFwdLinkSchedulerTimeSlicing::ScheduleBbFrames ()
{
  NS_LOG_FUNCTION (this);

  // Get scheduling objects from LLC
  std::vector< Ptr<SatSchedulingObject> > so;
  GetSchedulingObjects (so);

  for ( std::vector< Ptr<SatSchedulingObject> >::const_iterator it = so.begin ();
        ( it != so.end () ) && ( GetTotalDuration () < m_schedulingStopThresholdTime ); it++ )
    {
      uint32_t currentObBytes = (*it)->GetBufferedBytes ();
      uint32_t currentObMinReqBytes = (*it)->GetMinTxOpportunityInBytes ();
      uint8_t flowId = (*it)->GetFlowId ();
      Mac48Address address = (*it)->GetMacAddress ();

      if ( m_slicesMapping.find(address) == m_slicesMapping.end() )
        {
          std::cout << "First occurence of MAC address " << address;

          if (address == Mac48Address::GetBroadcast ())
            {
              // TODO handle address ff:ff:ff:ff:ff:ff
              // ask enverybody to subscribe
              // TODO make slice 0 broadcast ? or copy the message to all slices ?
            }
            m_slicesMapping.insert (std::pair<Mac48Address, uint8_t> (address, m_lastSliceAssigned));
            if (m_lastSliceAssigned == m_numberOfSlices)
              {
                m_lastSliceAssigned = 0;
              }
            m_lastSliceAssigned++;
            std::cout << ". Got slice " << (uint32_t) m_slicesMapping.at (address) << std::endl;

            SendTimeSliceSubscription (address, std::vector<uint8_t> {m_slicesMapping.at (address)});
        }
      uint8_t slice = m_slicesMapping.at (address);
      SatEnums::SatModcod_t modcod = m_bbFrameContainers.at (slice)->GetModcod ( flowId, GetSchedulingObjectCno (*it));

      uint32_t frameBytes = m_bbFrameContainers.at (slice)->GetBytesLeftInTailFrame (flowId, modcod);

      while ( (GetTotalDuration () < m_schedulingStopThresholdTime) && (currentObBytes > 0) )
        {
          if ( frameBytes < currentObMinReqBytes)
            {
              frameBytes = m_bbFrameContainers.at (slice)->GetMaxFramePayloadInBytes (flowId, modcod);

              // if frame bytes still too small, we must have too long control message, so let's crash
              if ( frameBytes < currentObMinReqBytes )
                {
                  NS_FATAL_ERROR ("Control package probably too long!!!");
                }
            }

          Ptr<Packet> p = m_txOpportunityCallback (frameBytes, address, flowId, currentObBytes, currentObMinReqBytes);

          if ( p )
            {
              m_bbFrameContainers.at (slice)->AddData (flowId, modcod, p);
              frameBytes = m_bbFrameContainers.at (slice)->GetBytesLeftInTailFrame (flowId, modcod);
            }
          else if ( m_bbFrameContainers.at (slice)->GetMaxFramePayloadInBytes (flowId, modcod ) != m_bbFrameContainers.at (slice)->GetBytesLeftInTailFrame (flowId, modcod))
            {
              frameBytes = m_bbFrameContainers.at (slice)->GetMaxFramePayloadInBytes (flowId, modcod);
            }
          else
            {
              NS_FATAL_ERROR ("Packet does not fit in empty BB Frame. Control package too long or fragmentation problem in user package!!!");
            }
        }

      m_bbFrameContainers.at (slice)->MergeBbFrames (m_carrierBandwidthInHz);
    }
}

void
SatFwdLinkSchedulerTimeSlicing::GetSchedulingObjects (std::vector< Ptr<SatSchedulingObject> > & output)
{
  NS_LOG_FUNCTION (this);

  if ( GetTotalDuration () < m_schedulingStopThresholdTime )
    {
      // Get scheduling objects from LLC
      m_schedContextCallback (output);

      SortSchedulingObjects (output);
    }
}

Time
SatFwdLinkSchedulerTimeSlicing::GetTotalDuration ()
{
  NS_LOG_FUNCTION (this);

  Time duration = Time (0);
  for (std::map<uint8_t, Ptr<SatBbFrameContainer>>::iterator it = m_bbFrameContainers.begin(); it != m_bbFrameContainers.end(); it++ )
    {
        duration += it->second->GetTotalDuration ();
    }

  return duration;
}

void
SatFwdLinkSchedulerTimeSlicing::SendTimeSliceSubscription (Mac48Address address, std::vector<uint8_t> slices)
{
  NS_LOG_FUNCTION (this);

  std::cout << "Create control message to " << address << std::endl;

  for(std::vector<uint8_t>::iterator it = slices.begin(); it != slices.end(); ++it)
    {
      Ptr<SatSliceSubscriptionMessage> sliceSubscription = CreateObject<SatSliceSubscriptionMessage> ();
      sliceSubscription->SetSliceId(*it);

      m_sendControlMsgCallback (sliceSubscription, address);
    }
}

} // namespace ns3
