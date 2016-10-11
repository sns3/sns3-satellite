/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#include <algorithm>
#include <iostream>
#include "ns3/mac48-address.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

#include "satellite-tbtp-container.h"
#include "satellite-const-variables.h"
#include "satellite-control-message.h"
#include "satellite-superframe-sequence.h"

NS_LOG_COMPONENT_DEFINE ("SatTbtpContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTbtpContainer);

TypeId
SatTbtpContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTbtpContainer")
    .SetParent<Object> ()
    .AddConstructor<SatTbtpContainer> ()
    .AddAttribute ("MaxStoredTbtps",
                   "Maximum amount of stored TBTPs",
                   UintegerValue (100),
                   MakeUintegerAccessor (&SatTbtpContainer::m_maxStoredTbtps),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

SatTbtpContainer::SatTbtpContainer ()
  : m_address (),
    m_maxStoredTbtps (100),
    m_rcvdTbtps (0),
    m_superFrameDuration (0)
{
  NS_FATAL_ERROR ("SatTbtpContainer::SatTbtpContainer - Constructor not in use");
}

SatTbtpContainer::SatTbtpContainer (Ptr<SatSuperframeSeq> seq)
  : m_address (),
    m_superframeSeq (seq),
    m_maxStoredTbtps (100),
    m_rcvdTbtps (0),
    m_superFrameDuration (seq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetDuration ())
{

}

SatTbtpContainer::~SatTbtpContainer ()
{

}

void SatTbtpContainer::DoDispose ()
{
  m_tbtps.clear ();
  Object::DoDispose ();
}


void
SatTbtpContainer::SetMacAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this);
  m_address = address;
}

void
SatTbtpContainer::Add (Time startTime, Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << startTime.GetSeconds ());

  ++m_rcvdTbtps;

  m_tbtps.insert (std::make_pair (startTime, tbtp));

  // If there are too many TBTPs in the container, erase the first
  if (m_tbtps.size () > m_maxStoredTbtps)
    {
      m_tbtps.erase (m_tbtps.begin ());
    }
}

void
SatTbtpContainer::RemovePastTbtps ()
{
  NS_LOG_FUNCTION (this);

  for (TbtpMap_t::iterator it = m_tbtps.begin (); it != m_tbtps.end (); )
    {
      if ((it->first + m_superFrameDuration) < Now ())
        {
          m_tbtps.erase (it++);
        }
      else
        {
          ++it;
        }
    }
}


bool
SatTbtpContainer::HasScheduledTimeSlots ()
{
  NS_LOG_FUNCTION (this);

  bool hasScheduledTimeSlots = false;

  if (!m_tbtps.empty ())
    {
      RemovePastTbtps ();

      SatTbtpMessage::DaTimeSlotInfoItem_t info;
      for (TbtpMap_t::const_reverse_iterator it = m_tbtps.rbegin ();
           it != m_tbtps.rend ();
           ++it)
        {
          info = it->second->GetDaTimeslots (m_address);

          // This TBTP has time slots for this UT
          if (!info.second.empty ())
            {
              Time superframeStartTime = it->first;

              // If superframe start time is in the future
              if (superframeStartTime >= Simulator::Now ())
                {
                  NS_LOG_INFO ("Superframe counter: " << it->second->GetSuperframeCounter () <<
                               ", start time: " << superframeStartTime.GetSeconds ());

                  hasScheduledTimeSlots = true;
                  break;
                }
              // On-going superframe
              else
                {
                  /**
                   * The time slots are not necessarily in increasing order in the TBTP.
                   * Sort the time slots into increasing order based on time.
                   */
                  std::sort (info.second.begin (), info.second.end (), SortTimeSlots ());

                  // Start time offset for the last time slot for this UT
                  Time startTimeOffsetForLastSlot = (*(info.second.rbegin ()))->GetStartTime ();

                  /**
                   * Calculate the duration of the last slot. To be able to do that we need the
                   * superframe conf, frame conf, time slot conf and symbol rate.
                   */
                  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
                  uint8_t frameId = info.first;
                  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
                  uint32_t wfId = (*(info.second.rbegin ()))->GetWaveFormId ();
                  Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (wfId);
                  Time lastSlotDuration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

                  NS_LOG_INFO ("Superframe counter: " << it->second->GetSuperframeCounter () <<
                               ", start time: " << superframeStartTime.GetSeconds () <<
                               ", last allocated slot start time: " << (superframeStartTime + startTimeOffsetForLastSlot).GetSeconds () <<
                               ", last allocated slot end time: " << (superframeStartTime + startTimeOffsetForLastSlot + lastSlotDuration).GetSeconds ());

                  /**
                   * Check that the TBTP has a time slot which is in the future. It does not matter
                   * how long to the future the time slot is, since the same method may be used for both
                   * CRDSA and SA, and we do not have any idea of what are their randomization intervals etc.
                   */
                  if ((superframeStartTime + startTimeOffsetForLastSlot + lastSlotDuration) > Simulator::Now ())
                    {
                      hasScheduledTimeSlots = true;
                    }
                  break;
                }
            }
        }
    }

  return hasScheduledTimeSlots;
}

} // namespace


