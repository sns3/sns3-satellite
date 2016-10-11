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

#include <cmath>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/nstime.h"
#include "satellite-const-variables.h"
#include "satellite-request-manager.h"
#include "satellite-enums.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatRequestManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRequestManager);

const uint32_t SatRequestManager::m_rbdcScalingFactors[4] = {1, 4, 16, 64};
const uint32_t SatRequestManager::m_vbdcScalingFactors[4] = {1, 8, 64, 512};

SatRequestManager::SatRequestManager ()
  : m_gwAddress (),
    m_lastCno (NAN),
    m_llsConf (),
    m_evaluationInterval (Seconds (0.1)),
    m_cnoReportInterval (Seconds (0.0)),
    m_gainValueK (1.0),
    m_rttEstimate (MilliSeconds (560)),
    m_overEstimationFactor (1.1),
    m_enableOnDemandEvaluation (false),
    m_pendingRbdcRequestsKbps (),
    m_pendingVbdcBytes (),
    m_previousEvaluationTime (),
    m_lastVbdcCrSent (Seconds (0)),
    m_superFrameDuration (Seconds (0)),
    m_forcedAvbdcUpdate (false),
    m_numValues (256)
{
  NS_LOG_FUNCTION (this);

}


SatRequestManager::~SatRequestManager ()
{

}

void
SatRequestManager::Initialize (Ptr<SatLowerLayerServiceConf> llsConf, Time superFrameDuration)
{
  NS_LOG_FUNCTION (this << superFrameDuration.GetSeconds ());

  m_llsConf = llsConf;

  m_pendingRbdcRequestsKbps = std::vector< std::deque<std::pair<Time, uint32_t> > > (m_llsConf->GetDaServiceCount (), std::deque<std::pair<Time, uint32_t> > ());
  m_pendingVbdcBytes = std::vector<uint32_t> (m_llsConf->GetDaServiceCount (), 0);
  m_assignedDaResourcesBytes = std::vector<uint32_t> (m_llsConf->GetDaServiceCount (), 0);
  m_previousEvaluationTime = std::vector<Time> (m_llsConf->GetDaServiceCount (), Seconds (0.0));

  // Superframe duration
  m_superFrameDuration = superFrameDuration;

  // Start the request manager evaluation cycle
  Simulator::ScheduleWithContext (m_nodeInfo->GetNodeId (), m_evaluationInterval, &SatRequestManager::DoPeriodicalEvaluation, this);

  // Start the C/N0 report cycle
  m_cnoReportEvent = Simulator::Schedule (m_cnoReportInterval, &SatRequestManager::SendCnoReport, this);
}

TypeId
SatRequestManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRequestManager")
    .SetParent<Object> ()
    .AddConstructor<SatRequestManager> ()
    .AddAttribute ( "EvaluationInterval",
                    "Evaluation interval time",
                    TimeValue (Seconds (0.1)),
                    MakeTimeAccessor (&SatRequestManager::m_evaluationInterval),
                    MakeTimeChecker ())
    .AddAttribute ( "CnoReportInterval",
                    "C/NO report interval time",
                    TimeValue (Seconds (1.1)),
                    MakeTimeAccessor (&SatRequestManager::m_cnoReportInterval),
                    MakeTimeChecker ())
    .AddAttribute ( "RttEstimate",
                    "Round trip time estimate for request manager",
                    TimeValue (MilliSeconds (560)),
                    MakeTimeAccessor (&SatRequestManager::m_rttEstimate),
                    MakeTimeChecker ())
    .AddAttribute ( "OverEstimationFactor",
                    "Over-estimation due to RLE and FPDU overhead.",
                    DoubleValue (1.1),
                    MakeDoubleAccessor (&SatRequestManager::m_overEstimationFactor),
                    MakeDoubleChecker<double_t> ())
    .AddAttribute ( "EnableOnDemandEvaluation",
                    "Enable on-demand resource evaluation.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatRequestManager::m_enableOnDemandEvaluation),
                    MakeBooleanChecker ())
    .AddAttribute ( "GainValueK",
                    "Gain value K for RBDC calculation.",
                    DoubleValue (1.0),
                    MakeDoubleAccessor (&SatRequestManager::m_gainValueK),
                    MakeDoubleChecker<double_t> ())
    .AddTraceSource ("CrTrace",
                     "Capacity request trace",
                     MakeTraceSourceAccessor (&SatRequestManager::m_crTrace),
                     "ns3::SatRequestManager::CapacityRequestTraceCallback")
    .AddTraceSource ("CrTraceLog",
                     "Capacity request trace log",
                     MakeTraceSourceAccessor (&SatRequestManager::m_crTraceLog),
                     "ns3::SatRequestManager::CapacityRequestTraceLogCallback")
    .AddTraceSource ("RbdcTrace",
                     "Trace for all sent RBDC capacity requests.",
                     MakeTraceSourceAccessor (&SatRequestManager::m_rbdcTrace),
                     "ns3::SatRequestManager::RbdcTraceCallback")
    .AddTraceSource ("VbdcTrace",
                     "Trace for all sent VBDC capacity requests.",
                     MakeTraceSourceAccessor (&SatRequestManager::m_vbdcTrace),
                     "ns3::SatRequestManager::VbdcTraceCallback")
    .AddTraceSource ("AvbdcTrace",
                     "Trace for all sent AVBDC capacity requests.",
                     MakeTraceSourceAccessor (&SatRequestManager::m_aVbdcTrace),
                     "ns3::SatRequestManager::AvbdcTraceCallback")
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

  for (CallbackContainer_t::iterator it = m_queueCallbacks.begin ();
       it != m_queueCallbacks.end ();
       ++it)
    {
      it->second.Nullify ();
    }
  m_queueCallbacks.clear ();

  m_ctrlCallback.Nullify ();

  m_ctrlMsgTxPossibleCallback.Nullify ();

  m_llsConf = NULL;

  Object::DoDispose ();
}

