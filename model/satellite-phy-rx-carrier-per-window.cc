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
    .AddTraceSource ("EssaRxError",
                     "Received a packet through Random Access ESSA",
                     MakeTraceSourceAccessor (&SatPhyRxCarrierPerWindow::m_essaRxErrorTrace),
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
  params.sicFlag = false;
  params.hasBeenUpdated = false;
  params.meanSinr = -1.0;
  params.preambleMeanSinr = -1.0;

  // Calculate SINR, gamma and ifPowerPerFragment
  CalculatePacketInterferenceVectors (params);

  AddEssaPacket (params);
}

void
SatPhyRxCarrierPerWindow::CalculatePacketInterferenceVectors (SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::CalculatePacketInterferenceVectors - Packet " << packet.rxParams->m_txInfo.crdsaUniquePacketId << " from " << packet.sourceAddress);

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

  /// Update probes (only the first time we access this method for this packet)
  if (packet.meanSinr < 0.0)
    {
      // Update link specific SINR trace
      m_linkSinrTrace (SatUtils::LinearToDb (snr));

      // Update composite SNR trace for DAMA and Slotted ALOHA packets
      // m_sinrTrace (SatUtils::LinearToDb (cSnr), packet.sourceAddress); // Done with effective SINR

      m_linkBudgetTrace (packet.rxParams, GetOwnAddress (),
                         packet.destAddress,
                         0.0, cSnr);

      /// composite sinr output trace
      if (IsCompositeSinrOutputTraceEnabled () && (packet.meanSinr == 0.0))
        {
          DoCompositeSinrOutputTrace (cSnr);
        }

      NS_LOG_INFO ("SatPhyRxCarrierPerWindow::CalculatePacketInterferenceVectors - Received packet SNR dB = " << SatUtils::LinearToDb (cSnr));
    }

  /// Calculate the gamma vector
  packet.gamma.clear ();
  /// Consider both the interference per fragment in the satellite, and the
  /// interference per fragment in the feeder. Since interference at the
  /// feeder does not consider intrabeam interference, the fragments
  /// will always be a subset of fragments in satellite (TODO: CHECK THIS).
  /// TODO: optimize. Find a way to do this without creating copy.
  std::vector< std::pair<double, double> > interferencePowerPerFragment = packet.rxParams->GetInterferencePowerPerFragment ();
  std::vector< std::pair<double, double> > interferencePowerPerFragmentInSatellite = packet.rxParams->GetInterferencePowerInSatellitePerFragment ();
  std::vector< std::pair<double, double> >::iterator interferencePower = interferencePowerPerFragment.begin ();
  double normalizedTime = interferencePower->first, normalizedTimeInSatellite = 0.0;
  for (std::vector< std::pair<double, double> >::iterator interferencePowerInSatellite = interferencePowerPerFragmentInSatellite.begin (); interferencePowerInSatellite != interferencePowerPerFragmentInSatellite.end (); interferencePowerInSatellite++)
    {
      normalizedTimeInSatellite += interferencePowerInSatellite->first;
      /// TODO: verify. Since Interference in feeder is a subset,
      /// fragments will never be smaller than those in satellite.
      if (normalizedTimeInSatellite > normalizedTime)
        {
          interferencePower++;
          normalizedTime += interferencePower->first;
        }

      /// For each iteration:
      /// gamma[k] = ( SNR^-1 + (C/Interference[k])^-1 )^-1

      /// Calculate composite C/I = (C_u/I_u^-1 + C_d/I_d^-1)^-1
      double cI = (packet.rxParams->m_rxPowerInSatellite_W * packet.rxParams->m_rxPower_W) / (interferencePowerInSatellite->second * packet.rxParams->m_rxPower_W + interferencePower->second * packet.rxParams->m_rxPowerInSatellite_W);

      /// Calculate gamma[k]
      double gamma = 1 / (1 / cI + 1 / cSnr);
      packet.gamma.emplace_back (interferencePowerInSatellite->first, gamma);

      NS_LOG_INFO ("SatPhyRxCarrierPerWindow::CalculatePacketInterferenceVectors - Gamma fragment of duration " << interferencePowerInSatellite->first << " : " << gamma);
    }


  /// Calculate the gamma vector for the preamble
  std::vector< std::pair<double, double> > gammaPreamble;
  Ptr<SatWaveform> wf = GetWaveformConf ()->GetWaveform (packet.rxParams->m_txInfo.waveformId);
  normalizedTime = 0.0;
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

  packet.hasBeenUpdated = true;
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
      while (true)
        {
          /// Select packet to decode (block 2)
          packetList_t::iterator packet_it = GetHighestSnirPacket (windowBounds);
          if (packet_it == windowBounds.second)
            {
              NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoWindowEnd - No more packets to decode");
              break;
            }
          NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoWindowEnd - Process packet " << packet_it->rxParams->m_txInfo.crdsaUniquePacketId << " from " << packet_it->sourceAddress);
          /// MIESM (block 3)
          packet_it->hasBeenUpdated = false;
          /// Get effective SINR
          double sinrEffective = GetEffectiveSnir (*packet_it);
          m_sinrTrace (SatUtils::LinearToDb (sinrEffective), packet_it->sourceAddress);
          NS_LOG_WARN (SatUtils::LinearToDb (packet_it->meanSinr) << " " << SatUtils::LinearToDb (sinrEffective) << " " << std::abs (SatUtils::LinearToDb (packet_it->meanSinr) - SatUtils::LinearToDb (sinrEffective)));
          /// Check against link results
          bool phyError = CheckAgainstLinkResults (sinrEffective, packet_it->rxParams);
          // Trace if the packet has been decoded or not
          m_essaRxErrorTrace (1,                        // number of packets
                              packet_it->sourceAddress, // sender address
                              phyError);                // error flag
          if (!phyError)
            {
              NS_LOG_WARN (SatUtils::LinearToDb (packet_it->meanSinr) << " " << SatUtils::LinearToDb (sinrEffective) << " " << std::abs (SatUtils::LinearToDb (packet_it->meanSinr) - SatUtils::LinearToDb (sinrEffective)) << " " << SatUtils::LinearToDb (GetInterferenceEliminationModel ()->GetResidualPower (packet_it->rxParams, 256 * packet_it->meanSinr)));
              /// SIC (block 4)
              DoSic (packet_it, windowBounds);
            }
          /// send packet upwards
          m_rxCallback (packet_it->rxParams, phyError);
          /// If decoded, Packet could be deleted, since interference information
          /// is stored in each packet; but we'll keep it for logging purposes
        }

      // increase iteration number
      i++;
    }
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoWindowEnd - Window processing finished");
}

