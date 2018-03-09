/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Joaquin Muguerza <jmuguerza@toulouse.viveris.fr>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include <ns3/boolean.h>

#include "satellite-utils.h"
#include "satellite-wave-form-conf.h"
#include "satellite-link-results.h"
#include "satellite-mutual-information-table.h"
#include "satellite-phy-rx-carrier-per-window.h"

#include <algorithm>
#include <ostream>
#include <limits>
#include <utility>

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierPerWindow");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierPerWindow);

SatPhyRxCarrierPerWindow::SatPhyRxCarrierPerWindow (uint32_t carrierId,
                                                    Ptr<SatPhyRxCarrierConf> carrierConf,
                                                    Ptr<SatWaveformConf> waveformConf,
                                                    bool randomAccessEnabled)
  : SatPhyRxCarrierPerSlot (carrierId, carrierConf, waveformConf, randomAccessEnabled),
  m_windowDuration (Seconds (0.004)),
  m_windowStep (Seconds (0.0005)),
  m_windowSicIterations (10),
  m_windowEndSchedulingInitialized (false),
  m_detectionThreshold (0.0)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Constructor called with arguments " << carrierId << ", " << carrierConf << ", and " << randomAccessEnabled);

  NS_ASSERT (m_randomAccessEnabled == true);
}

void
SatPhyRxCarrierPerWindow::BeginEndScheduling ()
{
  NS_LOG_FUNCTION (this);
  if (!m_windowEndSchedulingInitialized)
    {
      if (GetNodeInfo () == NULL)
        {
          NS_FATAL_ERROR ("SatPhyRxWindow::BeginWindowEndScheduling - m_nodeInfo not set");
        }

      m_windowEndSchedulingInitialized = true;

      Simulator::ScheduleWithContext (GetNodeInfo ()->GetNodeId (), m_windowDuration, &SatPhyRxCarrierPerWindow::DoWindowEnd, this);
    }
}


SatPhyRxCarrierPerWindow::~SatPhyRxCarrierPerWindow ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
SatPhyRxCarrierPerWindow::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierPerWindow")
    .SetParent<SatPhyRxCarrierPerSlot> ()
    .AddAttribute ( "WindowDuration",
                    "The duration of the sliding window",
                    TimeValue (Seconds (0.004)),
                    MakeTimeAccessor (&SatPhyRxCarrierPerWindow::m_windowDuration),
                    MakeTimeChecker ())
    .AddAttribute ( "WindowStep",
                    "The length of the step between two window iterations",
                    TimeValue (Seconds (0.0005)),
                    MakeTimeAccessor (&SatPhyRxCarrierPerWindow::m_windowStep),
                    MakeTimeChecker ())
    .AddAttribute ( "WindowSICIterations",
                    "The number of SIC iterations performed on each window",
                    UintegerValue (10),
                    MakeUintegerAccessor (&SatPhyRxCarrierPerWindow::m_windowSicIterations),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "DetectionThreshold",
                    "The SNIR Detection Threshold (in magnitude) for a packet",
                    DoubleValue (0.0),
                    MakeDoubleAccessor (&SatPhyRxCarrierPerWindow::m_detectionThreshold),
                    MakeDoubleChecker<double> (0, std::numeric_limits<double>::max ()))
    .AddTraceSource ("EssaRxCollision",
                     "Received a packet through Random Access ESSA",
                     MakeTraceSourceAccessor (&SatPhyRxCarrierPerWindow::m_essaRxCollisionTrace),
                     "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
  ;
  return tid;
}


void
SatPhyRxCarrierPerWindow::DoDispose ()
{
  SatPhyRxCarrierPerSlot::DoDispose ();
}

void
SatPhyRxCarrierPerWindow::ReceiveSlot (SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets)
{
  NS_ASSERT (packetRxParams.rxParams->m_txInfo.packetType != SatEnums::PACKET_TYPE_DEDICATED_ACCESS);

  if (packetRxParams.rxParams->m_txInfo.packetType != SatEnums::PACKET_TYPE_ESSA)
    {
      NS_LOG_ERROR ("SatPhyRxCarrierPerWindow::ReceiveSlot - Time: " << Now ().GetSeconds () << " - Non ESSA packet received");
      return;
    }

  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::ReceiveSlot - Time: " << Now ().GetSeconds () << " - ESSA packet received");

  SatPhyRxCarrierPerWindow::essaPacketRxParams_s params;

  params.destAddress = packetRxParams.destAddress;
  params.sourceAddress = packetRxParams.sourceAddress;
  params.duration = packetRxParams.rxParams->m_duration;
  params.arrivalTime = Now () - params.duration; // arrival time is the start of reception
  params.rxParams = packetRxParams.rxParams;
  params.packetHasBeenProcessed = false;

  // Calculate SINR, gamma and ifPowerPerFragment
  CalculatePacketInterferenceVectors (params);

  if (nPackets > 0)
    {
      // TODO
      m_essaRxCollisionTrace (nPackets,                     // number of packets
                              params.sourceAddress,         // sender address
                              false);         // collision flag
    }

  AddEssaPacket (params);
}

