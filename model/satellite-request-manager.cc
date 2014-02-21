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

#include <math.h>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "satellite-request-manager.h"
#include "satellite-enums.h"

NS_LOG_COMPONENT_DEFINE ("SatRequestManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRequestManager);


SatRequestManager::SatRequestManager ()
:m_gwAddress (),
 m_lastCno (NAN),
 m_llsConf (),
 m_evaluationInterval (MilliSeconds (100)),
 m_rttEstimate (MilliSeconds (560)),
 m_maxPendingCrEntries (0),
 m_gainValueK (1/100),
 m_pendingRequests ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);
}

SatRequestManager::SatRequestManager (Ptr<SatLowerLayerServiceConf> llsConf)
:m_gwAddress (),
 m_lastCno (NAN),
 m_llsConf (llsConf),
 m_evaluationInterval (MilliSeconds (100)),
 m_rttEstimate (MilliSeconds (560)),
 m_maxPendingCrEntries (0),
 m_gainValueK (1/100),
 m_pendingRequests ()
{
  ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_gainValueK = 1 / (2 * m_evaluationInterval.GetSeconds ());
  m_maxPendingCrEntries = (uint32_t)(std::floor(m_rttEstimate.GetSeconds () / m_evaluationInterval.GetSeconds ()));

  NS_LOG_LOGIC ("Gain value: " << m_gainValueK << ", maxPendinCrEntries: " << m_maxPendingCrEntries);

  // Start the request manager evaluation cycle
  Simulator::Schedule (m_evaluationInterval, &SatRequestManager::DoPeriodicalEvaluation, this);
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
    .AddAttribute( "PeriodicalEvaluationInterval",
                   "Periodical request manager evaluation interval",
                    TimeValue (MilliSeconds (100)),
                    MakeTimeAccessor (&SatRequestManager::m_evaluationInterval),
                    MakeTimeChecker ())
    .AddAttribute( "RttEstimate",
                   "Round trip time estimate for request manager",
                   TimeValue (MilliSeconds (560)),
                   MakeTimeAccessor (&SatRequestManager::m_rttEstimate),
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

  m_ctrlCallback.Nullify ();

  m_llsConf = NULL;

  Object::DoDispose ();
}

void
SatRequestManager::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex)
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
  Simulator::Schedule (m_evaluationInterval, &SatRequestManager::DoPeriodicalEvaluation, this);
}

void
SatRequestManager::DoEvaluation (bool periodical)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("---Start request manager evaluation---");

  bool reset = (periodical = true ? true : false);

  Ptr<SatCrMessage> crMsg = CreateObject<SatCrMessage> ();

  // Go through the RC indeces
  for (uint8_t rc = 0; rc < m_llsConf->GetDaServiceCount(); ++rc)
    {
      struct SatQueue::QueueStats_t stats = m_queueCallbacks.at(rc)(reset);

      NS_LOG_LOGIC ("Incoming kbitrate: " << stats.m_incomingRateKbps);
      NS_LOG_LOGIC ("Outgoing kbitrate: " << stats.m_outgoingRateKbps);
      NS_LOG_LOGIC ("Volume in bytes: " << stats.m_volumeInBytes);
      NS_LOG_LOGIC ("Total queue size: " << stats.m_queueSizeBytes);


      NS_LOG_LOGIC ("Evaluating the needs for RC: " << rc);

      // RBDC
      if (m_llsConf->GetDaRbdcAllowed (rc))
        {
          NS_LOG_LOGIC ("Evaluating RBDC needs for RC: " << rc);
          uint32_t rbdcRate = DoRbdc (rc, stats);

          NS_LOG_LOGIC ("Requested RBCD rate for RC: " << rc << " is " << rbdcRate << " kbps");

          crMsg->AddControlElement (rc, SatEnums::DA_RBDC, rbdcRate);
        }

      // VBDC
      if (m_llsConf->GetDaVolumeAllowed (rc))
        {
          NS_LOG_LOGIC ("Evaluation VBDC for RC: " << rc);
          uint32_t vbdcBytes = DoVbdc (rc, stats);
          NS_LOG_LOGIC ("Requested VBCD volume for RC: " << rc << " is " << vbdcBytes << " Bytes");
          crMsg->AddControlElement (rc, SatEnums::DA_VBDC, vbdcBytes);
        }
    }

  // If CR has some control elements with non-zero
  // content, send the CR
  if (crMsg->HasNonZeroContent ())
    {
      NS_LOG_LOGIC ("Send CR");
      SendCapacityRequest (crMsg);
    }

  NS_LOG_LOGIC ("---End request manager evaluation---");
}


