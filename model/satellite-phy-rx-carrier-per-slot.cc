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
#include <ns3/boolean.h>
#include <ns3/address.h>
#include "satellite-phy-rx-carrier-per-slot.h"

#include <algorithm>
#include <ostream>
#include <limits>
#include <utility>

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierPerSlot");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierPerSlot);

SatPhyRxCarrierPerSlot::SatPhyRxCarrierPerSlot (uint32_t carrierId,
                                                Ptr<SatPhyRxCarrierConf> carrierConf,
                                                bool randomAccessEnabled)
: SatPhyRxCarrier (carrierId, carrierConf, randomAccessEnabled),
	m_randomAccessBitsInFrame (0),
	m_randomAccessAllocationChannelId (0),
	m_randomAccessCollisionModel (SatPhyRxCarrierConf::RA_COLLISION_NOT_DEFINED),
	m_randomAccessConstantErrorRate (0.0),
	m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize (0),
	m_enableRandomAccessDynamicLoadControl (false)
{
  if (randomAccessEnabled)
    {
      m_randomAccessCollisionModel = carrierConf->GetRandomAccessCollisionModel ();
      m_randomAccessConstantErrorRate = carrierConf->GetRandomAccessConstantErrorRate ();
      m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize = carrierConf->GetRandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize ();
      m_enableRandomAccessDynamicLoadControl = carrierConf->IsRandomAccessDynamicLoadControlEnabled ();

      NS_LOG_INFO ("RA interference model: " << carrierConf->GetInterferenceModel (true) <<
                   ", RA collision model: " << m_randomAccessCollisionModel <<
                   ", RA avg. normalized offered load measurement window size: " << m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize);

    }
}

SatPhyRxCarrierPerSlot::~SatPhyRxCarrierPerSlot ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatPhyRxCarrierPerSlot::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierPerSlot")
	.SetParent<SatPhyRxCarrier> ()
  .AddTraceSource ("SlottedAlohaRxCollision",
                   "Received a packet burst through Random Access Slotted ALOHA",
                   MakeTraceSourceAccessor (&SatPhyRxCarrierPerSlot::m_slottedAlohaRxCollisionTrace),
                   "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
  .AddTraceSource ("SlottedAlohaRxError",
                   "Received a packet burst through Random Access Slotted ALOHA",
                   MakeTraceSourceAccessor (&SatPhyRxCarrierPerSlot::m_slottedAlohaRxErrorTrace),
                   "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
	;
  return tid;
}

void
SatPhyRxCarrierPerSlot::DoDispose ()
{
	NS_LOG_FUNCTION (this);

	SatPhyRxCarrier::DoDispose ();
	m_randomAccessDynamicLoadControlNormalizedOfferedLoad.clear ();
}

Ptr<SatInterference::InterferenceChangeEvent>
SatPhyRxCarrierPerSlot::CreateInterference (Ptr<SatSignalParameters> rxParams, Address senderAddress)
{
	SatEnums::ChannelType_t ct = GetChannelType ();
	if (ct == SatEnums::RETURN_FEEDER_CH)
	  {
	    // In feeder downlink the interference by UTs in the same
      // beam (intra-beam interference e.g. due to random access) SHOULD NOT be tracked.
      // Intra-beam interference is already taken into account at the satellite. Thus,
      // here we pass intra-beam transmissions with zero interference power to the
      // interference model.
      // Rx power in the rxParams is the received power of only one of the interfering
      // signals of the wanted signal. In reality, we are receiving the same signal
      // through all the co-channel transponders of the satellite, where the rxPower
      // is the C and all the other components are considered as interference I.
      // This can be compensated with the following equation:
      // rxPower = rxParams->m_rxPower_W * (1 + 1/rxParams->m_sinr);
      // See more from satellite module documentation.

	    double rxPower (0.0);

	    if (rxParams->m_beamId != GetBeamId ())
	      {
	        rxPower = rxParams->m_rxPower_W * (1 + 1/rxParams->m_sinr);
	      }

	    // Add the interference even regardless.
	    return GetInterferenceModel()->Add (rxParams->m_duration,
	                                        rxPower,
	                                        GetOwnAddress ());
	  }
	else if (ct == SatEnums::FORWARD_USER_CH)
	  {
	    return GetInterferenceModel()->Add (rxParams->m_duration, rxParams->m_rxPower_W, GetOwnAddress ());
	  }

	NS_FATAL_ERROR ("SatSatellitePhyRxCarrier::CreateInterference - Invalid channel type!");
	return NULL;
}

void
SatPhyRxCarrierPerSlot::EndRxData (uint32_t key)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (this << " state: " << GetState ());

  NS_ASSERT (GetState () == RX);

  rxParams_s packetRxParams = GetStoredRxParams (key);

  const uint32_t nPackets = packetRxParams.rxParams->m_packetsInBurst.size ();

  DecreaseNumOfRxState (packetRxParams.rxParams->m_txInfo.packetType);

  NS_ASSERT (packetRxParams.rxParams->m_sinr != 0);

  packetRxParams.rxParams->m_ifPower_W = GetInterferenceModel ()->Calculate (packetRxParams.interferenceEvent);

  ReceiveSlot (packetRxParams, nPackets);

  GetInterferenceModel ()->NotifyRxEnd (packetRxParams.interferenceEvent);

  /// erase the used Rx params
  packetRxParams.interferenceEvent = NULL;
  RemoveStoredRxParams (key);
}

