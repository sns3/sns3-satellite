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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include "satellite-phy-rx-carrier-uplink.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierUplink");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierUplink);

SatPhyRxCarrierUplink::SatPhyRxCarrierUplink (uint32_t carrierId,
                                              Ptr<SatPhyRxCarrierConf> carrierConf,
                                              bool randomAccessEnabled)
: SatPhyRxCarrier (carrierId, carrierConf, randomAccessEnabled)
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
	return GetInterferenceModel()->Add (rxParams->m_duration, rxParams->m_rxPower_W, senderAddress);
}

void
SatPhyRxCarrierUplink::EndRxData (uint32_t key)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (this << " state: " << GetState ());

  NS_ASSERT (GetState () == RX);

  auto packetRxParams = GetStoredRxParams (key);

  DecreaseNumOfRxState (packetRxParams.rxParams->m_txInfo.packetType);

  packetRxParams.rxParams->m_ifPower_W = GetInterferenceModel ()->Calculate (packetRxParams.interferenceEvent);

  /// save values for CRDSA receiver
  packetRxParams.rxParams->m_ifPowerInSatellite_W = packetRxParams.rxParams->m_ifPower_W;
  packetRxParams.rxParams->m_rxPowerInSatellite_W = packetRxParams.rxParams->m_rxPower_W;
  packetRxParams.rxParams->m_rxNoisePowerInSatellite_W = m_rxNoisePowerW;
  packetRxParams.rxParams->m_rxAciIfPowerInSatellite_W = m_rxAciIfPowerW;
  packetRxParams.rxParams->m_rxExtNoisePowerInSatellite_W = m_rxExtNoisePowerW;
  packetRxParams.rxParams->m_sinrCalculate = m_sinrCalculate;

  /// calculates sinr for 1st link
  double sinr = CalculateSinr ( packetRxParams.rxParams->m_rxPower_W,
  															packetRxParams.rxParams->m_ifPower_W,
                                m_rxNoisePowerW,
                                m_rxAciIfPowerW,
                                m_rxExtNoisePowerW,
                                m_sinrCalculate);

  // Update link specific SINR trace
  m_linkSinrTrace (SatUtils::LinearToDb (sinr));

  NS_ASSERT (packetRxParams.rxParams->m_sinr == 0);

  /// PHY transmission decoded successfully. Note, that at transparent satellite,
  /// all the transmissions are not decoded.
  bool phyError (false);

  /// save 1st link sinr value for 2nd link composite sinr calculations
  packetRxParams.rxParams->m_sinr = sinr;

  /// uses 1st link sinr
  m_linkBudgetTrace (packetRxParams.rxParams, GetOwnAddress (),
  		packetRxParams.destAddress, packetRxParams.rxParams->m_ifPower_W, sinr);

  /// Send packet upwards
  m_rxCallback ( packetRxParams.rxParams, phyError );

  GetInterferenceModel ()->NotifyRxEnd (packetRxParams.interferenceEvent);

  /// erase the used Rx params
  packetRxParams.rxParams = NULL;
  packetRxParams.interferenceEvent = NULL;
  RemoveStoredRxParams (key);
}

}