void
SatPhyRxCarrierPerWindow::DoSic (packetList_t::iterator processedPacket, std::pair<packetList_t::iterator, packetList_t::iterator> windowBounds)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoSic - eliminate interference from packet " << processedPacket->rxParams->m_txInfo.crdsaUniquePacketId << " from " << processedPacket->sourceAddress);

  // TODO: remove
//  processedPacket->sicFlag = true;
//  return;

  /// Get residual SIC power
  // double residualSicPower = GetInterferenceEliminationModel ()->GetResidualPower (processedPacket->rxParams, processedPacket->meanSinr);

  /// Update SIC on interfering packets
  for (packetList_t::iterator packet_it = windowBounds.first; packet_it != windowBounds.second; packet_it++)
    {
      /// Stop iterating for packets arriving after the processed packet
      if (packet_it->arrivalTime >= processedPacket->arrivalTime + processedPacket->duration)
        {
          break;
        }
      /// Except previous packets
      if (packet_it->arrivalTime + packet_it->duration <= processedPacket->arrivalTime)
        {
          continue;
        }
      /// Except already processed packets, and the packet being currently processed
      if (packet_it == processedPacket || packet_it->sicFlag)
        {
          continue;
        }
      NS_LOG_INFO ("SatPhyRxCarrierPerWindow::DoSic - eliminate interference with packet " << packet_it->rxParams->m_txInfo.crdsaUniquePacketId << " from " << packet_it->sourceAddress);
      /// Get normalized start and end time
      std::pair<double, double> normalizedTimes = GetNormalizedPacketInterferenceTime (*packet_it, *processedPacket);
      /// Eliminate residual interference and recalculate the packets vectors
      /// TODO: replace 256 by spreading factor
      GetInterferenceEliminationModel ()->EliminateInterferences (packet_it->rxParams, processedPacket->rxParams, 16.0 * processedPacket->meanSinr, normalizedTimes.first, normalizedTimes.second);
      CalculatePacketInterferenceVectors (*packet_it);
    }

  /// Update packet Rx power and set the SIC flag (what for ??)
  // processedPacket->rxParams->m_rxPowerInSatellite_W = residualSicPower;
  // CalculatePacketInterferenceVectors (*processedPacket);
  processedPacket->sicFlag = true;
}