bool
SatPhyRxCarrierPerSlot::ProcessSlottedAlohaCollisions (double cSinr,
																														Ptr<SatSignalParameters> rxParams,
																														Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions");

  bool phyError = false;

  if (m_randomAccessCollisionModel == SatPhyRxCarrierConf::RA_COLLISION_ALWAYS_DROP_ALL_COLLIDING_PACKETS)
    {
      /// check whether the packet has collided. This mode is intended to be used with constant interference and traced interference
      phyError = GetInterferenceModel ()->HasCollision (interferenceEvent);
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions - Time: " << Now ().GetSeconds () << " - Strict collision mode, phyError: " << phyError);
    }
  else if (m_randomAccessCollisionModel == SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR)
    {
      /// check cSinr against link results
      phyError = CheckAgainstLinkResults (cSinr,rxParams);
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions - Time: " << Now ().GetSeconds () << " - Composite SINR mode, phyError: " << phyError);
    }
  else if (m_randomAccessCollisionModel == SatPhyRxCarrierConf::RA_CONSTANT_COLLISION_PROBABILITY)
    {
      double r = GetUniformRandomValue (0, 1);
      if (r <  m_randomAccessConstantErrorRate)
        {
          phyError = true;
        }
      NS_LOG_INFO ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions - Time: " << Now ().GetSeconds () << " - Constant collision probability mode, phyError: " << phyError);
    }
  else
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::ProcessSlottedAlohaCollisions - Random access collision model not defined");
    }

  return phyError;
}