void
SatRequestManager::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << (uint32_t)(rcIndex));

  if (event == SatQueue::FIRST_BUFFERED_PKT)
    {
      NS_LOG_INFO ("FIRST_BUFFERED_PKT event received from queue: " << (uint32_t)(rcIndex));

      if (m_enableOnDemandEvaluation)
        {
          NS_LOG_INFO ("Do on-demand CR evaluation for RC index: " << (uint32_t)(rcIndex));
          DoEvaluation ();
        }
    }
  // Other queue events not handled here
}

void
SatRequestManager::DoPeriodicalEvaluation ()
{
  NS_LOG_FUNCTION (this);

  DoEvaluation ();

  // Schedule next evaluation interval
  Simulator::Schedule (m_evaluationInterval, &SatRequestManager::DoPeriodicalEvaluation, this);
}

void
SatRequestManager::DoEvaluation ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("---Start request manager evaluation---");

  // Check whether the ctrl msg transmission is possible
  bool ctrlMsgTxPossible = m_ctrlMsgTxPossibleCallback ();

  // The request manager evaluation is not done, if there is no
  // possibility to send the CR. Instead, we just we wait for the next
  // evaluation interval.
  if (ctrlMsgTxPossible)
    {
      // Update the VBDC counters based on received TBTP
      // resources
      UpdatePendingVbdcCounters ();

      // Check whether we should update the VBDC request
      // with AVBDC.
      CheckForVolumeBacklogPersistence ();

      Ptr<SatCrMessage> crMsg = CreateObject<SatCrMessage> ();

      // Go through the RC indices
      for (uint8_t rc = 0; rc < m_llsConf->GetDaServiceCount (); ++rc)
        {
          if (m_queueCallbacks.find (rc) != m_queueCallbacks.end ())
            {
              // Get statistics for LLC/SatQueue
              struct SatQueue::QueueStats_t stats = m_queueCallbacks.at (rc) (true);

              NS_LOG_INFO ("Evaluating the needs for RC: " << (uint32_t)(rc));
              NS_LOG_INFO ("RC: " << (uint32_t)(rc) << " incoming rate: " << stats.m_incomingRateKbps << " kbps");
              NS_LOG_INFO ("RC: " << (uint32_t)(rc) << " outgoing rate: " << stats.m_outgoingRateKbps << " kbps");
              NS_LOG_INFO ("RC: " << (uint32_t)(rc) << " volume in: " << stats.m_volumeInBytes << " bytes");
              NS_LOG_INFO ("RC: " << (uint32_t)(rc) << " volume out: " << stats.m_volumeOutBytes << " bytes");
              NS_LOG_INFO ("RC: " << (uint32_t)(rc) << " total queue size: " << stats.m_queueSizeBytes << " bytes");

              // RBDC only
              if (m_llsConf->GetDaRbdcAllowed (rc) && !m_llsConf->GetDaVolumeAllowed (rc))
                {
                  NS_LOG_INFO ("Evaluating RBDC needs for RC: " << (uint32_t)(rc));
                  uint32_t rbdcRateKbps = DoRbdc (rc, stats);

                  NS_LOG_INFO ("Requested RBDC rate for RC: " << (uint32_t)(rc) << " is " << rbdcRateKbps << " kbps");

                  if (rbdcRateKbps > 0)
                    {
                      // Add control element only if UT needs some rate
                      crMsg->AddControlElement (rc, SatEnums::DA_RBDC, rbdcRateKbps);

                      std::stringstream ss;
                      ss << Simulator::Now ().GetSeconds () << ", "
                         << m_nodeInfo->GetNodeId () << ", "
                         << static_cast<uint32_t> (rc) << ", "
                         << SatEnums::GetCapacityAllocationCategory (SatEnums::DA_RBDC) << ", "
                         << rbdcRateKbps << ", "
                         << stats.m_queueSizeBytes;
                      m_crTraceLog (ss.str ());
                      m_rbdcTrace (rbdcRateKbps);
                    }
                }

              // VBDC only
              else if (m_llsConf->GetDaVolumeAllowed (rc) && !m_llsConf->GetDaRbdcAllowed (rc))
                {
                  NS_LOG_INFO ("Evaluation VBDC for RC: " << (uint32_t)(rc));

                  uint32_t vbdcBytes (0);

                  SatEnums::SatCapacityAllocationCategory_t cac = DoVbdc (rc, stats, vbdcBytes);

                  if (vbdcBytes > 0)
                    {
                      // Add control element only if UT needs some bytes
                      crMsg->AddControlElement (rc, cac, vbdcBytes);

                      // Update the time when VBDC CR is sent
                      m_lastVbdcCrSent = Simulator::Now ();

                      std::stringstream ss;
                      ss << Simulator::Now ().GetSeconds () << ", "
                         << m_nodeInfo->GetNodeId () << ", "
                         << static_cast<uint32_t> (rc) << ", "
                         << SatEnums::GetCapacityAllocationCategory (cac) << ", "
                         << vbdcBytes << ", "
                         << stats.m_queueSizeBytes;
                      m_crTraceLog (ss.str ());

                      if (cac == SatEnums::DA_AVBDC)
                        {
                          m_aVbdcTrace (vbdcBytes);
                        }
                      else
                        {
                          m_vbdcTrace (vbdcBytes);
                        }
                    }

                  NS_LOG_INFO ("Requested VBDC volume for RC: " << (uint32_t)(rc) << " is " << vbdcBytes << " Bytes with CAC: " << cac);
                }

              // RBDC + VBDC
              else if (m_llsConf->GetDaRbdcAllowed (rc) && m_llsConf->GetDaVolumeAllowed (rc))
                {
                  NS_LOG_INFO ("Evaluation RBDC+VBDC for RC: " << (uint32_t)(rc));

                  /**
                   * TODO: Add the functionality here which handles a RC with both RBDC and
                   * VBDC enabled at the same time!
                   */
                  NS_FATAL_ERROR ("Simultaneous RBDC and VBDC for one RC is not currently supported!");
                }
              // No dynamic DA configured
              else
                {
                  NS_LOG_INFO ("RBDC nor VBDC was configured for RC: " << (uint32_t)(rc));
                }

              // Update evaluation time
              m_previousEvaluationTime.at (rc) = Simulator::Now ();
            }
        }

      // If CR has some valid elements
      if (crMsg->IsNotEmpty ())
        {
          NS_LOG_INFO ("Send CR");

          SendCapacityRequest (crMsg);
        }

      ResetAssignedResources ();
    }
  else
    {
      NS_LOG_INFO ("No transmission possibility, thus skipping CR evaluation!");
    }

  NS_LOG_INFO ("---End request manager evaluation---");
}


