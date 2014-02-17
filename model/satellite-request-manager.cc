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
:m_llsConf (),
 m_requestInterval (MilliSeconds (100)),
 m_rcIndex (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);
}

SatRequestManager::SatRequestManager (Ptr<SatLowerLayerServiceConf> llsConf)
:m_llsConf (llsConf),
 m_requestInterval (MilliSeconds (100)),
 m_rcIndex (0)
{
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

  for (CallbackContainer_t::iterator it = m_queueCallbacks.begin();
      it != m_queueCallbacks.end ();
      ++it)
    {
      it->second.Nullify();
    }
  m_queueCallbacks.clear ();

  m_llsConf = NULL;

  Object::DoDispose ();
}

void
SatRequestManager::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint32_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << rcIndex);

  if (event == SatQueue::FIRST_BUFFERED_PKT)
    {
      NS_LOG_LOGIC ("FIRST_BUFFERED_PKT event received from queue: " << rcIndex);

      //DoEvaluation (false);
    }
  else if (event == SatQueue::BUFFERED_PKT)
    {
      NS_LOG_LOGIC ("BUFFERED_PKT event received from queue: " << rcIndex);
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

  DoEvaluation (true);

  // Schedule next evaluation interval
  Simulator::Schedule (m_requestInterval, &SatRequestManager::DoPeriodicalEvaluation, this);
}

void
SatRequestManager::DoEvaluation (bool periodical)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("---Start request manager evaluation---");

  bool reset = (periodical = true ? true : false);

  // Go through the RC indeces
  for (uint32_t rc = 0; rc < m_llsConf->GetDaServiceCount(); ++rc)
    {
      struct SatQueue::QueueStats_t stats = m_queueCallbacks.at(rc)(reset);

      NS_LOG_LOGIC ("Enque bitrate: " << stats.m_enqueRate);
      NS_LOG_LOGIC ("Deque bitrate: " << stats.m_dequeRate);
      NS_LOG_LOGIC ("Buffered bytes: " << stats.m_bufferedBytes);

      //m_llsConf->GetDaConstantAssignmentProvided (rc);
      //m_llsConf->GetDaConstantServiceRateStream (rc);

      // RBDC
      if (m_llsConf->GetDaRbdcAllowed (rc))
        {
          double requestedRate = stats.m_enqueRate;
          if (requestedRate < m_llsConf->GetDaMinimumServiceRateInKbps (rc))
            {
              requestedRate = m_llsConf->GetDaMinimumServiceRateInKbps (rc);
            }
          else if (requestedRate > m_llsConf->GetDaMaximumServiceRateInKbps (rc))
            {
              requestedRate = m_llsConf->GetDaMaximumServiceRateInKbps (rc);
            }

          NS_LOG_LOGIC ("RBDC request [kbps]: " << requestedRate);
        }

      // VBDC
      if (m_llsConf->GetDaVolumeAllowed (rc))
        {
          uint32_t bytes = stats.m_bufferedBytes;
          if (bytes > m_llsConf->GetDaMaximumBacklogInBytes (rc))
            {
              bytes = m_llsConf->GetDaMaximumBacklogInBytes (rc);
            }

          NS_LOG_LOGIC ("VBDC request [bytes]: " << bytes);
        }
    }

  NS_LOG_LOGIC ("---End request manager evaluation---");
}


void
SatRequestManager::AddQueueCallback (uint8_t rcIndex, SatRequestManager::QueueCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_queueCallbacks.insert (std::make_pair<uint8_t, SatRequestManager::QueueCallback> (rcIndex, cb));
}


}
