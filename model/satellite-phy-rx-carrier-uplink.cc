/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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

#include <ns3/log.h>
#include <ns3/simulator.h>
#include "satellite-phy-rx-carrier-uplink.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierUplink");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierUplink);

SatPhyRxCarrierUplink::SatPhyRxCarrierUplink (uint32_t carrierId,
                                              Ptr<SatPhyRxCarrierConf> carrierConf,
                                              Ptr<SatWaveformConf> waveformConf,
                                              bool randomAccessEnabled)
  : SatPhyRxCarrier (carrierId, carrierConf, waveformConf, randomAccessEnabled)
{
  NS_LOG_FUNCTION (this);
}

SatPhyRxCarrierUplink::~SatPhyRxCarrierUplink ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
SatPhyRxCarrierUplink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierUplink")
    .SetParent<SatPhyRxCarrier> ()
  ;
  return tid;
}

Ptr<SatInterference::InterferenceChangeEvent>
SatPhyRxCarrierUplink::CreateInterference (Ptr<SatSignalParameters> rxParams, Address senderAddress)
{
  NS_LOG_FUNCTION (this << rxParams << senderAddress);
  return GetInterferenceModel ()->Add (rxParams->m_duration, rxParams->m_rxPower_W, senderAddress);
}

bool
SatPhyRxCarrierUplink::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);

  if (this->SatPhyRxCarrier::StartRx (rxParams))
    {
      /// PHY transmission decoded successfully. Note, that at transparent satellite,
      /// all the transmissions are not decoded.
      bool phyError (false);

      // Forward packet to ground entity without delay in the satellite
      m_rxCallback (rxParams, phyError);

      return true;
    }

  return false;
}

void
SatPhyRxCarrierUplink::EndRxData (uint32_t key)
{
  NS_LOG_FUNCTION (this << key);
  NS_LOG_INFO ("State: " << GetState ());

  NS_ASSERT (GetState () == RX);

  auto packetRxParams = GetStoredRxParams (key);

  DecreaseNumOfRxState (packetRxParams.rxParams->m_txInfo.packetType);

  packetRxParams.rxParams->SetInterferencePower (GetInterferenceModel ()->Calculate (packetRxParams.interferenceEvent));

  /// save values for CRDSA receiver
  packetRxParams.rxParams->SetInterferencePowerInSatellite (packetRxParams.rxParams->GetInterferencePowerPerFragment ());
  packetRxParams.rxParams->SetRxPowersInSatellite (packetRxParams.rxParams->m_rxPower_W, m_rxNoisePowerW, m_rxAciIfPowerW, m_rxExtNoisePowerW);

  /// calculates sinr for 1st link
  double sinr = CalculateSinr ( packetRxParams.rxParams->m_rxPower_W,
                                packetRxParams.rxParams->GetInterferencePower (),
                                m_rxNoisePowerW,
                                m_rxAciIfPowerW,
                                m_rxExtNoisePowerW,
                                m_sinrCalculate);

  // Update link specific SINR trace
  m_linkSinrTrace (SatUtils::LinearToDb (sinr));

  NS_ASSERT (!packetRxParams.rxParams->HasSinrComputed ());

  /// save 1st link sinr value for 2nd link composite sinr calculations
  packetRxParams.rxParams->SetSinr (sinr, m_sinrCalculate);

  /// uses 1st link sinr
  m_linkBudgetTrace (packetRxParams.rxParams, GetOwnAddress (),
                     packetRxParams.destAddress,
                     packetRxParams.rxParams->GetInterferencePower (), sinr);

  GetInterferenceModel ()->NotifyRxEnd (packetRxParams.interferenceEvent);

  /// erase the used Rx params
  packetRxParams.rxParams = NULL;
  packetRxParams.interferenceEvent = NULL;
  RemoveStoredRxParams (key);
}

}