void
SatRequestManager::AddQueueCallback (uint8_t rcIndex, SatRequestManager::QueueCallback cb)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rcIndex) << &cb);
  m_queueCallbacks.insert (std::make_pair (rcIndex, cb));
}

void
SatRequestManager::SetCtrlMsgCallback (SatRequestManager::SendCtrlCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_ctrlCallback = cb;
}

void
SatRequestManager::SetCtrlMsgTxPossibleCallback (SatRequestManager::CtrlMsgTxPossibleCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_ctrlMsgTxPossibleCallback = cb;
}

void
SatRequestManager::SetGwAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_gwAddress = address;
}

void
SatRequestManager::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this);

  m_nodeInfo = nodeInfo;
}

void
SatRequestManager::CnoUpdated (uint32_t beamId, Address /*utId*/, Address /*gwId*/, double cno)
{
  NS_LOG_FUNCTION (this << beamId << cno);

  NS_LOG_INFO ("C/No updated to request manager: " << cno);

  m_lastCno = cno;
}


uint32_t
SatRequestManager::DoRbdc (uint8_t rc, const SatQueue::QueueStats_t &stats)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  // Duration from last evaluation time
  Time duration = Simulator::Now () - m_previousEvaluationTime.at (rc);

  // Calculate the raw RBDC request
  //double gainValueK = 1.0 / (2.0 * duration.GetSeconds ());

  // This round kbits
  double inRateKbps = m_overEstimationFactor * stats.m_incomingRateKbps;
  double thisRbdcInKbits = inRateKbps * duration.GetSeconds ();
  double previousRbdcInKbits = GetPendingRbdcSumKbps (rc) * duration.GetSeconds ();
  double queueSizeInKbits = SatConstVariables::BITS_PER_BYTE * stats.m_queueSizeBytes / (double)(SatConstVariables::BITS_IN_KBIT);

  double queueOccupancy = std::max (0.0, m_gainValueK * (queueSizeInKbits - thisRbdcInKbits - previousRbdcInKbits) / duration.GetSeconds ());

  double reqRbdcKbps = inRateKbps + queueOccupancy;

  NS_LOG_INFO ("queueSizeInKbits: " << queueSizeInKbits << " thisRbdcInKbits: " << thisRbdcInKbits << " previousRbdcInKbits " << previousRbdcInKbits);
  NS_LOG_INFO ("gainValueK: " << m_gainValueK);
  NS_LOG_INFO ("In rate: " << inRateKbps << " queueOccupancy: " << queueOccupancy);

  // If CRA enabled, substract the CRA bitrate from the calculated RBDC bitrate
  if (m_llsConf->GetDaConstantAssignmentProvided (rc))
    {
      // If CRA is sufficient, no RBDC needed
      if (reqRbdcKbps <= m_llsConf->GetDaConstantServiceRateInKbps (rc))
        {
          reqRbdcKbps = 0.0;
        }
      // Else reduce the CRA from RBDC request
      else
        {
          reqRbdcKbps -= m_llsConf->GetDaConstantServiceRateInKbps (rc);
        }

      if (m_llsConf->GetDaConstantServiceRateInKbps (rc) > m_llsConf->GetDaMaximumServiceRateInKbps (rc))
        {
          NS_FATAL_ERROR ("SatRequestManager::DoRbdc - configured CRA is bigger than maximum RBDC for RC: " << uint32_t (rc));
        }

      // CRA + RBDC is too much
      if ((m_llsConf->GetDaConstantServiceRateInKbps (rc) + reqRbdcKbps) > m_llsConf->GetDaMaximumServiceRateInKbps (rc))
        {
          reqRbdcKbps = m_llsConf->GetDaMaximumServiceRateInKbps (rc) - m_llsConf->GetDaConstantServiceRateInKbps (rc);
        }
    }
  // CRA is disabled, but check that RBDC request is not by itself going over max service rate.
  else if (reqRbdcKbps > m_llsConf->GetDaMaximumServiceRateInKbps (rc))
    {
      reqRbdcKbps = m_llsConf->GetDaMaximumServiceRateInKbps (rc);
    }

  NS_LOG_INFO ("RBDC bitrate after CRA as been taken off: " << reqRbdcKbps << " kbps");

  uint32_t crRbdcKbps = GetQuantizedRbdcValue (rc, reqRbdcKbps);

  NS_LOG_INFO ("Quantized RBDC bitrate: " << crRbdcKbps << " kbps");

  UpdatePendingRbdcCounters (rc, crRbdcKbps);

  return crRbdcKbps;
}


