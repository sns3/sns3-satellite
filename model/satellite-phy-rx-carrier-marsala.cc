/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/boolean.h>

#include "satellite-phy-rx-carrier-marsala.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierMarsala");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierMarsala);

SatPhyRxCarrierMarsala::SatPhyRxCarrierMarsala (uint32_t carrierId,
                                                Ptr<SatPhyRxCarrierConf> carrierConf,
                                                Ptr<SatWaveformConf> waveformConf,
                                                bool randomAccessEnabled)
  : SatPhyRxCarrierPerFrame (carrierId, carrierConf, waveformConf, randomAccessEnabled)
{
  NS_LOG_FUNCTION (this);
}


SatPhyRxCarrierMarsala::~SatPhyRxCarrierMarsala ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
SatPhyRxCarrierMarsala::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierMarsala")
    .SetParent<SatPhyRxCarrierPerFrame> ()
  ;
  return tid;
}


void
SatPhyRxCarrierMarsala::PerformSicCycles (
  std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>& combinedPacketsForFrame)
{
  NS_LOG_FUNCTION (this);

  do
    {
      SatPhyRxCarrierPerFrame::PerformSicCycles (combinedPacketsForFrame);
    }
  while (PerformMarsala (combinedPacketsForFrame));
}


bool
SatPhyRxCarrierMarsala::PerformMarsala (
  std::vector<SatPhyRxCarrierPerFrame::crdsaPacketRxParams_s>& combinedPacketsForFrame)
{
  NS_LOG_FUNCTION (this);

  return false;
}


}