void
SatRequestManager::AddQueueCallback (uint8_t rcIndex, SatRequestManager::QueueCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_queueCallbacks.insert (std::make_pair<uint8_t, SatRequestManager::QueueCallback> (rcIndex, cb));
}

void
SatRequestManager::SetCtrlMsgCallback (SatRequestManager::SendCtrlCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_ctrlCallback = cb;
}

void
SatRequestManager::SetGwAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_gwAddress = address;
}

void
SatRequestManager::CnoUpdated (uint32_t beamId, Address /*utId*/, Address /*gwId*/, double cno)
{
  NS_LOG_FUNCTION (this << beamId << cno);

  NS_LOG_LOGIC ("C/No updated to request manager: " << cno);

  // TODO: Some estimation algorithm needed to use, now we just save the latest received C/N0 info.
  m_lastCno = cno;
}

uint32_t
SatRequestManager::DoRbdc (uint8_t rc, const SatQueue::QueueStats_t stats)
{
  NS_LOG_FUNCTION (this << rc);

  // Calculate the raw RBDC request
  double coefficient = m_gainValueK / m_evaluationInterval.GetSeconds ();
  double thisRbdc = stats.m_incomingRateKbps * m_evaluationInterval.GetSeconds ();
  double previousRbdc = GetPendingSum (rc, SatEnums::DA_RBDC) * m_evaluationInterval.GetSeconds ();

  double reqRbdc (stats.m_incomingRateKbps);
  if (stats.m_queueSizeBytes > (thisRbdc + previousRbdc))
    {
      reqRbdc += coefficient * (stats.m_queueSizeBytes - thisRbdc - previousRbdc);
    }
  // Else the latter term would be negative

  NS_LOG_LOGIC("Raw RBDC bitrate: " << reqRbdc << " kbps");

  // If CRA enabled, substract the CRA bitrate from the calculated RBDC bitrate
  if (m_llsConf->GetDaConstantAssignmentProvided (rc))
    {
      if (reqRbdc <= (double)(m_llsConf->GetDaConstantServiceRateInKbps(rc)))
        {
          reqRbdc = 0.0;
        }
      else
        {
          reqRbdc -= (double)(m_llsConf->GetDaConstantServiceRateInKbps(rc));
        }
    }

  NS_LOG_LOGIC("RBDC bitrate after CRA as been taken off: " << reqRbdc << " kbps");

  uint32_t crRbdc (0);
  // Quantization
  // 0 - 512 kbps     -> 2 kbps resolution
  if (reqRbdc <= M_RBDC_QUANTIZATION_THRESHOLD_KBPS)
      {
        crRbdc =
            (uint32_t)(round(reqRbdc / M_RBDC_QUANTIZATION_STEP_SMALL_KBPS) * M_RBDC_QUANTIZATION_STEP_SMALL_KBPS);
      }
  // 512 - 8160 kbps  -> 32 kbps resolution
  else
    {
      crRbdc =
          (uint32_t)(round(reqRbdc / M_RBDC_QUANTIZATION_STEP_LARGE_KBPS) * M_RBDC_QUANTIZATION_STEP_LARGE_KBPS);
    }

  NS_LOG_LOGIC("Quantized RBDC bitrate: " << crRbdc << " kbps");

  if (crRbdc > m_llsConf->GetDaMaximumServiceRateInKbps (rc))
    {
      crRbdc = m_llsConf->GetDaMaximumServiceRateInKbps (rc);
    }

  NS_LOG_LOGIC("RBDC bitrate after maximum check: " << crRbdc << " kbps");

  UpdatePendingCounters (rc, SatEnums::DA_RBDC, crRbdc);

  return crRbdc;
}