SatEnums::SatCapacityAllocationCategory_t
SatRequestManager::DoVbdc (uint8_t rc, const SatQueue::QueueStats_t &stats, uint32_t &rcVbdcBytes)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  uint32_t vbdcBytes (0);

  SatEnums::SatCapacityAllocationCategory_t cac = SatEnums::DA_VBDC;

  if (m_forcedAvbdcUpdate == true)
    {
      cac = SatEnums::DA_AVBDC;
      vbdcBytes = GetAvbdcBytes (rc, stats);
    }
  else
    {
      // If there is volume in, there is no need to ask for additional resources
      if (stats.m_volumeInBytes > 0)
        {
          NS_LOG_INFO ("VBDC volume in for RC: " << (uint32_t)(rc) << ": " << stats.m_volumeInBytes << " Bytes");

          // If we assume that we have received all requested resources,
          // send AVBDC request with total queue size.
          if (m_pendingVbdcBytes.at (rc) == 0)
            {
              cac = SatEnums::DA_AVBDC;
              vbdcBytes = GetAvbdcBytes (rc, stats);
            }
          else
            {
              cac = SatEnums::DA_VBDC;
              vbdcBytes = GetVbdcBytes (rc, stats);
            }
        }
    }
  // Return the VBDC as referenced uint32_t
  rcVbdcBytes = vbdcBytes;

  return cac;
}