std::pair<double, double>
SatPhyRxCarrierPerWindow::GetNormalizedPacketInterferenceTime (const SatPhyRxCarrierPerWindow::essaPacketRxParams_s &packet, const SatPhyRxCarrierPerWindow::essaPacketRxParams_s &interferingPacket)
{
  NS_LOG_FUNCTION (this);

  /// Get a first approach of the values
  /// (must redo calculations in the exact same order as they were
  //   done when calculating the interference vector).
  double startTimeA = std::max ( 0.0, 1.0 - ((packet.duration.GetDouble () + packet.arrivalTime.GetDouble () - interferingPacket.arrivalTime.GetDouble ()) / packet.duration.GetDouble ()));
  double endTimeA = std::min ( 1.0, 1.0 - (packet.arrivalTime.GetDouble () - interferingPacket.arrivalTime.GetDouble ()) / packet.duration.GetDouble ());
  double startTimeB = std::max ( 0.0, (interferingPacket.arrivalTime.GetDouble () - packet.arrivalTime.GetDouble ()) / packet.duration.GetDouble ());
  double endTimeB = std::min ( 1.0, 1.0 - (packet.arrivalTime.GetDouble () - interferingPacket.arrivalTime.GetDouble ()) / packet.duration.GetDouble ());

  /// Now get the actual values from the Interference vector (there may be
  //  some rounding errors)
  double normalizedTime = 0.0;
  double exactStartTime = (startTimeA == 0.0) ? 0.0 : -1.0;
  double exactEndTime = -1.0;
  auto ifPowerPerFragment = packet.rxParams->GetInterferencePowerInSatellitePerFragment ();
  for (std::pair<double, double>& ifPower : ifPowerPerFragment)
    {
      normalizedTime += ifPower.first;
      if ((exactStartTime < 0) && (normalizedTime == startTimeA || normalizedTime == startTimeB))
        {
          exactStartTime = normalizedTime;
        }
      if ((exactEndTime < 0) && (normalizedTime == endTimeA || normalizedTime == endTimeB))
        {
          exactEndTime = normalizedTime;
        }
    }

  if ((exactStartTime < 0) || (exactEndTime < 0))
    {
      NS_FATAL_ERROR ("Cannot find exact interference time between two packets");
    }

  return std::make_pair (exactStartTime, exactEndTime);
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
      // TODO: REMOVE !
      NS_LOG_INFO ("gamma vector : " << it->first << " " << it->second );
      meanMutualInformation += it->first * mutualInformationTable->GetNormalizedSymbolInformation (SatUtils::LinearToDb (it->second / beta));
    }

  double effectiveSnir = beta * SatUtils::DbToLinear (mutualInformationTable->GetSnirDb (meanMutualInformation));

  NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetEffectiveSnir - Packet " << packet.rxParams->m_txInfo.crdsaUniquePacketId << " from " << packet.sourceAddress << " - Effective SNIR dB: " << SatUtils::LinearToDb (effectiveSnir));

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
          // Trace if the packet has been decoded or not
          m_essaRxCollisionTrace (1,                        // number of packets
                                  it->sourceAddress, // sender address
                                  !(it->sicFlag));                // error flag
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
      if (it->sicFlag || !(it->hasBeenUpdated))
        {
          continue;
        }
      /// Check If packet can be detected
      if (!PacketCanBeDetected (*it))
        {
          NS_LOG_INFO ("SatPhyRxCarrierPerWindow::GetHighestSnirPacket - Packet " << it->rxParams->m_txInfo.crdsaUniquePacketId << " cannot be detected");
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
