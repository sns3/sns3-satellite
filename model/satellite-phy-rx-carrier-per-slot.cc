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

#include <algorithm>
#include <ostream>
#include <limits>
#include <utility>

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/boolean.h>
#include <ns3/address.h>

#include "satellite-uplink-info-tag.h"

#include "satellite-phy-rx-carrier-per-slot.h"


NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierPerSlot");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierPerSlot);

SatPhyRxCarrierPerSlot::SatPhyRxCarrierPerSlot (uint32_t carrierId,
                                                Ptr<SatPhyRxCarrierConf> carrierConf,
                                                Ptr<SatWaveformConf> waveformConf,
                                                bool randomAccessEnabled)
  : SatPhyRxCarrier (carrierId, carrierConf, waveformConf, randomAccessEnabled),
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

void
SatPhyRxCarrierPerSlot::BeginEndScheduling ()
{
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
  NS_LOG_FUNCTION (this << rxParams << senderAddress);

  // Case satellite regenerative
  if (GetLinkRegenerationMode () != SatEnums::TRANSPARENT)
  {
    return GetInterferenceModel ()->Add (rxParams->m_duration, rxParams->m_rxPower_W, GetOwnAddress ());
  }

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

      // Get first link sinr (stored in any packet of container)
      SatUplinkInfoTag satUplinkInfoTag;
      if (!rxParams->m_packetsInBurst[0]->PeekPacketTag (satUplinkInfoTag))
        {
          NS_FATAL_ERROR ("SatUplinkInfoTag not found !");
        }

      if (rxParams->m_beamId != GetBeamId ())
        {
          if (!rxParams->HasSinrComputed ())
            {
              NS_FATAL_ERROR ("SatPhyRx::StartRx - too long transmission time: packet started to be received in a ground entity while not being fully received on the satellite: interferences could not be properly computed.");
            }

          rxPower = rxParams->m_rxPower_W * (1 + 1 / satUplinkInfoTag.GetSinr ());
        }

      // Add the interference even regardless.
      return GetInterferenceModel ()->Add (rxParams->m_duration, rxPower, GetOwnAddress ());
    }
  else if (ct == SatEnums::FORWARD_USER_CH)
    {
      return GetInterferenceModel ()->Add (rxParams->m_duration, rxParams->m_rxPower_W, GetOwnAddress ());
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

  if (GetLinkRegenerationMode () == SatEnums::TRANSPARENT)
    {
      NS_ASSERT (packetRxParams.rxParams->HasSinrComputed ());
    }

  packetRxParams.rxParams->SetInterferencePower (GetInterferenceModel ()->Calculate (packetRxParams.interferenceEvent));

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

  bool phyError = false;

  if (m_randomAccessCollisionModel == SatPhyRxCarrierConf::RA_COLLISION_ALWAYS_DROP_ALL_COLLIDING_PACKETS)
    {
      /// check whether the packet has collided. This mode is intended to be used with constant interference and traced interference
      phyError = GetInterferenceModel ()->HasCollision (interferenceEvent);
      NS_LOG_INFO ("Strict collision mode, phyError: " << phyError);
    }
  else if (m_randomAccessCollisionModel == SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR)
    {
      /// check cSinr against link results
      phyError = CheckAgainstLinkResults (cSinr, rxParams);
      NS_LOG_INFO ("Composite SINR mode, phyError: " << phyError);
    }
  else if (m_randomAccessCollisionModel == SatPhyRxCarrierConf::RA_CONSTANT_COLLISION_PROBABILITY)
    {
      double r = GetUniformRandomValue (0, 1);
      if (r <  m_randomAccessConstantErrorRate)
        {
          phyError = true;
        }
      NS_LOG_INFO ("Constant collision probability mode, phyError: " << phyError);
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
  NS_LOG_FUNCTION (this << &packetRxParams << nPackets);

  NS_ASSERT (packetRxParams.rxParams->m_txInfo.packetType != SatEnums::PACKET_TYPE_CRDSA);
  /// calculates sinr for 2nd link
  double sinr = CalculateSinr ( packetRxParams.rxParams->m_rxPower_W,
                                packetRxParams.rxParams->GetInterferencePower (),
                                m_rxNoisePowerW,
                                m_rxAciIfPowerW,
                                m_rxExtNoisePowerW,
                                m_sinrCalculate);

  // Update link specific SINR trace
  if (GetChannelType () == SatEnums::RETURN_USER_CH || GetChannelType () == SatEnums::RETURN_FEEDER_CH)
    {
      m_linkSinrTrace (SatUtils::LinearToDb (sinr), packetRxParams.sourceAddress);
    }
  else if (GetChannelType () == SatEnums::FORWARD_USER_CH || GetChannelType () == SatEnums::FORWARD_FEEDER_CH)
    {
      m_linkSinrTrace (SatUtils::LinearToDb (sinr), packetRxParams.destAddress);
    }
  else
    {
      NS_FATAL_ERROR ("Incorrect channel for satPhyRxCarrierPerSlot: " << SatEnums::GetChannelTypeName (GetChannelType ()));
    }

  /// PHY transmission decoded successfully. Note, that at transparent satellite,
  /// all the transmissions are not decoded.
  bool phyError (false);

  /// calculate composite SINR if transparent. Otherwise take only current sinr.
  double cSinr;
  if (GetLinkRegenerationMode () == SatEnums::TRANSPARENT)
    {
      // Get first link sinr (stored in any packet of container)
      SatUplinkInfoTag satUplinkInfoTag;
      if (!packetRxParams.rxParams->m_packetsInBurst[0]->PeekPacketTag (satUplinkInfoTag))
        {
          NS_FATAL_ERROR ("SatUplinkInfoTag not found !");
        }
      cSinr = CalculateCompositeSinr (sinr, satUplinkInfoTag.GetSinr ());
    }
  else
    {
      cSinr = sinr;
    }

  double worstSinr;
  if (!m_cnoCallback.IsNull ())
    {
      // Get first link sinr (stored in any packet of container)
      SatUplinkInfoTag satUplinkInfoTag;
      if (!packetRxParams.rxParams->m_packetsInBurst[0]->PeekPacketTag (satUplinkInfoTag))
        {
          NS_FATAL_ERROR ("SatUplinkInfoTag not found !");
        }
      worstSinr = GetWorstSinr (sinr, satUplinkInfoTag.GetSinr ());
    }

  // Update composite SINR trace for DAMA and Slotted ALOHA packets
  m_sinrTrace (SatUtils::LinearToDb (cSinr), packetRxParams.sourceAddress);

  /// composite sinr output trace
  if (IsCompositeSinrOutputTraceEnabled ())
    {
      DoCompositeSinrOutputTrace (cSinr);
    }

  if (packetRxParams.rxParams->m_txInfo.packetType == SatEnums::PACKET_TYPE_SLOTTED_ALOHA)
    {
      NS_LOG_INFO ("Slotted ALOHA packet received");

      // Update the load with FEC block size!
      m_randomAccessBitsInFrame += packetRxParams.rxParams->m_txInfo.fecBlockSizeInBytes * SatConstVariables::BITS_PER_BYTE;

      /// check for slotted aloha packet collisions
      phyError = ProcessSlottedAlohaCollisions (cSinr, packetRxParams.rxParams, packetRxParams.interferenceEvent);

      if (nPackets > 0)
        {
          const bool hasCollision = GetInterferenceModel ()->HasCollision (packetRxParams.interferenceEvent);
          m_slottedAlohaRxCollisionTrace (nPackets,                      // number of packets
                                          packetRxParams.sourceAddress,  // sender address
                                          hasCollision                   // collision flag
                                          );
          m_slottedAlohaRxErrorTrace (nPackets,                      // number of packets
                                      packetRxParams.sourceAddress,  // sender address
                                      phyError                       // error flag
                                      );
        }
    }
  else
    {
      /// check against link results
      phyError = CheckAgainstLinkResults (cSinr, packetRxParams.rxParams);

      if (nPackets > 0)
        {
          if (GetChannelType () == SatEnums::FORWARD_FEEDER_CH)
            {
              m_daRxTrace (nPackets,                      // number of packets
                           packetRxParams.destAddress,    // destination address
                           phyError                       // error flag
                           );
            }
          else
            {
              m_daRxTrace (nPackets,                      // number of packets
                           packetRxParams.sourceAddress,  // sender address
                           phyError                       // error flag
                           );
            }
        }

      m_daRxCarrierIdTrace (GetCarrierId (),
                            packetRxParams.sourceAddress);
    }

  /// save 2nd link sinr value
  SatSignalParameters::PacketsInBurst_t packets = packetRxParams.rxParams->m_packetsInBurst;
  SatSignalParameters::PacketsInBurst_t::const_iterator i;
  for (i = packets.begin (); i != packets.end (); i++)
    {
      SatUplinkInfoTag satUplinkInfoTag;
      (*i)->RemovePacketTag (satUplinkInfoTag);
      satUplinkInfoTag.SetSinr (sinr, packetRxParams.rxParams->GetSinrCalculator ());
      (*i)->AddPacketTag (satUplinkInfoTag);
    }

  /// uses composite sinr
  m_linkBudgetTrace (packetRxParams.rxParams, GetOwnAddress (),
                     packetRxParams.destAddress,
                     packetRxParams.rxParams->GetInterferencePower (), cSinr);

  /// send packet upwards
  m_rxCallback (packetRxParams.rxParams, phyError);

  /// uses worst sinr, callback exists only on downlink receivers
  if (!m_cnoCallback.IsNull ())
    {
      /**
       * Channel estimation error is added to the cno measurement,
       * which is utilized e.g. for ACM.
       */
      double cno = (GetLinkRegenerationMode () == SatEnums::TRANSPARENT) ? cSinr : worstSinr;

      // Forward link (or return feeder + SCPC)
      if (GetNodeInfo ()->GetNodeType () == SatEnums::NT_UT)
        {
          cno = SatUtils::DbToLinear (GetChannelEstimationErrorContainer ()->AddError (SatUtils::LinearToDb (cno)));
        }
      // Return link
      else if (GetNodeInfo ()->GetNodeType () == SatEnums::NT_GW)
        {
          // If SCPC
          if (GetLinkRegenerationMode () == SatEnums::REGENERATION_LINK || GetLinkRegenerationMode () == SatEnums::REGENERATION_NETWORK)
            {
              cno = SatUtils::DbToLinear (GetChannelEstimationErrorContainer ()->AddError (SatUtils::LinearToDb (cno)));
            }
          else
            {
              cno = SatUtils::DbToLinear (GetChannelEstimationErrorContainer ()->AddError (
                                          SatUtils::LinearToDb (cno), packetRxParams.rxParams->m_txInfo.waveformId));
            }
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported node type for a NORMAL Rx model!");
        }

      cno *= m_rxBandwidthHz;

      SatAddressE2ETag addressE2ETag;
      packetRxParams.rxParams->m_packetsInBurst[0]->PeekPacketTag (addressE2ETag);

      m_cnoCallback (packetRxParams.rxParams->m_beamId,
                     addressE2ETag.GetE2ESourceAddress (),
                     GetOwnAddress (),
                     cno);
    }

  packetRxParams.rxParams = NULL;
}

void
SatPhyRxCarrierPerSlot::SaveMeasuredRandomAccessLoad (double measuredRandomAccessLoad)
{
  NS_LOG_FUNCTION (this);

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

  NS_LOG_INFO ("Average normalized offered load: " << averageNormalizedOfferedLoad);

  return averageNormalizedOfferedLoad;
}

}
