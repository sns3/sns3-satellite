/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-ut-mac-state.h"

NS_LOG_COMPONENT_DEFINE ("SatUtMacState");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtMacState);


TypeId
SatUtMacState::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtMacState")
    .SetParent<Object> ()
    .AddConstructor<SatUtMacState> ()
  ;
  return tid;
}

TypeId
SatUtMacState::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatUtMacState::SatUtMacState ()
  : m_rcstState (HOLD_STANDBY)
{
  NS_LOG_FUNCTION (this);
}

SatUtMacState::~SatUtMacState ()
{
  NS_LOG_FUNCTION (this);
}

SatUtMacState::RcstState_t
SatUtMacState::GetState ()
{
  NS_LOG_FUNCTION (this);

  return m_rcstState;
}

void
SatUtMacState::SwitchToHoldStandby ()
{
  NS_LOG_FUNCTION (this);

  // Can transition from all other states including itself
  m_rcstState = SatUtMacState::HOLD_STANDBY;
}

void
SatUtMacState::SwitchToOffStandby ()
{
  NS_LOG_FUNCTION (this);

  // Can transition from all other states including itself
  m_rcstState = SatUtMacState::HOLD_STANDBY;
}

void
SatUtMacState::SwitchToReadyForLogon ()
{
  NS_LOG_FUNCTION (this);

  // Can transition from HOLD_STANDBY, READY_FOR_TDMA_SYNC or itself
  if (m_rcstState == SatUtMacState::HOLD_STANDBY ||
      m_rcstState == SatUtMacState::READY_FOR_TDMA_SYNC ||
      m_rcstState == SatUtMacState::READY_FOR_LOGON)
    {
      m_rcstState = SatUtMacState::READY_FOR_LOGON;
    }
  else
    {
      NS_FATAL_ERROR ("Cannot transition to READY_FOR_LOGON state");
    }
}

void
SatUtMacState::SwitchToReadyForTdmaSync ()
{
  NS_LOG_FUNCTION (this);

  // Can transition from READY_FOR_LOGON or NCR_RECOVERY
  if (m_rcstState == SatUtMacState::READY_FOR_LOGON ||
      m_rcstState == SatUtMacState::NCR_RECOVERY)
    {
      m_rcstState = SatUtMacState::READY_FOR_TDMA_SYNC;
    }
  else
    {
      NS_FATAL_ERROR ("Cannot transition to READY_FOR_TDMA_SYNC state");
    }
}

void
SatUtMacState::SwitchToTdmaSync ()
{
  NS_LOG_FUNCTION (this);

  // Can transition from READY_FOR_TDMA_SYNC or itself
  if (m_rcstState == SatUtMacState::READY_FOR_TDMA_SYNC ||
      m_rcstState == SatUtMacState::TDMA_SYNC)
    {
      m_rcstState = SatUtMacState::TDMA_SYNC;
    }
  else
    {
      NS_FATAL_ERROR ("Cannot transition to TDMA_SYNC state");
    }
}

void
SatUtMacState::SwitchToNcrRecovery ()
{
  NS_LOG_FUNCTION (this);

  // Can transition from TDMA_SYNC
  if (m_rcstState == SatUtMacState::TDMA_SYNC)
    {
      m_rcstState = SatUtMacState::NCR_RECOVERY;
    }
  else
    {
      NS_FATAL_ERROR ("Cannot transition to NCR_RECOVERY state");
    }
}

} // namespace ns3
