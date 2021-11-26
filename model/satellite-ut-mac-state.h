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
#include <ns3/nstime.h>
#include <ns3/simulator.h>

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
   * LogOff callback.
   */
  typedef Callback<void> LogOffCallback;

  /**
   * Set logOff callback.
   */
  void SetLogOffCallback (LogOffCallback cb);

  /**
   * Get the current state.
   *
   * \return The current state.
   */
  RcstState_t GetState () const;

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

  /**
   * Inform the state diagram that a NCR message has been received. Reset timeouts.
   */
  void NcrControlMessageReceived ();

private:

  RcstState_t m_rcstState;            // Current state

  Time m_lastNcrDateReceived;         // Last time a NCR control message was received
  Time m_checkNcrRecoveryScheduled;   // Last time state switched to NCR_RECOVERY

  Time m_ncrSyncTimeout;              // Timeout to switch from TDMA_SYNC to NCR_RECOVERY
  Time m_ncrRecoveryTimeout;          // Timeout to switch from NCR_RECOVERY to OFF_STANDBY

  LogOffCallback m_logOffCallback;    // Callback to call LogOff of SatUtMac

  /**
   * Check if NCR has been received before sending a timeout.
   * Switching to NCR_RECOVERY if timeout reached.
   */
  void CheckNcrTimeout ();

  /**
   * Check if timeout reached in NCR_RECOVERY state.
   * If yes, switching to OFF_STANDBY and logoff UT Mac.
   */
  void CheckNcrRecoveryTimeout ();
};

} // namespace ns3

#endif /* SATELLITE_UT_MAC_STATE_H */
