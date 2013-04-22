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

#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-mac.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>


NS_LOG_COMPONENT_DEFINE ("SatPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhy);

SatPhy::SatPhy (void)
{
  NS_LOG_FUNCTION (this);
}

SatPhy::SatPhy (Ptr<SatPhyTx> phyTx, Ptr<SatPhyRx> phyRx, uint16_t beamId)
  :
  m_phyTx(phyTx),
  m_phyRx(phyRx),
  m_beamId(beamId)
{
  NS_LOG_FUNCTION (this << phyTx << phyRx << beamId);

  phyTx->SetBeamId(beamId);
  phyRx->SetBeamId(beamId);

  phyTx->SetPhy(this);
  phyRx->SetPhy(this);
}


SatPhy::~SatPhy ()
{

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
  return m_phyTx;
}

Ptr<SatPhyRx>
SatPhy::GetPhyRx ()
{
  return m_phyRx;
}

void
SatPhy::SetPhyTx (Ptr<SatPhyTx> phyTx)
{
  m_phyTx = phyTx;
}

void
SatPhy::SetPhyRx (Ptr<SatPhyRx> phyRx)
{
  m_phyRx = phyRx;
}

Ptr<SatMac>
SatPhy::GetMac ()
{
  return m_mac;
}

void
SatPhy::SetMac (Ptr<SatMac> mac)
{
  m_mac = mac;
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
SatPhy::SendPdu (Ptr<Packet> p, Time duration )
{
  NS_LOG_FUNCTION (this);

  m_phyTx->StartTx (p, duration);
}

void
SatPhy::SetBeamId (uint16_t beamId)
{
  m_beamId = beamId;
  m_phyTx->SetBeamId (beamId);
  m_phyRx->SetBeamId (beamId);
}

void
SatPhy::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_mac->Receive( packet);
}


} // namespace ns3
