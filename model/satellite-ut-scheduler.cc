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

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/pointer.h"
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
  ;
  return tid;
}

SatUtScheduler::SatUtScheduler (void)
:m_schedContextCallback (),
 m_txOpportunityCallback (),
 m_nodeInfo ()
{
  NS_LOG_FUNCTION (this);
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

Ptr<Packet>
SatUtScheduler::DoScheduling (uint32_t payloadBytes, int rcIndex, SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << payloadBytes << rcIndex);

  /**
   * TODO: DoScheduling is responsible of fetching the packet from upper layer. It may
   * obey the given RC index or decide by itself which RC index to serve. Note, that the
   * RC index is currently set to be 1 always, thus UT scheduler is not capable of scheduling
   * any other RC indices.
   */

  Ptr<Packet> packet;
  std::vector< Ptr<SatSchedulingObject> > schedulingObjects = m_schedContextCallback ();
  if (schedulingObjects.empty ())
    {
      NS_LOG_LOGIC ("No packets buffered in LLC");
      return packet;
    }

  if (rcIndex > 0)
    {
      Ptr<Packet> p = m_txOpportunityCallback (payloadBytes, m_nodeInfo->GetMacAddress (), (uint8_t)(rcIndex));
      if (p)
        {
          NS_LOG_LOGIC ("Created a packet from the wanted RC index queue, size: " << packet->GetSize ());
          return p;
        }
      else if (policy == STRICT)
        {
          NS_LOG_LOGIC ("Using STRICT compliance policy and no packet was found!");
          return packet;
        }
      // Else continue scheduling from other RC indeces
    }

  /**
   * Go through the scheduling objects and try to schedule
   * TODO: Here we should implement some simple scheduler fairness principle so that
   * unintended allocations would go bitwise fairly between different RC indices.
   */
  std::vector< Ptr<SatSchedulingObject> >::iterator it = schedulingObjects.begin ();
  for ( ; it != schedulingObjects.end (); ++it)
    {
      if (payloadBytes >= (*it)->GetMinTxOpportunityInBytes ())
        {
          packet = m_txOpportunityCallback (payloadBytes, (*it)->GetMacAddress (), (*it)->GetPriority ());
          NS_LOG_LOGIC ("Created a packet from RC index: " << (*it)->GetPriority () << ", size: " << packet->GetSize ());
          return packet;
        }
    }

  // Return NULL
  return packet;
}

void
SatUtScheduler::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this);

  m_nodeInfo = nodeInfo;
}

} // namespace ns3


