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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "ns3/antenna-model.h"
#include "ns3/object-factory.h"
#include "satellite-utils.h"
#include "satellite-net-device.h"
#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-rx-carrier.h"
#include "satellite-phy-rx-carrier-per-frame.h"
#include "satellite-phy-rx-carrier-per-slot.h"
#include "satellite-phy-rx-carrier-uplink.h"
#include "satellite-phy-rx-carrier-conf.h"
#include "satellite-signal-parameters.h"
#include "satellite-antenna-gain-pattern.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRx");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatPhyRx);

SatPhyRx::SatPhyRx ()
  : m_beamId (),
    m_maxAntennaGain (),
    m_antennaLoss (),
    m_defaultFadingValue ()
{
  NS_LOG_FUNCTION (this);
}

SatPhyRx::~SatPhyRx ()
{
  NS_LOG_FUNCTION (this);
}

void SatPhyRx::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_mobility = 0;
  m_device = 0;
  m_fadingContainer = 0;
  m_rxCarriers.clear ();
  Object::DoDispose ();
}

TypeId
SatPhyRx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRx")
    .SetParent<Object> ()
    .AddAttribute ("RxCarrierList", "The list of RX carriers associated to this Phy RX.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&SatPhyRx::m_rxCarriers),
                   MakeObjectVectorChecker<SatPhyRxCarrier> ())
  ;
  return tid;
}

Ptr<NetDevice>
SatPhyRx::GetDevice ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_device != 0);

  return m_device;
}

void
SatPhyRx::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  NS_ASSERT (m_device == 0);

  m_device = d;
}

void
SatPhyRx::SetMaxAntennaGain_Db (double gain_Db)
{
  NS_LOG_FUNCTION (this << gain_Db);

  m_maxAntennaGain = SatUtils::DbWToW (gain_Db);
}

double
SatPhyRx::GetAntennaGain (Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this);

  double gain_W (m_maxAntennaGain);

  // Get the receive antenna gain at the transmitter position.
  // E.g. UT transmits to the satellite receiver.
  if (m_antennaGainPattern)
    {
      Ptr<SatMobilityModel> m = DynamicCast<SatMobilityModel> (mobility);
      gain_W = m_antennaGainPattern->GetAntennaGain_lin (m->GetGeoPosition ());
    }

  /**
   * If antenna gain pattern is not set, we use the
   * set maximum antenna gain.
   */

  return gain_W;
}

void
SatPhyRx::SetDefaultFadingValue (double fadingValue)
{
  NS_LOG_FUNCTION (this << fadingValue);
  m_defaultFadingValue = fadingValue;
}

double
SatPhyRx::GetFadingValue (Address macAddress, SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << macAddress << channelType);

  double fadingValue = m_defaultFadingValue;

  if (m_fadingContainer)
    {
      fadingValue = m_fadingContainer->GetFading (macAddress, channelType);
    }
  // Returns value 1 if fading is not set, as fading value is used as multiplier
  return fadingValue;
}

void
SatPhyRx::SetFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_fadingContainer == 0);

  m_fadingContainer = fadingContainer;
}

void
SatPhyRx::SetAntennaLoss_Db (double gain_Db)
{
  NS_LOG_FUNCTION (this << gain_Db);

  m_antennaLoss = SatUtils::DbToLinear (gain_Db);
}

double
SatPhyRx::GetLosses ()
{
  NS_LOG_FUNCTION (this);

  return m_antennaLoss;
}

Mac48Address
SatPhyRx::GetAddress () const
{
  NS_LOG_FUNCTION (this);

  return m_macAddress;
}

void
SatPhyRx::SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo->GetNodeId ());

  m_macAddress = nodeInfo->GetMacAddress ();

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin ();
       it != m_rxCarriers.end ();
       ++it)
    {
      (*it)->SetNodeInfo (nodeInfo);
    }
}

void
SatPhyRx::BeginFrameEndScheduling ()
{
  NS_LOG_FUNCTION (this);

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin ();
       it != m_rxCarriers.end ();
       ++it)
    {
  		Ptr<SatPhyRxCarrierPerFrame> crdsaPrxc = (*it)->GetObject<SatPhyRxCarrierPerFrame> ();
      if (crdsaPrxc != 0) crdsaPrxc->BeginFrameEndScheduling ();
    }
}

void
SatPhyRx::SetReceiveCallback (SatPhyRx::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin ();
       it != m_rxCarriers.end ();
       ++it)
    {
      (*it)->SetReceiveCb (cb);
    }
}

void
SatPhyRx::SetCnoCallback (SatPhyRx::CnoCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin ();
       it != m_rxCarriers.end ();
       ++it)
    {
      (*it)->SetCnoCb (cb);
    }
}

