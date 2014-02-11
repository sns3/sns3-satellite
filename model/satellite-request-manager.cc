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
#include "ns3/simulator.h"
#include "satellite-request-manager.h"

NS_LOG_COMPONENT_DEFINE ("SatRequestManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRequestManager);


SatRequestManager::SatRequestManager ()
:m_requestInterval (MilliSeconds (100)),
 m_rcIndex (0)
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf(AttributeConstructionList ());

  // Start the request manager evaluation cycle
  Simulator::Schedule (m_requestInterval, &SatRequestManager::DoPeriodicalEvaluation, this);
}

SatRequestManager::~SatRequestManager ()
{

}

TypeId
SatRequestManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRequestManager")
    .SetParent<Object> ()
    .AddConstructor<SatRequestManager> ()
    .AddAttribute( "RequestManagerInterval",
                   "Request manager interval",
                    TimeValue (MilliSeconds (100)),
                    MakeTimeAccessor (&SatRequestManager::m_requestInterval),
                    MakeTimeChecker ())
  ;
  return tid;
}

TypeId
SatRequestManager::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}


void SatRequestManager::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_queueCallback.Nullify();
  Object::DoDispose ();
}

void
SatRequestManager::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << rcIndex);

  if (event == SatQueue::FIRST_BUFFERED_PKT)
    {
      NS_LOG_LOGIC ("FIRST_BUFFERED_PKT event received from queue: " << rcIndex);
    }
  else if (event == SatQueue::BUFFER_EMPTY)
    {
      NS_LOG_LOGIC ("BUFFER_EMPTY event received from queue: " << rcIndex);
    }
  else
    {
      NS_FATAL_ERROR ("Unsupported queue event received!");
    }
}

void
SatRequestManager::DoPeriodicalEvaluation ()
{
  NS_LOG_FUNCTION (this);

  DoEvaluation ();

  // Schedule next evaluation interval
  Simulator::Schedule (m_requestInterval, &SatRequestManager::DoPeriodicalEvaluation, this);
}

void
SatRequestManager::DoEvaluation ()
{
  NS_LOG_FUNCTION (this);

  /**
   * TODO: this does not do anything yet!
   */
  /*
  double enqueBitrate (0.0);
  if (!m_queueCallback.IsNull ())
    {
      enqueBitrate = m_queueCallback (m_rcIndex);
    }
  NS_LOG_LOGIC ("Periodically evaluated enque bitrate: " << enqueBitrate);
  */
}


void
SatRequestManager::SetQueueCallback (SatRequestManager::QueueCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_queueCallback = cb;
}


}
