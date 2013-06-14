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
{
  NS_LOG_FUNCTION (this);
}

SatPhy::SatPhy (Ptr<SatPhyTx> phyTx, Ptr<SatPhyRx> phyRx, uint32_t beamId, SatPhy::ReceiveCallback cb)
  :
  m_phyTx(phyTx),
  m_phyRx(phyRx),
  m_beamId(beamId),
  m_rxCallback(cb)
{
  NS_LOG_FUNCTION (this << phyTx << phyRx << beamId);

  phyTx->SetBeamId(beamId);
  phyRx->SetBeamId(beamId);

  phyTx->SetPhy(this);
  phyRx->SetPhy(this);
}


SatPhy::~SatPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

TypeId
SatPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhy")
    .SetParent<Object> ()
    .AddConstructor<SatPhy> ()
  ;
  return tid;
}

void
SatPhy::DoStart ()
{
  NS_LOG_FUNCTION (this);
  Object::DoStart ();
}

Ptr<SatPhyTx>
SatPhy::GetPhyTx ()
{
  NS_LOG_FUNCTION (this);
  return m_phyTx;
}

Ptr<SatPhyRx>
SatPhy::GetPhyRx ()
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
SatPhy::SetTxPower (double pow)
{
  NS_LOG_FUNCTION (this << pow);
  m_txPower = pow;
}

double
SatPhy::GetTxPower () const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
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

  m_phyTx->StartTx (p, txParams);
}

void
SatPhy::SendPdu (Ptr<Packet> p, Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << p << txParams);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

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


} // namespace ns3
