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
#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/packet.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-lower-layer-service.h>
#include <ns3/satellite-scheduling-object.h>
#include "satellite-ut-scheduler.h"

NS_LOG_COMPONENT_DEFINE ("SatUtScheduler");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtScheduler);

TypeId
SatUtScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtScheduler")
    .SetParent<Object> ()
    .AddConstructor<SatUtScheduler> ()
    .AddAttribute ("StrictPriorityForControl",
                   "Utilize strict priority for control packets",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatUtScheduler::m_prioritizeControl),
                   MakeBooleanChecker ())
    .AddAttribute ("FramePduHeaderSize",
                   "Frame PDU header size in bytes",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SatUtScheduler::m_framePduHeaderSizeInBytes),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TypeId
SatUtScheduler::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatUtScheduler::SatUtScheduler ()
  : m_schedContextCallback (),
    m_txOpportunityCallback (),
    m_llsConf (),
    m_prioritizeControl (true),
    m_framePduHeaderSizeInBytes (1),
    m_nodeInfo ()
{

}

SatUtScheduler::SatUtScheduler (Ptr<SatLowerLayerServiceConf> lls)
  : m_schedContextCallback (),
    m_txOpportunityCallback (),
    m_llsConf (lls),
    m_prioritizeControl (true),
    m_framePduHeaderSizeInBytes (1),
    m_nodeInfo ()
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  m_utScheduledByteCounters = std::vector<uint32_t> (m_llsConf->GetDaServiceCount (), 0);

  for (uint32_t i = 0; i < SatEnums::NUM_FIDS; ++i)
    {
      m_rcIndices.push_back (i);
    }
}


SatUtScheduler::~SatUtScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
SatUtScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_schedContextCallback.Nullify ();
  m_txOpportunityCallback.Nullify ();
  m_llsConf = NULL;
  m_nodeInfo = NULL;

  Object::DoDispose ();
}

void
SatUtScheduler::SetSchedContextCallback (SatUtScheduler::SchedContextCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_schedContextCallback = cb;
}

void
SatUtScheduler::SetTxOpportunityCallback (SatUtScheduler::TxOpportunityCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_txOpportunityCallback = cb;
}


void
SatUtScheduler::DoScheduling (std::vector<Ptr<Packet> > &packets, uint32_t payloadBytes, SatTimeSlotConf::SatTimeSlotType_t type, uint8_t rcIndex, SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << payloadBytes << (uint32_t) rcIndex << policy);

  NS_LOG_INFO ("UT scheduling RC: " << (uint32_t)(rcIndex) << " with " << payloadBytes << " bytes");

  if (type == SatTimeSlotConf::SLOT_TYPE_C && rcIndex != SatEnums::CONTROL_FID)
    {
      NS_FATAL_ERROR ("Confict in time slot data between RC index and slot type!");
    }

  // Schedule
  // - 1. control
  // - 2. rc index
  if (m_prioritizeControl)
    {
      DoSchedulingForRcIndex (packets, payloadBytes, SatEnums::CONTROL_FID);

      if (payloadBytes > 0)
        {
          DoSchedulingForRcIndex (packets, payloadBytes, rcIndex);
        }
    }
  // Schedule only the requested RC index
  else
    {
      DoSchedulingForRcIndex (packets, payloadBytes, rcIndex);
    }

  // If we still have bytes left and the
  // scheduling policy is loose
  if (payloadBytes > 0 && policy == LOOSE && type == SatTimeSlotConf::SLOT_TYPE_TRC)
    {
      std::vector<uint8_t> rcIndices = GetPrioritizedRcIndexOrder ();

      for (std::vector<uint8_t>::const_iterator it = rcIndices.begin ();
           it != rcIndices.end ();
           ++it)
        {
          // No use asking the given RC index again
          if (*it != rcIndex)
            {
              NS_LOG_INFO ("UT scheduling RC: " << (uint32_t)(rcIndex) << " with " << payloadBytes << " bytes");

              // Schedule the requested RC index
              uint32_t bytes = DoSchedulingForRcIndex (packets, payloadBytes, *it);

              // If the UT was scheduled update the payload counters
              if (bytes > 0)
                {
                  m_utScheduledByteCounters.at (*it) = m_utScheduledByteCounters.at (*it) + bytes;
                }
            }

          if (payloadBytes == 0)
            {
              break;
            }
        }
    }
}

uint32_t
SatUtScheduler::DoSchedulingForRcIndex (std::vector<Ptr<Packet> > &packets, uint32_t &payloadBytes, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << payloadBytes << (uint32_t) rcIndex);

  uint32_t schedBytes (0);
  uint32_t bytesLeft (0);
  uint32_t nextMinTxO (0);

  // User data packets are encapsulated within Frame PDU
  if (rcIndex != SatEnums::CONTROL_FID)
    {
      payloadBytes -= m_framePduHeaderSizeInBytes;
    }

  while (payloadBytes > 0)
    {
      Ptr<Packet> p = m_txOpportunityCallback (payloadBytes, m_nodeInfo->GetMacAddress (), rcIndex, bytesLeft, nextMinTxO);
      if (p)
        {
          NS_LOG_INFO ("Created a packet from RC: " << (uint32_t)(rcIndex) << " size: " << p->GetSize ());

          packets.push_back (p);

          NS_ASSERT (payloadBytes >= p->GetSize ());

          schedBytes += p->GetSize ();
          payloadBytes -= p->GetSize ();
        }
      // LLC returned NULL packet
      else
        {
          break;
        }
    }

  // If no packets were scheduled, return the frame PDU
  // header size
  if (schedBytes == 0 && rcIndex != SatEnums::CONTROL_FID)
    {
      payloadBytes += m_framePduHeaderSizeInBytes;
    }

  return schedBytes;
}

void
SatUtScheduler::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this);

  m_nodeInfo = nodeInfo;
}

std::vector<uint8_t>
SatUtScheduler::GetPrioritizedRcIndexOrder ()
{
  NS_LOG_FUNCTION (this);

  std::sort (m_rcIndices.begin (), m_rcIndices.end (), SortByMetric (m_utScheduledByteCounters));

  return m_rcIndices;
}

} // namespace ns3