void
SatPhyRxCarrierPerSlot::ReceiveSlot (SatPhyRxCarrier::rxParams_s packetRxParams, const uint32_t nPackets)
{
  NS_ASSERT (packetRxParams.rxParams->m_txInfo.packetType != SatEnums::PACKET_TYPE_CRDSA);
	/// calculates sinr for 2nd link
	double sinr = CalculateSinr ( packetRxParams.rxParams->m_rxPower_W,
																packetRxParams.rxParams->m_ifPower_W,
																m_rxNoisePowerW,
																m_rxAciIfPowerW,
																m_rxExtNoisePowerW,
																m_sinrCalculate);

	// Update link specific SINR trace
	m_linkSinrTrace (SatUtils::LinearToDb (sinr));

	/// PHY transmission decoded successfully. Note, that at transparent satellite,
	/// all the transmissions are not decoded.
	bool phyError (false);

	/// calculate composite SINR
	double cSinr = CalculateCompositeSinr (sinr, packetRxParams.rxParams->m_sinr);

	// Update composite SINR trace for DAMA and Slotted ALOHA packets
	m_sinrTrace (SatUtils::LinearToDb (cSinr), packetRxParams.sourceAddress);

	/// composite sinr output trace
	if (IsCompositeSinrOutputTraceEnabled ())
		{
			DoCompositeSinrOutputTrace (cSinr);
		}

	if (packetRxParams.rxParams->m_txInfo.packetType == SatEnums::PACKET_TYPE_SLOTTED_ALOHA)
		{
			NS_LOG_INFO ("SatPhyRxCarrier::EndRxDataNormal - Time: " << Now ().GetSeconds () << " - Slotted ALOHA packet received");

			// Update the load with FEC block size!
			m_randomAccessBitsInFrame += packetRxParams.rxParams->m_txInfo.fecBlockSizeInBytes * SatConstVariables::BITS_PER_BYTE;

			/// check for slotted aloha packet collisions
			phyError = ProcessSlottedAlohaCollisions (cSinr, packetRxParams.rxParams, packetRxParams.interferenceEvent);

			if (nPackets > 0)
				{
					const bool hasCollision = GetInterferenceModel ()->HasCollision (packetRxParams.interferenceEvent);
					m_slottedAlohaRxCollisionTrace (nPackets,                    // number of packets
																					packetRxParams.sourceAddress,  // sender address
																					hasCollision                 // collision flag
																					);
					m_slottedAlohaRxErrorTrace (nPackets,                    // number of packets
																			packetRxParams.sourceAddress,  // sender address
																			phyError                     // error flag
																			);
				}
		}
	else
		{
			/// check against link results
			phyError = CheckAgainstLinkResults (cSinr, packetRxParams.rxParams);

			if (nPackets > 0)
				{
					m_daRxTrace (nPackets,                    // number of packets
											 packetRxParams.sourceAddress,  // sender address
											 phyError                     // error flag
											 );
				}
		}

	/// save 2nd link sinr value
	packetRxParams.rxParams->m_sinr = sinr;

	/// uses composite sinr
	m_linkBudgetTrace (packetRxParams.rxParams, GetOwnAddress (), packetRxParams.destAddress, packetRxParams.rxParams->m_ifPower_W, cSinr);

	/// send packet upwards
	m_rxCallback (packetRxParams.rxParams, phyError);

	/// uses composite sinr
	if (!m_cnoCallback.IsNull ())
		{
		/**
		 * Channel estimation error is added to the cno measurement,
		 * which is utilized e.g. for ACM.
		 */
		double cno = cSinr;

			// Forward link
			if (GetNodeInfo ()->GetNodeType () == SatEnums::NT_UT)
				{
					cno = SatUtils::DbToLinear (GetChannelEstimationErrorContainer ()->AddError (SatUtils::LinearToDb (cno)));
				}
			// Return link
			else if (GetNodeInfo ()->GetNodeType () == SatEnums::NT_GW)
				{
					cno = SatUtils::DbToLinear (GetChannelEstimationErrorContainer ()->AddError (
							SatUtils::LinearToDb (cno), packetRxParams.rxParams->m_txInfo.waveformId));
				}
			else
				{
					NS_FATAL_ERROR ("Unsupported node type for a NORMAL Rx model!");
				}

		cno *= m_rxBandwidthHz;

		m_cnoCallback (packetRxParams.rxParams->m_beamId,
									 packetRxParams.sourceAddress,
									 GetOwnAddress (),
									 cno);
		}

	packetRxParams.rxParams = NULL;
}

void
SatPhyRxCarrierPerSlot::SaveMeasuredRandomAccessLoad (double measuredRandomAccessLoad)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrier::SaveMeasuredRandomAccessLoad");

  NS_LOG_INFO ("SatPhyRxCarrier::SaveMeasuredRandomAccessLoad - Time: " << Now ().GetSeconds ());

  m_randomAccessDynamicLoadControlNormalizedOfferedLoad.push_back (measuredRandomAccessLoad);

  while (m_randomAccessDynamicLoadControlNormalizedOfferedLoad.size () >
          m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize)
    {
      m_randomAccessDynamicLoadControlNormalizedOfferedLoad.pop_front ();
    }
}

double
SatPhyRxCarrierPerSlot::CalculateAverageNormalizedOfferedRandomAccessLoad ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("SatPhyRxCarrier::CalculateAverageNormalizedOfferedRandomAccessLoad");

  NS_LOG_INFO ("SatPhyRxCarrier::CalculateAverageNormalizedOfferedRandomAccessLoad - Time: " << Now ().GetSeconds ());

  double sum = 0.0;
  double averageNormalizedOfferedLoad = 0.0;
  std::deque<double>::iterator it;

  for (it = m_randomAccessDynamicLoadControlNormalizedOfferedLoad.begin (); it != m_randomAccessDynamicLoadControlNormalizedOfferedLoad.end (); it++)
    {
      sum += (*it);
    }

  if (!m_randomAccessDynamicLoadControlNormalizedOfferedLoad.empty ())
    {
      averageNormalizedOfferedLoad = sum / m_randomAccessDynamicLoadControlNormalizedOfferedLoad.size ();
    }

  NS_LOG_INFO ("SatPhyRxCarrier::CalculateAverageNormalizedOfferedRandomAccessLoad - average normalized offered load: " << averageNormalizedOfferedLoad);

  return averageNormalizedOfferedLoad;
}

}
