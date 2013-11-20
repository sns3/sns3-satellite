/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/log.h"
#include "satellite-ncc.h"

NS_LOG_COMPONENT_DEFINE ("SatNcc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatNcc);

TypeId 
SatNcc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatNcc")
    .SetParent<Object> ()
    .AddConstructor<SatNcc> ()
    //
    // Trace sources
    //
    .AddTraceSource ("NccRx",
                     "Trace source indicating a CR has received by NCC",
                     MakeTraceSourceAccessor (&SatNcc::m_nccRxTrace))
    .AddTraceSource ("NccTx",
                     "Trace source indicating a TBTP has sentby NCC",
                     MakeTraceSourceAccessor (&SatNcc::m_nccTxTrace))
  ;
  return tid;
}

SatNcc::SatNcc ()
{
  NS_LOG_FUNCTION (this);

  //Simulator::Schedule (Seconds(0.01), &SatNcc::TransmitReady, this);
}

SatNcc::~SatNcc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatNcc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatNcc::Receive (Ptr<Packet> packet, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << packet << beamId);
}

void
SatNcc::UtCnoUpdated (uint32_t beamId, Address utId, double cno)
{
  NS_LOG_FUNCTION (this << beamId << utId << cno);

  m_beamSchedulers[beamId]->UpdateUtCno (utId, cno);
}

void
SatNcc::AddBeam (uint32_t beamId, SatNcc::SendCallback cb, Ptr<SatSuperframeSeq> seq)
{
  NS_LOG_FUNCTION (this << &cb);

  Ptr<SatBeamScheduler> scheduler;
  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find(beamId);
  NS_ASSERT(iterator == m_beamSchedulers.end());

  scheduler = CreateObject<SatBeamScheduler>();
  scheduler->Initialize(beamId, cb, seq);

  m_beamSchedulers.insert(std::make_pair(beamId, scheduler));
}

void
SatNcc::AddUt (Address utId, double cra, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << utId << beamId);

  m_beamSchedulers[beamId]->AddUt (utId, cra);
}

} // namespace ns3
