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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "satellite-signal-parameters.h"
#include "satellite-perfect-interference-elimination.h"

NS_LOG_COMPONENT_DEFINE ("SatPerfectInterferenceElimination");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPerfectInterferenceElimination);

TypeId
SatPerfectInterferenceElimination::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPerfectInterferenceElimination")
    .SetParent<SatInterferenceElimination> ()
    .AddConstructor<SatPerfectInterferenceElimination> ();

  return tid;
}

TypeId
SatPerfectInterferenceElimination::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatPerfectInterferenceElimination::SatPerfectInterferenceElimination ()
{
  NS_LOG_FUNCTION (this);
}

SatPerfectInterferenceElimination::~SatPerfectInterferenceElimination ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPerfectInterferenceElimination::EliminateInterferences (
  Ptr<SatSignalParameters> packetInterferedWith,
  Ptr<SatSignalParameters> processedPacket,
  double EsNo)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO (
    "Removing interference power of packet from Beam[Carrier] " <<
      processedPacket->m_beamId <<
      "[" << processedPacket->m_carrierId << "]");
  auto oldIfPower = packetInterferedWith->m_ifPowerInSatellite_W;
  packetInterferedWith->m_ifPowerInSatellite_W -= processedPacket->m_rxPowerInSatellite_W;

  if (std::abs (packetInterferedWith->m_ifPowerInSatellite_W) < std::numeric_limits<double>::epsilon ())
    {
      packetInterferedWith->m_ifPowerInSatellite_W = 0;
    }

  NS_LOG_INFO (
    "Interfered packet ifPower went from " << oldIfPower <<
      " to " << packetInterferedWith->m_ifPowerInSatellite_W);
}

}  // namespace ns3
