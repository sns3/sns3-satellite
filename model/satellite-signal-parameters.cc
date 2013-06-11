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
#include "ns3/ptr.h"

#include "satellite-signal-parameters.h"
#include "satellite-phy-tx.h"


NS_LOG_COMPONENT_DEFINE ("SatSignalParameters");

namespace ns3 {

SatSignalParameters::SatSignalParameters ()
{
  NS_LOG_FUNCTION (this);
}

SatSignalParameters::SatSignalParameters ( const SatSignalParameters& p )
{
  m_packet = p.m_packet->Copy();
  m_beamId = p.m_beamId;
  m_carrierId = p.m_carrierId;
  m_duration = p.m_duration;
  m_phyTx = p.m_phyTx;
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


} // namespace ns3