void
SatPhyRxCarrierPerWindow::CalculatePacketInterferenceVectors (SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::CalculatePacketInterferenceVectors");

  // TODO: should we check the interference model used ?
  // TODO: should we check the collision model used (check against sinr, collision always drops, etc) ?

  /// Calculate the SNR
  double snrSatellite = CalculateSinr ( packet.rxParams->m_rxPowerInSatellite_W,
                                        0.0,
                                        packet.rxParams->m_rxNoisePowerInSatellite_W,
                                        packet.rxParams->m_rxAciIfPowerInSatellite_W,
                                        packet.rxParams->m_rxExtNoisePowerInSatellite_W,
                                        packet.rxParams->m_sinrCalculate);

  double snr = CalculateSinr ( packet.rxParams->m_rxPower_W,
                               0.0,
                               m_rxNoisePowerW,
                               m_rxAciIfPowerW,
                               m_rxExtNoisePowerW,
                               m_sinrCalculate);

  double cSnr = CalculateCompositeSinr (snr, snrSatellite);

  /// Calculate the gamma vector
  for (std::vector< std::pair<double, double> >::iterator it = packet.rxParams->GetInterferencePowerPerFragment ().begin (); it != packet.rxParams->GetInterferencePowerPerFragment ().end (); it++)
    {
      /// gamma[k] = ( SNR^-1 + (Interference[k]/C)^-1 )^-1
      double gamma = 1.0 / (1.0 / cSnr + packet.rxParams->m_rxPower_W / it->second);
      packet.gamma.emplace_back (it->first, gamma);
    }

  /// Calculate the gamma vector for the preamble
  std::vector< std::pair<double, double> > gammaPreamble;
  Ptr<SatWaveform> wf = GetWaveformConf ()->GetWaveform (packet.rxParams->m_txInfo.waveformId);
  double normalizedTime = 0.0;
  double normalizedPreambleTime = wf->GetPreambleLengthInSymbols () / wf->GetBurstLengthInSymbols (); // This asumes that preamble and burst have the same symbol rate
  for (std::vector< std::pair<double, double> >::iterator it = packet.gamma.begin (); it != packet.gamma.end (); it++)
    {
      if (normalizedTime + it->first > normalizedPreambleTime)
        {
          gammaPreamble.emplace_back (1.0 - normalizedTime, it->second);
          break;
        }
      gammaPreamble.emplace_back (it->first / normalizedPreambleTime, it->second);
      normalizedTime += it->first;
    }

  /// Calculate the mean SNIR
  packet.meanSinr = SatUtils::ScalarProduct (packet.gamma);
  packet.preambleMeanSinr = SatUtils::ScalarProduct (gammaPreamble);

}

void
SatPhyRxCarrierPerWindow::DoWindowEnd ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoWindowEnd");

  ProcessWindow (Now () - m_windowDuration, Now ());

  // Advance the window (schedule DoWindowEnd for m_windowStep)
  Simulator::Schedule (m_windowStep, &SatPhyRxCarrierPerWindow::DoWindowEnd, this);
}

void
SatPhyRxCarrierPerWindow::ProcessWindow (Time startTime, Time endTime)
{
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoProcessWindow - Process window between " << startTime.GetSeconds () << " and " << endTime.GetSeconds ());

  /// Clean old packets
  CleanOldPackets (startTime);

  /// Get packets in window
  std::pair<packetList_t::iterator, packetList_t::iterator> windowBounds = GetWindowBounds ();

  uint32_t i = 0;
  while (i < m_windowSicIterations)
    {
      /// Select packet to decode (block 2)
      packetList_t::iterator packet_it = GetHighestSnirPacket (windowBounds);
      if (packet_it == windowBounds.second)
        {
          // No more packets to decode
          break;
        }
      NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoWindowEnd - Process packet " << packet_it->rxParams->m_txInfo.crdsaUniquePacketId << " from " << packet_it->sourceAddress);
      /// MIESM (block 3)
      /// Get effective SINR
      double sinrEffective = GetEffectiveSnir (*packet_it);
      // TODO: check if BER/PER/Snir table is correct
      bool phyError = CheckAgainstLinkResults (sinrEffective, packet_it->rxParams);
      if (!phyError)
        {
          /// SIC (block 4)
        }
      else
        {
          /// Update iteration
          i++;
        }
      /// send packet upwards
      m_rxCallback (packet_it->rxParams, phyError);
      /// Packet could now be deleted, since interference information
      /// is stored on each packet; but for logging purposes we'll keep it
      packet_it->packetHasBeenProcessed = true;
    }

  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoWindowEnd - Window processing finished");
}