uint32_t
SatRequestManager::GetAvbdcBytes (uint8_t rc, const SatQueue::QueueStats_t &stats)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  Reset (rc);

  uint32_t craBytes (0);
  uint32_t vbdcBytes = m_overEstimationFactor * stats.m_queueSizeBytes;

  // If CRA enabled, substract the CRA Bytes from VBDC
  if (m_llsConf->GetDaConstantAssignmentProvided (rc))
    {
      NS_LOG_INFO ("CRA is enabled together with VBDC for RC: " << (uint32_t)(rc));

      // Duration from last evaluation time
      Time duration = Simulator::Now () - m_previousEvaluationTime.at (rc);

      // Calculate how much bytes would be given to this RC index with configured CRA
      craBytes = (uint32_t)((SatConstVariables::BITS_IN_KBIT * m_llsConf->GetDaConstantServiceRateInKbps (rc) * duration.GetSeconds ())
                            / (double)(SatConstVariables::BITS_PER_BYTE));
    }

  // If there is still need for Bytes after CRA
  if (craBytes < vbdcBytes)
    {
      vbdcBytes -= craBytes;
      vbdcBytes = GetQuantizedVbdcValue (rc, vbdcBytes);

      // Update the pending counters
      m_pendingVbdcBytes.at (rc) = vbdcBytes;

      NS_LOG_INFO ("Pending VBDC bytes: " << (uint32_t)(rc) << ": " << m_pendingVbdcBytes.at (rc) << " Bytes");
    }
  else
    {
      vbdcBytes = 0;
    }

  return vbdcBytes;
}

uint32_t
SatRequestManager::GetVbdcBytes (uint8_t rc, const SatQueue::QueueStats_t &stats)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  uint32_t craBytes (0);
  uint32_t vbdcBytes = m_overEstimationFactor * stats.m_volumeInBytes;

  // If CRA enabled, substract the CRA Bytes from VBDC
  if (m_llsConf->GetDaConstantAssignmentProvided (rc))
    {
      NS_LOG_INFO ("CRA is enabled together with VBDC for RC: " << (uint32_t)(rc));

      // Duration from last evaluation time
      Time duration = Simulator::Now () - m_previousEvaluationTime.at (rc);

      // Calculate how much bytes would be given to this RC index with configured CRA
      craBytes = (uint32_t)((SatConstVariables::BITS_IN_KBIT * m_llsConf->GetDaConstantServiceRateInKbps (rc) * duration.GetSeconds ())
                            / (double)(SatConstVariables::BITS_PER_BYTE));
    }

  // If there is still need for Bytes after CRA
  if (craBytes < vbdcBytes)
    {
      vbdcBytes -= craBytes;

      NS_LOG_INFO ("VBDC volume after CRA for RC: " << (uint32_t)(rc) << ": " << vbdcBytes << " Bytes");

      vbdcBytes = GetQuantizedVbdcValue (rc, vbdcBytes);
      m_pendingVbdcBytes.at (rc) = m_pendingVbdcBytes.at (rc) + vbdcBytes;

      NS_LOG_INFO ("Pending VBDC bytes: " << (uint32_t)(rc) << ": " << m_pendingVbdcBytes.at (rc) << " Bytes");
      NS_LOG_INFO ("VBDC volume after pending: " << (uint32_t)(rc) << ": " << vbdcBytes << " Bytes");
    }
  else
    {
      vbdcBytes = 0;
    }

  return vbdcBytes;
}

