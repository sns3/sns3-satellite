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
 m_evaluationIntervalInSeconds (100.0),
 m_rttEstimate (MilliSeconds (560)),
 m_maxPendingCrEntries (0),
 m_gainValueK (1/100),
 m_pendingRbdcRequests (),
 m_pendingVbdcCounters ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);
}

SatRequestManager::SatRequestManager (Ptr<SatLowerLayerServiceConf> llsConf, double evaluationInterval)
:m_gwAddress (),
 m_lastCno (NAN),
 m_llsConf (llsConf),
 m_evaluationIntervalInSeconds (evaluationInterval),
 m_rttEstimate (MilliSeconds (560)),
 m_maxPendingCrEntries (0),
 m_gainValueK (1/100)
{
  ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_gainValueK = 1 / (2 * m_evaluationIntervalInSeconds);
  m_maxPendingCrEntries = (uint32_t)(std::floor(m_rttEstimate.GetSeconds () / m_evaluationIntervalInSeconds));

  m_pendingRbdcRequests = std::vector< std::deque<uint32_t> > (m_llsConf->GetDaServiceCount (), std::deque<uint32_t> ());
  m_pendingVbdcCounters = std::vector<uint32_t> (m_llsConf->GetDaServiceCount (), 0);

  NS_LOG_LOGIC ("Gain value: " << m_gainValueK << ", maxPendinCrEntries: " << m_maxPendingCrEntries);

  // Start the request manager evaluation cycle
  Simulator::Schedule (Time::FromDouble(m_evaluationIntervalInSeconds, Time::S), &SatRequestManager::DoPeriodicalEvaluation, this);
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
  Simulator::Schedule (Time::FromDouble(m_evaluationIntervalInSeconds, Time::S), &SatRequestManager::DoPeriodicalEvaluation, this);
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

          uint32_t vbdcBytes (0);

          SatEnums::SatCapacityAllocationCategory_t cac = DoVbdc (rc, stats, vbdcBytes);
          NS_LOG_LOGIC ("Requested VBCD volume for RC: " << rc << " is " << vbdcBytes << " Bytes");
          crMsg->AddControlElement (rc, cac, vbdcBytes);
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

  m_lastCno = cno;
}

uint32_t
SatRequestManager::DoRbdc (uint8_t rc, const SatQueue::QueueStats_t stats)
{
  NS_LOG_FUNCTION (this << rc);

  // Calculate the raw RBDC request
  double coefficient = m_gainValueK / m_evaluationIntervalInSeconds;
  double thisRbdc = stats.m_incomingRateKbps * m_evaluationIntervalInSeconds;
  double previousRbdc = GetPendingRbdcSum (rc) * m_evaluationIntervalInSeconds;

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
            (uint32_t)(floor(reqRbdc / M_RBDC_QUANTIZATION_STEP_SMALL_KBPS) * M_RBDC_QUANTIZATION_STEP_SMALL_KBPS);
      }
  // 512 - 8160 kbps  -> 32 kbps resolution
  else
    {
      crRbdc =
          (uint32_t)(floor(reqRbdc / M_RBDC_QUANTIZATION_STEP_LARGE_KBPS) * M_RBDC_QUANTIZATION_STEP_LARGE_KBPS);
    }

  NS_LOG_LOGIC("Quantized RBDC bitrate: " << crRbdc << " kbps");

  if (crRbdc > m_llsConf->GetDaMaximumServiceRateInKbps (rc))
    {
      crRbdc = m_llsConf->GetDaMaximumServiceRateInKbps (rc);
    }

  NS_LOG_LOGIC("RBDC bitrate after maximum check: " << crRbdc << " kbps");

  UpdatePendingRbdcCounters (rc, crRbdc);

  return crRbdc;
}


