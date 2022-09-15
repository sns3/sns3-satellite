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
                    MakeUintegerChecker<uint8_t> (1,255))
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

  std::vector<SatEnums::SatModcod_t> modCods = conf->GetModCodsUsed ();

  // Create control and broadcast container
  m_bbFrameContainers.insert (std::pair<uint8_t, Ptr<SatBbFrameContainer>> (0, CreateObject<SatBbFrameContainer> (modCods, m_bbFrameConf)));
  m_bbFrameContainers.at (0)->SetMaxSymbolRate (m_carrierBandwidthInHz);

  // Initialize containers
  for(uint8_t i = 0; i < m_numberOfSlices; i++)
    {
      Ptr <SatBbFrameContainer> container = CreateObject<SatBbFrameContainer> (modCods, m_bbFrameConf);
      container->SetMaxSymbolRate (m_carrierBandwidthInHz / m_numberOfSlices);
      m_bbFrameContainers.insert (std::pair<uint8_t, Ptr<SatBbFrameContainer>> (i+1, container));
    }

  // Initialize number of symbols sent per slice
  for(uint8_t i = 0; i <= m_numberOfSlices; i++)
    {
      m_symbolsSent.insert (std::pair<uint8_t, uint32_t> (i, 0));
    }

  // Check if all symbol rates are high enough
  for (std::map<uint8_t, Ptr <SatBbFrameContainer>>::iterator it = m_bbFrameContainers.begin(); it != m_bbFrameContainers.end(); it++ )
    {
      uint32_t maxSymbolPerCycle = it->second->GetMaxSymbolRate ()*m_periodicInterval.GetSeconds ();
      uint32_t symbolsMostRobustModcod;
      if (m_bbFrameConf->GetMostRobustModcod (SatEnums::NORMAL_FRAME) != SatEnums::SAT_NONVALID_MODCOD)
        {
          symbolsMostRobustModcod = it->second->GetFrameSymbols(m_bbFrameConf->GetMostRobustModcod (SatEnums::NORMAL_FRAME));
        }
      else
        {
          //We are using only short Frames, as new ModCod exists for normal Frames.
          symbolsMostRobustModcod = it->second->GetFrameSymbols(m_bbFrameConf->GetMostRobustModcod (SatEnums::SHORT_FRAME));
        }
      if (symbolsMostRobustModcod > maxSymbolPerCycle)
        {
          NS_FATAL_ERROR ("Symbol rate of slice " + std::to_string(it->first) + " (" + std::to_string(it->second->GetMaxSymbolRate ())
              + " Baud) is too low to allow at least one BBFrame of the most robust ModCod. Must be at least "
              + std::to_string((uint32_t) (symbolsMostRobustModcod / m_periodicInterval.GetSeconds ())) + " Baud");
        }
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


std::pair<Ptr<SatBbFrame>, const Time>
SatFwdLinkSchedulerTimeSlicing::GetNextFrame ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SatBbFrame> frame;
  Time frameDuration;

  // Send slice control messages first if there is any.
  if (!m_bbFrameContainers.at (0)->IsEmpty (0, m_bbFrameConf->GetDefaultModCod ()))
    {
      frame = m_bbFrameContainers.at (0)->GetNextFrame ();
      if (frame != NULL)
        {
          frame->SetSliceId (0);
          frameDuration = frame->GetDuration ();
          m_symbolsSent.at(0) += ceil(frame->GetDuration ().GetSeconds ()*m_carrierBandwidthInHz);
        }
      else
        {
          frameDuration = m_bbFrameConf->GetDummyBbFrameDuration ();
        }
    }
  else
    {
      uint8_t firstDeque = m_lastSliceDequeued;
      do
        {
          uint32_t symbols = m_symbolsSent.at(m_lastSliceDequeued) + m_symbolsSent.at(0);
          double maxSymbolRate = m_bbFrameContainers.at (m_lastSliceDequeued)->GetMaxSymbolRate ();

          frame = m_bbFrameContainers.at (m_lastSliceDequeued)->GetNextFrame ();
          if (frame != NULL)
            {
              m_symbolsSent.at(m_lastSliceDequeued) += ceil(frame->GetDuration ().GetSeconds ()*m_carrierBandwidthInHz);
              symbols += ceil(frame->GetDuration ().GetSeconds ()*m_carrierBandwidthInHz);
              frame->SetSliceId (m_lastSliceDequeued);
              frameDuration = frame->GetDuration ();

              if (symbols/m_periodicInterval.GetSeconds () > maxSymbolRate)
                {
                  NS_LOG_WARN ("Symbol rate not respected for slice " + std::to_string(m_lastSliceDequeued)
                    + ". Got " + std::to_string(symbols/m_periodicInterval.GetSeconds ()) + "Baud"
                    + " while max is " + std::to_string (maxSymbolRate) + " Baud");
                }
            }
          if (m_lastSliceDequeued == m_numberOfSlices)
            {
              m_lastSliceDequeued = -1;
            }
          m_lastSliceDequeued++;
        }
      while (frame == NULL && m_lastSliceDequeued != firstDeque);
    }

  // create dummy frame
  if ( m_dummyFrameSendingEnabled && frame == NULL )
    {
      frame = Create<SatBbFrame> (m_bbFrameConf->GetDefaultModCod (), SatEnums::DUMMY_FRAME, m_bbFrameConf);

      // create dummy packet
      Ptr<Packet> dummyPacket = Create<Packet> (1);

      // Add MAC tag
      SatMacTag mTag;
      mTag.SetDestAddress (Mac48Address::GetBroadcast ());
      mTag.SetSourceAddress (m_macAddress);
      dummyPacket->AddPacketTag (mTag);

      // Add E2E address tag
      SatAddressE2ETag addressE2ETag;
      addressE2ETag.SetFinalDestAddress (Mac48Address::GetBroadcast ());
      addressE2ETag.SetFinalSourceAddress (m_macAddress);
      dummyPacket->AddPacketTag (addressE2ETag);

      // Add dummy packet to dummy frame
      frame->AddPayload (dummyPacket);
      frame->SetSliceId (0);
      frameDuration = frame->GetDuration ();
    }
  // If no bb frame available and dummy frames disabled
  else if (frame == NULL)
    {
      frameDuration = m_bbFrameConf->GetDummyBbFrameDuration ();
    }

  return std::make_pair(frame, frameDuration);
}

