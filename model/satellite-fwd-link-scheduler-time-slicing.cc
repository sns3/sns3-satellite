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
    SatFwdLinkScheduler (conf, address, carrierBandwidthInHz)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  std::vector<SatEnums::SatModcod_t> modCods;
  SatEnums::GetAvailableModcodsFwdLink (modCods);

  /* TODO compute number of slices depending on decoding capacity
  uint32_t maxModulatedBits = SatUtils::GetModulatedBits (modCods.back ());
  m_numberOfSlices = 1;*/

  for(uint8_t i = 0; i < m_numberOfSlices; i++)
    {
      m_bbFrameContainers.insert (std::pair<uint8_t, Ptr<SatBbFrameContainer>> (i, CreateObject<SatBbFrameContainer> (modCods, m_bbFrameConf)));
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

  if ( m_bbFrameContainers.at (0)->GetTotalDuration () < m_schedulingStartThresholdTime )
    {
      ScheduleBbFrames ();
    }

  Ptr<SatBbFrame> frame = m_bbFrameContainers.at (0)->GetNextFrame ();

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
        ( it != so.end () ) && ( m_bbFrameContainers.at (0)->GetTotalDuration () < m_schedulingStopThresholdTime ); it++ )
    {
      uint32_t currentObBytes = (*it)->GetBufferedBytes ();
      uint32_t currentObMinReqBytes = (*it)->GetMinTxOpportunityInBytes ();
      uint8_t flowId = (*it)->GetFlowId ();
      SatEnums::SatModcod_t modcod = m_bbFrameContainers.at (0)->GetModcod ( flowId, GetSchedulingObjectCno (*it));

      uint32_t frameBytes = m_bbFrameContainers.at (0)->GetBytesLeftInTailFrame (flowId, modcod);

      while ( ( (m_bbFrameContainers.at (0)->GetTotalDuration () < m_schedulingStopThresholdTime ))
              && (currentObBytes > 0) )
        {
          if ( frameBytes < currentObMinReqBytes)
            {
              frameBytes = m_bbFrameContainers.at (0)->GetMaxFramePayloadInBytes (flowId, modcod);

              // if frame bytes still too small, we must have too long control message, so let's crash
              if ( frameBytes < currentObMinReqBytes )
                {
                  NS_FATAL_ERROR ("Control package too probably too long!!!");
                }
            }

          Ptr<Packet> p = m_txOpportunityCallback (frameBytes, (*it)->GetMacAddress (), flowId, currentObBytes, currentObMinReqBytes);

          if ( p )
            {
              m_bbFrameContainers.at (0)->AddData (flowId, modcod, p);
              frameBytes = m_bbFrameContainers.at (0)->GetBytesLeftInTailFrame (flowId, modcod);
            }
          else if ( m_bbFrameContainers.at (0)->GetMaxFramePayloadInBytes (flowId, modcod ) != m_bbFrameContainers.at (0)->GetBytesLeftInTailFrame (flowId, modcod))
            {
              frameBytes = m_bbFrameContainers.at (0)->GetMaxFramePayloadInBytes (flowId, modcod);
            }
          else
            {
              NS_FATAL_ERROR ("Packet does not fit in empty BB Frame. Control package too long or fragmentation problem in user package!!!");
            }
        }

      m_bbFrameContainers.at (0)->MergeBbFrames (m_carrierBandwidthInHz);
    }
}

void
SatFwdLinkSchedulerTimeSlicing::GetSchedulingObjects (std::vector< Ptr<SatSchedulingObject> > & output)
{
  NS_LOG_FUNCTION (this);

  if ( m_bbFrameContainers.at (0)->GetTotalDuration () < m_schedulingStopThresholdTime )
    {
      // Get scheduling objects from LLC
      m_schedContextCallback (output);

      SortSchedulingObjects (output);
    }
}

} // namespace ns3