SatEnums::SatCapacityAllocationCategory_t
SatRequestManager::DoVbdc (uint8_t rc, const SatQueue::QueueStats_t stats, uint32_t &rcVbdc)
{
  NS_LOG_FUNCTION (this << rc);

  SatEnums::SatCapacityAllocationCategory_t cac = SatEnums::DA_VBDC;
  rcVbdc = stats.m_volumeInBytes;

  // If CRA enabled, substract the CRA bitrate from VBDC

  if (m_llsConf->GetDaConstantAssignmentProvided (rc))
    {
      NS_LOG_LOGIC("CRA is enabled together with VBDC for RC: " << rc);
      double craBytes = 8.0 * m_llsConf->GetDaConstantServiceRateInKbps(rc) * m_evaluationIntervalInSeconds;
      if (craBytes < rcVbdc)
        {
          rcVbdc -= (uint32_t)(round(craBytes));
        }
      else
        {
          rcVbdc = 0;
        }
      NS_LOG_LOGIC("VBDC bytes after reduction of CRA: " << rcVbdc);
    }

  if (m_pendingVbdcCounters.at (rc) > stats.m_volumeOutBytes)
    {
      m_pendingVbdcCounters.at (rc) = m_pendingVbdcCounters.at (rc) - stats.m_volumeOutBytes;
    }
  else
    {
      m_pendingVbdcCounters.at (rc) = 0;
    }

  // Calculate the raw VBDC request
  if (stats.m_queueSizeBytes > (stats.m_volumeInBytes + m_pendingVbdcCounters.at (rc)))
  {
    rcVbdc += m_gainValueK * (stats.m_queueSizeBytes - stats.m_volumeInBytes - m_pendingVbdcCounters.at (rc));
  }

  NS_LOG_LOGIC("Raw VBDC bytes: " << rcVbdc << " rcVbdc");

  // Check DA volume allowed
  if ( (rcVbdc + m_pendingVbdcCounters.at (rc)) > m_llsConf->GetDaMaximumBacklogInBytes (rc) )
    {
      rcVbdc = m_llsConf->GetDaMaximumBacklogInBytes (rc) - m_pendingVbdcCounters.at (rc);
    }

  NS_LOG_LOGIC("Raw VBDC bytes after taking the pending requests into account: " << rcVbdc << " Bytes");

  // Quantization
  // 0 - 255 bytes     -> 1 byte resolution
  if (rcVbdc <= M_VBDC_QUANTIZATION_THRESHOLD_BYTES)
      {
        rcVbdc =
            (uint32_t)(floor(rcVbdc / M_VBDC_QUANTIZATION_STEP_SMALL) * M_VBDC_QUANTIZATION_STEP_SMALL);
      }
  // 256 - 4080 kbps  -> 16 kbps resolution
  else
    {
      rcVbdc =
          (uint32_t)(floor(rcVbdc / M_VBDC_QUANTIZATION_STEP_LARGE) * M_VBDC_QUANTIZATION_STEP_LARGE);
    }

  if (rcVbdc > 0 && m_pendingVbdcCounters.at (rc) == 0)
    {
      cac = SatEnums::DA_AVBDC;
    }

  NS_LOG_LOGIC("Quantized VBDC volume: " << rcVbdc << " Bytes");

  // Update the pending counters
  m_pendingVbdcCounters.at (rc) = m_pendingVbdcCounters.at (rc) + rcVbdc;

  return cac;
}

uint32_t
SatRequestManager::GetPendingRbdcSum (uint8_t rc) const
{
  NS_LOG_FUNCTION (this << rc);

  uint32_t value (0);

  // Check if the key is found
  std::deque<uint32_t> cont = m_pendingRbdcRequests.at (rc);

  for (std::deque<uint32_t>::const_iterator it = cont.begin ();
      it != cont.end ();
      ++it)
    {
      value += (*it);
    }

  NS_LOG_LOGIC("Pending RBDC sum for RC: " << rc << " is " << value);

  return value;
}


void
SatRequestManager::UpdatePendingRbdcCounters (uint8_t rc, uint32_t value)
{
  NS_LOG_FUNCTION (this << rc << value);

  m_pendingRbdcRequests.at(rc).push_back (value);

  // If there are too many entries, pop front
  if (m_pendingRbdcRequests.at (rc).size () > m_maxPendingCrEntries)
    {
      m_pendingRbdcRequests.at (rc).pop_front ();
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