double
SatPhyRxCarrierPerWindow::GetEffectiveSnir (const SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetEffectiveSnir");

  Ptr<SatMutualInformationTable> mutualInformationTable = (GetLinkResults ()->GetObject <SatLinkResultsFSim> ())->GetMutualInformationTable ();
  double beta = mutualInformationTable->GetBeta ();

  double meanMutualInformation = 0.0;
  for (std::vector< std::pair<double, double> >::const_iterator it = packet.gamma.begin (); it != packet.gamma.end (); it++)
    {
      meanMutualInformation += it->first * mutualInformationTable->GetNormalizedSymbolInformation (SatUtils::LinearToDb (it->second / beta));
    }

  double effectiveSnir = SatUtils::DbToLinear (beta * mutualInformationTable->GetSnirDb (meanMutualInformation));

  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetEffectiveSnir - Effective SNIR : " << effectiveSnir);

  return effectiveSnir;
}

void
SatPhyRxCarrierPerWindow::CleanOldPackets (const Time windowStartTime)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::CleanOldPackets");

  packetList_t::iterator it = m_essaPacketContainer.begin ();
  while (it != m_essaPacketContainer.end ())
    {
      /// Check first if packet is not entirely in window (not an old packet)
      Time offset = it->arrivalTime - windowStartTime;
      if (offset.IsStrictlyPositive ())
        {
          break;
        }
      /// Check if packet is previous to window
      offset = it->arrivalTime + it->duration - windowStartTime;
      if (!offset.IsStrictlyPositive ())
        {
          // Delete element from list
          NS_LOG_INFO ("SatPhyRxCarrierPerWindow::CleanOldPackets - Remove packet " << it->rxParams->m_txInfo.crdsaUniquePacketId << " from " << it->sourceAddress);
          it->rxParams = NULL;
          m_essaPacketContainer.erase (it++);
        }
      else
        {
          ++it;
        }
    }
}

std::pair<SatPhyRxCarrierPerWindow::packetList_t::iterator, SatPhyRxCarrierPerWindow::packetList_t::iterator>
SatPhyRxCarrierPerWindow::GetWindowBounds ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetWindowBounds");

  packetList_t::iterator last;

  for (last = m_essaPacketContainer.begin (); last != m_essaPacketContainer.end (); last++)
    {
      const Time windowStartTime = Now () - m_windowDuration;
      /// Check first if packet is not after window
      Time offset = last->arrivalTime - Now ();
      if (offset.IsStrictlyPositive ())
        {
          break;
        }
      NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetWindowBounds - Packet " << last->rxParams->m_txInfo.crdsaUniquePacketId << " from " << last->sourceAddress << " is inside the window");
    }

  /// First packet will always be the first of the list, since previous packets were deleted in
  /// CleanOldPackets
  return std::make_pair (m_essaPacketContainer.begin (), last);
}

SatPhyRxCarrierPerWindow::packetList_t::iterator
SatPhyRxCarrierPerWindow::GetHighestSnirPacket (const std::pair<SatPhyRxCarrierPerWindow::packetList_t::iterator, SatPhyRxCarrierPerWindow::packetList_t::iterator> windowBounds)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetHighestSnirPacket");

  SatPhyRxCarrierPerWindow::packetList_t::iterator it, max = windowBounds.second;
  for (it = windowBounds.first; it != windowBounds.second; it++)
    {
      if (it->packetHasBeenProcessed)
        {
          continue;
        }
      /// Check If packet can be detected
      if (!PacketCanBeDetected (*it))
        {
          continue;
        }
      /// Check if bigger SINR
      if ((max == windowBounds.second) || (it->meanSinr > max->meanSinr))
        {
          max = it;
        }
    }

  return max;
}

bool
SatPhyRxCarrierPerWindow::PacketCanBeDetected (const SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::PacketCanBeDetected");

  return (packet.preambleMeanSinr >= m_detectionThreshold);
}

void
SatPhyRxCarrierPerWindow::AddEssaPacket (SatPhyRxCarrierPerWindow::essaPacketRxParams_s essaPacketParams)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::AddEssaPacket - Time: " << Now ().GetSeconds ());

  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::AddEssaPacket - Add packet " << essaPacketParams.rxParams->m_txInfo.crdsaUniquePacketId << " from " << essaPacketParams.sourceAddress << " Arrival Time: " << essaPacketParams.arrivalTime.GetSeconds () << " Duration: " << essaPacketParams.duration.GetSeconds ());

  /// Insert received packet in packets container
  m_essaPacketContainer.push_back (essaPacketParams);
}

}
