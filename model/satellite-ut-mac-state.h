/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#ifndef SATELLITE_UT_MAC_STATE_H
#define SATELLITE_UT_MAC_STATE_H

#include <ns3/object.h>

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Class handling UT Mac states and transitions.
 *
 */
class SatUtMacState : public Object
{
public:

  typedef enum
  {
    HOLD_STANDBY,
    OFF_STANDBY,
    READY_FOR_LOGON,
    READY_FOR_TDMA_SYNC,
    TDMA_SYNC,
    NCR_RECOVERY,
  } RcstState_t;

  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);
  /**
   * Derived from Object
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor, which is not used.
   */
  SatUtMacState ();

  /**
   * Destroy a SatUtMacState
   *
   * This is the destructor for the SatUtMacState.
   */
  ~SatUtMacState ();

  /**
   * Get the current state.
   *
   * \return The current state..
   */
  // TODO is it useful ?
  RcstState_t GetState ();

  /**
   * Change state to HOLD_STANDBY. Raise a FATAL_ERROR if transition not possible from current state.
   */
  void SwitchToHoldStandby ();

  /**
   * Change state to OFF_STANDBY. Raise a FATAL_ERROR if transition not possible from current state.
   */
  void SwitchToOffStandby ();

  /**
   * Change state to READY_FOR_LOGON. Raise a FATAL_ERROR if transition not possible from current state.
   */
  void SwitchToReadyForLogon ();

  /**
   * Change state to READY_FOR_TDMA_SYNC. Raise a FATAL_ERROR if transition not possible from current state.
   */
  void SwitchToReadyForTdmaSync ();

  /**
   * Change state to TDMA_SYNC. Raise a FATAL_ERROR if transition not possible from current state.
   */
  void SwitchToTdmaSync ();

  /**
   * Change state to NCR_RECOVERY. Raise a FATAL_ERROR if transition not possible from current state.
   */
  void SwitchToNcrRecovery ();

private:

  RcstState_t m_rcstState;
};

} // namespace ns3

#endif /* SATELLITE_UT_MAC_STATE_H */
