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
#include <ns3/boolean.h>

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
  m_windowSicIterations (10)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Constructor called with arguments " << carrierId << ", " << carrierConf << ", and " << randomAccessEnabled);

  NS_ASSERT (m_randomAccessEnabled == true);
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

  if (packetRxParams.rxParams->m_txInfo.packetType == SatEnums::PACKET_TYPE_ESSA)
    {
      NS_LOG_INFO ("SatPhyRxCarrierPerWindow::ReceiveSlot - Time: " << Now ().GetSeconds () << " - ESSA packet received");
      if (nPackets > 0)
        {
          m_essaRxCollisionTrace (nPackets,                     // number of packets
                                  packetRxParams.sourceAddress, // sender address
                                  false);                       // collision flag
        }
      /// send packet upwards
      m_rxCallback (packetRxParams.rxParams, false);
    }
}


}