void
SatPhyRx::SetAverageNormalizedOfferedLoadCallback (SatPhyRx::AverageNormalizedOfferedLoadCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  NS_ASSERT (!m_rxCarriers.empty ());

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin ();
       it != m_rxCarriers.end ();
       ++it)
    {
      (*it)->SetAverageNormalizedOfferedLoadCallback (cb);
    }
}

Ptr<MobilityModel>
SatPhyRx::GetMobility ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_mobility != 0);

  return m_mobility;
}

void
SatPhyRx::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}

void
SatPhyRx::SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  NS_LOG_FUNCTION (this << agp);
  NS_ASSERT (m_antennaGainPattern == 0);

  m_antennaGainPattern = agp;
}

void
SatPhyRx::ConfigurePhyRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatSuperframeConf> superFrameConf)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_rxCarriers.empty ());

  Ptr<SatPhyRxCarrier> rxc (nullptr);
  bool isRandomAccessCarrier (false);

  SatEnums::RandomAccessModel_t raModel = carrierConf->GetRandomAccessModel();

  for (uint32_t i = 0; i < carrierConf->GetCarrierCount (); ++i)
    {
      NS_LOG_INFO (this << " Create carrier: " << i);

      switch (carrierConf->GetChannelType ())
				{
          case SatEnums::FORWARD_FEEDER_CH:
            {
              // Satellite is the receiver in feeder uplink
              rxc = CreateObject<SatPhyRxCarrierUplink> (i, carrierConf, false);
              break;
            }
					case SatEnums::FORWARD_USER_CH:
					  {
					    // UT has only per slot non-random access carriers
					    rxc = CreateObject<SatPhyRxCarrierPerSlot> (i, carrierConf, false);
					    break;
					  }
          case SatEnums::RETURN_USER_CH:
            {
              isRandomAccessCarrier = superFrameConf->IsRandomAccessCarrier (i);

              // Satellite is the receiver in either user or feeder uplink
              rxc = CreateObject<SatPhyRxCarrierUplink> (i, carrierConf, isRandomAccessCarrier);
              break;
            }
					case SatEnums::RETURN_FEEDER_CH:
					  {
					    isRandomAccessCarrier = superFrameConf->IsRandomAccessCarrier (i);

					    // DA carrier
					    if (!isRandomAccessCarrier)
                {
                  rxc = CreateObject<SatPhyRxCarrierPerSlot> (i, carrierConf, false);
                }
					    // RA slotted aloha
					    else if (raModel == SatEnums::RA_MODEL_SLOTTED_ALOHA)
					      {
                  rxc = CreateObject<SatPhyRxCarrierPerSlot> (i, carrierConf, true);
                  DynamicCast<SatPhyRxCarrierPerSlot> (rxc)->
                      SetRandomAccessAllocationChannelId (superFrameConf->GetRaChannel (i));
					      }
					    // Note, that random access model of DVB-RCS2 specification may be configured
					    // to be either slotted ALOHA and CRDSA (wit no of unique payloads attribute).
					    // Here we make a short-cut such that the RCS2_SPECIFICATION random access
					    // always uses the CRDSA frame type receiver.
					    else if (raModel == SatEnums::RA_MODEL_CRDSA || raModel == SatEnums::RA_MODEL_RCS2_SPECIFICATION)
					      {
                  rxc = CreateObject<SatPhyRxCarrierPerFrame> (i, carrierConf, true);
                  DynamicCast<SatPhyRxCarrierPerSlot> (rxc)->
                      SetRandomAccessAllocationChannelId (superFrameConf->GetRaChannel (i));
					      }
					    break;
          }
					case SatEnums::UNKNOWN_CH:
					default:
					{
						NS_FATAL_ERROR ("SatPhyRx::ConfigurePhyRxCarriers - Invalid channel type!");
					}
				}
      m_rxCarriers.push_back (rxc);
    }
}

void
SatPhyRx::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  NS_ASSERT (beamId >= 0);
  NS_ASSERT (!m_rxCarriers.empty ());

  m_beamId = beamId;

  for (std::vector< Ptr<SatPhyRxCarrier> >::iterator it = m_rxCarriers.begin (); it != m_rxCarriers.end (); ++it)
    {
      (*it)->SetBeamId (beamId);
    }
}

uint32_t
SatPhyRx::GetBeamId () const
{
  NS_LOG_FUNCTION (this);

  return m_beamId;
}

void
SatPhyRx::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);

  uint32_t cId = rxParams->m_carrierId;

  if (cId >= m_rxCarriers.size ())
    {
      NS_FATAL_ERROR ("SatPhyRx::StartRx - unvalid carrier id: " << cId);
    }

  m_rxCarriers[cId]->StartRx (rxParams);
}

} // namespace ns3