void
SatRequestManager::CheckForVolumeBacklogPersistence ()
{
  NS_LOG_FUNCTION (this);

  // Volume backlog shall expire at the NCC?
  // Last sent CR + backlog persistence duration is smaller than now
  if ((m_lastVbdcCrSent + (m_llsConf->GetVolumeBacklogPersistence () - 1) * m_superFrameDuration) < Simulator::Now ())
    {
      // Go through all RC indeces
      for (std::vector<uint32_t>::iterator it = m_pendingVbdcBytes.begin ();
           it != m_pendingVbdcBytes.end ();
           ++it)
        {
          // Check if UT is still in need for resources
          if (*it > 0)
            {
              // Volume backlog is about to expire, and we still have buffered data,
              // send forced AVBDC
              m_forcedAvbdcUpdate = true;
              return;
            }
        }
    }

  // No need to send forced AVBDC
  m_forcedAvbdcUpdate = false;
  return;
}

uint32_t
SatRequestManager::GetPendingRbdcSumKbps (uint8_t rc)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  RemoveOldEntriesFromPendingRbdcContainer (rc);

  uint32_t value (0);
  std::deque<std::pair<Time, uint32_t> > cont = m_pendingRbdcRequestsKbps.at (rc);

  for (std::deque<std::pair<Time, uint32_t> >::const_iterator it = cont.begin ();
       it != cont.end ();
       ++it)
    {
      // Add the kbps
      value += (*it).second;
    }

  NS_LOG_INFO ("Pending RBDC sum for RC: " << (uint32_t)(rc) << " is " << value);

  return value;
}


void
SatRequestManager::UpdatePendingRbdcCounters (uint8_t rc, uint32_t kbps)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc) << kbps);

  if (kbps > 0)
    {
      Time now = Simulator::Now ();
      std::pair<Time, uint32_t> item = std::make_pair (now, kbps);
      m_pendingRbdcRequestsKbps.at (rc).push_back (item);
    }
}

void
SatRequestManager::RemoveOldEntriesFromPendingRbdcContainer (uint8_t rc)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  std::deque<std::pair<Time, uint32_t> > &cont = m_pendingRbdcRequestsKbps.at (rc);
  std::deque<std::pair<Time, uint32_t> >::const_iterator it = cont.begin ();

  while (it != cont.end ())
    {
      if ((*it).first < (Simulator::Now () - m_rttEstimate))
        {
          cont.pop_front ();
          it = cont.begin ();
        }
      else
        {
          break;
        }
    }
}

void
SatRequestManager::UpdatePendingVbdcCounters ()
{
  NS_LOG_FUNCTION (this);

  for (uint8_t rc = 0; rc < m_llsConf->GetDaServiceCount (); rc++)
    {
      UpdatePendingVbdcCounters (rc);
    }
}

void
SatRequestManager::UpdatePendingVbdcCounters (uint8_t rc)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  /*
   * m_pendingVbdcBytes is updated with requested bytes and reduce by allocated
   * bytes via TBTP. This information comes from UT MAC.
   * m_assignedDaResources holds the amount of resources allocated during the previous
   * superframe.
   */

  if (m_pendingVbdcBytes.at (rc) > m_assignedDaResourcesBytes.at (rc))
    {
      m_pendingVbdcBytes.at (rc) = m_pendingVbdcBytes.at (rc) - m_assignedDaResourcesBytes.at (rc);
      m_assignedDaResourcesBytes.at (rc) = 0;
    }
  else
    {
      m_pendingVbdcBytes.at (rc) = 0;
      m_assignedDaResourcesBytes.at (rc) = 0;
    }
}

void
SatRequestManager::SendCapacityRequest (Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this);

  // Cancel the C/No report event
  m_cnoReportEvent.Cancel ();

  if ( !m_ctrlCallback.IsNull ())
    {
      NS_LOG_INFO ("Send C/N0 report to GW: " << m_gwAddress);

      m_crTrace (Simulator::Now (), m_nodeInfo->GetMacAddress (), crMsg);

      crMsg->SetCnoEstimate (m_lastCno);
      m_ctrlCallback (crMsg, m_gwAddress);

      m_lastCno = NAN;
    }
  else
    {
      NS_FATAL_ERROR ("Unable to send capacity request, since the Ctrl callback is NULL!");
    }

  // Re-schedule the C/No report event
  m_cnoReportEvent = Simulator::Schedule (m_cnoReportInterval, &SatRequestManager::SendCnoReport, this);
}

