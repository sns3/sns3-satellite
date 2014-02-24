/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#ifndef SAT_BEAM_SCHEDULER_H
#define SAT_BEAM_SCHEDULER_H

#include <set>

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/mac48-address.h"
#include "ns3/satellite-superframe-sequence.h"
#include "satellite-dama-entry.h"
#include "satellite-cno-estimator.h"
#include "satellite-control-message.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief class for module Beam Scheduler.
 *
 * This SatBeamScheduler class implements scheduler used to for one specific beam.
 * It's is created and used by NCC.
 *
 */
class SatBeamScheduler : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatBeamScheduler
   *
   * This is the constructor for the SatBeamScheduler
   *
   */
  SatBeamScheduler ();

  /**
   * Destroy a SatBeamScheduler
   *
   * This is the destructor for the SatBeamScheduler.
   */
  ~SatBeamScheduler ();

  /**
   * Receive a packet from a beam.
   *
   * The SatBeamScheduler receives CR packets from own beam (sent by UTs)
   * and takes CRs into account when making schedule decisions.
   *
   * \param p       Pointer to the received CR packet.
   */
  void Receive (Ptr<Packet> p);

  /**
   * \param msg        the message send
   * \param address    Packet destination address
   * \return Result of sending, true success or false failure
   */
  typedef Callback<bool, Ptr<SatControlMessage>, const Address& > SendCtrlMsgCallback;

  /**
   * \param id    Id of the TBTP message to add.
   * \param tbtp  Pointer to the TBTP message to add.
   */
  typedef Callback<void, uint32_t, Ptr<SatTbtpMessage> > TbtpAddCallback;

  /**
   * \param beamId ID of the beam which for callback is set
   * \param cb callback to invoke whenever a TBTP is ready for sending and must
   *        be forwarded to the Beam UTs.
   * \param seq Superframe sequence.
   * \parma tbtpCb  TBTP message add callback.
   */
  void Initialize (uint32_t beamId, SatBeamScheduler::SendCtrlMsgCallback cb, Ptr<SatSuperframeSeq> seq);

  /**
   * Add UT to scheduler.
   *
   * \param utId ID (mac address) of the UT to be added
   * \param llsConf Lower layer service configuration for the UT.
   * \return Index of the RA channel allocated to added UT
   */
  uint32_t AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf);

  /**
   * Update UT C/N0 info with the latest value.
   *
   * \param utId Id of the UT (address).
   * \param cno C/N0 value
   */
  void UpdateUtCno (Address utId, double cno);

  /**
   * Receive capacity requests from UTs.
   *
   * \param utId Id of the UT (address).
   * \param crMsg Pointer to the CR message
   */
  void UtCrReceived (Address utId, Ptr<SatCrMessage> crMsg);

private:

  // UT information
  class UtInfo
  {
    public:
      typedef std::vector< Ptr<SatCrMessage> > CrContainer_t;

      Ptr<SatDamaEntry>     m_damaEntry;    // DAMA entry
      Ptr<SatCnoEstimator>  m_cnoEstimator; // Estimator for C/N0
      CrContainer_t         m_crContainer;  // received CRs since last scheduling round.

      UtInfo() {}
  };

  typedef std::map<Address, UtInfo> UtInfoMap_t;

  /**
   * ID of the beam
   */
  uint32_t m_beamId;

  /**
   * Pointer to super frame sequence.
   */
  Ptr<SatSuperframeSeq> m_superframeSeq;

  /**
   * Counter for super frame sequence.
   */
  uint32_t m_superFrameCounter;

  /**
   * The control message send callback.
   */
  SatBeamScheduler::SendCtrlMsgCallback m_txCallback;

  /**
   * The TBTP send callback.
   */
  SatBeamScheduler::TbtpAddCallback m_tbtpAddCb;

  /**
   * Set to store UTs in beam.
   */
  UtInfoMap_t m_uts;

  /**
   * Iterator of the current UT to schedule
   */
  std::map<Address, UtInfo>::iterator m_currentUt;

  /**
   * Iterator of the UT scheduled first
   */
  UtInfoMap_t::iterator m_firstUt;

  /**
   * Shuffled list of carrier ids.
   */
  std::vector<uint32_t> m_carrierIds;

  /**
   * Iterator of the currently used carrier id from shuffled list.
   */
  std::vector<uint32_t>::iterator m_currentCarrier;

  /**
   * Time slot IDs of the currently used carrier.
   */
  SatFrameConf::SatTimeSlotIdList_t m_timeSlots;

  /**
   * Iterator of the currently used time slot id for time slot id list.
   */
  SatFrameConf::SatTimeSlotIdList_t::iterator m_currentSlot;

  /**
   * Current frame id scheduled.
   */
  uint32_t m_currentFrame;

  /**
   * Counter for total time slots left for scheduling
   */
  uint32_t m_totalSlotLeft;

  /**
   * Counter for time slots left when there is even number of slots available for UTs
   */
  uint32_t m_additionalSlots;

  /**
   * Number of time slots reserved per every UTs
   */
  uint32_t m_slotsPerUt;

  /**
   * Round trip time estimate to be used in scheduling the TBTPs well in
   * advance compared to the expected receive time.
   */
  Time m_rttEstimate;

  /**
   * Random variable stream to select RA channel for a UT.
   */
  Ptr<RandomVariableStream> m_raChRandomIndex;

  /**
   * CRA bytes for the super frame scheduled next
   */
  uint32_t  m_craBasedBytes;

  /**
   * RBDC bytes for the super frame scheduled next
   */
  uint32_t  m_rbdcBasedBytes;

  /**
   * VBDC bytes for the super frame scheduled next
   */
  uint32_t  m_vbdcBasedBytes;

  /**
   * Mode used for C/N0 estimator.
   */
  SatCnoEstimator::EstimationMode_t m_cnoEstimatorMode;

  /**
   * Time window for C/N0 estimation.
   */
  Time m_cnoEstimationWindow;

  SatBeamScheduler& operator = (const SatBeamScheduler &);
  SatBeamScheduler (const SatBeamScheduler &);

  void DoDispose (void);
  bool Send ( Ptr<SatControlMessage> packet );
  void Schedule ();

  void UpdateDamaEntries ();
  void InitializeScheduling ();
  void ScheduleUts (Ptr<SatTbtpMessage> tbtpMsg);
  void AddRaChannels (Ptr<SatTbtpMessage> tbtpMsg);
  uint32_t AddUtTimeSlots (Ptr<SatTbtpMessage> tbtpMsg);
  uint16_t GetNextTimeSlot ();

  /**
   * Create estimator for the UT according to set attributes.
   * \return pointer to created estimator
   */
  Ptr<SatCnoEstimator> CreateCnoEstimator ();
};

} // namespace ns3

#endif /* SAT_BEAM_SCHEDULER_H */
