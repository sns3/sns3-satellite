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
#include "satellite-arq-buffer-context.h"

NS_LOG_COMPONENT_DEFINE ("SatArqBufferContext");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatArqBufferContext);


SatArqBufferContext::SatArqBufferContext ()
  : m_pdu (),
    m_seqNo (0),
    m_retransmissionCount (0),
    m_waitingTimer (),
    m_rxStatus (false)
{

}

TypeId
SatArqBufferContext::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatArqBufferContext")
    .SetParent<Object> ()
  ;
  return tid;
}

void
SatArqBufferContext::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_pdu = 0;
  m_waitingTimer.Cancel ();
}

}