uint32_t
SatRequestManager::DoVbdc (uint8_t rc, const SatQueue::QueueStats_t stats)
{
  NS_LOG_FUNCTION (this << rc);

  uint32_t pendingVbdc = GetPendingSum (rc, SatEnums::DA_VBDC);

  // Calculate the raw VBDC request

  uint32_t rcVbdc (stats.m_volumeInBytes);

  if (stats.m_queueSizeBytes > (stats.m_volumeInBytes + pendingVbdc))
  {
    rcVbdc += m_gainValueK * (stats.m_queueSizeBytes - stats.m_volumeInBytes - pendingVbdc);
  }

  NS_LOG_LOGIC("Raw VBDC bytes: " << rcVbdc << " rcVbdc");

  // Check DA volume allowed
  if ( (rcVbdc + pendingVbdc) > m_llsConf->GetDaMaximumBacklogInBytes (rc) )
    {
      rcVbdc = m_llsConf->GetDaMaximumBacklogInBytes (rc) - pendingVbdc;
    }

  NS_LOG_LOGIC("Raw VBDC bytes after taking the pending requests into account: " << rcVbdc << " Bytes");

  // Quantization
  // 0 - 255 bytes     -> 1 byte resolution
  if (rcVbdc <= M_VBDC_QUANTIZATION_THRESHOLD_BYTES)
      {
        rcVbdc =
            (uint32_t)(round(rcVbdc / M_VBDC_QUANTIZATION_STEP_SMALL) * M_VBDC_QUANTIZATION_STEP_SMALL);
      }
  // 256 - 4080 kbps  -> 16 kbps resolution
  else
    {
      rcVbdc =
          (uint32_t)(round(rcVbdc / M_VBDC_QUANTIZATION_STEP_LARGE) * M_VBDC_QUANTIZATION_STEP_LARGE);
    }

  NS_LOG_LOGIC("Quantized VBDC volume: " << rcVbdc << " Bytes");

  UpdatePendingCounters (rc, SatEnums::DA_VBDC, rcVbdc);

  return rcVbdc;
}

uint32_t
SatRequestManager::GetPendingSum (uint8_t rc, SatEnums::SatCapacityAllocationCategory_t cac) const
{
  NS_LOG_FUNCTION (this << rc << cac);

  uint32_t value (0);

  // Make a key
  PendingRequestKey_t key = std::make_pair <uint8_t, SatEnums::SatCapacityAllocationCategory_t> (rc, cac);

  // Check if the key is found
  if (m_pendingRequests.find (key) != m_pendingRequests.end ())
    {
      std::deque<uint32_t> cont = m_pendingRequests.at (key);

      for (std::deque<uint32_t>::const_iterator it = cont.begin ();
          it != cont.end ();
          ++it)
        {
          value += (*it);
        }
    }

  NS_LOG_LOGIC("Pending sum for RC: " << rc << ", CAC: " << cac << " is " << value);

  return value;
}


void
SatRequestManager::UpdatePendingCounters (uint8_t rc, SatEnums::SatCapacityAllocationCategory_t cac, uint32_t value)
{
  NS_LOG_FUNCTION (this << rc << cac << value);

  // Make a key
  PendingRequestKey_t key = std::make_pair <uint8_t, SatEnums::SatCapacityAllocationCategory_t> (rc, cac);

  // Check if the key is found
  if (m_pendingRequests.find (key) != m_pendingRequests.end ())
    {
      m_pendingRequests.at (key).push_back (value);
    }
  // Else add a new deque container
  else
    {
      NS_LOG_LOGIC("Insert request to pending container for RC: " << rc << ", CAC: " << cac << ", value: " << value);

      std::deque<uint32_t> valueCont;
      valueCont.push_back (value);
      m_pendingRequests.insert (std::make_pair<PendingRequestKey_t, std::deque<uint32_t> > (key, valueCont));
    }

  // If there are too many entries, pop front
  if (m_pendingRequests.at (key).size () > m_maxPendingCrEntries)
    {
      m_pendingRequests.at (key).pop_front ();
    }
}

void
SatRequestManager::SendCapacityRequest (Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this);

  if ( !m_ctrlCallback.IsNull ())
    {
      NS_LOG_LOGIC ("Send capacity request to GW: " << m_gwAddress);
      crMsg->SetCnoEstimate (m_lastCno);
      m_ctrlCallback (crMsg, m_gwAddress);
    }
}

}