void
SatFwdLinkSchedulerTimeSlicing::PeriodicTimerExpired ()
{
  NS_LOG_FUNCTION (this);

  SendAndClearSymbolsSentStat ();
  ScheduleBbFrames ();

  Simulator::Schedule (m_periodicInterval, &SatFwdLinkSchedulerTimeSlicing::PeriodicTimerExpired, this);
}

void
SatFwdLinkSchedulerTimeSlicing::SendAndClearSymbolsSentStat ()
{
  NS_LOG_FUNCTION (this);

  for (std::map<uint8_t, uint32_t>::iterator it = m_symbolsSent.begin(); it != m_symbolsSent.end(); it++ )
    {
      m_schedulingSymbolRateTrace (it->first, it->second / Seconds (1).GetSeconds ());
    }

  m_symbolsSent.clear ();

  for(uint8_t i = 0; i <= m_numberOfSlices; i++)
    {
      m_symbolsSent.insert (std::pair<uint8_t, uint32_t> (i, 0));
    }
}

void
SatFwdLinkSchedulerTimeSlicing::ScheduleBbFrames ()
{
  NS_LOG_FUNCTION (this);

  // Get scheduling objects from LLC
  std::vector< Ptr<SatSchedulingObject> > so;
  GetSchedulingObjects (so);

  for ( std::vector< Ptr<SatSchedulingObject> >::const_iterator it = so.begin ();
        ( it != so.end () ) && ( GetTotalDuration () < m_periodicInterval ); it++ )
    {
      uint32_t currentObBytes = (*it)->GetBufferedBytes ();
      uint32_t currentObMinReqBytes = (*it)->GetMinTxOpportunityInBytes ();
      uint8_t flowId = (*it)->GetFlowId ();
      Mac48Address address = (*it)->GetMacAddress ();

      if ( (m_slicesMapping.find(address) == m_slicesMapping.end()) && (address != Mac48Address::GetBroadcast ()) )
        {
          m_slicesMapping.insert (std::pair<Mac48Address, uint8_t> (address, m_lastSliceAssigned));
          if (m_lastSliceAssigned == m_numberOfSlices)
            {
              m_lastSliceAssigned = 0;
            }
          m_lastSliceAssigned++;

          SendTimeSliceSubscription (address, std::vector<uint8_t> {m_slicesMapping.at (address)});

          // Begin again scheduling to insert slice subscription control packet.
          Simulator::Schedule (Seconds (0), &SatFwdLinkSchedulerTimeSlicing::ScheduleBbFrames, this);
          return;
        }
      uint8_t slice = (address == Mac48Address::GetBroadcast ()) ? 0 : m_slicesMapping.at (address);
      SatEnums::SatModcod_t modcod = m_bbFrameContainers.at (slice)->GetModcod ( flowId, GetSchedulingObjectCno (*it));

      uint32_t frameBytes = m_bbFrameContainers.at (slice)->GetBytesLeftInTailFrame (flowId, modcod);

      if ((m_bbFrameContainers.at (slice)->IsEmpty (flowId, modcod)) && (currentObBytes > 0) && !CanOpenBbFrame (address, flowId, modcod))
        {
          continue;
        }

      while ( (GetTotalDuration () < m_periodicInterval) && (currentObBytes > 0) )
        {
          if ( frameBytes < currentObMinReqBytes)
            {
              frameBytes = m_bbFrameContainers.at (slice)->GetMaxFramePayloadInBytes (flowId, modcod) - m_bbFrameConf->GetBbFrameHeaderSizeInBytes ();

              if (!CanOpenBbFrame (address, flowId, modcod))
                {
                  break;
                }

              // if frame bytes still too small, we must have too long control message, so let's crash
              if ( frameBytes < currentObMinReqBytes )
                {
                  NS_FATAL_ERROR ("Control package probably too long!!!");
                }
            }

          Ptr<Packet> p = m_txOpportunityCallback (frameBytes, address, flowId, currentObBytes, currentObMinReqBytes);

          if ( p )
            {
              if ((flowId == 0) || (address == Mac48Address::GetBroadcast ()))
                {
                  m_bbFrameContainers.at (0)->AddData (flowId, modcod, p);
                }
              else
                {
                  m_bbFrameContainers.at (slice)->AddData (flowId, modcod, p);
                  frameBytes = m_bbFrameContainers.at (slice)->GetBytesLeftInTailFrame (flowId, modcod);
                }
            }
          else if (m_bbFrameContainers.at(slice)->GetMaxFramePayloadInBytes (flowId, modcod ) != m_bbFrameContainers.at(slice)->GetBytesLeftInTailFrame (flowId, modcod))
            {
              frameBytes = m_bbFrameContainers.at (slice)->GetMaxFramePayloadInBytes (flowId, modcod);

              if (!CanOpenBbFrame (address, flowId, modcod))
                {
                  break;
                }
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

  if ( GetTotalDuration () < m_periodicInterval )
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

  for(std::vector<uint8_t>::iterator it = slices.begin(); it != slices.end(); ++it)
    {
      Ptr<SatSliceSubscriptionMessage> sliceSubscription = CreateObject<SatSliceSubscriptionMessage> ();
      sliceSubscription->SetSliceId(*it);
      sliceSubscription->SetAddress(address);

      m_sendControlMsgCallback (sliceSubscription, Mac48Address::GetBroadcast ());
    }
}

bool
SatFwdLinkSchedulerTimeSlicing::CanOpenBbFrame (Mac48Address address, uint32_t priorityClass, SatEnums::SatModcod_t modcod)
{
  NS_LOG_FUNCTION (this);

  if (priorityClass == 0)
    {
      // Always allow control messages to be send
      // TODO add a margin to take this into account ?
      return true;
    }

  uint8_t sliceId = (address == Mac48Address::GetBroadcast ()) ? 0 : m_slicesMapping.at (address);
  double maxSymbolRate = m_bbFrameContainers.at (sliceId)->GetMaxSymbolRate ();

  if (sliceId == 0)
    {
      // This is broadcast -> need to test all slices >= 1
      uint32_t symbols = GetSymbols (0, modcod);
        for (std::map<uint8_t, Ptr<SatBbFrameContainer>>::iterator it = m_bbFrameContainers.begin(); it != m_bbFrameContainers.end(); it++ )
          {
            if (it->first == 0)
              {
                continue;
              }
            double symbolRate = (symbols + GetSymbols (it->first, SatEnums::SAT_NONVALID_MODCOD))/ m_periodicInterval.GetSeconds ();
            if (symbolRate > maxSymbolRate)
              {
                // One slice will not respect constraints
                return false;
              }
          }
      // Constraints are respected for all slices
      return true;
    }
  else
    {
      uint32_t symbols = GetSymbols (sliceId, modcod) + GetSymbols (0, SatEnums::SAT_NONVALID_MODCOD);
      double symbolRate = symbols/ m_periodicInterval.GetSeconds ();
      return symbolRate < maxSymbolRate;
    }
}

uint32_t
SatFwdLinkSchedulerTimeSlicing::GetSymbols (uint8_t sliceId, SatEnums::SatModcod_t modcod)
{
  NS_LOG_FUNCTION (this);

  uint32_t symbols = m_bbFrameContainers.at (sliceId)->GetTotalDuration ().GetSeconds ()*m_carrierBandwidthInHz;

  if (modcod != SatEnums::SAT_NONVALID_MODCOD)
    {
      symbols += m_bbFrameContainers.at (sliceId)->GetFrameSymbols(modcod);
    }

  return symbols;
}

} // namespace ns3
