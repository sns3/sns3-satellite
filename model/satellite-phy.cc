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
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/node.h"

#include "satellite-utils.h"
#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"


NS_LOG_COMPONENT_DEFINE ("SatPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhy);

SatPhy::SatPhy (void)
  : m_eirpWoGain_W(0),
    m_rxMaxAntennaGain_db (0),
    m_txMaxAntennaGain_db (0),
    m_txMaxPower_dbW (0),
    m_txOutputLoss_db (0),
    m_txPointingLoss_db (0),
    m_txOboLoss_db (0),
    m_txAntennaLoss_db (0),
    m_rxAntennaLoss_db (0),
    m_defaultFadingValue (1)
{
  NS_LOG_FUNCTION (this);

  // Create the first needed SatPhyTx and SatPhyRx modules
}

SatPhy::SatPhy (Ptr<NetDevice> d, Ptr<SatChannel> txCh, Ptr<SatChannel> rxCh, uint32_t beamId)
 : m_eirpWoGain_W(0),
   m_rxMaxAntennaGain_db (0),
   m_txMaxAntennaGain_db (0),
   m_txMaxPower_dbW (0),
   m_txOutputLoss_db (0),
   m_txPointingLoss_db (0),
   m_txOboLoss_db (0),
   m_txAntennaLoss_db (0),
   m_rxAntennaLoss_db (0),
   m_defaultFadingValue (1)
{
  NS_LOG_FUNCTION (this << d << txCh << rxCh << beamId);
  ObjectBase::ConstructSelf(AttributeConstructionList ());

  Ptr<MobilityModel> mobility = d->GetNode()->GetObject<MobilityModel>();

  m_phyTx = CreateObject<SatPhyTx> ();
  m_phyTx->SetChannel (txCh);
  m_phyRx = CreateObject<SatPhyRx> ();
  m_beamId = beamId;

  rxCh->AddRx (m_phyRx);
  m_phyRx->SetDevice (d);
  m_phyTx->SetMobility (mobility);
  m_phyRx->SetMobility (mobility);
}

TypeId
SatPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhy")
    .SetParent<Object> ()
    .AddConstructor<SatPhy> ()
    .AddAttribute ("ReceiveCb", "The receive callback for this phy.",
                    CallbackValue (),
                    MakeCallbackAccessor (&SatPhy::m_rxCallback),
                    MakeCallbackChecker ())
    .AddAttribute ("CnoCb", "The C/N0 info callback for this phy.",
                    CallbackValue (),
                    MakeCallbackAccessor (&SatPhy::m_cnoCallback),
                    MakeCallbackChecker ())
  ;
  return tid;
}

void
SatPhy::Initialize ()
{
 // calculate EIRP without Gain (maximum)
  double eirpWoGain_DbW = m_txMaxPower_dbW - m_txOutputLoss_db - m_txPointingLoss_db - m_txOboLoss_db - m_txAntennaLoss_db;

  m_eirpWoGain_W = SatUtils::DbWToW ( eirpWoGain_DbW );

  m_phyTx->SetBeamId (m_beamId);
  m_phyRx->SetBeamId (m_beamId);

  m_phyRx->SetReceiveCallback ( MakeCallback (&SatPhy::Receive, this) );

  if ( m_cnoCallback.IsNull() == false )
    {
      m_phyRx->SetCnoCallback( MakeCallback (&SatPhy::CnoInfo, this) );
    }

  m_phyTx->SetMaxAntennaGain_Db (m_txMaxAntennaGain_db);
  m_phyRx->SetMaxAntennaGain_Db (m_rxMaxAntennaGain_db);

  m_phyTx->SetDefaultFadingValue (m_defaultFadingValue);
  m_phyRx->SetDefaultFadingValue (m_defaultFadingValue);

  m_phyRx->SetAntennaLoss_Db (m_rxAntennaLoss_db);
}

SatPhy::~SatPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phyTx->DoDispose ();
  m_phyTx = 0;
  m_phyRx->DoDispose ();
  m_phyRx = 0;

  Object::DoDispose ();
}

void
SatPhy::SetTxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  NS_LOG_FUNCTION (this);
  m_phyTx->SetAntennaGainPattern (agp);
}

void
SatPhy::SetRxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  NS_LOG_FUNCTION (this);
  m_phyRx->SetAntennaGainPattern (agp);
}

void
SatPhy::ConfigureRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf)
{
  NS_LOG_FUNCTION (this);
  m_phyRx->ConfigurePhyRxCarriers (carrierConf);
}

void
SatPhy::SetRxFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);

  m_phyRx->SetFadingContainer (fadingContainer);
}

void
SatPhy::SetTxFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);

  m_phyTx->SetFadingContainer (fadingContainer);
}

void
SatPhy::SetAddress (Mac48Address ownAddress)
{
  NS_LOG_FUNCTION (this);

  m_phyRx->SetAddress (ownAddress);
}

TypeId
SatPhy::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SatPhy::DoStart ()
{
  NS_LOG_FUNCTION (this);
  Object::DoStart ();
}

Ptr<SatPhyTx>
SatPhy::GetPhyTx () const
{
  NS_LOG_FUNCTION (this);
  return m_phyTx;
}

Ptr<SatPhyRx>
SatPhy::GetPhyRx () const
{
  NS_LOG_FUNCTION (this);
  return m_phyRx;
}

void
SatPhy::SetPhyTx (Ptr<SatPhyTx> phyTx)
{
  NS_LOG_FUNCTION (this << phyTx);
  m_phyTx = phyTx;
}

void
SatPhy::SetPhyRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);
  m_phyRx = phyRx;
}

Ptr<SatChannel>
SatPhy::GetTxChannel ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyTx);

  return m_phyTx->GetChannel ();
}

void
SatPhy::SendPdu (Ptr<Packet> p, uint32_t carrierId, Time duration )
{
  NS_LOG_FUNCTION (this << p << carrierId << duration);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << carrierId << " duration: " << duration);

  // Create a new SatSignalParameters related to this packet transmission
  Ptr<SatSignalParameters> txParams = Create<SatSignalParameters> ();
  txParams->m_duration = duration;
  txParams->m_phyTx = m_phyTx;
  txParams->m_packet = p;
  txParams->m_beamId = m_beamId;
  txParams->m_carrierId = carrierId;
  txParams->m_sinr = 0;
  txParams->m_txPower_W = m_eirpWoGain_W;

  m_phyTx->StartTx (p, txParams);
}

void
SatPhy::SendPdu (Ptr<Packet> p, Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << p << txParams);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

  // copy as sender own PhyTx object (at satellite) to ensure right distance calculation
  // and antenna gain getting at receiver (UT or GW)
  // copy on tx power too.

  txParams->m_phyTx = m_phyTx;
  txParams->m_txPower_W = m_eirpWoGain_W;
  m_phyTx->StartTx (p, txParams);
}

void
SatPhy::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  m_beamId = beamId;
  m_phyTx->SetBeamId (beamId);
  m_phyRx->SetBeamId (beamId);
}

void
SatPhy::Receive (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);
  m_rxCallback( rxParams->m_packet, rxParams);
}

void
SatPhy::CnoInfo (uint32_t beamId, Address source, double cno)
{
  NS_LOG_FUNCTION (this << beamId << source << cno);
  m_cnoCallback( beamId, source, cno);
}

} // namespace ns3