void
SatRequestManager::SendCnoReport ()
{
  NS_LOG_FUNCTION (this);

  if (!m_ctrlCallback.IsNull ())
    {
      // Check if we have the possiblity to send a ctrl msg
      bool ctrlMsgTxPossible = m_ctrlMsgTxPossibleCallback ();

      if (ctrlMsgTxPossible && m_lastCno != NAN)
        {
          NS_LOG_INFO ("Send C/No report to GW: " << m_gwAddress);

          Ptr<SatCnoReportMessage> cnoReport = CreateObject<SatCnoReportMessage> ();

          cnoReport->SetCnoEstimate (m_lastCno);
          m_ctrlCallback (cnoReport, m_gwAddress);

          m_lastCno = NAN;
        }
    }

  m_cnoReportEvent = Simulator::Schedule (m_cnoReportInterval, &SatRequestManager::SendCnoReport, this);
}

void
SatRequestManager::AssignedDaResources (uint8_t rcIndex, uint32_t bytes)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rcIndex) << bytes);

  NS_LOG_INFO ("TBTP resources assigned for RC: " << (uint32_t)(rcIndex) << " bytes: " << bytes);

  m_assignedDaResourcesBytes.at (rcIndex) = m_assignedDaResourcesBytes.at (rcIndex) + bytes;
}

void
SatRequestManager::ResetAssignedResources ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_llsConf->GetDaServiceCount (); ++i)
    {
      m_assignedDaResourcesBytes.at (i) = 0;
    }
}

void
SatRequestManager::Reset (uint8_t rc)
{
  NS_LOG_FUNCTION (this << (uint32_t)(rc));

  m_assignedDaResourcesBytes.at (rc) = 0;
  m_pendingVbdcBytes.at (rc) = 0;
}

uint16_t
SatRequestManager::GetQuantizedRbdcValue (uint8_t index, uint16_t reqRbdcKbps) const
{
  NS_LOG_FUNCTION (this << (uint32_t)(index) << reqRbdcKbps);

  uint32_t maxRbdc = m_llsConf->GetDaMaximumServiceRateInKbps (index);
  uint32_t quantValue (0);

  // Maximum configured RBDC rate
  if (reqRbdcKbps > maxRbdc)
    {
      return maxRbdc;
    }

  // Else quantize based on the predefined scaling factors from the specification
  for (uint32_t i = 0; i < 4; i++)
    {
      // If the value can be represented with this scaling value
      if (reqRbdcKbps <= m_rbdcScalingFactors[i] * m_numValues)
        {
          quantValue = (uint16_t)( ceil ( reqRbdcKbps / (double)(m_rbdcScalingFactors[i])) * m_rbdcScalingFactors[i] );
          return quantValue;
        }
    }

  NS_FATAL_ERROR ("Quantized value for RBDC not calculated!");

  return quantValue;
}

uint16_t
SatRequestManager::GetQuantizedVbdcValue (uint8_t index, uint16_t reqVbdcBytes) const
{
  NS_LOG_FUNCTION (this << (uint32_t)(index) << reqVbdcBytes);

  uint32_t maxBacklogBytes = SatConstVariables::BYTES_IN_KBYTE * m_llsConf->GetDaMaximumBacklogInKbytes (index);
  uint32_t quantValue (0);

  // If maximum backlog reached
  if (reqVbdcBytes > maxBacklogBytes)
    {
      return maxBacklogBytes;
    }

  // Else quantize based on the predefined scaling factors from the specification
  for (uint32_t i = 0; i < 4; ++i)
    {

      // If the value can be represented with this scaling value
      if (reqVbdcBytes <= m_vbdcScalingFactors[i] * m_numValues)
        {
          quantValue = (uint16_t)( ceil ( reqVbdcBytes / (double)(m_vbdcScalingFactors[i])) * m_vbdcScalingFactors[i] );
          return quantValue;
        }
    }

  NS_FATAL_ERROR ("Quantized value for VBDC not calculated!");

  return quantValue;
}


}

