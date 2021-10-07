/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#include "ns3/log.h"
#include "ns3/ptr.h"

#include "satellite-signal-parameters.h"
#include "satellite-phy-tx.h"

NS_LOG_COMPONENT_DEFINE ("SatSignalParameters");

namespace ns3 {

SatSignalParameters::SatSignalParameters ()
  : m_beamId (),
  m_carrierId (),
  m_carrierFreq_hz (),
  m_duration (),
  m_txPower_W (),
  m_rxPower_W (),
  m_phyTx (),
  m_channelType ()
{
  NS_LOG_FUNCTION (this);
  m_ifParams = CreateObject<SatInterferenceParameters> ();
}

SatSignalParameters::SatSignalParameters ( const SatSignalParameters& p )
{
  for ( PacketsInBurst_t::const_iterator i = p.m_packetsInBurst.begin (); i != p.m_packetsInBurst.end (); i++  )
    {
      m_packetsInBurst.push_back ((*i)->Copy ());
    }

  m_beamId = p.m_beamId;
  m_carrierId = p.m_carrierId;
  m_duration = p.m_duration;
  m_phyTx = p.m_phyTx;
  m_txPower_W = p.m_txPower_W;
  m_rxPower_W = p.m_rxPower_W;
  m_channelType = p.m_channelType;
  m_carrierFreq_hz = p.m_carrierFreq_hz;
  m_txInfo.modCod = p.m_txInfo.modCod;
  m_txInfo.sliceId = p.m_txInfo.sliceId;
  m_txInfo.fecBlockSizeInBytes = p.m_txInfo.fecBlockSizeInBytes;
  m_txInfo.frameType = p.m_txInfo.frameType;
  m_txInfo.waveformId = p.m_txInfo.waveformId;
  m_txInfo.packetType = p.m_txInfo.packetType;
  m_txInfo.crdsaUniquePacketId = p.m_txInfo.crdsaUniquePacketId;
  m_ifParams = p.m_ifParams;
}

SatSignalParameters::~SatSignalParameters ()
{
  NS_LOG_FUNCTION (this);
  m_ifParams = nullptr;
}

Ptr<SatSignalParameters>
SatSignalParameters::Copy ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SatSignalParameters> p (new SatSignalParameters (*this), false);
  return p;
}

TypeId
SatSignalParameters::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSignalParameters")
    .SetParent<Object> ()
  ;
  return tid;
}

void
SatSignalParameters::SetRxPowersInSatellite (double rxPowerW, double rxNoisePowerW, double rxAciIfPowerW, double rxExtNoisePowerW)
{
  m_ifParams->m_rxPowerInSatellite_W = rxPowerW;
  m_ifParams->m_rxNoisePowerInSatellite_W = rxNoisePowerW;
  m_ifParams->m_rxAciIfPowerInSatellite_W = rxAciIfPowerW;
  m_ifParams->m_rxExtNoisePowerInSatellite_W = rxExtNoisePowerW;
}

void
SatSignalParameters::SetSinr (double sinr, Callback<double, double> sinrCalculate)
{
  m_ifParams->m_sinr = sinr;
  m_ifParams->m_sinrCalculate = sinrCalculate;
  m_ifParams->m_sinrComputed = true;
}


SatInterferenceParameters::~SatInterferenceParameters ()
{
  m_sinrCalculate.Nullify ();
}

} // namespace ns3
